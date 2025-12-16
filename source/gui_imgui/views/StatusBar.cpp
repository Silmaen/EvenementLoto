/**
 * @file StatusBar.cpp
 * @author Silmaen
 * @date 15/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "StatusBar.h"
#include "gui_imgui/Application.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

StatusBar::StatusBar() = default;

StatusBar::~StatusBar() = default;

void StatusBar::onUpdate() {
	// Get the main viewport size
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiStyle& style = ImGui::GetStyle();

	// Height of the status bar
	constexpr float statusBarHeight = 25.0f;

	// Position the status bar at the bottom of the window
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - statusBarHeight -
																style.WindowPadding.y));
	ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, statusBarHeight));

	// Create a window without decorations
	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
									   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
									   ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("StatusBar", nullptr, flags)) {
		// Contenu de la status bar (exemple)
		ImGui::Text("%s", std::format("{}", magic_enum::enum_name(Application::get().getState())).c_str());
		ImGui::SameLine(viewport->WorkSize.x - 200.0f);
		ImGui::Text("%s",
					std::format("Status: {}", magic_enum::enum_name(Application::get().getCurrentEvent().getStatus()))
							.c_str());
	}
	ImGui::End();
}

}// namespace evl::gui_imgui::views
