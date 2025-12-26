/**
 * @file ToolBar.cpp
 * @author Silmaen
 * @date 15/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "ToolBar.h"

#include "core/Log.h"
#include "gui_imgui/Application.h"
#include "gui_imgui/utils/Rendering.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

ToolBar::ToolBar() = default;

ToolBar::~ToolBar() = default;

void ToolBar::onUpdate() {
	// Get the main viewport to position the toolbar correctly
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	// Calculate position and size
	const float menuBarHeight = ImGui::GetFrameHeight();// Height of the menu bar
	// Set window position and size
	ImGui::SetNextWindowPos({viewport->Pos.x, viewport->Pos.y + menuBarHeight});
	ImGui::SetNextWindowSize({viewport->Size.x, ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y * 2});
	// Create the toolbar window
	ImGui::Begin("Tool Bar", nullptr,
				 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
						 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

	// Buttons in a horizontal line
	utils::defineActionButtonItem("Nouveau", "new_file");
	utils::defineActionButtonItem("Charger", "load_file");
	utils::defineActionButtonItem("Sauver", "save_file");
	utils::defineActionButtonItem("Sauver sous...", "save_file_as");
	ImGui::SameLine();
	ImGui::Separator();
	utils::defineActionButtonItem("Commencer", "start_game");
	utils::defineActionButtonItem("Arrêter", "stop_game");
	ImGui::SameLine();
	ImGui::Separator();
	utils::defineActionButtonItem("Préférences", "preferences");
	utils::defineActionButtonItem("Paramètres Événement", "event_settings");
	utils::defineActionButtonItem("Paramètres Partie", "game_settings");
	ImGui::SameLine();
	ImGui::Separator();
	utils::defineActionButtonItem(utils::getNextStepStr(Application::get().getCurrentEvent()), "game_next_step");
	utils::defineActionButtonItem("Tirage Aléatoire", "random_pick");
	utils::defineActionButtonItem("Annuler dernier tirage", "cancel_pick");
	ImGui::End();
}

}// namespace evl::gui_imgui::views
