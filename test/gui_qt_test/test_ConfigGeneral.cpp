/**
 * @file test_ConfigGeneral.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui_qt/ConfigGeneral.h"
#include "test_GuiHelpers.h"

using namespace evl::gui;

TEST(gui_ConfigGeneral, ControlButtons) {
	test::initialize();
	ConfigGeneral g;
	g.actOk();
	g.actApply();
	g.actCancel();
	test::finalize();
}
