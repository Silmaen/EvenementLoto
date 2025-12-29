/**
 * @file DisplayView.cpp
 * @author Silmaen
 * @date 22/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "DisplayView.h"
#include "gui_imgui/Application.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

DisplayView::DisplayView(core::Event& iEvent) : m_currentEvent{iEvent} {}

DisplayView::~DisplayView() = default;

void DisplayView::onUpdate() {
	auto& app = Application::get();
	m_currentEvent = app.getCurrentEvent();
	ImGuiWindowFlags flags = ImGuiWindowFlags_None;
	const auto monitors = app.getMonitorsInfo();
	if (monitors.size() < 2) {
		m_fullscreen = false;
	}
	const auto& desiredMonitor = monitors[m_monitorId];
	if (m_fullscreen) {
		// Full screen window on desired monitor
		ImGui::SetNextWindowPos({static_cast<float>(desiredMonitor.workAreaPosition.x()),
								 static_cast<float>(desiredMonitor.workAreaPosition.y())},
								ImGuiCond_Always);
		ImGui::SetNextWindowSize({static_cast<float>(desiredMonitor.workAreaSize.x() - 1),
								  static_cast<float>(desiredMonitor.workAreaSize.y() - 1)},
								 ImGuiCond_Always);
		flags |= ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings;
	} else {
		if (m_lastFullscreen) {
			ImGui::SetNextWindowPos({static_cast<float>(desiredMonitor.workAreaPosition.x() + 50),
									 static_cast<float>(desiredMonitor.workAreaPosition.y() + 50)},
									ImGuiCond_Always);
			ImGui::SetNextWindowSize({static_cast<float>(desiredMonitor.workAreaSize.x() - 100),
									  static_cast<float>(desiredMonitor.workAreaSize.y() - 100)},
									 ImGuiCond_Always);
		}
		// Windowed mode with free size and position
		flags |= ImGuiWindowFlags_None;
	}
	m_lastFullscreen = m_fullscreen;
	if (ImGui::Begin("DisplayView", nullptr, flags)) {
		// Render based on event status
		switch (m_currentEvent.getStatus()) {
			case core::Event::Status::Ready:
			case core::Event::Status::EventStarting:
				renderRoundReady();
				break;
			case core::Event::Status::GameRunning:
				if (const auto currentRound = m_currentEvent.getCurrentGameRound();
					currentRound != m_currentEvent.endRounds() &&
					currentRound->getStatus() == core::GameRound::Status::Running) {
					renderRoundRunning();
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
	ImGui::End();
}

void DisplayView::renderRoundReady() const {
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
	ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.1f);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	const ImVec2 titleSize = ImGui::CalcTextSize("Partie 1");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - titleSize.x) * 0.5f);
	ImGui::Text("%s", currentRound->getName().c_str());
	ImGui::PopFont();

	// Frame box with round info
	const float frameHeight = ImGui::GetContentRegionAvail().y * 0.4f;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.1f);
	if (ImGui::BeginChild("RoundInfoFrame", {0, frameHeight}, ImGuiChildFlags_Border)) {
		ImGui::Text("%s", currentSubRound->getPrices().c_str());
		ImGui::Separator();
		ImGui::Text("Valeur");
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
		ImGui::Text("%.2f €", currentSubRound->getValue());
		ImGui::PopFont();
	}
	ImGui::EndChild();

	// Logo area
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.1f);
	if (ImGui::BeginChild("LogoArea", {0, 0}, ImGuiChildFlags_None)) {
		ImGui::Text("<logo>");
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

	// Title section
	ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.05f);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	const ImVec2 titleSize = ImGui::CalcTextSize(currentRound->getName().c_str());
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - titleSize.x) * 0.5f);
	ImGui::Text("%s", currentRound->getName().c_str());
	ImGui::PopFont();

	// Grid area on left, info on right
	const float leftPanelWidth = ImGui::GetContentRegionAvail().x * 0.75f;
	const float contentHeight = ImGui::GetContentRegionAvail().y * 0.8f;

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.05f);

	// Left panel - Number grid
	if (ImGui::BeginChild("NumberGridPanel", {leftPanelWidth, contentHeight}, ImGuiChildFlags_Border)) {
		// Render 9x10 grid
		const ImVec2 availWidth = ImGui::GetContentRegionAvail();
		const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
		const ImVec2 buttonSize{(availWidth.x - spacing.x * 9) / 10.0f, (availWidth.y - spacing.y * 8) / 9.0f};

		const auto drawnNumbers = currentRound->getAllDraws();
		const std::unordered_set drawnSet(drawnNumbers.begin(), drawnNumbers.end());

		for (uint8_t row = 0; row < 9; ++row) {
			for (uint8_t col = 0; col < 10; ++col) {
				const uint8_t number = row * 10 + col + 1;
				const bool isDrawn = drawnSet.contains(number);

				if (isDrawn) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				}

				const std::string label = std::format("{}", number);
				ImGui::Button(label.c_str(), buttonSize);

				if (isDrawn) {
					ImGui::PopStyleColor();
				}

				if (col < 9) {
					ImGui::SameLine();
				}
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// Right panel - Info and display
	if (ImGui::BeginChild("InfoPanel", {0, contentHeight}, ImGuiChildFlags_Border)) {
		// Current draw
		ImGui::Text("Numéro tiré");
		ImGui::Separator();
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
		if (const auto drawnNumbers = currentRound->getAllDraws(); !drawnNumbers.empty()) {
			ImGui::Text("%d", drawnNumbers.back());
		} else {
			ImGui::Text("--");
		}
		ImGui::PopFont();

		ImGui::Spacing();
		ImGui::Spacing();

		// Logo
		ImGui::Text("<logo>");

		ImGui::Spacing();
		ImGui::Spacing();

		// Timing info
		ImGui::Text("Durée partie");
		ImGui::Separator();
		ImGui::Text("00:15");

		ImGui::Spacing();

		ImGui::Text("Heure");
		ImGui::Separator();
		ImGui::Text("%s", core::formatClock(core::clock::now()).c_str());
	}
	ImGui::EndChild();

	// Subround info at bottom
	if (ImGui::BeginChild("SubRoundInfo", {0, 0}, ImGuiChildFlags_None)) {
		ImGui::Text("Un vélo en plastique");
		ImGui::SameLine();
		ImGui::Text("Valeur");
	}
	ImGui::EndChild();
}

void DisplayView::renderRoundEnd() const {
	if (m_previewMode)// nothing to render in preview mode
		return;
	ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.2f);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	const ImVec2 titleSize = ImGui::CalcTextSize("Fin de la partie");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - titleSize.x) * 0.5f);
	ImGui::Text("Fin de la partie");
	ImGui::PopFont();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.15f);
	const ImVec2 msgSize = ImGui::CalcTextSize("Veuillez démarquer vos cartons.");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - msgSize.x) * 0.5f);
	ImGui::Text("Veuillez démarquer vos cartons.");

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.3f);
	if (ImGui::BeginChild("EndLogo", {0, 0}, ImGuiChildFlags_None)) {
		ImGui::Text("<logo>");
	}
	ImGui::EndChild();
}

void DisplayView::renderEventPause() const {
	auto round = m_currentEvent.getCurrentGameRound();
	if (m_previewMode) {
		round = m_currentEvent.getGameRound(static_cast<uint32_t>(m_previewRound));
	}
	ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.3f);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	const ImVec2 titleSize = ImGui::CalcTextSize("Pause");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - titleSize.x) * 0.5f);
	ImGui::Text("Pause");
	ImGui::PopFont();

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
	ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.2f);

	// Logos at top
	ImGui::BeginGroup();
	ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.1f);
	ImGui::Text("<logo>");
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5f - 30);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	const ImVec2 titleSize = ImGui::CalcTextSize("Fin");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - titleSize.x) * 0.5f);
	ImGui::Text("Fin");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.9f);
	ImGui::Text("<logo>");
	ImGui::EndGroup();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.2f);
	const ImVec2 msgSize = ImGui::CalcTextSize("Merci pour votre participation");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - msgSize.x) * 0.5f);
	ImGui::Text("Merci pour votre participation");
}

void DisplayView::renderEventRules() const {
	ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.05f);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
	const ImVec2 titleSize = ImGui::CalcTextSize("Reglement");
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - titleSize.x) * 0.5f);
	ImGui::Text("Reglement");
	ImGui::PopFont();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.05f);
	if (ImGui::BeginChild("RulesContent", {0, 0}, ImGuiChildFlags_None, ImGuiWindowFlags_NoInputs)) {
		ImGui::TextWrapped("%s", m_currentEvent.getRules().c_str());
	}
	ImGui::EndChild();
}

}// namespace evl::gui_imgui::views
