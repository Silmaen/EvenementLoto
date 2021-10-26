/**
 * @file test_MainWindow.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/MainWindow.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_MainWindow, base) {
    test::initialize();
    MainWindow main{};
    EXPECT_STREQ(main.getSettings().applicationName().toStdString().c_str(), "");
    test::finalize();
}
