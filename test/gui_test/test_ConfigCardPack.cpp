/**
 * @file test_ConfigCardPack.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/ConfigCardPack.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_ConfigCardPack, base) {
    test::initialize();
    ConfigCardPack a{};
    EXPECT_EQ(a.getCardPack().size(), 0);
    test::finalize();
}
