/**
 * @file test_DisplayWindow.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/DisplayWindow.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_DisplayWindow, base) {
    test::initialize();
    evl::core::Event evl;
    DisplayWindow a{&evl};
    test::finalize();
}
