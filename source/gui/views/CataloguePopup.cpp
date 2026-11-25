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
#include "core/PrizePdf.h"
#include "gui/Application.h"
#include "gui/utils/FileDialog.h"
#include "gui/utils/Layout.h"
#include "gui/utils/Rendering.h"

#include <imgui.h>

#include <numeric>
#include <optional>
#include <vector>

namespace evl::gui::views {

namespace {

/// Hauteur d'un histogramme : de quoi lire une tendance sans manger la page.
auto chartHeight() -> float { return utils::em(4.5f); }

/// La place que prennent sous la liste les réglages de la répartition.
auto settingsHeight() -> float {
	const auto& style = ImGui::GetStyle();
	return ImGui::GetTextLineHeightWithSpacing() * 2.0f + ImGui::GetFrameHeightWithSpacing() * 2.0f +
		   style.ItemSpacing.y * 2.0f;
}

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
		utils::beginDialogFooter(1);
		if (utils::dialogButton("Fermer"))
			ImGui::CloseCurrentPopup();
		return;
	}

	// Deux pages : ce dont on dispose, et ce qu'on en a fait. La seconde est celle qui
	// manquait — répartir sans voir le résultat, c'est répartir à l'aveugle.
	if (ImGui::BeginChild(
				"CataloguePages",
				{0, ImGui::GetContentRegionAvail().y - utils::dialogFooterHeight() - ImGui::GetStyle().ItemSpacing.y},
				ImGuiChildFlags_None)) {
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

	utils::beginDialogFooter(1);
	if (utils::dialogButton("Fermer"))
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
			const auto firstNew = m_catalogue.size();
			m_catalogue.insert(m_catalogue.end(), imported.prizes.begin(), imported.prizes.end());
			commitCatalogue();
			if (imported.placements.empty())
				return;
			// Un fichier déjà réparti : chaque article va où il dit, dans l'ordre des lignes.
			const auto placement = core::placeImported(Application::get().getCurrentEvent(), firstNew, imported);
			// A children's round takes the winner number into the article itself.
			refreshCatalogue();
			m_lastResult += std::format(" {} mis en jeu", placement.placed);
			if (placement.refused > 0)
				m_lastResult += std::format(", {} laissé(s) au catalogue (partie ou phase introuvable, ou déjà entamée)",
											placement.refused);
			m_lastResult += ".";
			log_info("{}", m_lastResult);
			Application::get().saveProgress();
		});
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Lit un fichier CSV exporté d'un tableur et ajoute ses lignes au catalogue.\n"
						  "Colonnes reconnues : désignation, donateur, valeur, attrait, enfant.\n"
						  "Avec les colonnes partie et phase (numéros), les lots sont placés où le fichier le dit,\n"
						  "dans l'ordre des lignes, sans répartition automatique.\n"
						  "Pour une partie enfant, la phase est le numéro du gagnant qui emporte l'article.");
	ImGui::SameLine();
	if (ImGui::Button("Vider la liste") && !m_catalogue.empty())
		ImGui::OpenPopup("Vider le catalogue ?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Retire tous les articles du catalogue, et des parties où ils étaient placés.");
	if (utils::confirmDialog("Vider le catalogue ?",
							 std::format("Retirer les {} article(s) du catalogue ? Ceux placés dans une partie "
										 "pas encore entamée en sont retirés aussi. Les parties entamées gardent leurs lots.",
										 m_catalogue.size()),
							 "Vider")) {
		const auto removed = event.clearCatalogue();
		refreshCatalogue();
		m_lastResult = std::format("{} article(s) retiré(s).", removed);
		log_info("{}", m_lastResult);
		Application::get().saveProgress();
	}
	ImGui::SameLine();
	if (ImGui::Button("Exporter en PDF...")) {
		utils::FileDialog::saveFile("PDF|pdf", [this](const std::filesystem::path& iPath) -> void {
			auto target = iPath;
			if (target.extension() != ".pdf")
				target += ".pdf";
			if (core::writePrizePdf(Application::get().getCurrentEvent(), target))
				m_lastResult = std::format("Liste des lots écrite dans {}.", target.string());
			else
				m_lastResult = "Rien à exporter, ou fichier impossible à écrire.";
			log_info("{}", m_lastResult);
		});
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Une page par partie et par phase, les lots écrits en gros pour les annoncer le jour J.");
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

	if (utils::renderPrizeList(m_catalogue, {.editable = true, .size = {0, -settingsHeight()}, .withAddButton = false}))
		commitCatalogue();

	ImGui::SeparatorText("Répartition automatique");
	ImGui::SetNextItemWidth(utils::em(9.0f));
	ImGui::SliderFloat("Part de l'attrait", &m_settings.attractivenessWeight, 0.0f, 1.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Zéro : seule la valeur compte. Un : seul l'attrait compte.\n"
						  "Un jambon fait souvent plus d'effet qu'un objet plus cher.");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(utils::em(9.0f));
	ImGui::SliderFloat("Montée vers la fin", &m_settings.climaxStrength, 0.0f, 1.0f, "%.2f");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Zéro : toutes les parties se valent.\n"
						  "Un : la dernière partie emporte les plus beaux lots.");
	ImGui::SameLine();
	ImGui::Checkbox("Respecter les parties enfant", &m_settings.respectChildRounds);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Une partie enfant ne reçoit que les articles marqués compatibles.");

	if (utils::dialogButton("Répartir")) {
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
	if (!m_lastResult.empty()) {
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::TextDisabled("%s", m_lastResult.c_str());
	}
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
						 subMax > 0.0f ? subMax * 1.1f : 1.0f, {-1, chartHeight()});
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Chaque barre est une manche : quine, double quine, carton plein, partie après partie.");

	std::vector<float> roundValues;
	roundValues.reserve(iOverview.roundValues.size());
	for (const auto& value: iOverview.roundValues) roundValues.push_back(static_cast<float>(value));
	const auto roundMax = static_cast<float>(iOverview.highestRoundValue());

	ImGui::Text("Valeur par partie — la progression de l'après-midi");
	ImGui::PlotLines("##RoundChart", roundValues.data(), static_cast<int>(roundValues.size()), 0, nullptr, 0.0f,
					 roundMax > 0.0f ? roundMax * 1.1f : 1.0f, {-1, chartHeight()});
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
	if (!ImGui::BeginTable("Assignments", 6,
						   ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
								   ImGuiTableFlags_SizingStretchProp)) {
		return;
	}
	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("Article");
	ImGui::TableSetupColumn("Valeur", ImGuiTableColumnFlags_WidthFixed, utils::em(4.5f));
	ImGui::TableSetupColumn("Attrait", ImGuiTableColumnFlags_WidthFixed, utils::em(3.5f));
	ImGui::TableSetupColumn("Enfant", ImGuiTableColumnFlags_WidthFixed, utils::em(3.0f));
	ImGui::TableSetupColumn("Mis en jeu dans");
	ImGui::TableSetupColumn("Gagnant", ImGuiTableColumnFlags_WidthFixed, utils::em(6.5f));
	ImGui::TableHeadersRow();

	// By index and on a copy: a winner change refreshes the catalogue under the loop.
	for (size_t index = 0; index < m_catalogue.size(); ++index) {
		const auto prize = m_catalogue[index];
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
		// The slot column is drawn below; the winner column only concerns a children's round.
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
		ImGui::TableNextColumn();
		// Only an article put in a children's round has a winner: elsewhere there is nothing to say.
		if (current.has_value() && event.getGameRound(current->round)->getType() == core::GameRound::Type::Enfant) {
			auto set = static_cast<int>(prize.getSet());
			ImGui::SetNextItemWidth(-1);
			if (ImGui::SliderInt("##set", &set, 0, static_cast<int>(core::Prize::g_maxSet), "gagnant %d") &&
				event.setPrizeSet(prize.getId(), static_cast<uint8_t>(set))) {
				refreshCatalogue();
				Application::get().saveProgress();
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Le gagnant qui emporte cet article, avec les autres du même numéro.\n"
								  "0 : l'article suit celui qui le précède.");
		}
		ImGui::PopID();
	}
	ImGui::EndTable();
}

}// namespace evl::gui::views
