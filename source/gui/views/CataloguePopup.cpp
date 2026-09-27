/**
 * @file CataloguePopup.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "CataloguePopup.h"

#include "core/Log.h"
#include "gui/Application.h"
#include "gui/utils/Rendering.h"

#include <imgui.h>

namespace evl::gui::views {

namespace {

constexpr float g_buttonWidth = 110.0f;
constexpr float g_settingsHeight = 130.0f;

}// namespace

PopupCatalogue::PopupCatalogue() = default;
PopupCatalogue::~PopupCatalogue() = default;

void PopupCatalogue::onOpen() {
	m_catalogue = Application::get().getCurrentEvent().getCatalogue();
	m_lastResult.clear();
}

void PopupCatalogue::onPopupUpdate() {
	auto& event = Application::get().getCurrentEvent();
	if (event.isFinished()) {
		ImGui::TextDisabled("L'événement est terminé : son catalogue ne bouge plus.");
		if (ImGui::Button("Fermer", {g_buttonWidth, 0}))
			ImGui::CloseCurrentPopup();
		return;
	}

	ImGui::TextWrapped("Tous les lots dont vous disposez, saisis une fois pour toutes. La répartition les place "
					   "ensuite sur les manches, du plus discret au plus marquant.");
	ImGui::Text("Valeur totale du catalogue : %.2f €", core::totalValue(m_catalogue));
	ImGui::Separator();

	if (utils::renderPrizeList(m_catalogue, true, {0, -g_settingsHeight})) {
		event.setCatalogue(m_catalogue);
		Application::get().saveProgress();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reprendre les lots des parties")) {
		event.gatherCatalogueFromRounds();
		m_catalogue = event.getCatalogue();
		m_lastResult = std::format("{} article(s) repris des parties.", m_catalogue.size());
		log_info("{}", m_lastResult);
		Application::get().saveProgress();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Remplace le catalogue par les lots actuellement placés dans les parties.\n"
						  "C'est ce qui permet de repartir d'une répartition faite à la main.");

	ImGui::Separator();
	ImGui::Text("Répartition automatique");
	ImGui::SetNextItemWidth(240);
	ImGui::SliderFloat("Part de l'attrait", &m_settings.attractivenessWeight, 0.0f, 1.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Zéro : seule la valeur compte. Un : seul l'attrait compte.\n"
						  "Un jambon fait souvent plus d'effet qu'un objet plus cher.");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(240);
	ImGui::SliderFloat("Montée vers la fin", &m_settings.climaxStrength, 0.0f, 1.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Zéro : toutes les parties se valent.\n"
						  "Un : la dernière partie emporte les plus beaux lots.");
	ImGui::SameLine();
	ImGui::Checkbox("Respecter les parties enfant", &m_settings.respectChildRounds);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Une partie enfant ne reçoit que les articles marqués compatibles.");

	if (!m_lastResult.empty())
		ImGui::TextWrapped("%s", m_lastResult.c_str());

	if (ImGui::Button("Répartir", {g_buttonWidth, 0})) {
		event.setCatalogue(m_catalogue);
		const auto result = core::distributePrizes(event, m_settings);
		m_lastResult = result.summary;
		log_info("Répartition des lots : {}", result.summary);
		Application::get().saveProgress();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Reprend tout le catalogue à zéro : les manches déjà entamées ne sont pas touchées,\n"
						  "et l'ajustement à la main se fait ensuite dans le réglage des parties.");
	ImGui::SameLine();
	if (ImGui::Button("Fermer", {g_buttonWidth, 0}))
		ImGui::CloseCurrentPopup();
}

}// namespace evl::gui::views
