/**
 * @file CataloguePopup.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "CataloguePopup.h"

#include "core/CsvCatalogue.h"
#include "core/Log.h"
#include "gui/Application.h"
#include "gui/utils/FileDialog.h"
#include "gui/utils/Rendering.h"

#include <imgui.h>

#include <numeric>
#include <optional>
#include <vector>

namespace evl::gui::views {

namespace {

constexpr float g_buttonWidth = 110.0f;
constexpr float g_settingsHeight = 130.0f;
/// Hauteur d'un histogramme : de quoi lire une tendance sans manger la page.
constexpr float g_chartHeight = 90.0f;

}// namespace

PopupCatalogue::PopupCatalogue() = default;
PopupCatalogue::~PopupCatalogue() = default;

void PopupCatalogue::onOpen() {
	// Un lot saisi dans le réglage des parties n'est pas au catalogue : il y entre ici,
	// sinon il resterait invisible de la seule fenêtre censée tout montrer.
	if (const auto adopted = Application::get().getCurrentEvent().adoptOrphanPrizes(); adopted > 0)
		m_lastResult = std::format("{} lot(s) des parties entrés au catalogue.", adopted);
	else
		m_lastResult.clear();
	m_catalogue = Application::get().getCurrentEvent().getCatalogue();
}

void PopupCatalogue::refreshCatalogue() { m_catalogue = Application::get().getCurrentEvent().getCatalogue(); }

void PopupCatalogue::commitCatalogue() {
	Application::get().getCurrentEvent().setCatalogue(m_catalogue);
	// L'événement vient d'attribuer un identifiant aux nouveaux articles : sans le
	// relire, la page « Répartition » les croirait sans identité.
	refreshCatalogue();
	Application::get().saveProgress();
}

void PopupCatalogue::onPopupUpdate() {
	if (Application::get().getCurrentEvent().isFinished()) {
		ImGui::TextDisabled("L'événement est terminé : son catalogue ne bouge plus.");
		if (ImGui::Button("Fermer", {g_buttonWidth, 0}))
			ImGui::CloseCurrentPopup();
		return;
	}

	// Deux pages : ce dont on dispose, et ce qu'on en a fait. La seconde est celle qui
	// manquait — répartir sans voir le résultat, c'est répartir à l'aveugle.
	if (ImGui::BeginChild("CataloguePages", {0, ImGui::GetContentRegionAvail().y - 40}, ImGuiChildFlags_None)) {
		if (ImGui::BeginTabBar("CatalogueTabs")) {
			if (ImGui::BeginTabItem("Catalogue")) {
				renderCatalogueTab();
				ImGui::EndTabItem();
			}
			const ImGuiTabItemFlags distributionFlags =
					m_showDistribution ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
			m_showDistribution = false;
			if (ImGui::BeginTabItem("Répartition", nullptr, distributionFlags)) {
				renderDistributionTab();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::EndChild();

	ImGui::Separator();
	if (ImGui::Button("Fermer", {g_buttonWidth, 0}))
		ImGui::CloseCurrentPopup();
}

void PopupCatalogue::renderCatalogueTab() {
	auto& event = Application::get().getCurrentEvent();
	ImGui::TextWrapped("Tous les lots dont vous disposez, saisis une fois pour toutes. La répartition les place "
					   "ensuite sur les manches, du plus discret au plus marquant.");
	ImGui::Text("Valeur totale du catalogue : %.2f €", core::totalValue(m_catalogue));
	ImGui::Separator();

	// Une barre d'outils au-dessus de la liste : les trois façons de garnir le catalogue
	// se voient d'emblée, au lieu d'un bouton poussé sous une liste qui remplit sa zone.
	if (ImGui::Button("Ajouter un lot")) {
		m_catalogue.emplace_back();
		commitCatalogue();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Ajoute une ligne vide à remplir.");
	ImGui::SameLine();
	if (ImGui::Button("Importer un tableur...")) {
		utils::FileDialog::openFile("Tableur|csv,tsv,txt", [this](const std::filesystem::path& iPath) -> void {
			const auto imported = core::importCatalogueCsv(iPath);
			m_lastResult = imported.summary;
			if (!imported.read || imported.prizes.empty())
				return;
			// Ajoutés à la suite : on importe souvent une liste après en avoir déjà
			// saisi une partie, et écraser serait une perte silencieuse.
			m_catalogue.insert(m_catalogue.end(), imported.prizes.begin(), imported.prizes.end());
			commitCatalogue();
		});
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Lit un fichier CSV exporté d'un tableur et ajoute ses lignes au catalogue.\n"
						  "Colonnes reconnues : désignation, donateur, valeur, attrait, enfant.");
	ImGui::SameLine();
	if (ImGui::Button("Reprendre les lots des parties")) {
		const auto adopted = event.adoptOrphanPrizes();
		refreshCatalogue();
		m_lastResult = adopted == 0 ? "Tous les lots des parties sont déjà au catalogue."
									: std::format("{} lot(s) des parties entrés au catalogue.", adopted);
		log_info("{}", m_lastResult);
		Application::get().saveProgress();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Fait entrer au catalogue les lots placés dans les parties qui n'y sont pas encore.\n"
						  "N'enlève rien : c'est ce qui permet de repartir d'une répartition faite à la main.");

	if (utils::renderPrizeList(m_catalogue, {.editable = true, .size = {0, -g_settingsHeight}, .withAddButton = false}))
		commitCatalogue();

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
		commitCatalogue();
		const auto result = core::distributePrizes(event, m_settings);
		m_lastResult = result.summary;
		log_info("Répartition des lots : {}", result.summary);
		Application::get().saveProgress();
		// Le résultat se regarde, il ne se devine pas.
		showDistribution();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Reprend tout le catalogue à zéro : les manches déjà entamées ne sont pas touchées,\n"
						  "et l'ajustement à la main se fait ensuite dans le réglage des parties.");
}

void PopupCatalogue::renderDistributionTab() {
	const auto snapshot = core::overview(Application::get().getCurrentEvent());
	renderCharts(snapshot);
	ImGui::Separator();
	renderAssignmentTable(snapshot);
}

void PopupCatalogue::renderCharts(const core::DistributionOverview& iOverview) {
	if (iOverview.subRounds.empty()) {
		ImGui::TextDisabled("Aucune manche à montrer : l'événement n'a pas encore de partie.");
		return;
	}

	// Une barre par manche, dans l'ordre du programme : la montée au sein de chaque
	// partie et la montée vers la dernière se lisent d'un coup d'œil.
	std::vector<float> subValues;
	subValues.reserve(iOverview.subRounds.size());
	for (const auto& entry: iOverview.subRounds) subValues.push_back(static_cast<float>(entry.value));
	const auto subMax = static_cast<float>(iOverview.highestSubRoundValue());

	ImGui::Text("Valeur par manche, dans l'ordre du programme");
	ImGui::PlotHistogram("##SubRoundChart", subValues.data(), static_cast<int>(subValues.size()), 0, nullptr, 0.0f,
						 subMax > 0.0f ? subMax * 1.1f : 1.0f, {-1, g_chartHeight});
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Chaque barre est une manche : quine, double quine, carton plein, partie après partie.");

	std::vector<float> roundValues;
	roundValues.reserve(iOverview.roundValues.size());
	for (const auto& value: iOverview.roundValues) roundValues.push_back(static_cast<float>(value));
	const auto roundMax = static_cast<float>(iOverview.highestRoundValue());

	ImGui::Text("Valeur par partie — la progression de l'après-midi");
	ImGui::PlotLines("##RoundChart", roundValues.data(), static_cast<int>(roundValues.size()), 0, nullptr, 0.0f,
					 roundMax > 0.0f ? roundMax * 1.1f : 1.0f, {-1, g_chartHeight});
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("La courbe doit monter jusqu'à la dernière partie, qui est le point d'orgue.");

	ImGui::Text("Total réparti : %.2f €",
				std::accumulate(iOverview.roundValues.begin(), iOverview.roundValues.end(), 0.0));
	if (iOverview.unassignedCount > 0) {
		ImGui::SameLine();
		ImGui::TextDisabled("— %zu article(s) non affecté(s), %.2f €", iOverview.unassignedCount,
							iOverview.unassignedValue);
	}
}

void PopupCatalogue::renderAssignmentTable(const core::DistributionOverview& iOverview) {
	auto& event = Application::get().getCurrentEvent();

	// La liste des destinations possibles, dans l'ordre du programme, plus « nulle part ».
	std::string slots{"non affecté"};
	slots += '\0';
	std::vector<std::optional<core::Event::PrizeSlot>> targets{std::nullopt};
	for (const auto& entry: iOverview.subRounds) {
		if (!entry.editable)
			continue;
		slots += std::format("{} — {}", entry.roundName, entry.subRoundName);
		slots += '\0';
		targets.emplace_back(core::Event::PrizeSlot{.round = entry.round, .subRound = entry.subRound});
	}
	slots += '\0';

	ImGui::Text("Affectation article par article");
	if (!ImGui::BeginTable("Assignments", 5,
						   ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
								   ImGuiTableFlags_SizingStretchProp)) {
		return;
	}
	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("Article");
	ImGui::TableSetupColumn("Valeur", ImGuiTableColumnFlags_WidthFixed, 90.0f);
	ImGui::TableSetupColumn("Attrait", ImGuiTableColumnFlags_WidthFixed, 70.0f);
	ImGui::TableSetupColumn("Enfant", ImGuiTableColumnFlags_WidthFixed, 60.0f);
	ImGui::TableSetupColumn("Mis en jeu dans");
	ImGui::TableHeadersRow();

	for (const auto& prize: m_catalogue) {
		ImGui::TableNextRow();
		ImGui::PushID(static_cast<int>(prize.getId()));
		ImGui::TableNextColumn();
		ImGui::TextWrapped("%s", prize.getDesignation().c_str());
		ImGui::TableNextColumn();
		ImGui::Text("%.2f €", prize.getValue());
		ImGui::TableNextColumn();
		ImGui::Text("%u", prize.getAttractiveness());
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(prize.isChildFriendly() ? "oui" : "non");
		ImGui::TableNextColumn();

		const auto current = event.findPrizeSlot(prize.getId());
		// Une manche entamée ne figure pas parmi les destinations : l'article qui y est
		// en jeu ne peut plus en bouger, et le dire vaut mieux qu'une liste trompeuse.
		if (current.has_value() && std::ranges::find(targets, current) == targets.end()) {
			const auto entry = std::ranges::find_if(
					iOverview.subRounds, [&current](const core::DistributionOverview::SubRoundEntry& iItem) -> bool {
						return iItem.round == current->round && iItem.subRound == current->subRound;
					});
			if (entry != iOverview.subRounds.end())
				ImGui::TextDisabled("%s — %s (en jeu)", entry->roundName.c_str(), entry->subRoundName.c_str());
			else
				ImGui::TextDisabled("en jeu");
			ImGui::PopID();
			continue;
		}
		int selected = 0;
		for (size_t i = 0; i < targets.size(); ++i) {
			if (targets[i] == current)
				selected = static_cast<int>(i);
		}
		ImGui::SetNextItemWidth(-1);
		if (ImGui::Combo("##slot", &selected, slots.c_str())) {
			// Les courbes du dessus sont recalculées à l'image suivante : le déplacement
			// se voit immédiatement.
			if (event.assignPrize(prize.getId(), targets[static_cast<size_t>(selected)]))
				Application::get().saveProgress();
		}
		ImGui::PopID();
	}
	ImGui::EndTable();
}

}// namespace evl::gui::views
