/**
 * @file test_Application.cpp
 * @author Silmaen
 * @date 03/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"
#include "gui_imgui/Application.h"

using namespace evl::gui_imgui;

TEST(gui_imgui_Application, InstantiateAndRun) {
	auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	app->run();
	app.reset();
	EXPECT_EQ(app, nullptr);
}
