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
	app->setMaxFrame(2);// Limit to 2 frames for the test
	EXPECT_EQ(app->getState(), Application::State::Running);
	EXPECT_EQ(app->getMaxFrame(), 2);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Closed);
	app.reset();
	EXPECT_EQ(app, nullptr);
}
