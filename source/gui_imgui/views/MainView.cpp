/**
 * @file MainView.cpp
 * @author Silmaen
 * @date 22/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "MainView.h"
#include "gui_imgui/Application.h"

#include <imgui.h>
namespace evl::gui_imgui::views {

MainView::MainView() = default;

MainView::~MainView() = default;

void MainView::onUpdate() {
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiStyle& style = ImGui::GetStyle();

	// Calculate the available space
	const float menuBarHeight = ImGui::GetFrameHeight();
	const float toolBarHeight = ImGui::GetFrameHeight() + style.WindowPadding.y * 2;
	const float statusBarHeight = 25.0f + style.WindowPadding.y;

	// Position and size for the main view
	const ImVec2 pos = {viewport->WorkPos.x, viewport->WorkPos.y + menuBarHeight + toolBarHeight};
	const ImVec2 size = {viewport->WorkSize.x, viewport->WorkSize.y - menuBarHeight - toolBarHeight - statusBarHeight};

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);

	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
									   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
									   ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (ImGui::Begin("MainView", nullptr, flags)) {
		// Create horizontal split
		ImGui::BeginChild("LeftPanel", {size.x * 0.75f, 0}, ImGuiWindowFlags_NoResize);
		renderLeftPanel();
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("RightPanel", {0, 0}, ImGuiWindowFlags_NoResize);
		renderRightPanel();
		ImGui::EndChild();
	}
	ImGui::End();
}

void MainView::renderLeftPanel() {
	// Main tabs area
	if (ImGui::BeginChild("MainTabs", {0, -ImGui::GetFrameHeightWithSpacing() * 5}, ImGuiWindowFlags_NoResize)) {
		if (ImGui::BeginTabBar("MainTabBar")) {
			if (ImGui::BeginTabItem("Numéros tirés")) {
				// TODO: Implement drawn numbers display
				ImGui::Text("Numéros tirés - À implémenter");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Statistiques")) {
				renderStatisticsTab();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::EndChild();

	// Event info section at bottom
	if (ImGui::BeginChild("EventInfo", {0, 0}, ImGuiWindowFlags_NoResize)) {
		renderEventInfo();
	}
	ImGui::EndChild();
}

//TODO: refactor to use internal data and actions
//NOLINTBEGIN(readability-convert-member-functions-to-static)
void MainView::renderRightPanel() {
	// Current and last numbers display
	if (ImGui::BeginChild("NumbersDisplay", {0, -ImGui::GetFrameHeightWithSpacing() * 5}, ImGuiWindowFlags_NoResize)) {
		// Current draw
		if (ImGui::BeginChild("CurrentDraw", {0, 0.75f * ImGui::GetContentRegionAvail().y},
							  ImGuiWindowFlags_NoResize)) {
			ImGui::Text("Numéro tiré");
			ImGui::Separator();
			// TODO: Add LCD number display
			ImGui::Text("--");
		}
		ImGui::EndChild();

		// Last numbers
		if (ImGui::BeginChild("LastNumbers", {0, 0}, ImGuiWindowFlags_NoResize)) {
			ImGui::Text("Derniers numéros");
			ImGui::Separator();
			// TODO: Add last 3 numbers display
			ImGui::Text("-- -- --");
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	// Commands section at bottom
	if (ImGui::BeginChild("Commands", {0, 0}, ImGuiWindowFlags_NoResize)) {
		ImGui::Text("Commandes");
		ImGui::Separator();

		if (ImGui::Button("Debut/fin partie", {-1, 0})) {
			if (const auto action = Application::get().getAction("start_stop_round"); action != nullptr) {
				action->execute();
			}
		}
		if (ImGui::Button("Affichage règlement", {-1, 0})) {
			if (const auto action = Application::get().getAction("display_rules"); action != nullptr) {
				action->execute();
			}
		}
		if (ImGui::Button("Tirage Aléatoire", {-1, 0})) {
			if (const auto action = Application::get().getAction("random_pick"); action != nullptr) {
				action->execute();
			}
		}
		if (ImGui::Button("Annuler dernier tirage", {-1, 0})) {
			if (const auto action = Application::get().getAction("cancel_pick"); action != nullptr) {
				action->execute();
			}
		}
	}
	ImGui::EndChild();
}

void MainView::renderStatisticsTab() {
	// Two columns layout
	ImGui::Columns(2, "StatsColumns", true);

	// Left column - Number statistics
	if (ImGui::BeginChild("NumberStats", {0, 0}, ImGuiWindowFlags_NoResize)) {
		// Less picked numbers
		if (ImGui::CollapsingHeader("Numéros le moins sortis", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Nb: --");
			ImGui::Separator();
			ImGui::TextWrapped("Liste des numéros...");
		}

		// Most picked numbers
		if (ImGui::CollapsingHeader("Numéros le plus sortis", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Nb: --");
			ImGui::Separator();
			ImGui::TextWrapped("Liste des numéros...");
		}
	}
	ImGui::EndChild();

	ImGui::NextColumn();

	// Right column - Game statistics
	if (ImGui::BeginChild("GameStats", {0, 0}, ImGuiWindowFlags_NoResize)) {
		// Round duration stats
		if (ImGui::CollapsingHeader("Durée des parties", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Plus longue partie: --");
			ImGui::Text("Plus courte partie: --");
			ImGui::Text("Temps moyen partie: --");
		}

		// Round draws stats
		if (ImGui::CollapsingHeader("Tirages par partie", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Plus nombreux tirages: --");
			ImGui::Text("Moins nombreux tirages: --");
			ImGui::Text("Nombre tirages moyen: --");
		}
	}
	ImGui::EndChild();

	ImGui::Columns(1);
}

void MainView::renderEventInfo() {
	ImGui::Text("Info Événement");
	ImGui::Separator();

	// Event information in columns
	ImGui::Columns(3, "EventInfoColumns", false);

	ImGui::Text("Date:");
	ImGui::SameLine();
	ImGui::TextDisabled("--");

	ImGui::NextColumn();
	ImGui::Text("Heure de début:");
	ImGui::SameLine();
	ImGui::TextDisabled("--");

	ImGui::NextColumn();
	ImGui::Text("Heure de fin:");
	ImGui::SameLine();
	ImGui::TextDisabled("--");

	ImGui::Columns(1);

	ImGui::Separator();

	ImGui::Columns(2, "DurationColumns", false);

	ImGui::Text("Durée:");
	ImGui::SameLine();
	ImGui::TextDisabled("--:--:--");

	ImGui::NextColumn();
	ImGui::Text("Progression:");
	ImGui::SameLine();
	ImGui::ProgressBar(0.0f, {-1, 0});

	ImGui::Columns(1);

	// Large current time display
	ImGui::Separator();
	const ImVec2 textSize = ImGui::CalcTextSize("00:00:00");
	const float centerX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerX);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);// TODO: Use larger font
	ImGui::Text("--:--:--");
	ImGui::PopFont();
}
//NOLINTEND(readability-convert-member-functions-to-static)

}// namespace evl::gui_imgui::views
