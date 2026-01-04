/**
 * @file DisplayView.cpp
 * @author Silmaen
 * @date 22/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "DisplayView.h"

#include "core/utilities.h"
#include "gui_imgui/Application.h"
#include "gui_imgui/utils/Convert.h"
#include "gui_imgui/utils/Rendering.h"

#include <imgui.h>

#include <algorithm>

#include <ranges>
#include <utility>

namespace evl::gui_imgui::views {

namespace {

void renderTitle(const std::string& iTitle, const math::vec2& iRegion, const float iExtraScale = 1.0f) {
	// Part title
	const auto gui_settings = core::getSettings()->extract("gui");
	ImGui::SetCursorPosY(iRegion.y() * 0.05f);

	if (const auto scale = gui_settings.getValue("title_scale", 4.0f) * iExtraScale; scale > 0.0f) {
		ImGui::SetWindowFontScale(scale);
	}
	const ImVec2 titleSize = ImGui::CalcTextSize(iTitle.c_str());
	ImGui::SetCursorPosX((iRegion.x() - titleSize.x) * 0.5f);
	ImGui::Text("%s", iTitle.c_str());
	ImGui::SetWindowFontScale(1.0f);
}

void drawImage(const std::string& iTextureName, const math::vec2& iPosition, const math::vec2& iSize) {
	auto& app = Application::get();
	const auto& texLib = app.getTextureLibrary();

	if (const uint64_t texId = texLib.getTextureId(iTextureName); texId != 0) {
		const auto imgInfo = texLib.getRawPixels(iTextureName);
		const auto scale =
				std::min(iSize.x() / static_cast<float>(imgInfo.width), iSize.y() / static_cast<float>(imgInfo.height));
		const ImVec2 adaptedSize = {static_cast<float>(imgInfo.width) * scale,
									static_cast<float>(imgInfo.height) * scale};
		ImGui::SetCursorPos({iPosition.x() + (iSize.x() - adaptedSize.x) * 0.5f,
							 iPosition.y() + (iSize.y() - adaptedSize.y) * 0.5f});
		ImGui::Image(texId, adaptedSize);
	} else {
		ImGui::SetCursorPos({iPosition.x(), iPosition.y()});
		utils::adaptTextToRegion("<no logo>", {.autoRegion = false,
											   .contentSize = iSize,
											   .vCenter = true,
											   .hCenter = true,
											   .drawText = true,
											   .textAdapt = ""});
	}
}

void loadEventImages(const core::Event& iEvent) {
	auto& app = Application::get();
	auto& texLib = app.getTextureLibrary();
	// Organizer logo
	if (const std::string organizerLogoPath = iEvent.getOrganizerLogoFull().string(); !organizerLogoPath.empty()) {
		if (texLib.getOrLoadTextureId("logo_organizer", organizerLogoPath) == 0) {
			log_warning("Failed to load organizer logo from '{}'", organizerLogoPath);
		}
	}
	// Event logo
	if (const std::string eventLogoPath = iEvent.getLogoFull().string(); !eventLogoPath.empty()) {
		if (texLib.getOrLoadTextureId("logo_event", eventLogoPath) == 0) {
			log_warning("Failed to load event logo from '{}'", eventLogoPath);
		}
	}
}

}// namespace

DisplayView::DisplayView(core::Event& iEvent) : m_currentEvent{iEvent} {}

DisplayView::~DisplayView() = default;

void DisplayView::onUpdate() {
	auto& style = ImGui::GetStyle();
	const auto style_backup = style;
	applyCommonStyle();
	auto& app = Application::get();
	m_currentEvent = app.getCurrentEvent();
	loadEventImages(m_currentEvent);
	ImGuiWindowFlags flags = ImGuiWindowFlags_None;
	const auto monitors = app.getMonitorsInfo();
	if (monitors.size() < 2 && m_fullscreen) {
		m_fullscreen = false;
		m_lastFullscreen = true;
	}
	const auto& desiredMonitor = monitors[m_monitorId];
	math::vec2 monitorPos = {static_cast<float>(desiredMonitor.workAreaPosition.x()),
							 static_cast<float>(desiredMonitor.workAreaPosition.y())};
	math::vec2 monitorSize = {static_cast<float>(desiredMonitor.workAreaSize.x() - 1),
							  static_cast<float>(desiredMonitor.workAreaSize.y() - 1)};
	if (m_fullscreen) {
		// Full screen window on desired monitor
		ImGui::SetNextWindowPos({monitorPos.x(), monitorPos.y()}, ImGuiCond_Always);
		ImGui::SetNextWindowSize({monitorSize.x(), monitorSize.y()}, ImGuiCond_Always);
		flags |= ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings;
	} else {
		if (m_lastFullscreen) {
			ImGui::SetNextWindowPos({monitorPos.x() + 50, monitorPos.y() + 50}, ImGuiCond_Always);
			ImGui::SetNextWindowSize({monitorSize.x() - 200, monitorSize.y() - 200}, ImGuiCond_Always);
		}
		// Windowed mode with free size and position
		flags |= ImGuiWindowFlags_None;
	}
	m_lastFullscreen = m_fullscreen;
	if (ImGui::Begin("DisplayView", nullptr, flags)) {
		// Render based on event status
		if (m_previewMode) {
			const auto currentRound = m_currentEvent.getCurrentGameRound();
			if (currentRound == m_currentEvent.endRounds()) {
				ImGui::TextDisabled("Invalid round to preview");
				return;
			}
			if (currentRound->getType() == core::GameRound::Type::Pause)
				renderEventPause();
			else
				renderRoundReady();
		} else {
			switch (m_currentEvent.getStatus()) {
				case core::Event::Status::Ready:
				case core::Event::Status::EventStarting:
					renderEventStart();
					break;
				case core::Event::Status::GameRunning:
					if (const auto currentRound = m_currentEvent.getCurrentGameRound();
						currentRound->getStatus() == core::GameRound::Status::Running) {
						if (currentRound->getType() == core::GameRound::Type::Pause) {
							renderEventPause();
						} else {
							if (currentRound->getCurrentSubRound()->getStatus() ==
								core::SubGameRound::Status::PreScreen) {
								renderRoundReady();
							} else {
								renderRoundRunning();
							}
						}
					} else {
						renderRoundEnd();
					}
					break;
				case core::Event::Status::DisplayRules:
					renderEventRules();
					break;
				case core::Event::Status::EventEnding:
					renderEventEnd();
					break;
				case core::Event::Status::Invalid:
				case core::Event::Status::MissingParties:
				case core::Event::Status::Finished:
					ImGui::TextDisabled("Invalid event state");
					break;
			}
		}
	}
	ImGui::End();
	style = style_backup;
}

void DisplayView::renderEventStart() const {
	const float contentWidth = ImGui::GetContentRegionAvail().x;
	const float contentHeight = ImGui::GetContentRegionAvail().y;
	const auto gui_settings = core::getSettings()->extract("gui");

	// Top row: Organizer logo (left) and organizer name (right)
	if (ImGui::BeginChild("TopRow", {0, contentHeight * 0.15f}, ImGuiChildFlags_None)) {
		// Organizer logo on the left
		if (ImGui::BeginChild("OrganizerLogo", {contentWidth * 0.2f, 0}, ImGuiChildFlags_None)) {
			drawImage("logo_organizer", {0, 0}, {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y});
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Spacer
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + contentWidth * 0.3f);
		ImGui::SameLine();

		// Organizer name on the right
		if (ImGui::BeginChild("OrganizerName", {0, 0}, ImGuiChildFlags_None)) {
			const char* organizerName = m_currentEvent.getOrganizerName().c_str();
			const ImVec2 nameSize = ImGui::CalcTextSize(organizerName);
			ImGui::SetCursorPos({ImGui::GetContentRegionAvail().x - nameSize.x,
								 (ImGui::GetContentRegionAvail().y - nameSize.y) * 0.5f});
			ImGui::Text("%s", organizerName);
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	// Event title
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + contentHeight * 0.05f);
	if (const float scale = gui_settings.getValue("title_scale", 4.0f); scale > 0.f)
		ImGui::SetWindowFontScale(scale);
	const ImVec2 titleSize = ImGui::CalcTextSize(m_currentEvent.getName().c_str());
	ImGui::SetCursorPosX((contentWidth - titleSize.x) * 0.5f);
	ImGui::Text("%s", m_currentEvent.getName().c_str());
	ImGui::SetWindowFontScale(1.0);

	// Event logo (centered, large area)
	const float logoAreaHeight = contentHeight * 0.5f;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + contentHeight * 0.05f);
	if (ImGui::BeginChild("EventLogo", {0, logoAreaHeight}, ImGuiChildFlags_None)) {
		drawImage("logo_event", {ImGui::GetContentRegionAvail().x * 0.1f, 0},
				  {ImGui::GetContentRegionAvail().x * 0.8f, ImGui::GetContentRegionAvail().y});
	}
	ImGui::EndChild();

	// Bottom row: Location (left) and date (right)
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + contentHeight * 0.05f);
	if (ImGui::BeginChild("BottomRow", {0, 0}, ImGuiChildFlags_None)) {
		// Location on the left
		if (ImGui::BeginChild("EventLocation", {contentWidth * 0.4f, 0}, ImGuiChildFlags_None)) {
			const char* location = m_currentEvent.getLocation().c_str();
			ImGui::SetCursorPosY((ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight()) * 0.5f);
			ImGui::Text("%s", location);
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::SetCursorPosX(contentWidth * 0.6f);
		ImGui::SameLine();

		// Date on the right
		if (ImGui::BeginChild("EventDate", {0, 0}, ImGuiChildFlags_None)) {
			const auto dateStr = core::formatCalendar(m_currentEvent.getStarting());
			const ImVec2 dateSize = ImGui::CalcTextSize(dateStr.c_str());
			ImGui::SetCursorPos({ImGui::GetContentRegionAvail().x - dateSize.x,
								 (ImGui::GetContentRegionAvail().y - dateSize.y) * 0.5f});
			ImGui::Text("%s", dateStr.c_str());
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void DisplayView::renderEventRules() const {
	renderTitle("Règlement", utils::imVec2ToVec2(ImGui::GetContentRegionAvail()));
	drawImage("logo_organizer", {0, 0},
			  {ImGui::GetContentRegionAvail().x * 0.1f, ImGui::GetContentRegionAvail().x * 0.1f});
	drawImage("logo_organizer", {ImGui::GetContentRegionAvail().x * 0.9f, 0},
			  {ImGui::GetContentRegionAvail().x * 0.1f, ImGui::GetContentRegionAvail().x * 0.1f});

	constexpr float paddingX = 0.1f;
	ImGui::SetCursorPos({ImGui::GetContentRegionAvail().x * paddingX,
						 ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.1f});

	if (ImGui::BeginChild("RulesContent", {ImGui::GetContentRegionAvail().x * (1 - 2 * paddingX), 0},
						  ImGuiChildFlags_None, ImGuiWindowFlags_NoInputs)) {
		ImGui::TextWrapped("%s", m_currentEvent.getRules().c_str());
	}
	ImGui::EndChild();
}

void DisplayView::renderRoundReady() const {
	const auto gui_settings = core::getSettings()->extract("gui");
	const auto& style = ImGui::GetStyle();
	math::vec2 region = utils::imVec2ToVec2(ImGui::GetContentRegionAvail());
	auto currentRound = m_currentEvent.getCurrentGameRound();
	if (m_previewMode)
		currentRound = m_currentEvent.getGameRound(static_cast<uint32_t>(m_previewRound));
	if (currentRound == m_currentEvent.endRounds()) {
		return;
	}
	auto currentSubRound = currentRound->getCurrentSubRound();
	if (m_previewMode)
		currentSubRound = currentRound->getSubRound(static_cast<uint32_t>(m_previewSubRound));

	// Part title
	const std::string title = std::format("{} - {}", currentRound->getName(), currentSubRound->getTypeStr());
	renderTitle(title, region, 1.5f);
	const float titleHeight = ImGui::GetCursorPosY();
	region.y() -= titleHeight;

	// Frame box with round info (centered)
	const float frameHeight = region.y() * 0.65f;
	const float frameWidth = region.x() * 0.90f;

	const ImVec2 vSize = ImGui::CalcTextSize("Valeur");
	const auto value_scale = gui_settings.getValue("value_scale", 3.f);
	const auto price_scale = gui_settings.getValue("prices_scale", 2.5f);
	const std::string valueText = std::format("{:.2f} €", currentSubRound->getValue());
	const ImVec2 valueSize = ImGui::CalcTextSize(valueText.c_str());
	const float valueHeight = (value_scale > 0 ? value_scale * valueSize.y : valueSize.y) + vSize.y +
							  style.ItemSpacing.y * 2 + style.FramePadding.y + style.SeparatorTextPadding.y * 2.0f;

	ImGui::SetCursorPosX((region.x() - frameWidth) * 0.5f);
	if (ImGui::BeginChild("RoundInfoFrame", {frameWidth, frameHeight}, ImGuiChildFlags_Borders)) {
		// SubRound prices
		if (price_scale > 0.0f)
			ImGui::SetWindowFontScale(price_scale);
		ImGui::Text("%s", currentSubRound->getPrices().c_str());
		ImGui::SetWindowFontScale(1.0f);

		// Value area at bottom of frame
		ImGui::SetCursorPosY(frameHeight - valueHeight);
		ImGui::Separator();
		// Value label
		ImGui::SetCursorPosX((frameWidth - vSize.x) * 0.5f);
		ImGui::Text("Valeur");
		// Value display
		ImGui::SetCursorPosX((frameWidth - valueSize.x * value_scale) * 0.5f);
		if (value_scale > 0.0f)
			ImGui::SetWindowFontScale(value_scale);
		ImGui::Text("%s", valueText.c_str());
		ImGui::SetWindowFontScale(1.0f);
	}
	ImGui::EndChild();

	// Logo area (centered, large)
	if (const float logoHeight = region.y() * 0.35f;
		ImGui::BeginChild("LogoArea", {0, logoHeight}, ImGuiChildFlags_None)) {
		const float size = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		drawImage("logo_organizer", {(ImGui::GetContentRegionAvail().x - size) * 0.5f, 0}, {size, size});
	}
	ImGui::EndChild();
}

void DisplayView::renderRoundRunning() const {
	if (m_previewMode)// nothing to render in preview mode
		return;

	const auto currentRound = m_currentEvent.getCurrentGameRound();
	if (currentRound == m_currentEvent.endRounds()) {
		return;
	}
	const auto currentSubRound = currentRound->getCurrentSubRound();

	const auto gui_settings = core::getSettings()->extract("gui");
	const auto& style = ImGui::GetStyle();
	math::vec2 region = utils::imVec2ToVec2(ImGui::GetContentRegionAvail());

	// Part title
	ImGui::SetCursorPosY(region.y() * 0.05f);
	const std::string title = std::format("{} - {}", currentRound->getName(), currentSubRound->getTypeStr());
	if (const auto scale = gui_settings.getValue("title_scale", 4.0f); scale > 0.0f) {
		ImGui::SetWindowFontScale(scale);
	}
	const ImVec2 titleSize = ImGui::CalcTextSize(title.c_str());
	ImGui::SetCursorPosX((region.x() - titleSize.x) * 0.5f);
	ImGui::Text("%s", title.c_str());
	ImGui::SetWindowFontScale(1.0f);

	// Grid area on left, info on right
	const float leftPanelWidth = ImGui::GetContentRegionAvail().x * 0.8f;
	const float contentHeight = ImGui::GetContentRegionAvail().y * 0.8f;

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.05f);
	const auto nextPos = ImGui::GetCursorPosY() + contentHeight;
	// Left panel - Number grid
	if (ImGui::BeginChild("NumberGridPanel", {leftPanelWidth, contentHeight}, ImGuiChildFlags_Borders)) {
		// Render 9x10 grid
		const ImVec2 availWidth = ImGui::GetContentRegionAvail();
		const math::vec2 spacing = gui_settings.getValue("grid_button_spacing", math::vec2{4.0f, 4.0f});
		const ImVec2 buttonSize{(availWidth.x - spacing.x() * 9) / 10.0f, (availWidth.y - spacing.y() * 8) / 9.0f};

		const auto buttonColor = gui_settings.getValue("grid_background_color", math::vec4{0.1f, 0.1f, 0.1f, 1.0f});

		const auto fading = gui_settings.getValue("fade_numbers", true);
		const auto fadingCount = gui_settings.getValue("fade_amount", 3);
		const auto fadingStrength = gui_settings.getValue("fade_strength", 0.5f);
		const auto buttonColorActiveLast =
				gui_settings.getValue("selected_number_color", math::vec4{1.f, 0.44f, 0.f, 1.0f});
		const auto buttonColorActivePrev = buttonColorActiveLast * (1.f - fadingStrength);
		const auto buttonColorActive = buttonColorActivePrev * (1.f - fadingStrength);
		const auto gridTextScale = gui_settings.getValue("grid_text_scale", 0.9f);
		const auto drawnNumbers = currentRound->getAllDraws();

		for (uint8_t row = 0; row < 9; ++row) {
			for (uint8_t col = 0; col < 10; ++col) {
				const uint8_t number = row * 10 + col + 1;
				ImGui::SetCursorPos({static_cast<float>(col) * (buttonSize.x + spacing.x()) + style.WindowPadding.x,
									 static_cast<float>(row) * (buttonSize.y + spacing.y()) + style.WindowPadding.y});
				if (auto it = std::ranges::find(std::ranges::reverse_view(drawnNumbers), number);
					it != drawnNumbers.rend()) {
					if (fading) {
						// Determine how recent the number was drawn
						if (const auto index = std::distance(drawnNumbers.rbegin(), it); index == 0) {// Most recent
							ImGui::PushStyleColor(ImGuiCol_Button, utils::vec4ToImVec4(buttonColorActiveLast));
						} else if (std::cmp_less(index, fadingCount + 1)) {// Within fading range
							ImGui::PushStyleColor(ImGuiCol_Button, utils::vec4ToImVec4(buttonColorActivePrev));
						} else {
							ImGui::PushStyleColor(ImGuiCol_Button, utils::vec4ToImVec4(buttonColorActive));
						}
					} else {
						ImGui::PushStyleColor(ImGuiCol_Button, utils::vec4ToImVec4(buttonColorActive));
					}
				} else {
					ImGui::PushStyleColor(ImGuiCol_Button, utils::vec4ToImVec4(buttonColor));
				}
				const std::string label = std::format("{}", number);
				const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
				const float scaleX = buttonSize.x / textSize.x;
				const float scaleY = buttonSize.y / textSize.y;
				if (const float scale = std::min(scaleX, scaleY) * gridTextScale; scale > 0.f)
					ImGui::SetWindowFontScale(scale);
				ImGui::Button(label.c_str(), buttonSize);
				ImGui::SetWindowFontScale(1.0f);
				ImGui::PopStyleColor();
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Right panel - Info and display
	if (ImGui::BeginChild("InfoPanel", {0, 0}, ImGuiChildFlags_Borders)) {
		auto fullWidth = ImGui::GetContentRegionAvail().x;
		// Current draw
		ImGui::Text("Numéro tiré");
		ImGui::Separator();

		ImGui::BeginGroup();
		const auto drawnNumbers = currentRound->getAllDraws();
		const std::string lastNumberText = drawnNumbers.empty() ? "--" : std::format("{}", drawnNumbers.back());
		utils::adaptTextToRegion(lastNumberText, {.autoRegion = false,
												  .contentSize = {fullWidth, ImGui::GetContentRegionAvail().y},
												  .vCenter = false,
												  .hCenter = true,
												  .drawText = true,
												  .textAdapt = "00"});
		ImGui::EndGroup();

		// Logo
		ImGui::BeginGroup();
		const float size = std::min(fullWidth, ImGui::GetContentRegionAvail().y - ImGui::GetCursorPosY());
		drawImage(
				"logo_organizer",
				{(fullWidth - size) * 0.5f, (ImGui::GetContentRegionAvail().y + ImGui::GetCursorPosY() - size) * 0.5f},
				{size, size});
		ImGui::EndGroup();

		// Timing info
		const auto textHeigh = ImGui::CalcTextSize("D").y;
		const float timeScale = gui_settings.getValue("time_scale", 1.6f);
		auto now = core::clock::now();
		auto elapsed = now - currentSubRound->getStarting();
		const std::string nowStr = core::formatClockNoSecond(now);
		auto nowSize = ImGui::CalcTextSize(nowStr.c_str()).x * (timeScale > 0.0f ? timeScale : 1.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
							 (ImGui::GetContentRegionAvail().y - textHeigh * (1 + timeScale) - style.WindowPadding.y));
		if (ImGui::BeginChild("##TimingInfo", {0, 0}, ImGuiChildFlags_None)) {
			ImGui::BeginGroup();
			ImGui::Text("Durée partie");
			ImGui::Separator();
			if (timeScale > 0.0f)
				ImGui::SetWindowFontScale(timeScale);
			ImGui::Text("%s", core::formatDuration(elapsed).c_str());
			ImGui::SetWindowFontScale(1.0f);
			ImGui::EndGroup();

			ImGui::SameLine();
			ImGui::SetCursorPosX(fullWidth - nowSize);

			ImGui::BeginGroup();
			ImGui::Text("Heure");
			ImGui::Separator();
			if (timeScale > 0.0f)
				ImGui::SetWindowFontScale(timeScale);
			ImGui::Text("%s", core::formatClockNoSecond(now).c_str());
			ImGui::SetWindowFontScale(1.0f);
			ImGui::EndGroup();

			ImGui::EndChild();
		}
	}
	ImGui::EndChild();

	// Subround info at bottom
	ImGui::SetCursorPosY(nextPos);
	if (ImGui::BeginChild("SubRoundInfo", {leftPanelWidth, 0}, ImGuiChildFlags_None)) {
		const auto currentWidth = ImGui::GetContentRegionAvail().x;
		const auto truncate_price = gui_settings.getValue("truncate_price", false);
		const auto truncate_price_lines = gui_settings.getValue("truncate_price_lines", 3);
		const auto value_scale = gui_settings.getValue("value_scale", 3.0f) * 0.8f;
		auto price_scale = gui_settings.getValue("prices_scale", 2.5f) * 0.8f;
		const std::string priceText = std::format("{:.2f} €", currentSubRound->getValue());
		const auto valueSize =
				std::max(ImGui::CalcTextSize(priceText.c_str()).x, ImGui::CalcTextSize("Valeur").x) * value_scale;
		std::string all_prices = currentSubRound->getPrices();
		if (auto nbLine = std::ranges::count(all_prices, '\n'); truncate_price && nbLine > truncate_price_lines) {
			// Truncate to specified number of lines
			size_t pos = 0;
			int line_count = 0;
			while (line_count < truncate_price_lines && pos != std::string::npos) {
				pos = all_prices.find('\n', pos);
				if (pos != std::string::npos) {
					++line_count;
					++pos;
				}
			}
			all_prices = all_prices.substr(0, pos) + "\n...";
		}
		auto tSize = ImGui::CalcTextSize(all_prices.c_str());
		tSize.x *= (price_scale > 0.0f ? price_scale : 1.0f);
		tSize.y *= (price_scale > 0.0f ? price_scale : 1.0f);
		if (tSize.y > ImGui::GetContentRegionAvail().y) {
			// Adjust size to fit
			price_scale *= ImGui::GetContentRegionAvail().y / tSize.y;
		}
		ImGui::BeginGroup();
		if (price_scale > 0.0f)
			ImGui::SetWindowFontScale(price_scale);
		ImGui::Text("%s", all_prices.c_str());
		ImGui::SetWindowFontScale(1.0f);
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::SetCursorPosX(currentWidth - valueSize - style.WindowPadding.x);
		ImGui::BeginGroup();
		if (value_scale > 0.0f)
			ImGui::SetWindowFontScale(value_scale * 0.5f);
		ImGui::Text("Valeur");
		if (value_scale > 0.0f)
			ImGui::SetWindowFontScale(value_scale);
		ImGui::Text("%s", priceText.c_str());
		ImGui::SetWindowFontScale(1.0f);
		ImGui::EndGroup();
	}
	ImGui::EndChild();
}

void DisplayView::renderRoundEnd() const {
	if (m_previewMode)// nothing to render in preview mode
		return;
	renderTitle("Fin de la partie", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y}, 2.0f);
	drawImage("logo_organizer", {0, 0},
			  {ImGui::GetContentRegionAvail().x * 0.1f, ImGui::GetContentRegionAvail().x * 0.1f});
	drawImage("logo_organizer", {ImGui::GetContentRegionAvail().x * 0.9f, 0},
			  {ImGui::GetContentRegionAvail().x * 0.1f, ImGui::GetContentRegionAvail().x * 0.1f});


	ImGui::SetWindowFontScale(1.5f);
	const ImVec2 msgSize = ImGui::CalcTextSize("Veuillez démarquer vos cartons.");
	ImGui::SetCursorPos({(ImGui::GetContentRegionAvail().x - msgSize.x) * 0.5f,
						 ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.15f});
	ImGui::Text("Veuillez démarquer vos cartons.");
	ImGui::SetWindowFontScale(1.0f);

	ImGui::SetCursorPos({0, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.3f});
	if (ImGui::BeginChild("EndLogo", {0, 0}, ImGuiChildFlags_None)) {
		const float size = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		drawImage("logo_organizer",
				  {(ImGui::GetContentRegionAvail().x - size) * 0.5f, (ImGui::GetContentRegionAvail().y - size) * 0.5f},
				  {size, size});
	}
	ImGui::EndChild();
}

void DisplayView::renderEventPause() const {
	auto round = m_currentEvent.getCurrentGameRound();
	if (m_previewMode) {
		round = m_currentEvent.getGameRound(static_cast<uint32_t>(m_previewRound));
	}
	renderTitle("Pause", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.3f / 0.05f}, 2.0f);
	drawImage("logo_organizer", {0, 0},
			  {ImGui::GetContentRegionAvail().x * 0.1f, ImGui::GetContentRegionAvail().x * 0.1f});
	drawImage("logo_organizer", {ImGui::GetContentRegionAvail().x * 0.9f, 0},
			  {ImGui::GetContentRegionAvail().x * 0.1f, ImGui::GetContentRegionAvail().x * 0.1f});

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.1f);
	if (round->hasDiapo()) {
		const ImVec2 msgSize = ImGui::CalcTextSize("Diaporama en cours");
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - msgSize.x) * 0.5f);
		ImGui::Text("Diaporama en cours");
	} else {
		const ImVec2 msgSize = ImGui::CalcTextSize("Une buvette est à votre disposition");
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - msgSize.x) * 0.5f);
		ImGui::Text("Une buvette est à votre disposition");
	}
}

void DisplayView::renderEventEnd() const {
	if (m_previewMode)// nothing to render in preview mode
		return;
	renderTitle("Fin", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.3f / 0.05f}, 2.0f);

	ImGui::SameLine();
	drawImage("logo_event", {ImGui::GetContentRegionAvail().x * 0.9f, ImGui::GetCursorPosY()}, {100.0f, 100.0f});

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.2f);
	const ImVec2 msgSize = ImGui::CalcTextSize("Merci pour votre participation");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - msgSize.x) * 0.5f);
	ImGui::Text("Merci pour votre participation");
}

void DisplayView::applyCommonStyle() const {
	if (!m_customStyle) {
		return;
	}
	auto& style = ImGui::GetStyle();
	const auto gui_settings = core::getSettings()->extract("gui");
	const auto back = gui_settings.getValue("background_color", utils::imVec4ToVec4(style.Colors[ImGuiCol_WindowBg]));
	style.Colors[ImGuiCol_WindowBg] = utils::vec4ToImVec4(back);
	style.Colors[ImGuiCol_DockingEmptyBg] = utils::vec4ToImVec4(back);
	style.Colors[ImGuiCol_FrameBg] = utils::vec4ToImVec4(back);
	style.Colors[ImGuiCol_ChildBg] = utils::vec4ToImVec4(back);
	style.Colors[ImGuiCol_PopupBg] = utils::vec4ToImVec4(back);
	style.Colors[ImGuiCol_Text] =
			utils::vec4ToImVec4(gui_settings.getValue("text_color", utils::imVec4ToVec4(style.Colors[ImGuiCol_Text])));
}

}// namespace evl::gui_imgui::views
