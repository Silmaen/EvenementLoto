/**
 * @file TestMainHelper.h
 * @author Silmaen
 * @date 01/10/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "core/Log.h"
#include "core/utilities.h"
#include <filesystem>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

constexpr auto g_logLv = evl::Log::Level::Off;

/// True where the application cannot be given a display.
///
/// On Linux `ctest` runs the GUI suite under `xvfb-run`, so it really runs. The TeamCity
/// Windows agent is a service with no desktop session and no software Vulkan, so the
/// window could not be created there — better to say so than to fail for a reason that
/// has nothing to do with the code. Remove this the day that agent has a session.
constexpr auto g_needsDisplay =
#ifdef EVL_PLATFORM_WINDOWS
		true;
#else
		false;
#endif
