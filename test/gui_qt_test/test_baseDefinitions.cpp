/**
 * @file test_baseDefinitions.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui_qt/baseDefinitions.h"
#include <gtest/gtest.h>

TEST(baseDefinitions, configfile) { EXPECT_EQ(evl::gui::getIniFile(), evl::gui::iniFile); }
