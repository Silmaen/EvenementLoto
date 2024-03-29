/**
 * @file TestMainHelper.h
 * @author Silmaen
 * @date 01/10/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#ifdef TEST_TRACE
#include <spdlog/sinks/basic_file_sink.h>
#endif
#include <spdlog/spdlog.h>

inline void startSpdlog() {
#ifdef TEST_TRACE
#ifdef WIN32
    spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>((baseExecPath / "test_exec.log").generic_wstring()));
#else
    spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>((baseExecPath / "test_exec.log").generic_string()));
#endif
#ifdef EVL_DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
#endif
#else
    spdlog::set_level(spdlog::level::off);
#endif
}
