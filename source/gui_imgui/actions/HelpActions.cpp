/**
 * @file HelpActions.cpp
 * @author Silmaen
 * @date 17/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "HelpActions.h"

#include "gui_imgui/Application.h"
#include "gui_imgui/views/Popups.h"

#include <imgui.h>

namespace evl::gui_imgui::actions {

HelpAction::HelpAction() { setIconName("help"); }
HelpAction::~HelpAction() = default;
void HelpAction::onExecute() {
	if (const auto popup = Application::get().getPopup("popup_aide"); popup != nullptr)
		popup->open();
}

AboutAction::AboutAction() { setIconName("info"); }
AboutAction::~AboutAction() = default;
void AboutAction::onExecute() {
	if (const auto popup = Application::get().getPopup("popup_about"); popup != nullptr)
		popup->open();
}

}// namespace evl::gui_imgui::actions
