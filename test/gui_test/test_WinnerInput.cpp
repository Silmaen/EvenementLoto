/**
* @file test_WinnerInput.cpp
* @author Silmaen
* @date 15/09/2022
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/

#include "gui/WinnerInput.h"
#include "test_GuiHelpers.h"
#include <gtest/gtest.h>

using namespace evl::gui;

TEST(gui_WinnerInput, base) {
	test::initialize();
	WinnerInput a;
	EXPECT_TRUE(a.isEnabled());
	EXPECT_STREQ(a.getWinner().toStdString().c_str(), "");
	test::finalize();
}
