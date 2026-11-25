/**
 * @file Log.cpp
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Log.h"
#include "defines.h"

#include "baseDefine.h"
#include "external/spdlog.h"
#include "utilities.h"

#include <fstream>
#include <iostream>
#include <mutex>
EVL_DIAG_PUSH
EVL_DIAG_DISABLE_CLANG("-Wweak-vtables")
EVL_DIAG_DISABLE_CLANG("-Wundefined-func-template")
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
EVL_DIAG_POP

namespace evl {

namespace {
auto fromLevel(const Log::Level& iLevel) -> spdlog::level::level_enum {
	switch (iLevel) {
		case Log::Level::Error:
			return spdlog::level::err;
		case Log::Level::Warning:
			return spdlog::level::warn;
		case Log::Level::Info:
			return spdlog::level::info;
		case Log::Level::Debug:
			return spdlog::level::debug;
		case Log::Level::Trace:
			return spdlog::level::trace;
		case Log::Level::Off:
			return spdlog::level::off;
		case Log::Level::Critical:
			return spdlog::level::critical;
	}
	return spdlog::level::off;
}
std::shared_ptr<spdlog::logger> g_logger;

/// Maximum size of a single log file.
constexpr std::size_t g_logMaxSize = 5UL * 1024UL * 1024UL;
/// Number of past log files kept beside the current one.
constexpr std::size_t g_logMaxFiles = 5;

/**
 * @brief Appending file sink that starts a new file past a size limit.
 *
 * Replaces spdlog's rotating sink, whose file name formatting crashed with this
 * toolchain the moment a log reached its limit — that is, right at the next start-up.
 * Never throws: a log that cannot be written must not take the application down.
 */
class RotatingFileSink final : public spdlog::sinks::base_sink<std::mutex> {
public:
	RotatingFileSink(std::filesystem::path iPath, const std::size_t iMaxSize, const std::size_t iMaxFiles) :
		m_path(std::move(iPath)), m_maxSize(iMaxSize), m_maxFiles(iMaxFiles) {
		std::error_code ec;
		m_size = exists(m_path, ec) ? static_cast<std::size_t>(std::filesystem::file_size(m_path, ec)) : 0;
		if (ec)
			m_size = 0;
		if (m_size >= m_maxSize)
			rotate();
		else
			open(std::ios::app);
	}

protected:
	void sink_it_(const spdlog::details::log_msg& iMsg) override {
		spdlog::memory_buf_t formatted;
		formatter_->format(iMsg, formatted);
		if (m_size + formatted.size() > m_maxSize)
			rotate();
		m_file.write(formatted.data(), static_cast<std::streamsize>(formatted.size()));
		m_size += formatted.size();
	}

	void flush_() override { m_file.flush(); }

private:
	void open(const std::ios::openmode iMode) {
		m_file.close();
		m_file.clear();
		m_file.open(m_path, std::ios::out | std::ios::binary | iMode);
	}

	[[nodiscard]] auto numbered(const std::size_t iIndex) const -> std::filesystem::path {
		if (iIndex == 0)
			return m_path;
		auto name = m_path.stem().string();
		name += '.';
		name += std::to_string(iIndex);
		name += m_path.extension().string();
		return m_path.parent_path() / name;
	}

	void rotate() {
		m_file.close();
		std::error_code ec;
		for (std::size_t i = m_maxFiles; i > 0; --i) {
			const auto source = numbered(i - 1);
			if (!exists(source, ec))
				continue;
			const auto target = numbered(i);
			std::filesystem::remove(target, ec);
			std::filesystem::rename(source, target, ec);
		}
		// Whatever happened to the renames, the current file starts again empty.
		open(std::ios::trunc);
		m_size = 0;
	}

	std::filesystem::path m_path;
	std::size_t m_maxSize;
	std::size_t m_maxFiles;
	std::size_t m_size = 0;
	std::ofstream m_file;
};

}// namespace

auto getLogPath() -> std::filesystem::path { return core::getExecPath() / "exec.log"; }

Log::Level Log::m_verbosity = Level::Trace;


