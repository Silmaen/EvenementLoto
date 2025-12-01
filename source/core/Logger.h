/**
 * @file Logger.h
 * @author argawaen
 * @date 10/11/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "../baseDefine.h"
#include <fstream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace evl {

inline path getLogPath() {
	return baseExecPath / "exec.log";
}

inline void resetLogFile() {
	std::ofstream flog(getLogPath());
	flog.close();
}

inline void startSpdlog() {
#ifdef WIN32
	spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(getLogPath().generic_wstring()));
#else
	spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(getLogPath().generic_string()));
#endif
#ifdef EVL_DEBUG
	spdlog::set_level(spdlog::level::trace);
	spdlog::flush_every(std::chrono::seconds(1U));
#else
	spdlog::set_level(spdlog::level::info);
#endif
}

}// namespace evl
