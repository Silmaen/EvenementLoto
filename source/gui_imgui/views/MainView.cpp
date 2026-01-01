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
#include "gui_imgui/utils/Convert.h"
#include "gui_imgui/utils/Rendering.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

constexpr float g_splitterThickness = 4.0f;
constexpr float g_statusBarHeight = 25.0f;
constexpr float g_minLeftPanelWidth = 200.0f;
constexpr float g_minRightPanelWidth = 320.0f;
constexpr float g_minTopPanelHeight = 150.0f;
constexpr float g_minBottomPanelHeight = 200.0f;

constexpr float g_commandSectionHeight = 150.0f;


MainView::MainView(core::Event& iEvent) : m_currentEvent{iEvent} {}

MainView::~MainView() = default;

void MainView::onUpdate() {
	m_currentEvent = Application::get().getCurrentEvent();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const ImGuiStyle& style = ImGui::GetStyle();

	// Calculate the available space
	const float toolBarHeight = ImGui::GetFrameHeight() + style.WindowPadding.y * 2;
	const float statusBarHeight = g_statusBarHeight + style.WindowPadding.y;

	// Position and size for the main view
	const ImVec2 pos = {viewport->WorkPos.x, viewport->WorkPos.y + toolBarHeight};
	const ImVec2 size = {viewport->WorkSize.x, viewport->WorkSize.y - toolBarHeight - statusBarHeight};
	bool resized = false;
	if (std::abs(size.x - m_lastSize.x()) > .001f || std::abs(size.y - m_lastSize.y()) > .001f) {
		// Size changed, you can handle it here if needed
		m_lastSize = utils::imVec2ToVec2(size);
		resized = true;
		log_debug("MainView resized to: {} x {}", size.x, size.y);
	}


	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(size);

	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
									   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
									   ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (ImGui::Begin("MainView", nullptr, flags)) {
		// Initialize splitter positions if needed
		const float minBottom = g_minTopPanelHeight / size.y;
		const float maxBottom = 1.f - g_minBottomPanelHeight / size.y;
		const float minLeft = g_minLeftPanelWidth / size.x;
		const float maxLeft = 1.f - g_minRightPanelWidth / size.x;
		if (resized) {
			m_leftRightSplit = maxLeft;
			m_topBottomSplit = maxBottom;
			log_debug("MainView splitters reset due to resize: Left-Right {}, Top-Bottom {}", m_leftRightSplit,
					  m_topBottomSplit);
		}

		// Top panel
		const float topPanelHeight = size.y * m_topBottomSplit;
		ImGui::BeginChild("TopPanel", {0, topPanelHeight}, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

		// Left panel
		const float leftPanelWidth = size.x * m_leftRightSplit;
		ImGui::BeginChild("LeftPanel", {leftPanelWidth, 0}, ImGuiChildFlags_None, ImGuiWindowFlags_NoResize);
		renderLeftPanel();
		ImGui::EndChild();

		// Vertical splitter between left and right
		ImGui::SameLine();
		ImGui::Button("##vsplitter", {g_splitterThickness, -1});
		if (ImGui::IsItemActive()) {
			m_leftRightSplit += ImGui::GetIO().MouseDelta.x / size.x;
			m_leftRightSplit = std::clamp(m_leftRightSplit, minLeft, maxLeft);
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
		ImGui::Button("##hsplitter", {-1, g_splitterThickness});
		if (ImGui::IsItemActive()) {
			m_topBottomSplit += ImGui::GetIO().MouseDelta.y / size.y;
			m_topBottomSplit = std::clamp(m_topBottomSplit, minBottom, maxBottom);
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
			const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
			const float scaleX = buttonSize.x / textSize.x;
			const float scaleY = buttonSize.y / textSize.y;
			const float scale = std::min(scaleX, scaleY) * 0.8f;
			if (scale > 0.f)
				ImGui::SetWindowFontScale(scale);
			const bool clicked = ImGui::Button(label.c_str(), buttonSize);
			ImGui::SetWindowFontScale(1.0f);

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
	const ImGuiStyle& style = ImGui::GetStyle();
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

	auto size = ImGui::GetContentRegionAvail();
	size.y -= 2 * style.WindowPadding.y;
	size.x -= style.WindowPadding.x;


	if (ImGui::BeginChild("NumbersDisplay", {size.x, size.y - g_commandSectionHeight}, ImGuiWindowFlags_NoTitleBar)) {
		// Current draw
		if (ImGui::BeginChild("CurrentDraw", {0, 0.70f * ImGui::GetContentRegionAvail().y},
							  ImGuiWindowFlags_NoResize)) {
			ImGui::Text("Numéro tiré");
			ImGui::Separator();
			const std::string numberText = (prevDrawnNumber != -1) ? std::format("{}", prevDrawnNumber) : "--";
			utils::adaptTextToRegion(
					numberText,
					{.autoRegion = true, .vCenter = true, .hCenter = true, .drawText = true, .textAdapt = "00"});
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
			utils::adaptTextToRegion(
					lastNumbers,
					{.autoRegion = true, .vCenter = true, .hCenter = true, .drawText = true, .textAdapt = "00 00 00"});
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	// Commands section at bottom
	if (ImGui::BeginChild("Commands", {size.x, g_commandSectionHeight}, ImGuiWindowFlags_NoTitleBar)) {
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
			case core::Event::Status::Ready:
			case core::Event::Status::EventEnding:
				btnDisabled = true;
				break;
			case core::Event::Status::EventStarting:
			case core::Event::Status::DisplayRules:
			case core::Event::Status::GameRunning:
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
	utils::adaptTextToRegion(
			core::formatClock(core::clock::now()),
			{.autoRegion = true, .vCenter = true, .hCenter = true, .drawText = true, .textAdapt = "00:00:00"});

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
			renderBottomConfigPanel();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Information partie")) {
			renderBottomStatisticsPanel();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("log")) {
			renderBottomLogsPanel();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
void MainView::renderBottomConfigPanel() {
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
		if (infos.size() == 1)
			displayView->setFullscreen(false);
		bool fullscreen = displayView->isFullscreen();
		//ImGui::SetNextItemWidth(400.0f);
		if (auto [scr_list_name, scr_list_idx] = screenInfoTolist(infos);
			ImGui::Combo("##ScreenSelect", &m_selectedScreen, scr_list_name.c_str())) {
			const size_t monitorIndex = scr_list_idx[static_cast<size_t>(m_selectedScreen)];
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
}

void MainView::renderBottomStatisticsPanel() const {
	ImGui::BeginChild("RoundInfoContent", {0, 0}, ImGuiChildFlags_None);

	const float drawWidth = ImGui::GetContentRegionAvail().x * 0.30f;
	const float timingWidth = ImGui::GetContentRegionAvail().x * 0.10f;
	const float roundWidth = ImGui::GetContentRegionAvail().x * 0.15f;
	const float pricesWidth = ImGui::GetContentRegionAvail().x * 0.45f;

	const auto currentRound = m_currentEvent.getCurrentCGameRound();
	// Draws group
	ImGui::BeginGroup();
	ImGui::Text("Tirages");
	ImGui::Separator();
	ImGui::Text("Nombre:");
	ImGui::SameLine();
	ImGui::Text("%u",
				currentRound == m_currentEvent.endRounds() ? 0 : static_cast<uint32_t>(currentRound->drawsCount()));
	ImGui::Spacing();
	ImGui::BeginChild("DrawsList", {drawWidth, 0}, ImGuiChildFlags_Border);
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
	ImGui::BeginChild("TimingStat", {timingWidth, 0}, ImGuiChildFlags_Border);
	if (currentRound == m_currentEvent.endRounds()) {
		ImGui::Text("Début:");
		ImGui::SameLine();
		ImGui::Text("--");
		ImGui::Text("Durée:");
		ImGui::SameLine();
		ImGui::Text("--");
	} else {
		ImGui::Text("Début:");
		ImGui::SameLine();
		ImGui::Text("%s", core::formatClockNoSecond(currentRound->getStarting()).c_str());
		ImGui::Text("Durée:");
		ImGui::SameLine();
		ImGui::Text("%s", core::formatDuration(core::clock::now() - currentRound->getStarting()).c_str());
	}
	ImGui::EndChild();
	ImGui::EndGroup();

	ImGui::SameLine();

	// Current round group
	ImGui::BeginGroup();
	ImGui::Text("Partie en cours");
	ImGui::Separator();
	ImGui::BeginChild("OngoingRound", {roundWidth, 0}, ImGuiChildFlags_Border);
	ImGui::Text("Partie:");
	ImGui::SameLine();
	if (currentRound == m_currentEvent.endRounds()) {
		ImGui::Text("");
		ImGui::Text("Phase:");
		ImGui::SameLine();
		ImGui::Text("");
		ImGui::Text("Valeur:");
		ImGui::SameLine();
		ImGui::Text("0 €");
	} else {
		ImGui::Text("%s", currentRound->getName().c_str());
		ImGui::Text("Phase:");
		ImGui::SameLine();
		ImGui::Text("%s", currentRound->getStateString().c_str());
		ImGui::Text("Valeur:");
		ImGui::SameLine();
		if (currentRound->getType() == core::GameRound::Type::Pause) {
			ImGui::Text("N/A");
		} else {
			const auto value = std::format("{:.2f} €", currentRound->getCurrentSubRound()->getValue());
			ImGui::Text("%s", value.c_str());
		}
	}
	ImGui::EndChild();
	ImGui::EndGroup();
	ImGui::SameLine();

	// Prizes group
	ImGui::BeginGroup();
	ImGui::Text("Lots");
	ImGui::Separator();
	ImGui::BeginChild("PrizesList", {pricesWidth, 0}, ImGuiChildFlags_Border);
	if (currentRound == m_currentEvent.endRounds()) {
		ImGui::TextWrapped("");
	} else if (currentRound->getType() == core::GameRound::Type::Pause) {
		ImGui::TextWrapped("Pas de lots pour une pause.");
	} else {
		ImGui::Text("%s", currentRound->getCurrentSubRound()->getPrices().c_str());
	}
	ImGui::EndChild();
	ImGui::EndGroup();

	ImGui::EndChild();
}

void MainView::renderBottomLogsPanel() {
	ImGui::BeginChild("LogContent", {0, 0}, ImGuiChildFlags_None);
	const auto& logs = logs::LogBuffer::get().getLogs();
	if (m_logScale > 0.f)
		ImGui::SetWindowFontScale(m_logScale);
	for (const auto& [message, level, timestamp]: logs) {
		ImVec4 color{0.85f, 0.85f, 0.85f, 1.0f};// Default text color
		switch (level) {
			case Log::Level::Trace:
			case Log::Level::Debug:
				color = {0.50f, 0.75f, 1.0f, 1.0f};// Blue
				break;
			case Log::Level::Info:
				color = {0.85f, 0.85f, 0.85f, 1.0f};// White
				break;
			case Log::Level::Warning:
				color = {1.0f, 0.85f, 0.0f, 1.0f};// Yellow
				break;
			case Log::Level::Error:
			case Log::Level::Critical:
				color = {1.0f, 0.30f, 0.30f, 1.0f};// Red
				break;
			case Log::Level::Off:
				break;
		}
		auto line = std::format("[{}] [{}] {}", core::formatClock(timestamp), magic_enum::enum_name(level), message);
		ImGui::TextColored(color, "%s", line.c_str());
	}
	if (m_lineInLog != logs.size()) {
		ImGui::SetScrollHereY(1.0f);
		m_lineInLog = logs.size();
	}
	ImGui::SetWindowFontScale(1.0f);
	ImGui::EndChild();
}

}// namespace evl::gui_imgui::views
