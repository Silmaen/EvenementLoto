/**
 * @file test_ConfigGameRounds.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/ConfigGameRounds.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_ConfigGameRounds, base) {
    test::initialize();
    ConfigGameRounds gr;
    EXPECT_STREQ(gr.getEvent().getName().c_str(), "");
    gr.setEvent(gr.getEvent());
    test::finalize();
}
