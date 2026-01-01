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
#include <imgui_internal.h>

namespace evl::gui_imgui::views {

StatusBar::StatusBar() = default;

StatusBar::~StatusBar() = default;

void StatusBar::onUpdate() {
	// Get the main viewport size
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiStyle& style = ImGui::GetStyle();

	auto& app = Application::get();
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

	const std::string leftText = std::format("App : {}", magic_enum::enum_name(app.getState()));
	const std::string centerText = std::format("Event: {}", app.getCurrentEvent().getStateString());
	const std::string rightText = std::format("Status: {}", magic_enum::enum_name(app.getCurrentEvent().getStatus()));
	const auto leftSize = ImGui::CalcTextSize(leftText.c_str());
	const auto centerSize = ImGui::CalcTextSize(centerText.c_str());
	const auto rightsize = ImGui::CalcTextSize(rightText.c_str());
	const float rightPos = viewport->WorkSize.x - rightsize.x - style.WindowPadding.x;
	const float centerPos = (viewport->WorkSize.x - centerSize.x) / 2.0f;

	if (ImGui::Begin("StatusBar", nullptr, flags)) {
		// Contenu de la status bar (exemple)
		ImGui::Text("%s", leftText.c_str());
		// add separators to position center between en of left text and start of center text
		ImGui::SameLine((leftSize.x + centerPos) / 2.0f);
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine(centerPos);
		ImGui::Text("%s", centerText.c_str());
		ImGui::SameLine((centerPos + centerSize.x + rightPos) / 2.0f);
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine(rightPos);
		ImGui::Text("%s", rightText.c_str());
	}
	ImGui::End();
}

}// namespace evl::gui_imgui::views
