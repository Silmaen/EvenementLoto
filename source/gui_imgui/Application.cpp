/**
 * @file Application.cpp
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "Application.h"


namespace evl::gui_imgui {

Application::Application() = default;

Application::~Application() = default;

void Application::run() {}

auto createApplication([[maybe_unused]] int iArgc, [[maybe_unused]] char* iArgv[]) -> std::shared_ptr<Application> {
	return std::make_shared<Application>();
}

}// namespace evl::gui_imgui
