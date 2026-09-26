/**
 * @file WinnerPopups.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "WinnerPopups.h"

#include "core/Log.h"
#include "gui/Application.h"

#include <imgui.h>
#include <imgui_stdlib.h>

namespace evl::gui::views {

namespace {

constexpr float g_buttonWidth = 100.0f;

/**
 * @brief Describe the running sub-round, so the organizer sees what is being awarded.
 * @param iEvent The event being played.
 * @return The round name and the sub-round type, or an empty pair outside a game.
 */
auto runningLabels(const core::Event& iEvent) -> std::pair<std::string, std::string> {
	if (iEvent.getStatus() != core::Event::Status::GameRunning)
		return {};
	const auto round = iEvent.getCurrentCGameRound();
	if (round == iEvent.endRounds() || round->getType() == core::GameRound::Type::Pause)
		return {};
	return {round->getName(), round->getCurrentSubRound()->getTypeStr()};
}

}// namespace

PopupWinner::PopupWinner() = default;
PopupWinner::~PopupWinner() = default;

void PopupWinner::onOpen() {
	m_claimants.assign(1, std::string{});
	m_selected = 0;
}

void PopupWinner::settle(const std::string& iWinner) {
	auto& app = Application::get();
	app.getCurrentEvent().addWinnerToCurrentRound(iWinner);
	// A phase that just ended starts the next one with its own numbers.
	if (const auto round = app.getCurrentEvent().getCurrentCGameRound();
		round != app.getCurrentEvent().endRounds() && round->getType() != core::GameRound::Type::Pause &&
		round->drawsCount() == 0) {
		app.getRng().resetPick();
	}
	app.clearDrawDelay();
	app.saveProgress();
}

void PopupWinner::onPopupUpdate() {
	const auto& event = Application::get().getCurrentEvent();
	const auto [roundName, subRoundType] = runningLabels(event);
	if (roundName.empty()) {
		ImGui::TextDisabled("Aucune manche en cours.");
		if (ImGui::Button("Fermer", {g_buttonWidth, 0}))
			ImGui::CloseCurrentPopup();
		return;
	}

	ImGui::Text("%s — %s", roundName.c_str(), subRoundType.c_str());
	const auto subRound = event.getCurrentCGameRound()->getCurrentSubRound();
	if (const auto& prices = subRound->getPrices(); !prices.empty())
		ImGui::TextWrapped("Lots : %s (%.2f €)", prices.c_str(), subRound->getValue());
	ImGui::Separator();

	ImGui::Text("Prétendants");
	if (ImGui::BeginChild("Claimants", {0, ImGui::GetContentRegionAvail().y - 90}, ImGuiChildFlags_Borders)) {
		for (size_t i = 0; i < m_claimants.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			if (m_claimants.size() > 1) {
				// Several claimants means a tie: which one carries the prize is a choice.
				if (const bool chosen = i == m_selected; ImGui::RadioButton("##chosen", chosen))
					m_selected = i;
				ImGui::SameLine();
			}
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - g_buttonWidth);
			ImGui::InputText("##claimant", &m_claimants[i]);
			ImGui::SameLine();
			if (ImGui::Button("Retirer") && m_claimants.size() > 1) {
				m_claimants.erase(m_claimants.begin() + static_cast<ptrdiff_t>(i));
				if (m_selected >= m_claimants.size())
					m_selected = m_claimants.size() - 1;
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	if (ImGui::Button("Ajouter un prétendant"))
		m_claimants.emplace_back();
	if (m_claimants.size() > 1) {
		ImGui::SameLine();
		if (ImGui::Button("Tirer au sort")) {
			std::uniform_int_distribution<size_t> dist{0, m_claimants.size() - 1};
			m_selected = dist(m_rng);
			log_info("Départage au sort : prétendant {} retenu.", m_selected + 1);
		}
		ImGui::SameLine();
		ImGui::TextDisabled("retenu : %s", m_claimants[m_selected].c_str());
	}

	ImGui::Separator();
	const bool hasWinner = !m_claimants[m_selected].empty();
	if (!hasWinner)
		ImGui::BeginDisabled();
	if (ImGui::Button("Valider", {g_buttonWidth, 0})) {
		settle(m_claimants[m_selected]);
		ImGui::CloseCurrentPopup();
	}
	if (!hasWinner)
		ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Passer", {g_buttonWidth, 0})) {
		// Nobody claimed it: the round moves on all the same.
		log_info("Manche validée sans gagnant.");
		settle({});
		ImGui::CloseCurrentPopup();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Passer l'étape sans enregistrer de gagnant.");
	ImGui::SameLine();
	if (ImGui::Button("Annuler", {g_buttonWidth, 0}))
		ImGui::CloseCurrentPopup();
}

//----------------------------------------------------------------------------------------------------------------------

PopupWinners::PopupWinners() = default;
PopupWinners::~PopupWinners() = default;

void PopupWinners::onPopupUpdate() {
	auto& event = Application::get().getCurrentEvent();
	bool edited = false;

	if (ImGui::BeginChild("WinnersList", {0, ImGui::GetContentRegionAvail().y - 40}, ImGuiChildFlags_Borders)) {
		bool anyPlayed = false;
		for (uint32_t roundIndex = 0; roundIndex < static_cast<uint32_t>(event.sizeRounds()); ++roundIndex) {
			const auto round = event.getGameRound(roundIndex);
			if (round->getType() == core::GameRound::Type::Pause)
				continue;
			ImGui::PushID(static_cast<int>(roundIndex));
			ImGui::SeparatorText(round->getName().c_str());
			for (uint32_t subIndex = 0; subIndex < static_cast<uint32_t>(round->sizeSubRound()); ++subIndex) {
				const auto subRound = round->getSubRound(subIndex);
				ImGui::PushID(static_cast<int>(subIndex));
				ImGui::Text("%s", subRound->getTypeStr().c_str());
				ImGui::SameLine(200);
				ImGui::SetNextItemWidth(-1);
				if (subRound->isFinished()) {
					anyPlayed = true;
					std::string winner = subRound->getWinner();
					if (ImGui::InputText("##winner", &winner)) {
						subRound->editWinner(winner);
						edited = true;
					}
				} else {
					ImGui::TextDisabled("manche non jouée");
				}
				ImGui::PopID();
			}
			ImGui::PopID();
		}
		if (!anyPlayed)
			ImGui::TextDisabled("Aucune manche terminée pour l'instant.");
	}
	ImGui::EndChild();

	if (edited) {
		// A correction is a change to the game like any other: it goes to disk at once.
		Application::get().saveProgress();
	}

	ImGui::Separator();
	if (ImGui::Button("Fermer", {g_buttonWidth, 0}))
		ImGui::CloseCurrentPopup();
}

}// namespace evl::gui::views
