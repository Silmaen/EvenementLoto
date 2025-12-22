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

#include <imgui.h>

namespace evl::gui_imgui::views {
namespace {
void defineToolBarItem(const std::string& iLabel, const std::string& iActionName) {
	if (const auto action = Application::get().getAction(iActionName); action == nullptr) {
		log_warn("Action '{}' not found ({}).", iLabel, iActionName);
		ImGui::Button(iLabel.c_str());
	} else {
		if (!action->isEnabled()) {
			ImGui::BeginDisabled();
		}
		uint64_t texId = 0;
		if (action->hasIcon()) {
			const auto& texLib = Application::get().getTextureLibrary();
			texId = texLib.getTextureId(action->getIconName());
		}
		if (texId != 0) {
			ImGui::PushID(iActionName.c_str());
			if (ImGui::ImageButton(std::format("##{}", iLabel).c_str(), texId, {24.0f, 24.0f})) {
				action->execute();
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", iLabel.c_str());
			}
			ImGui::PopID();
		} else {
			if (ImGui::Button(iLabel.c_str())) {
				action->execute();
			}
		}
		if (!action->isEnabled()) {
			ImGui::EndDisabled();
		}
	}
	ImGui::SameLine();
}
}// namespace

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
	defineToolBarItem("Nouveau", "new_file");
	defineToolBarItem("Charger", "load_file");
	defineToolBarItem("Sauver", "save_file");
	defineToolBarItem("Sauver sous...", "save_file_as");
	ImGui::Separator();
	ImGui::SameLine();
	defineToolBarItem("Commencer", "start_game");
	defineToolBarItem("Arrêter", "stop_game");
	ImGui::Separator();
	ImGui::SameLine();
	defineToolBarItem("Préférences", "preferences");
	defineToolBarItem("Paramètres Événement", "event_settings");
	defineToolBarItem("Paramètres Partie", "game_settings");
	ImGui::End();
}

}// namespace evl::gui_imgui::views
