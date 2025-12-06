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
