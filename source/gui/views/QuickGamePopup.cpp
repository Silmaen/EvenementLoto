/**
 * @file QuickGamePopup.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "QuickGamePopup.h"

#include "core/Log.h"
#include "gui/Application.h"
#include "gui/utils/Rendering.h"

#include <imgui.h>

namespace evl::gui::views {

namespace {

constexpr float g_buttonWidth = 100.0f;

/// The round types, as one ImGui combo string, `Invalid` left out.
auto roundTypes() -> std::string {
	std::string result;
	for (const auto& type: magic_enum::enum_values<core::GameRound::Type>()) {
		if (type == core::GameRound::Type::Invalid)
			continue;
		const core::GameRound round(type);
		result += round.getTypeStr();
		result += '\0';
	}
	result += '\0';// Double null terminator for ImGui
	return result;
}

/// Where the round can go, named by the round it would come before.
auto positionLabel(const core::Event& iEvent, const uint32_t iIndex) -> std::string {
	if (iIndex >= iEvent.sizeRounds())
		return "à la fin du programme";
	const auto round = std::next(iEvent.beginRounds(), iIndex);
	return std::format("avant « {} »", round->getName());
}

}// namespace

PopupQuickGame::PopupQuickGame() = default;
PopupQuickGame::~PopupQuickGame() = default;

void PopupQuickGame::onOpen() {
	const auto& event = Application::get().getCurrentEvent();
	m_round = core::GameRound{core::GameRound::Type::OneQuine};
	m_round.setId(event.nextFreeRoundId());
	// Right after the round being played is where an improvised round usually belongs.
	m_position = event.firstInsertableIndex();
	m_selectedSubRound = 0;
}

auto PopupQuickGame::insertRound() const -> bool {
	auto& event = Application::get().getCurrentEvent();
	const auto placed = event.insertGameRound(m_position, m_round);
	if (!placed.has_value()) {
		Application::get().tell("Partie improvisée", "La partie n'a pas pu être ajoutée.", "L'événement est terminé.");
		return false;
	}
	log_info("Partie improvisée « {} » insérée en position {}.", m_round.getName(), placed.value());
	Application::get().saveProgress();
	return true;
}

void PopupQuickGame::onPopupUpdate() {
	const auto& event = Application::get().getCurrentEvent();
	if (event.isFinished()) {
		ImGui::TextDisabled("L'événement est terminé, plus rien ne peut y être ajouté.");
		if (ImGui::Button("Fermer", {g_buttonWidth, 0}))
			ImGui::CloseCurrentPopup();
		return;
	}

	ImGui::TextWrapped("Une partie ajoutée au programme en cours de route. Les lots sont facultatifs : "
					   "une partie sans lot se joue aussi bien, et le rapport de fin la compte comme les autres.");
	ImGui::Separator();

	// Type and number.
	ImGui::Text("Type de partie");
	ImGui::SetNextItemWidth(-140);
	int type = static_cast<int>(m_round.getType()) - 1;// Invalid is not offered
	if (ImGui::Combo("##QuickType", &type, roundTypes().c_str()))
		m_round.setType(static_cast<core::GameRound::Type>(type + 1));
	ImGui::SameLine();
	ImGui::Text("N°");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	int id = m_round.getId();
	if (ImGui::InputInt("##QuickNumber", &id, 1, 10, ImGuiInputTextFlags_CharsDecimal))
		m_round.setId(std::max(0, id));

	// Position.
	const auto first = event.firstInsertableIndex();
	const auto last = static_cast<uint32_t>(event.sizeRounds());
	m_position = std::clamp(m_position, first, last);
	ImGui::Spacing();
	ImGui::Text("Position : %s", positionLabel(event, m_position).c_str());
	if (first < last) {
		ImGui::SetNextItemWidth(-1);
		int position = static_cast<int>(m_position);
		if (ImGui::SliderInt("##QuickPosition", &position, static_cast<int>(first), static_cast<int>(last), ""))
			m_position = static_cast<uint32_t>(position);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Une partie improvisée ne peut pas passer devant la partie en cours "
							  "ni devant une partie déjà jouée.");
	}

	// Prizes, per sub-round, and optional.
	ImGui::Spacing();
	ImGui::Separator();
	if (m_round.getType() == core::GameRound::Type::Pause) {
		ImGui::TextDisabled("Une pause n'a pas de lot.");
	} else {
		if (m_selectedSubRound >= m_round.sizeSubRound())
			m_selectedSubRound = 0;
		ImGui::Text("Lots (facultatifs)");
		if (m_round.sizeSubRound() > 1) {
			for (size_t i = 0; i < m_round.sizeSubRound(); ++i) {
				const auto sub = m_round.getSubRound(static_cast<uint32_t>(i));
				ImGui::SameLine();
				if (const bool selected = i == m_selectedSubRound;
					ImGui::RadioButton(sub->getTypeStr().c_str(), selected))
					m_selectedSubRound = i;
			}
		}
		const auto subRound = m_round.getSubRound(static_cast<uint32_t>(m_selectedSubRound));
		ImGui::Text("Valeur de la phase : %.2f €", subRound->getValue());
		auto prizes = subRound->getPrizes();
		if (utils::renderPrizeList(prizes, true, {0, -50}))
			subRound->setPrizes(prizes);
	}

	ImGui::Separator();
	if (ImGui::Button("Ajouter", {g_buttonWidth, 0})) {
		if (insertRound())
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Annuler", {g_buttonWidth, 0}))
		ImGui::CloseCurrentPopup();
}

}// namespace evl::gui::views
