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
#include "StatusBar.h"
#include "core/Schedule.h"
#include "gui/Application.h"
#include "gui/utils/Convert.h"
#include "gui/utils/Layout.h"
#include "gui/utils/Rendering.h"

#include <imgui.h>

namespace evl::gui::views {

constexpr float g_splitterThickness = 4.0f;

namespace {

/// The four action buttons of the command section, stacked.
auto commandSectionHeight() -> float {
	const auto& style = ImGui::GetStyle();
	return 4.0f * (utils::actionButtonHeight() + style.ItemSpacing.y) + style.WindowPadding.y * 2.0f;
}

}// namespace

MainView::MainView(core::Event& iEvent) : m_currentEvent{iEvent} {}

MainView::~MainView() = default;

void MainView::onUpdate() {
	m_currentEvent = Application::get().getCurrentEvent();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	// Calculate the available space
	// La même hauteur que celle que la barre d'outils se donne, sinon la vue principale
	// la recouvre ou laisse un trou.
	const float toolBarHeight = utils::toolBarHeight();
	const float statusBarHeight = StatusBar::height();

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
		// In font heights, so a larger font does not squeeze the panels it fills.
		const float minBottom = utils::em(12.0f) / size.y;
		// What the bottom panel needs to show its tab bar and the three draw modes without a
		// scrollbar, and the same for the right one: where the splitters start. They can be
		// pushed further, down to a strip: that panel then scrolls.
		const float defaultBottom = 1.f - utils::em(11.0f) / size.y;
		const float defaultLeft = 1.f - utils::em(17.0f) / size.x;
		const float maxBottom = 1.f - utils::em(4.0f) / size.y;
		const float minLeft = utils::em(14.0f) / size.x;
		const float maxLeft = 1.f - utils::em(6.0f) / size.x;
		if (resized) {
			m_leftRightSplit = std::min(defaultLeft, maxLeft);
			m_topBottomSplit = std::min(defaultBottom, maxBottom);
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
	if (ImGui::BeginChild("MainTabs", {0, -ImGui::GetFrameHeightWithSpacing() * 5}, ImGuiChildFlags_None)) {
		if (ImGui::BeginTabBar("MainTabBar")) {
			if (ImGui::BeginTabItem("Numéros tirés")) {
				renderDrawnNumbersTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Présentateur")) {
				renderPresenterTab();
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

	if (ImGui::BeginChild("EventInfo", {0, 0}, ImGuiChildFlags_None)) {
		renderEventInfo();
	}
	ImGui::EndChild();
}

auto MainView::renderPrizeAnnouncement() const -> bool {
	if (m_currentEvent.getStatus() != core::Event::Status::GameRunning)
		return false;
	const auto round = m_currentEvent.getCurrentCGameRound();
	if (round == m_currentEvent.endRounds() || round->getType() == core::GameRound::Type::Pause ||
		round->getStatus() != core::GameRound::Status::Running)
		return false;
	const auto sub = round->getCurrentSubRound();
	if (sub == round->endSubRound() || sub->getStatus() != core::SubGameRound::Status::PreScreen)
		return false;

	const auto index = static_cast<size_t>(std::distance(round->beginSubRound(), sub));
	ImGui::SetWindowFontScale(1.4f);
	ImGui::Text("%s — %s", round->getName().c_str(), round->getSubRoundLabel(index).c_str());
	ImGui::SetWindowFontScale(1.0f);
	ImGui::TextDisabled("Les lots sont à l'écran : à énoncer avant de lancer les tirages.");
	ImGui::Separator();

	// Everything must be readable at once, with no scrolling: the font shrinks, and the list
	// goes on to several columns, until the whole of it fits the room that is left.
	// A children's round announces one set of prizes per winner.
	const auto prizes =
			core::announcedPrizes(sub->getPrizes(), round->getType() == core::GameRound::Type::Enfant);
	const auto detailsOf = [](const core::Prize& iPrize) -> std::string {
		std::string details;
		if (!iPrize.getDonor().empty())
			details = "offert par " + iPrize.getDonor();
		if (iPrize.getValue() > 0.0)
			details += std::format("{}valeur {:.2f} €", details.empty() ? "" : "  —  ", iPrize.getValue());
		return details;
	};
	constexpr float detailsFactor = 0.7f;
	constexpr float setTitleFactor = 0.8f;
	const std::string total = std::format("Valeur totale : {:.2f} €", sub->getValue());
	const ImVec2 room = ImGui::GetContentRegionAvail();
	const float columnGap = utils::em(1.5f);
	const float totalHeight = ImGui::GetTextLineHeightWithSpacing();
	const float availHeight = std::max(room.y - totalHeight * 1.5f - ImGui::GetStyle().ItemSpacing.y, 1.0f);

	// Height of one column holding a slice of the prizes, at a given scale and column width.
	const auto sliceHeight = [&](const size_t iFirst, const size_t iLast, const float iScale,
								 const float iWidth) -> float {
		float height = 0.0f;
		for (size_t i = iFirst; i < iLast; ++i) {
			const std::string& head = prizes[i].prize->getDesignation();
			// The spacing between two items does not follow the font scale.
			if (prizes[i].firstOfSet)
				height += ImGui::GetTextLineHeight() * iScale * setTitleFactor + ImGui::GetStyle().ItemSpacing.y;
			height += ImGui::CalcTextSize(head.c_str(), nullptr, false, iWidth / iScale).y * iScale +
					  ImGui::GetStyle().ItemSpacing.y;
			if (const auto details = detailsOf(*prizes[i].prize); !details.empty()) {
				const float small = iScale * detailsFactor;
				height += ImGui::CalcTextSize(details.c_str(), nullptr, false, iWidth / small).y * small +
						  ImGui::GetStyle().ItemSpacing.y;
			}
		}
		return height;
	};
	const auto fits = [&](const size_t iColumns, const float iScale) -> bool {
		const float width = (room.x - columnGap * static_cast<float>(iColumns - 1)) / static_cast<float>(iColumns);
		const size_t perColumn = (prizes.size() + iColumns - 1) / iColumns;
		for (size_t first = 0; first < prizes.size(); first += perColumn) {
			if (sliceHeight(first, std::min(first + perColumn, prizes.size()), iScale, width) > availHeight)
				return false;
		}
		return true;
	};
	// The largest scale that fits, for each number of columns, and the best of the five.
	constexpr float maxScale = 3.0f;
	constexpr float minScale = 0.3f;
	size_t bestColumns = 1;
	float bestScale = minScale;
	for (size_t columns = 1; columns <= 5; ++columns) {
		float low = minScale;
		float high = maxScale;
		for (int step = 0; step < 14; ++step) {
			const float middle = (low + high) * 0.5f;
			(fits(columns, middle) ? low : high) = middle;
		}
		if (low > bestScale * 1.05f) {
			bestScale = low;
			bestColumns = columns;
		}
	}

	const float width = (room.x - columnGap * static_cast<float>(bestColumns - 1)) / static_cast<float>(bestColumns);
	const size_t perColumn = std::max<size_t>((prizes.size() + bestColumns - 1) / bestColumns, 1);
	// In a child of the room left: should even the smallest type not fit, it scrolls instead of
	// running over what is below.
	ImGui::BeginChild("PrizeAnnouncement", {0, availHeight + ImGui::GetStyle().ItemSpacing.y}, ImGuiChildFlags_None);
	const ImVec2 origin = ImGui::GetCursorPos();
	for (size_t i = 0; i < prizes.size(); ++i) {
		const size_t column = i / perColumn;
		// An item always starts a new line at the left of the window: the column's own left
		// edge has to be given again before every one.
		const float columnX = origin.x + static_cast<float>(column) * (width + columnGap);
		if (i % perColumn == 0)
			ImGui::SetCursorPos({columnX, origin.y});
		ImGui::SetCursorPosX(columnX);
		ImGui::PushTextWrapPos(columnX + width);
		if (prizes[i].firstOfSet) {
			ImGui::SetWindowFontScale(bestScale * setTitleFactor);
			ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram), "Gagnant %u", prizes[i].set);
			ImGui::SetCursorPosX(columnX);
		}
		ImGui::SetWindowFontScale(bestScale);
		ImGui::TextUnformatted(prizes[i].prize->getDesignation().c_str());
		if (const auto details = detailsOf(*prizes[i].prize); !details.empty()) {
			ImGui::SetCursorPosX(columnX);
			ImGui::SetWindowFontScale(bestScale * detailsFactor);
			ImGui::TextDisabled("%s", details.c_str());
		}
		ImGui::SetWindowFontScale(1.0f);
		ImGui::PopTextWrapPos();
	}
	ImGui::EndChild();
	ImGui::Separator();
	ImGui::SetWindowFontScale(1.2f);
	ImGui::TextUnformatted(total.c_str());
	ImGui::SetWindowFontScale(1.0f);
	return true;
}

void MainView::renderDrawnNumbersTab() const {
	// While the lots are on the players' screen, the presenter reads them here instead.
	if (renderPrizeAnnouncement())
		return;
	// The end screen is up: no number to draw, the presenter is told what the room sees.
	if (m_currentEvent.getStatus() == core::Event::Status::GameRunning) {
		const auto round = m_currentEvent.getCurrentCGameRound();
		if (round != m_currentEvent.endRounds() && round->getStatus() == core::GameRound::Status::PostScreen) {
			const char* message = round->getType() == core::GameRound::Type::Pause ? "Reprise du jeu" : "Fin de partie";
			const float scale = 3.0f;
			ImGui::SetWindowFontScale(scale);
			const ImVec2 size = ImGui::CalcTextSize(message);
			const ImVec2 avail = ImGui::GetContentRegionAvail();
			ImGui::SetCursorPos({ImGui::GetCursorPosX() + std::max((avail.x - size.x) * 0.5f, 0.0f),
								 ImGui::GetCursorPosY() + std::max((avail.y - size.y) * 0.4f, 0.0f)});
			ImGui::TextUnformatted(message);
			ImGui::SetWindowFontScale(1.0f);
			return;
		}
	}
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
	const bool manualDisabled = (m_drawMode == DrawMode::Random) || Application::get().isDrawHeld();
	if (manualDisabled)
		ImGui::BeginDisabled();
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
			if (clicked && !isDrawn && !manualDisabled) {
				currentRound->addPickedNumber(number);
				rng.addPick(number);
				Application::get().notifyDraw();
				Application::get().saveProgress();
			}

			// Same line except last column
			if (col < 9) {
				ImGui::SameLine();
			}
		}
	}
	if (manualDisabled)
		ImGui::EndDisabled();
}

void MainView::renderPresenterTab() {
	const auto displayView = std::static_pointer_cast<DisplayView>(Application::get().getView("display_window"));
	if (displayView == nullptr) {
		ImGui::TextDisabled("Affichage indisponible");
		return;
	}
	displayView->renderInline(utils::imVec2ToVec2(ImGui::GetContentRegionAvail()));
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

	const float commandsHeight = commandSectionHeight();
	const float numbersHeight = ImGui::GetContentRegionAvail().y - commandsHeight - style.ItemSpacing.y;

	if (ImGui::BeginChild("NumbersDisplay", {0, std::max(numbersHeight, utils::em(4.0f))}, ImGuiChildFlags_Borders)) {
		// Current draw
		if (ImGui::BeginChild("CurrentDraw", {0, 0.70f * ImGui::GetContentRegionAvail().y}, ImGuiChildFlags_None)) {
			ImGui::Text("Numéro tiré");
			ImGui::Separator();
			const std::string numberText = (prevDrawnNumber != -1) ? std::format("{}", prevDrawnNumber) : "--";
			utils::adaptTextToRegion(
					numberText,
					{.autoRegion = true, .vCenter = true, .hCenter = true, .drawText = true, .textAdapt = "00"});
		}
		ImGui::EndChild();

		// Last numbers
		if (ImGui::BeginChild("LastNumbers", {0, 0}, ImGuiChildFlags_None)) {
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
	if (ImGui::BeginChild("Commands", {0, 0}, ImGuiChildFlags_Borders)) {
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
	const double held = Application::get().getDrawDelayRemaining();
	const std::string pickLabel = held > 0.0 ? std::format("Tirage Aléatoire ({:.1f} s)", held) : "Tirage Aléatoire";
	utils::defineActionButtonItem(
			pickLabel, "random_pick",
			{.showLabel = true,
			 .disabled = !m_currentEvent.canDraw() || m_drawMode == DrawMode::Manual || held > 0.0,
			 .sameLine = false,
			 .setDisabled = true});
	utils::defineActionButtonItem(
			"Annuler dernier tirage", "cancel_pick",
			{.showLabel = true, .disabled = prevDrawnNumber == -1, .sameLine = false, .setDisabled = true});
}

namespace {

/// A gap to the programme, in words: late, ahead, or on time within the minute.
auto delayText(const core::duration& iDelay) -> std::string {
	const auto minutes = static_cast<int>(std::lround(iDelay.count() / 60.0));
	if (minutes == 0)
		return "à l'heure";
	const int amount = std::abs(minutes);
	const std::string length =
			amount >= 60 ? std::format("{} h {:02}", amount / 60, amount % 60) : std::format("{} min", amount);
	return minutes > 0 ? std::format("+{}", length) : std::format("-{}", length);
}

/// A time between two numbers, in seconds while it is under a minute.
auto intervalText(const core::duration& iInterval) -> std::string {
	if (iInterval < std::chrono::minutes{1})
		return std::format("{:.0f} s", iInterval.count());
	return core::formatDuration(iInterval);
}

/// A sum of money for a tile.
auto euros(const double iValue) -> std::string { return std::format("{:.2f} €", iValue); }

/// A duration for the statistics, a dash while there is nothing measured.
auto durationText(const core::duration& iDuration) -> std::string {
	return iDuration > core::duration::zero() ? core::formatDuration(iDuration) : std::string{"--"};
}

/**
 * @brief A figure in a tile: what it is, the figure itself in large, and what it means.
 * @param iId The child id.
 * @param iTitle What the figure is.
 * @param iValue The figure.
 * @param iHint A line under it, possibly empty.
 * @param iWidth The tile width.
 */
void statTile(const char* iId, const char* iTitle, const std::string& iValue, const std::string& iHint,
			  const float iWidth) {
	const float height = ImGui::GetTextLineHeightWithSpacing() * 2.0f + ImGui::GetFontSize() * 1.8f +
						 ImGui::GetStyle().WindowPadding.y * 2.0f;
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg));
	if (ImGui::BeginChild(iId, {iWidth, height}, ImGuiChildFlags_Borders,
						  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
		ImGui::TextDisabled("%s", iTitle);
		ImGui::SetWindowFontScale(1.8f);
		ImGui::TextUnformatted(iValue.c_str());
		ImGui::SetWindowFontScale(1.0f);
		if (!iHint.empty())
			ImGui::TextDisabled("%s", iHint.c_str());
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
}

/**
 * @brief How often each number came out, one bar per number.
 *
 * Drawn by hand rather than with `PlotHistogram`, whose tooltip counts from zero: the
 * bar under the mouse has to say which number it is.
 * @param iCounts One count per number, number 1 first.
 * @param iHeight The height of the chart.
 */
void pickChart(const core::Statistics::pick_list& iCounts, const float iHeight) {
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const float width = ImGui::GetContentRegionAvail().x;
	ImGui::InvisibleButton("##PickChart", {std::max(width, 1.0f), iHeight});
	if (iCounts.empty())
		return;
	auto* draw = ImGui::GetWindowDrawList();
	draw->AddRectFilled(origin, {origin.x + width, origin.y + iHeight}, ImGui::GetColorU32(ImGuiCol_FrameBg),
						ImGui::GetStyle().FrameRounding);
	const auto [least, most] = std::ranges::minmax(iCounts);
	const float barWidth = width / static_cast<float>(iCounts.size());
	const bool hovered = ImGui::IsItemHovered();
	const float mouseX = ImGui::GetIO().MousePos.x - origin.x;
	for (size_t i = 0; i < iCounts.size(); ++i) {
		const float x = origin.x + barWidth * static_cast<float>(i);
		const float ratio = most > 0 ? static_cast<float>(iCounts[i]) / static_cast<float>(most) : 0.0f;
		const float top = origin.y + iHeight - (iHeight - 2.0f) * ratio;
		const bool isHovered = hovered && mouseX >= x - origin.x && mouseX < x - origin.x + barWidth;
		ImU32 colour = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
		if (most > least && iCounts[i] == most)
			colour = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered);
		if (isHovered)
			colour = ImGui::GetColorU32(ImGuiCol_Text);
		if (ratio > 0.0f)
			draw->AddRectFilled({x + 1.0f, top}, {x + barWidth - 1.0f, origin.y + iHeight}, colour);
		if (isHovered)
			ImGui::SetTooltip("Numéro %zu : %u sortie%s", i + 1, iCounts[i], iCounts[i] > 1 ? "s" : "");
	}
}

}// namespace

void MainView::renderStatisticsTab() const {
	const auto stats = m_currentEvent.getStats();
	const auto schedule = core::estimateSchedule(m_currentEvent, Application::getPauseLength(), core::clock::now());
	const auto& counts = stats.pickCounts();
	uint32_t totalDraws = 0;
	for (const auto count: counts) totalDraws += count;

	// The figures the presenter is asked about, in tiles: four per row, two when narrow.
	const auto& style = ImGui::GetStyle();
	const float avail = ImGui::GetContentRegionAvail().x;
	const int perRow = avail < utils::em(30.0f) ? 2 : 4;
	const float tileWidth = (avail - style.ItemSpacing.x * static_cast<float>(perRow - 1)) / static_cast<float>(perRow);
	int index = 0;
	const auto tile = [&](const char* iId, const char* iTitle, const std::string& iValue,
						  const std::string& iHint) -> void {
		statTile(iId, iTitle, iValue, iHint, tileWidth);
		if (++index % perRow != 0)
			ImGui::SameLine();
	};
	tile("EndTile", "Fin estimée",
		 schedule.estimatedEnd ? core::formatClockNoSecond(schedule.estimatedEnd.value()) : "--",
		 schedule.estimatedEnd ? std::format("dans {}", core::formatDuration(schedule.remaining))
							   : "après la première manche");
	tile("DelayTile", "Retard", schedule.delay ? delayText(schedule.delay.value()) : "--",
		 schedule.plannedEnd ? std::format("prévue {}", core::formatClockNoSecond(schedule.plannedEnd.value()))
							 : "pas de fin prévue");
	tile("LeftTile", "Reste à jouer",
		 std::format("{} manche{}", schedule.subRoundsLeft, schedule.subRoundsLeft > 1 ? "s" : ""),
		 schedule.pausesLeft == 0
				 ? std::string{"sans pause"}
				 : std::format("et {} pause{}", schedule.pausesLeft, schedule.pausesLeft > 1 ? "s" : ""));
	tile("RhythmTile", "Rythme", schedule.drawInterval ? intervalText(schedule.drawInterval.value()) : "--",
		 schedule.currentDrawInterval ? std::format("ici : {}", intervalText(schedule.currentDrawInterval.value()))
									  : "par numéro");
	tile("AverageTile", "Manche moyenne", durationText(stats.subRoundAverage),
		 stats.subRoundAverageNb > 0.0 ? std::format("{:.0f} tirages", stats.subRoundAverageNb) : "");
	tile("DrawsTile", "Numéros tirés", std::format("{}", totalDraws),
		 std::format("{:.0f} % joué", m_currentEvent.getProgression() * 100.0f));
	const auto prizes = core::tallyPrizes(m_currentEvent);
	tile("GivenTile", "Lots distribués", euros(prizes.given),
		 prizes.unclaimed > 0.0 ? std::format("{:.0f} € non réclamés", prizes.unclaimed) : "");
	tile("RemainingTile", "Lots restants", euros(prizes.remaining), "encore en jeu");
	if (index % perRow != 0)
		ImGui::NewLine();

	ImGui::SeparatorText("Sorties par numéro");
	pickChart(counts, utils::em(4.0f));
	if (totalDraws > 0) {
		ImGui::TextDisabled("Les plus sortis (%d fois)", stats.mostPickNb);
		ImGui::SameLine();
		ImGui::TextWrapped("%s", stats.mostPickStr().c_str());
		ImGui::TextDisabled("Les moins sortis (%d fois)", stats.lessPickNb);
		ImGui::SameLine();
		ImGui::TextWrapped("%s", stats.lessPickStr().c_str());
	}

	ImGui::SeparatorText("Parties et manches");
	if (ImGui::BeginTable("StatsDetails", 3,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp)) {
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 1.4f);
		ImGui::TableSetupColumn("Parties", ImGuiTableColumnFlags_WidthStretch, 1.0f);
		ImGui::TableSetupColumn("Manches", ImGuiTableColumnFlags_WidthStretch, 1.0f);
		ImGui::TableHeadersRow();
		const auto row = [](const char* iLabel, const std::string& iRounds, const std::string& iSubRounds) -> void {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextDisabled("%s", iLabel);
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(iRounds.c_str());
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(iSubRounds.c_str());
		};
		row("Plus longue", durationText(stats.roundLongest), durationText(stats.subRoundLongest));
		row("Plus courte", durationText(stats.roundShortest), durationText(stats.subRoundShortest));
		row("Durée moyenne", durationText(stats.roundAverage), durationText(stats.subRoundAverage));
		row("Plus de tirages", std::format("{}", stats.roundMostNb), std::format("{}", stats.subRoundMostNb));
		row("Moins de tirages", std::format("{}", stats.roundLessNb), std::format("{}", stats.subRoundLessNb));
		row("Tirages moyens", std::format("{:.1f}", stats.roundAverageNb),
			std::format("{:.1f}", stats.subRoundAverageNb));
		ImGui::EndTable();
	}
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
		date = std::format("{}", core::formatCalendar(start));
		startTime = core::formatClock(start);
		// While playing, the end is the one expected; the recorded one only exists after.
		const auto schedule = core::estimateSchedule(m_currentEvent, Application::getPauseLength(), core::clock::now());
		endTime = schedule.estimatedEnd ? std::format("~ {}", core::formatClockNoSecond(schedule.estimatedEnd.value()))
										: std::string{"--"};
		duration = core::formatDuration(core::clock::now() - start);
		progress = m_currentEvent.getProgression();
	}

	// The figures on the left, the clock on the right taking what is left: the clock is
	// what the presenter reads from afar, the figures are read up close.
	if (!ImGui::BeginTable("EventInfoLayout", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV,
						   {-1, -1}))
		return;
	ImGui::TableSetupColumn("Chiffres", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	ImGui::TableSetupColumn("Horloge", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	if (ImGui::BeginTable("EventFigures", 4, ImGuiTableFlags_SizingFixedFit)) {
		ImGui::TableSetupColumn("l1", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("v1", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("l2", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("v2", ImGuiTableColumnFlags_WidthStretch);
		const auto figure = [](const char* iLabel, const std::string& iValue) -> void {
			ImGui::TableNextColumn();
			ImGui::TextUnformatted(iLabel);
			ImGui::TableNextColumn();
			ImGui::TextDisabled("%s", iValue.c_str());
		};
		figure("Date", date);
		figure("Durée", duration);
		figure("Début", startTime);
		figure("Fin", endTime);
		ImGui::EndTable();
	}
	ImGui::ProgressBar(progress, {-1, 0});
	ImGui::TableSetColumnIndex(1);
	if (ImGui::BeginChild("EventClock", {0, std::max(ImGui::GetContentRegionAvail().y, ImGui::GetFrameHeight())},
						  ImGuiChildFlags_None)) {
		utils::adaptTextToRegion(
				core::formatClock(core::clock::now()),
				{.autoRegion = true, .vCenter = true, .hCenter = true, .drawText = true, .textAdapt = "00:00:00"});
	}
	ImGui::EndChild();
	ImGui::EndTable();
}

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
		if (ImGui::BeginTabItem("Journal")) {
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
			m_drawMode = static_cast<DrawMode>(drawMode);
			log_info("Changement du mode de tirage: {}", drawMode);
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
	ImGui::Text("Sélection de l'écran");
	{
		const auto infos = Application::get().getMonitorsInfo();
		const auto displayView = static_pointer_cast<DisplayView>(Application::get().getView("display_window"));
		if (displayView == nullptr) {
			ImGui::EndGroup();
			ImGui::EndChild();
			return;
		}
		const auto candidates = displayCandidates(infos);
		if (candidates.empty()) {
			ImGui::TextDisabled("Un seul écran : affichage en fenêtre");
		} else {
			const size_t current = displayView->getMonitor(infos);
			const auto label = [&infos](const size_t iIndex) -> std::string {
				const auto& info = infos[iIndex];
				return std::format("Écran {} : {} ({}×{})", iIndex + 1, info.name, info.size.x(), info.size.y());
			};
			ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x, 22.0f * ImGui::GetFontSize()));
			if (ImGui::BeginCombo("##ScreenSelect", label(current).c_str())) {
				for (const size_t index: candidates) {
					if (ImGui::Selectable(label(index).c_str(), index == current))
						displayView->setMonitor(infos[index]);
					if (index == current)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			bool fullscreen = displayView->isFullscreen();
			if (ImGui::Checkbox("Plein écran", &fullscreen)) {
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
	ImGui::BeginChild("DrawsList", {drawWidth, 0}, ImGuiChildFlags_Borders);
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
	ImGui::Text("Durée");
	ImGui::Separator();
	ImGui::BeginChild("TimingStat", {timingWidth, 0}, ImGuiChildFlags_Borders);
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
	ImGui::BeginChild("OngoingRound", {roundWidth, 0}, ImGuiChildFlags_Borders);
	ImGui::Text("Partie:");
	ImGui::SameLine();
	if (currentRound == m_currentEvent.endRounds()) {
		ImGui::Text(" ");
		ImGui::Text("Phase:");
		ImGui::SameLine();
		ImGui::Text(" ");
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
		if (const auto sub = currentRound->getCurrentSubRound(); sub == currentRound->endSubRound()) {
			ImGui::Text("N/A");
		} else {
			ImGui::Text("%s", std::format("{:.2f} €", sub->getValue()).c_str());
		}
	}
	ImGui::EndChild();
	ImGui::EndGroup();
	ImGui::SameLine();

	// Prizes group
	ImGui::BeginGroup();
	ImGui::Text("Lots");
	ImGui::Separator();
	ImGui::BeginChild("PrizesList", {pricesWidth, 0}, ImGuiChildFlags_Borders);
	if (currentRound == m_currentEvent.endRounds()) {
		ImGui::TextWrapped(" ");
	} else if (currentRound->getType() == core::GameRound::Type::Pause) {
		ImGui::TextWrapped("Pas de lots pour une pause.");
	} else if (currentRound->getCurrentSubRound() == currentRound->endSubRound()) {
		ImGui::TextDisabled("Partie terminée.");
	} else if (const auto& prizes = currentRound->getCurrentSubRound()->getPrizes(); prizes.empty()) {
		ImGui::TextDisabled("Aucun lot pour cette phase.");
	} else if (ImGui::BeginTable("Prizes", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
		// What the presenter reads out during the announcement: the article, what it is
		// worth, and who to thank for it.
		ImGui::TableSetupColumn("Article");
		ImGui::TableSetupColumn("Valeur", ImGuiTableColumnFlags_WidthFixed, utils::em(4.5f));
		ImGui::TableSetupColumn("Donateur");
		ImGui::TableHeadersRow();
		for (const auto& prize: prizes) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextWrapped("%s", prize.getDesignation().c_str());
			ImGui::TableNextColumn();
			if (prize.getValue() > 0.0)
				ImGui::Text("%.2f €", prize.getValue());
			ImGui::TableNextColumn();
			ImGui::TextWrapped("%s", prize.getDonor().c_str());
		}
		ImGui::EndTable();
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

}// namespace evl::gui::views
