/**
 * @file SettingsActions.cpp
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "SettingsActions.h"

namespace evl::gui_imgui::actions {
PreferencesAction::PreferencesAction() { setIconName("adjust"); }
PreferencesAction::~PreferencesAction() = default;
EventSettingsAction::EventSettingsAction() { setIconName("details"); }
EventSettingsAction::~EventSettingsAction() = default;
GameSettingsAction::GameSettingsAction() { setIconName("services"); }
GameSettingsAction::~GameSettingsAction() = default;
ThemeSettingsAction::ThemeSettingsAction() { setIconName("windows-10-personalization"); }
ThemeSettingsAction::~ThemeSettingsAction() = default;
}// namespace evl::gui_imgui::actions
