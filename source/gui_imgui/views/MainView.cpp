/**
 * @file MainView.cpp
 * @author Silmaen
 * @date 22/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "MainView.h"

#include "DisplayView.h"
#include "gui_imgui/Application.h"
#include "gui_imgui/utils/Rendering.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

MainView::MainView(core::Event& iEvent) : m_currentEvent{iEvent} {}

MainView::~MainView() = default;

void MainView::onUpdate() {
	m_currentEvent = Application::get().getCurrentEvent();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiStyle& style = ImGui::GetStyle();

	// Calculate the available space
	const float toolBarHeight = ImGui::GetFrameHeight() + style.WindowPadding.y * 2;
	const float statusBarHeight = 25.0f + style.WindowPadding.y;

	// Position and size for the main view
	const ImVec2 pos = {viewport->WorkPos.x, viewport->WorkPos.y + toolBarHeight};
	const ImVec2 size = {viewport->WorkSize.x, viewport->WorkSize.y - toolBarHeight - statusBarHeight};

	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);

	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
									   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
									   ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (ImGui::Begin("MainView", nullptr, flags)) {
		// Initialize splitter positions if needed
		static float topBottomSplit = 0.85f;
		static float leftRightSplit = 0.75f;

		// Top panel
		const float topPanelHeight = size.y * topBottomSplit;
		ImGui::BeginChild("TopPanel", {0, topPanelHeight}, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

		// Left panel
		const float leftPanelWidth = size.x * leftRightSplit;
		ImGui::BeginChild("LeftPanel", {leftPanelWidth, 0}, ImGuiChildFlags_None, ImGuiWindowFlags_NoResize);
		renderLeftPanel();
		ImGui::EndChild();

		// Vertical splitter between left and right
		ImGui::SameLine();
		ImGui::Button("##vsplitter", {4.0f, -1});
		if (ImGui::IsItemActive()) {
			leftRightSplit += ImGui::GetIO().MouseDelta.x / size.x;
			leftRightSplit = std::clamp(leftRightSplit, 0.2f, 0.9f);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		}

		ImGui::SameLine();

		// Right panel
		ImGui::BeginChild("RightPanel", {0, 0}, ImGuiChildFlags_None, ImGuiWindowFlags_NoResize);
		renderRightPanel();
		ImGui::EndChild();

		ImGui::EndChild();

		// Horizontal splitter between top and bottom
		ImGui::Button("##hsplitter", {-1, 4.0f});
		if (ImGui::IsItemActive()) {
			topBottomSplit += ImGui::GetIO().MouseDelta.y / size.y;
			topBottomSplit = std::clamp(topBottomSplit, 0.2f, 0.9f);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
		}

		// Bottom panel
		ImGui::BeginChild("BottomPanel", {0, 0}, ImGuiChildFlags_None, ImGuiWindowFlags_NoResize);
		renderBottomPanel();
		ImGui::EndChild();
	}
	ImGui::End();
}

void MainView::renderLeftPanel() const {
	// Main tabs area
	if (ImGui::BeginChild("MainTabs", {0, -ImGui::GetFrameHeightWithSpacing() * 5}, ImGuiWindowFlags_NoResize)) {
		if (ImGui::BeginTabBar("MainTabBar")) {
			if (ImGui::BeginTabItem("Numéros tirés")) {
				renderDrawnNumbersTab();
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

	if (ImGui::BeginChild("EventInfo", {0, 0}, ImGuiWindowFlags_NoResize)) {
		renderEventInfo();
	}
	ImGui::EndChild();
}

void MainView::renderDrawnNumbersTab() const {
	// Get current event and round
	if (!m_currentEvent.canDraw()) {
		ImGui::TextDisabled("Aucun événement en cours");
		return;
	}

	const auto currentRound = m_currentEvent.getCurrentGameRound();
	auto& rng = Application::get().getRng();
	// Get all drawn numbers for current round
	const auto drawnNumbers = currentRound->getAllDraws();
	const std::unordered_set drawnSet(drawnNumbers.begin(), drawnNumbers.end());

	// Calculate button size to fit 10 per row
	const ImVec2 availWidth = ImGui::GetContentRegionAvail();
	const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
	const ImVec2 buttonSize{(availWidth.x - spacing.x * 9) / 10.0f, (availWidth.y - spacing.y * 8) / 9.0f};

	// Render 9 rows of 10 buttons (1-90)
	for (uint8_t row = 0; row < 9; ++row) {
		for (uint8_t col = 0; col < 10; ++col) {
			const uint8_t number = row * 10 + col + 1;
			const bool isDrawn = drawnSet.contains(number);

			// Push style for drawn numbers
			if (isDrawn) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			}

			// Create button
			const std::string label = std::format("{}", number);
			const bool clicked = ImGui::Button(label.c_str(), buttonSize);

			// Pop style
			if (isDrawn) {
				ImGui::PopStyleColor();
			}

			// Handle click only if not already drawn
			if (clicked && !isDrawn) {
				currentRound->addPickedNumber(number);
				rng.addPick(number);
			}

			// Same line except last column
			if (col < 9) {
				ImGui::SameLine();
			}
		}
	}
}

void MainView::renderRightPanel() const {
	int prevDrawnNumber = -1;
	int secondPrevDrawnNumber = -1;
	int thirdPrevDrawnNumber = -1;

	if (m_currentEvent.getStatus() == core::Event::Status::GameRunning) {
		if (const auto currentRound = m_currentEvent.getCurrentGameRound();
			currentRound->getStatus() == core::GameRound::Status::Running) {
			const auto drawnNumbers = currentRound->getAllDraws();
			const size_t drawnCount = drawnNumbers.size();
			if (drawnCount >= 1) {
				prevDrawnNumber = drawnNumbers[drawnCount - 1];
			}
			if (drawnCount >= 2) {
				secondPrevDrawnNumber = drawnNumbers[drawnCount - 2];
			}
			if (drawnCount >= 3) {
				thirdPrevDrawnNumber = drawnNumbers[drawnCount - 3];
			}
		}
	}

	if (ImGui::BeginChild("NumbersDisplay", {0, -ImGui::GetFrameHeightWithSpacing() * 5},
						  ImGuiWindowFlags_NoTitleBar)) {
		// Current draw
		if (ImGui::BeginChild("CurrentDraw", {0, 0.75f * ImGui::GetContentRegionAvail().y},
							  ImGuiWindowFlags_NoResize)) {
			ImGui::Text("Numéro tiré");
			ImGui::Separator();
			if (prevDrawnNumber != -1) {
				ImGui::Text("%d", prevDrawnNumber);
			} else {
				ImGui::Text("--");
			}
		}
		ImGui::EndChild();

		// Last numbers
		if (ImGui::BeginChild("LastNumbers", {0, 0}, ImGuiWindowFlags_NoResize)) {
			ImGui::Text("Derniers numéros");
			ImGui::Separator();
			std::string lastNumbers;
			if (thirdPrevDrawnNumber != -1) {
				lastNumbers += std::to_string(thirdPrevDrawnNumber) + " ";
			} else {
				lastNumbers += "-- ";
			}
			if (secondPrevDrawnNumber != -1) {
				lastNumbers += std::to_string(secondPrevDrawnNumber) + " ";
			} else {
				lastNumbers += "-- ";
			}
			if (prevDrawnNumber != -1) {
				lastNumbers += std::to_string(prevDrawnNumber);
			} else {
				lastNumbers += "--";
			}
			ImGui::Text("%s", lastNumbers.c_str());
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	// Commands section at bottom
	if (ImGui::BeginChild("Commands", {0, 0}, ImGuiWindowFlags_NoTitleBar)) {
		renderCommandsTab();
	}
	ImGui::EndChild();
}

void MainView::renderCommandsTab() const {
	int prevDrawnNumber = -1;
	if (m_currentEvent.getStatus() == core::Event::Status::GameRunning) {
		if (const auto currentRound = m_currentEvent.getCurrentGameRound();
			currentRound->getStatus() == core::GameRound::Status::Running) {
			const auto drawnNumbers = currentRound->getAllDraws();
			if (const size_t drawnCount = drawnNumbers.size(); drawnCount >= 1) {
				prevDrawnNumber = drawnNumbers[drawnCount - 1];
			}
		}
	}
	{
		bool btnDisabled = false;
		switch (m_currentEvent.getStatus()) {
			case core::Event::Status::Invalid:
			case core::Event::Status::MissingParties:
			case core::Event::Status::Finished:
				btnDisabled = true;
				break;
			case core::Event::Status::Ready:
			case core::Event::Status::EventStarting:
			case core::Event::Status::DisplayRules:
			case core::Event::Status::GameRunning:
			case core::Event::Status::EventEnding:
				break;
		}
		utils::defineActionButtonItem(
				utils::getNextStepStr(m_currentEvent), "game_next_step",
				{.showLabel = true, .disabled = btnDisabled, .sameLine = false, .setDisabled = true});
	}
	{
		bool btnDisabled = false;
		switch (m_currentEvent.getStatus()) {
			case core::Event::Status::Invalid:
			case core::Event::Status::MissingParties:
			case core::Event::Status::Finished:
			case core::Event::Status::DisplayRules:
			case core::Event::Status::Ready:
				btnDisabled = true;
				break;
			case core::Event::Status::EventStarting:
			case core::Event::Status::GameRunning:
			case core::Event::Status::EventEnding:
				break;
		}
		utils::defineActionButtonItem(
				"Affichage règlement", "display_rules",
				{.showLabel = true, .disabled = btnDisabled, .sameLine = false, .setDisabled = true});
	}
	utils::defineActionButtonItem(
			"Tirage Aléatoire", "random_pick",
			{.showLabel = true, .disabled = !m_currentEvent.canDraw(), .sameLine = false, .setDisabled = true});
	utils::defineActionButtonItem(
			"Annuler dernier tirage", "cancel_pick",
			{.showLabel = true, .disabled = prevDrawnNumber == -1, .sameLine = false, .setDisabled = true});
}

void MainView::renderStatisticsTab() const {
	const auto stats = m_currentEvent.getStats();
	ImGui::Columns(2, "StatsColumns", true);

	if (ImGui::BeginChild("LeftColomunStats")) {
		// Less picked numbers
		if (ImGui::CollapsingHeader("Numéros le moins sortis", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::TextWrapped("%s", stats.lessPickStr().c_str());
			ImGui::Text("Nombre de sortie: %d", stats.lessPickNb);
		}

		// Round duration stats
		if (ImGui::CollapsingHeader("Durée des parties", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Plus longue partie: %s", core::formatDuration(stats.roundLongest).c_str());
			ImGui::Text("Plus courte partie: %s", core::formatDuration(stats.roundShortest).c_str());
			ImGui::Text("Temps moyen partie: %s", core::formatDuration(stats.roundAverage).c_str());
		}
		// Round duration stats
		if (ImGui::CollapsingHeader("Durée des manches", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Plus longue manche: %s", core::formatDuration(stats.subRoundLongest).c_str());
			ImGui::Text("Plus courte manche: %s", core::formatDuration(stats.subRoundShortest).c_str());
			ImGui::Text("Temps moyen manche: %s", core::formatDuration(stats.subRoundAverage).c_str());
		}
	}
	ImGui::EndChild();

	ImGui::NextColumn();

	if (ImGui::BeginChild("RightColomunStats")) {
		// Most picked numbers
		if (ImGui::CollapsingHeader("Numéros le plus sortis", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::TextWrapped("%s", stats.mostPickStr().c_str());
			ImGui::Text("Nombre de sortie: %d", stats.mostPickNb);
		}

		// Round draws stats
		if (ImGui::CollapsingHeader("Tirages par partie", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Plus nombreux tirages: %d", stats.roundMostNb);
			ImGui::Text("Moins nombreux tirages: %d", stats.roundLessNb);
			ImGui::Text("Nombre tirages moyen: %f", stats.roundAverageNb);
		}

		// Round draws stats
		if (ImGui::CollapsingHeader("Tirages par manches", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Plus nombreux tirages: %d", stats.subRoundMostNb);
			ImGui::Text("Moins nombreux tirages: %d", stats.subRoundLessNb);
			ImGui::Text("Nombre tirages moyen: %f", stats.subRoundAverageNb);
		}
	}
	ImGui::EndChild();

	ImGui::Columns(1);
}

void MainView::renderEventInfo() const {
	ImGui::Text("Info Événement");
	ImGui::Separator();
	std::string date = "--";
	std::string startTime = "--";
	std::string endTime = "--";
	std::string duration = "--:--:--";
	float progress = 0.0f;

	if (m_currentEvent.getStatus() == core::Event::Status::GameRunning) {
		const auto start = m_currentEvent.getStarting();
		const auto end = m_currentEvent.getEnding();

		date = std::format("{}", core::formatCalendar(start));
		startTime = core::formatClock(start);
		endTime = core::formatClock(end);
		const auto dur = end - start;
		duration = core::formatDuration(dur);
		progress = m_currentEvent.getProgression();
	}

	ImGui::Columns(2, "EventInfoColumns", true);
	ImGui::BeginChild("LeftColomunEventInfo", ImVec2(0, 0), ImGuiWindowFlags_NoTitleBar);
	// Event information in columns
	ImGui::Columns(3, "EventInfoColumns", false);

	ImGui::Text("Date:");
	ImGui::SameLine();
	ImGui::TextDisabled("%s", date.c_str());

	ImGui::NextColumn();
	ImGui::Text("Heure de début:");
	ImGui::SameLine();
	ImGui::TextDisabled("%s", startTime.c_str());

	ImGui::NextColumn();
	ImGui::Text("Heure de fin:");
	ImGui::SameLine();
	ImGui::TextDisabled("%s", endTime.c_str());

	ImGui::Columns(1);

	ImGui::Separator();

	ImGui::Columns(2, "DurationColumns", false);

	ImGui::Text("Durée:");
	ImGui::SameLine();
	ImGui::TextDisabled("%s", duration.c_str());

	ImGui::NextColumn();
	ImGui::Text("Progression:");
	ImGui::SameLine();
	ImGui::ProgressBar(progress, {-1, 0});

	ImGui::Columns(1);
	ImGui::EndChild();

	ImGui::NextColumn();
	ImGui::BeginChild("RightColomunEventInfo", ImVec2(0, 0), ImGuiWindowFlags_NoTitleBar);
	// Large current time display
	const ImVec2 textSize = ImGui::CalcTextSize("00:00:00");
	const float centerX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerX);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	ImGui::Text("%s", core::formatClock(core::clock::now()).c_str());
	ImGui::PopFont();
	ImGui::EndChild();
	ImGui::Columns(1);
}

namespace {

auto screenInfoTolist(std::vector<MonitorInfo> iInfos) -> std::tuple<std::string, std::unordered_map<size_t, size_t>> {
	std::string monitor_list;
	std::unordered_map<size_t, size_t> index_map;
	size_t a = 0;
	for (size_t i = 0; i < iInfos.size(); ++i) {
		if (iInfos[i].isMainWindow)
			continue;
		index_map[a++] = i;
		monitor_list += std::format("Écran {}: {}", i + 1, iInfos[i].name) + '\0';
	}
	return {monitor_list, index_map};
}

}// namespace

void MainView::renderBottomPanel() {
	// Could be used for notifications or logs in the future
	if (ImGui::BeginTabBar("MainTabBar")) {
		if (ImGui::BeginTabItem("Configuration")) {
			ImGui::BeginChild("ConfigContent", {0, 0}, ImGuiChildFlags_None);

			// Draw type selection group
			ImGui::BeginGroup();
			ImGui::Text("Tirage Aléatoire");
			ImGui::Separator();
			{
				int drawMode = static_cast<int>(m_drawMode);
				ImGui::RadioButton("Tirage aléatoire seul", &drawMode, 0);
				ImGui::RadioButton("Tirage manuel seul", &drawMode, 1);
				ImGui::RadioButton("Les Deux", &drawMode, 2);
				if (drawMode != static_cast<int>(m_drawMode)) {
					// Update draw mode in application settings
					// Application::get().getSettings().setDrawMode(static_cast<core::DrawMode>(drawMode));
					log_info("Changement du mode de tirage: {} Wip", drawMode);
					m_drawMode = static_cast<DrawMode>(drawMode);
				}
			}
			ImGui::EndGroup();

			ImGui::SameLine();
			ImGui::Spacing();
			ImGui::SameLine();

			// Display settings group
			ImGui::BeginGroup();
			ImGui::Text("Affichage");
			ImGui::Separator();
			ImGui::Text("Selection de l'écran");
			{
				const auto infos = Application::get().getMonitorsInfo();
				const auto displayView = static_pointer_cast<DisplayView>(Application::get().getView("display_window"));
				static int selectedScreen = 0;
				if (infos.size() == 1)
					displayView->setFullscreen(false);
				bool fullscreen = displayView->isFullscreen();
				ImGui::SetNextItemWidth(400.0f);
				if (auto [scr_list_name, scr_list_idx] = screenInfoTolist(infos);
					ImGui::Combo("##ScreenSelect", &selectedScreen, scr_list_name.c_str())) {
					const size_t monitorIndex = scr_list_idx[static_cast<size_t>(selectedScreen)];
					displayView->setMonitorNumber(monitorIndex);
					log_info("Changement de l'écran d'affichage: {} ({})", monitorIndex, infos[monitorIndex].name);
				}
				if (ImGui::Checkbox("Plein écran", &fullscreen)) {
					if (infos.size() == 1)
						fullscreen = false;
					else {
						displayView->setFullscreen(fullscreen);
						log_info("Changement du mode plein écran: {}", fullscreen);
					}
				}
			}
			ImGui::EndGroup();

			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Information partie")) {
			ImGui::BeginChild("RoundInfoContent", {0, 0}, ImGuiChildFlags_None);

			const float groupWidth = ImGui::GetContentRegionAvail().x * 0.25f;
			const auto currentRound = m_currentEvent.getCurrentCGameRound();
			// Draws group
			ImGui::BeginGroup();
			ImGui::Text("Tirages");
			ImGui::Separator();
			ImGui::Text("Nombre:");
			ImGui::SameLine();
			ImGui::Text("%u", currentRound == m_currentEvent.endRounds()
									  ? 0
									  : static_cast<uint32_t>(currentRound->drawsCount()));
			ImGui::Spacing();
			ImGui::BeginChild("DrawsList", {groupWidth, 0}, ImGuiChildFlags_Border);
			if (currentRound == m_currentEvent.endRounds() || currentRound->drawsCount() == 0) {
				ImGui::TextWrapped("Aucun tirage effectué.");
			} else {
				ImGui::TextWrapped("%s", currentRound->getDrawStr().c_str());
			}
			ImGui::EndChild();
			ImGui::EndGroup();

			ImGui::SameLine();

			// Timing group
			ImGui::BeginGroup();
			ImGui::Text("Timing");
			ImGui::Separator();
			ImGui::Text("Début:");
			ImGui::SameLine();
			ImGui::Text("%s", currentRound == m_currentEvent.endRounds()
									  ? "--"
									  : core::formatClockNoSecond(currentRound->getStarting()).c_str());
			ImGui::Text("Durée:");
			ImGui::SameLine();
			ImGui::Text("%s", currentRound == m_currentEvent.endRounds()
									  ? "--"
									  : core::formatDuration(core::clock::now() - currentRound->getStarting()).c_str());
			ImGui::EndGroup();

			ImGui::SameLine();

			// Current round group
			ImGui::BeginGroup();
			ImGui::Text("Partie en cours");
			ImGui::Separator();
			ImGui::Text("Partie:");
			ImGui::SameLine();
			ImGui::Text("%s", currentRound == m_currentEvent.endRounds() ? "" : currentRound->getName().c_str());
			ImGui::Text("Phase:");
			ImGui::SameLine();
			ImGui::Text("%s", currentRound == m_currentEvent.endRounds() ? "" : currentRound->getStateString().c_str());
			ImGui::Text("Valeur:");
			ImGui::SameLine();
			ImGui::Text("%f €", currentRound == m_currentEvent.endRounds()
										? 0
										: currentRound->getCurrentSubRound()->getValue());
			ImGui::EndGroup();
			ImGui::SameLine();

			// Prizes group
			ImGui::BeginGroup();
			ImGui::Text("Lots");
			ImGui::Separator();
			ImGui::BeginChild("PrizesList", {groupWidth, 0}, ImGuiChildFlags_Border);
			ImGui::Text("%s", currentRound == m_currentEvent.endRounds()
									  ? ""
									  : currentRound->getCurrentSubRound()->getPrices().c_str());
			ImGui::EndChild();
			ImGui::EndGroup();

			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("log")) {
			ImGui::BeginChild("LogContent", {0, 0}, ImGuiChildFlags_None);
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);// Monospace font if available
			ImGui::TextWrapped("Log messages will appear here...");
			ImGui::PopFont();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

}// namespace evl::gui_imgui::views
