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

#include <iostream>
EVL_DIAG_PUSH
EVL_DIAG_DISABLE_CLANG("-Wweak-vtables")
EVL_DIAG_DISABLE_CLANG("-Wundefined-func-template")
#include <spdlog/sinks/basic_file_sink.h>
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
	logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(getLogPath(), true));

	g_logger = std::make_shared<spdlog::logger>("EVL", begin(logSinks), end(logSinks));
	register_logger(g_logger);

	setVerbosityLevel(iLevel);
#ifdef EVL_DEBUG
	spdlog::flush_every(std::chrono::seconds(1U));
#endif
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
		m_logs.erase(m_logs.begin());
	}
}
auto LogBuffer::getLogs() const -> const std::vector<LogEntry>& {
	const std::scoped_lock<std::mutex> lock(m_mutex);
	return m_logs;
}

void LogBuffer::clear() {
	const std::scoped_lock<std::mutex> lock(m_mutex);
	m_logs.clear();
}
}// namespace logs

}// namespace evl
