/**
* @file test_About.cpp
* @author Silmaen
* @date 24/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/

#include "gui/About.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_About, base) {
    test::initialize();
    About a;
    EXPECT_TRUE(a.isEnabled());
    test::finalize();
}
