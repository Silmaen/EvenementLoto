/**
 * @file View.cpp
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "View.h"

namespace evl::gui_imgui::views {

View::View() = default;

View::~View() = default;

void View::update() {
	if (m_showWindows) {
		onUpdate();
	}
}

}// namespace evl::gui_imgui::views