void Log::init(const Level& iLevel) {
	if (g_logger != nullptr) {
		log_info("Logger already initiated.");
		return;
	}
	std::vector<spdlog::sink_ptr> logSinks;
	logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	// Rotating and appending: relaunching after a crash must not erase its trace.
	logSinks.emplace_back(std::make_shared<RotatingFileSink>(getLogPath(), g_logMaxSize, g_logMaxFiles));

	g_logger = std::make_shared<spdlog::logger>("EVL", begin(logSinks), end(logSinks));
	register_logger(g_logger);

	setVerbosityLevel(iLevel);
	// Also in release: the last seconds before a crash are the interesting ones.
	spdlog::flush_every(std::chrono::seconds(1U));
	logSessionHeader();
}

void Log::logSessionHeader() {
	// First lines of every session: an incident report is worth little without knowing
	// which build produced it. The log is appended and rotated, so this is also what
	// separates one afternoon from the next in the file.
	log_info("=== EvenementLoto {}.{}.{} ===", EVL_MAJOR, EVL_MINOR, EVL_PATCH);
#ifdef __clang__
	log_info("Compilateur : clang {}.{}.{}", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elifdef __GNUC__
	log_info("Compilateur : gcc {}.{}.{}", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
	log_info("Compilateur : inconnu");
#endif
#ifdef EVL_PLATFORM_WINDOWS
	log_info("Plateforme : Windows");
#elifdef EVL_PLATFORM_LINUX
	log_info("Plateforme : Linux");
#else
	log_info("Plateforme : inconnue");
#endif
	log_info("Version de sauvegarde : {}", core::getSaveVersion());
	log_info("Journal : '{}'", getLogPath().string());
}

void Log::setVerbosityLevel(const Level& iLevel) {
	if (!initiated())
		return;
	m_verbosity = iLevel;
	g_logger->set_level(fromLevel(m_verbosity));
	g_logger->flush_on(fromLevel(m_verbosity));
	setPattern();
}

void Log::invalidate() {
	spdlog::drop_all();
	g_logger.reset();
	spdlog::shutdown();
}

auto Log::initiated() -> bool { return g_logger != nullptr; }

void Log::log(const Level& iLevel, const char* iFile, const int iLine, const std::string_view& iMsg) {
	if (!initiated())
		return;
	g_logger->log(spdlog::source_loc{iFile, iLine, SPDLOG_FUNCTION}, fromLevel(iLevel), iMsg);
	logs::LogBuffer::get().addLog(std::string(iMsg), iLevel);
}


void Log::setPattern() {
	const std::string file_pattern_dbg = "[%T.%e] [%l] [%s:%#] %v";
	const std::string file_pattern_rls = "[%T.%e] [%l] %v";
	const std::string console_pattern_dbg = "[\033[38;5;31m%T.%e\033[0m] [%^%l%$] [\033[38;5;240m%s:%#\033[0m] %v";
	const std::string console_pattern_rls = "[\033[38;5;31m%T.%e\033[0m] [%^%l%$] %v";
	if (!initiated())
		return;
	const auto& sk = g_logger->sinks();
	if (sk.size() < 2)
		return;
	if (m_verbosity == Level::Debug || m_verbosity == Level::Trace) {
		sk[0]->set_pattern(console_pattern_dbg);
		sk[1]->set_pattern(file_pattern_dbg);
	} else {
		sk[0]->set_pattern(console_pattern_rls);
		sk[1]->set_pattern(file_pattern_rls);
	}
}

namespace logs {
void LogBuffer::addLog(const std::string& iMessage, Log::Level iLevel) {
	const std::scoped_lock<std::mutex> lock(m_mutex);
	m_logs.emplace_back(iMessage, iLevel, core::clock::now());
	if (m_logs.size() > 1000) {// Limit to 1000 entries
		m_logs.pop_front();
	}
}
auto LogBuffer::getLogs() const -> std::deque<LogEntry> {
	const std::scoped_lock<std::mutex> lock(m_mutex);
	return m_logs;
}

void LogBuffer::clear() {
	const std::scoped_lock<std::mutex> lock(m_mutex);
	m_logs.clear();
}
}// namespace logs

}// namespace evl
