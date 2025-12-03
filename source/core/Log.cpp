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

auto getLogPath() -> std::filesystem::path { return g_baseExecPath / "exec.log"; }

Log::Level Log::m_verbosity = Level::Trace;

void Log::init(const Level& iLevel) {
	if (g_logger != nullptr) {
		log_info("Logger already initiated.");
		return;
	}
	std::vector<spdlog::sink_ptr> logSinks;
	logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(getLogPath(), true));

	logSinks[0]->set_pattern("%^[%T] %n: %v%$");
	logSinks[1]->set_pattern("[%T] [%l] %n: %v");

	g_logger = std::make_shared<spdlog::logger>("EVL", begin(logSinks), end(logSinks));
	register_logger(g_logger);

	setVerbosityLevel(iLevel);
#ifdef EVL_DEBUG
	spdlog::flush_every(std::chrono::seconds(1U));
#endif
}

void Log::setVerbosityLevel(const Level& iLevel) {
	m_verbosity = iLevel;
	if (g_logger) {
		g_logger->set_level(fromLevel(m_verbosity));
		g_logger->flush_on(fromLevel(m_verbosity));
	}
}

void Log::invalidate() {
	spdlog::drop_all();
	g_logger.reset();
}

auto Log::initiated() -> bool { return g_logger != nullptr; }

void Log::log(const Level& iLevel, const std::string_view& iMsg) { g_logger->log(fromLevel(iLevel), iMsg); }

}// namespace evl
