/**
 * @file SettingsActions.cpp
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "SettingsActions.h"

#include <gui_imgui/Application.h>

namespace evl::gui_imgui::actions {
PreferencesAction::PreferencesAction() { setIconName("adjust"); }
PreferencesAction::~PreferencesAction() = default;
void PreferencesAction::onExecute() {
	log_trace("Preference Settings action executed.");
	if (const auto pop = Application::get().getPopup("popup_main_config")) {
		pop->open();
	} else {
		log_warn("Popup 'popup_main_config' not found.");
	}
}

EventSettingsAction::EventSettingsAction() { setIconName("details"); }
EventSettingsAction::~EventSettingsAction() = default;
void EventSettingsAction::onExecute() {
	log_trace("Event Settings action executed.");
	if (const auto pop = Application::get().getPopup("popup_event_config")) {
		pop->open();
	} else {
		log_warn("Popup 'popup_event_config' not found.");
	}
}

GameSettingsAction::GameSettingsAction() { setIconName("services"); }
GameSettingsAction::~GameSettingsAction() = default;
void GameSettingsAction::onExecute() {
	log_trace("Game Settings action executed.");
	if (const auto pop = Application::get().getPopup("popup_game_round_config")) {
		pop->open();
	} else {
		log_warn("Popup 'popup_game_round_config' not found.");
	}
}

ThemeSettingsAction::ThemeSettingsAction() { setIconName("windows-10-personalization"); }
ThemeSettingsAction::~ThemeSettingsAction() = default;
void ThemeSettingsAction::onExecute() { log_trace("Theme Settings action executed."); }

}// namespace evl::gui_imgui::actions
