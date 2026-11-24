/**
 * @file StatusBar.cpp
 * @author Silmaen
 * @date 15/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "StatusBar.h"
#include "gui/Application.h"

#include <imgui.h>

namespace evl::gui::views {

StatusBar::StatusBar() = default;

StatusBar::~StatusBar() = default;

auto StatusBar::height() -> float { return ImGui::GetFontSize() + ImGui::GetStyle().WindowPadding.y * 2.0f; }

void StatusBar::onUpdate() {
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiStyle& style = ImGui::GetStyle();
	auto& app = Application::get();
	const float barHeight = height();

	ImGui::SetNextWindowPos({viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - barHeight});
	ImGui::SetNextWindowSize({viewport->WorkSize.x, barHeight});

	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
									   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
									   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

	// What the organizer wants at a glance: which event, where it stands, and whether
	// what is on screen is saved somewhere.
	const auto& event = app.getCurrentEvent();
	const std::string leftText = std::format(
			"{} — {}", event.getName().empty() ? "Événement sans nom" : event.getName(), event.getStatusStr());
	std::string centerText;
	if (event.getStatus() == core::Event::Status::GameRunning) {
		if (const auto round = event.getCurrentCGameRound(); round != event.endRounds())
			centerText = round->getStateString();
	}
	const auto& file = app.getCurrentFile();
	const std::string rightText = file.empty() ? "Non enregistré" : file.filename().string();
	const float rightPos = viewport->WorkSize.x - ImGui::CalcTextSize(rightText.c_str()).x - style.WindowPadding.x;
	const float centerPos = (viewport->WorkSize.x - ImGui::CalcTextSize(centerText.c_str()).x) / 2.0f;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
	if (ImGui::Begin("StatusBar", nullptr, flags)) {
		ImGui::TextUnformatted(leftText.c_str());
		if (!centerText.empty()) {
			ImGui::SameLine(centerPos);
			ImGui::TextDisabled("%s", centerText.c_str());
		}
		ImGui::SameLine(rightPos);
		ImGui::TextDisabled("%s", rightText.c_str());
		if (ImGui::IsItemHovered() && !file.empty())
			ImGui::SetTooltip("%s", file.string().c_str());
	}
	ImGui::End();
	ImGui::PopStyleColor();
}

}// namespace evl::gui::views
