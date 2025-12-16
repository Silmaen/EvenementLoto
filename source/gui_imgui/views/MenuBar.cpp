/**
 * @file MenuBar.cpp
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "MenuBar.h"

#include "core/Log.h"
#include "gui_imgui/Application.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

namespace {

void defineMenuItem(const std::string& iLabel, const std::string& iActionName) {
	if (const auto action = Application::get().getAction(iActionName); action == nullptr) {
		log_warn("Action '{}' not found ({}).", iLabel, iActionName);
		ImGui::MenuItem(iLabel.c_str(), "", false, false);
	} else {
		if (action->hasIcon()) {
			const auto& texLib = Application::get().getTextureLibrary();

			if (const uint64_t texId = texLib.getTextureId(action->getIconName()); texId != 0) {
				constexpr ImVec2 iconSize = {16.0f, 16.0f};
				ImGui::Image(texId, iconSize);
				ImGui::SameLine();
			}
		}
		if (ImGui::MenuItem(iLabel.c_str(), action->getShortcut().c_str(), false, action->isEnabled())) {
			action->execute();
		}
	}
}

}// namespace

MenuBar::MenuBar() = default;

MenuBar::~MenuBar() = default;

void MenuBar::onUpdate() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Fichier")) {
			defineMenuItem("Nouveau", "new_file");
			ImGui::Separator();
			defineMenuItem("Charger", "load_file");
			defineMenuItem("Sauver", "save_file");
			defineMenuItem("Sauver sous...", "save_file_as");
			ImGui::Separator();
			defineMenuItem("Commencer", "start_game");
			defineMenuItem("Arrêter", "stop_game");
			ImGui::Separator();
			defineMenuItem("Quitter", "quit_application");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Paramètres")) {
			defineMenuItem("General", "preferences");
			defineMenuItem("Événement", "event_settings");
			defineMenuItem("Parties", "game_settings");
			defineMenuItem("Thème", "theme_settings");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("About")) {
				ImGui::OpenPopup("About");
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Aide", "F1")) {
				// Open documentation
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

}// namespace evl::gui_imgui::views
