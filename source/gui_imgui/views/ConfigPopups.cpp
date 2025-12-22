/**
 * @file ConfigPopups.cpp
 * @author Silmaen
 * @date 20/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "ConfigPopups.h"

#include "core/Settings.h"
#include "core/maths/vectors.h"
#include "core/utilities.h"
#include "gui_imgui/Application.h"
#include "gui_imgui/utils/FileDialog.h"

#include <cstring>
#include <imgui.h>


namespace evl::gui_imgui::views {

constexpr float g_buttonWidth = 100.0f;
constexpr float g_buttonSectionHeight = 50.0f;

namespace {

auto getOffset(const int iNbButtons) -> float {
	return (ImGui::GetContentRegionAvail().x - g_buttonWidth * static_cast<float>(iNbButtons) +
			ImGui::GetStyle().ItemSpacing.x * static_cast<float>(iNbButtons - 1)) *
		   0.5f;
}

}// namespace


MainConfigPopups::MainConfigPopups() = default;

MainConfigPopups::~MainConfigPopups() = default;

void MainConfigPopups::onOpen() { settingsToData(); }

void MainConfigPopups::onPopupUpdate() {
	// Répertoires par défaut
	if (ImGui::BeginChild("DefaultDirectories", ImVec2(0, 80), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Répertoires par défaut");
		ImGui::Separator();
		ImGui::Text("Données:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-80);
		char dataLocationBuffer[512] = "";
		std::strncpy(dataLocationBuffer, m_data.dataLocation.string().c_str(), sizeof(dataLocationBuffer) - 1);
		if (ImGui::InputText("##DataLocation", dataLocationBuffer, sizeof(dataLocationBuffer))) {
			m_data.dataLocation = dataLocationBuffer;
		}
		ImGui::SameLine();
		if (ImGui::Button("...##SearchFolder")) {
			if (const auto path = utils::FileDialog::selectFolder(); !path.empty()) {
				m_data.dataLocation = path;
				std::strncpy(dataLocationBuffer, path.string().c_str(), sizeof(dataLocationBuffer) - 1);
			}
		}
	}
	ImGui::EndChild();

	// Thème
	if (const float themeHeight = ImGui::GetContentRegionAvail().y - g_buttonSectionHeight;
		ImGui::BeginChild("Theme", ImVec2(0, themeHeight), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Thème");
		ImGui::Separator();

		// Personnalisation
		if (ImGui::BeginChild("Customization", ImVec2(0, 500), ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Personnalisation");
			ImGui::Separator();

			ImGui::Columns(2, "ConfigColumns");
			ImGui::SetColumnWidth(1, 250);

			// Échelles
			ImGui::Text("Facteur d'échelle principal");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##MainScale", &m_data.mainScale, 0.001f, 0.01f, 0.05f, "%.3f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle de titre");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##TitleScale", &m_data.titleScale, 0.1f, 1.0f, 5.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle texte court");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##ShortTextScale", &m_data.shortTextScale, 0.1f, 0.2f, 3.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle texte long");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##LongTextScale", &m_data.longTextScale, 0.1f, 0.2f, 3.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle de la grille");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##GridTextScale", &m_data.gridTextScale, 0.01f, 0.2f, 2.0f, "%.2f");
			ImGui::NextColumn();

			ImGui::Separator();

			// Couleurs
			ImGui::Text("Couleur de l'arrière plan");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##BackgroundColor", reinterpret_cast<float*>(&m_data.backgroundColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Couleur de l'arrière plan de la grille");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##GridBackgroundColor", reinterpret_cast<float*>(&m_data.gridBackgroundColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Tronquer lignes de lots");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::Checkbox("##TruncatePrice", &m_data.truncatePrice);
			ImGui::NextColumn();

			ImGui::Text("Nombre maximum de lignes de lots");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragInt("##TruncatePriceLines", &m_data.truncatePriceLines, 1.0f, 1, 15);
			ImGui::NextColumn();

			ImGui::Text("Couleur du texte");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##TextColor", reinterpret_cast<float*>(&m_data.textColor), ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Separator();

			// Numéros sélectionnés
			ImGui::Text("Couleur chiffre sélectionné");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##SelectedNumberColor", reinterpret_cast<float*>(&m_data.selectedNumberColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Activation de la fondu de couleur");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::Checkbox("##FadeNumbers", &m_data.fadeNumbers);
			ImGui::NextColumn();

			ImGui::Text("Fondu sur combien de chiffres");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragInt("##FadeAmount", &m_data.fadeAmount, 1.0f, 0, 10);
			ImGui::NextColumn();

			ImGui::Text("Force de la fondue");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragInt("##FadeStrength", &m_data.fadeStrength, 4.0f, -100, 100);
			ImGui::NextColumn();

			ImGui::Columns(1);
		}
		ImGui::EndChild();

		ImGui::Spacing();

		// Boutons Reset/Restore
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + getOffset(2));
		if (ImGui::Button("Reset", ImVec2(g_buttonWidth, 0))) {
			m_data = {};
		}
		ImGui::SameLine();
		if (ImGui::Button("Restorer", ImVec2(g_buttonWidth, 0))) {
			settingsToData();
		}
	}
	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Boutons Ok/Appliquer/Annuler
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + getOffset(3));
	if (ImGui::Button("Ok", ImVec2(g_buttonWidth, 0))) {
		dataToSettings();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Appliquer", ImVec2(g_buttonWidth, 0))) {
		dataToSettings();
	}
	ImGui::SameLine();
	if (ImGui::Button("Annuler", ImVec2(g_buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
}

void MainConfigPopups::dataToSettings() {
	core::Settings settings;
	settings.setValue("data_location", std::string{m_data.dataLocation});
	settings.setValue("main_scale", m_data.mainScale);
	settings.setValue("title_scale", m_data.titleScale);
	settings.setValue("short_text_scale", m_data.shortTextScale);
	settings.setValue("long_text_scale", m_data.longTextScale);
	settings.setValue("grid_text_scale", m_data.gridTextScale);
	settings.setValue("background_color", m_data.backgroundColor);
	settings.setValue("grid_background_color", m_data.gridBackgroundColor);
	settings.setValue("text_color", m_data.textColor);
	settings.setValue("selected_number_color", m_data.selectedNumberColor);
	settings.setValue("truncate_price", m_data.truncatePrice);
	settings.setValue("truncate_price_lines", m_data.truncatePriceLines);
	settings.setValue("fade_numbers", m_data.fadeNumbers);
	settings.setValue("fade_amount", m_data.fadeAmount);
	settings.setValue("fade_strength", m_data.fadeStrength);
	core::getSettings()->include(settings, "gui");
}

void MainConfigPopups::settingsToData() {
	const auto settings = core::getSettings()->extract("gui");
	const Data defaults{};

	m_data.dataLocation = settings.getValue<std::string>("data_location", defaults.dataLocation.string());
	m_data.mainScale = settings.getValue<float>("main_scale", defaults.mainScale);
	m_data.titleScale = settings.getValue<float>("title_scale", defaults.titleScale);
	m_data.shortTextScale = settings.getValue<float>("short_text_scale", defaults.shortTextScale);
	m_data.longTextScale = settings.getValue<float>("long_text_scale", defaults.longTextScale);
	m_data.gridTextScale = settings.getValue<float>("grid_text_scale", defaults.gridTextScale);
	m_data.backgroundColor = settings.getValue<math::vec4>("background_color", defaults.backgroundColor);
	m_data.gridBackgroundColor = settings.getValue<math::vec4>("grid_background_color", defaults.gridBackgroundColor);
	m_data.textColor = settings.getValue<math::vec4>("text_color", defaults.textColor);
	m_data.selectedNumberColor = settings.getValue<math::vec4>("selected_number_color", defaults.selectedNumberColor);
	m_data.truncatePrice = settings.getValue<bool>("truncate_price", defaults.truncatePrice);
	m_data.truncatePriceLines = settings.getValue<int>("truncate_price_lines", defaults.truncatePriceLines);
	m_data.fadeNumbers = settings.getValue<bool>("fade_numbers", defaults.fadeNumbers);
	m_data.fadeAmount = settings.getValue<int>("fade_amount", defaults.fadeAmount);
	m_data.fadeStrength = settings.getValue<int>("fade_strength", defaults.fadeStrength);
}

//----------------------------------------------------------------------------------------------------------------------

EventConfigPopups::EventConfigPopups() = default;

EventConfigPopups::~EventConfigPopups() = default;

void EventConfigPopups::onOpen() {
	// Load data from current event
	fromCurrentEvent();
}

void EventConfigPopups::onPopupUpdate() {
	// Configuration événement
	if (ImGui::BeginChild("EventConfig", ImVec2(0, 140), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Configuration événement");
		ImGui::Separator();

		ImGui::Columns(2, "EventColumns");
		ImGui::SetColumnWidth(0, 200);

		ImGui::Text("Nom de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1);
		char eventName[256];
		std::strncpy(eventName, m_event.getName().c_str(), sizeof(eventName) - 1);
		if (ImGui::InputText("##EventName", eventName, sizeof(eventName))) {
			m_event.setName(eventName);
		}
		ImGui::NextColumn();

		ImGui::Text("Lieu de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1);
		char eventLocation[256] = "";
		std::strncpy(eventLocation, m_event.getLocation().c_str(), sizeof(eventLocation) - 1);
		if (ImGui::InputText("##EventLocation", eventLocation, sizeof(eventLocation))) {
			m_event.setLocation(eventLocation);
		}
		ImGui::NextColumn();

		ImGui::Text("Logo de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-80);
		char eventLogo[256] = "";
		std::strncpy(eventLogo, m_event.getLogo().c_str(), sizeof(eventLogo) - 1);
		if (ImGui::InputText("##EventLogo", eventLogo, sizeof(eventLogo))) {
			m_event.setLogo(eventLogo);
		}
		ImGui::SameLine();
		if (ImGui::Button("...##SearchEventLogo")) {
			if (const auto path = utils::FileDialog::openFile(utils::g_imageFilter); !path.empty())
				m_event.setLogo(path.string());
		}
		ImGui::NextColumn();

		ImGui::Columns(1);
	}
	ImGui::EndChild();

	// Organisateur
	if (ImGui::BeginChild("Organizer", ImVec2(0, 100), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Organisateur");
		ImGui::Separator();

		ImGui::Columns(2, "OrganizerColumns");
		ImGui::SetColumnWidth(0, 200);

		ImGui::Text("Nom de l'organisateur");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1);
		char orgaName[256] = "";
		std::strncpy(orgaName, m_event.getOrganizerName().c_str(), sizeof(orgaName) - 1);
		if (ImGui::InputText("##OrgaName", orgaName, sizeof(orgaName))) {
			m_event.setOrganizerName(orgaName);
		}
		ImGui::NextColumn();

		ImGui::Text("Logo de l'organisateur:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-80);
		char orgaLogo[256] = "";
		std::strncpy(orgaLogo, m_event.getOrganizerLogo().c_str(), sizeof(orgaLogo) - 1);
		if (ImGui::InputText("##OrgaLogo", orgaLogo, sizeof(orgaLogo))) {
			m_event.setOrganizerLogo(orgaLogo);
		}
		ImGui::SameLine();
		if (ImGui::Button("...##SearchOrgaLogo")) {
			if (const auto path = utils::FileDialog::openFile(utils::g_imageFilter); !path.empty())
				m_event.setLogo(path.string());
		}
		ImGui::NextColumn();

		ImGui::Columns(1);
	}
	ImGui::EndChild();

	// Règlement
	if (const float rulesHeight = ImGui::GetContentRegionAvail().y - g_buttonSectionHeight;
		ImGui::BeginChild("Rules", ImVec2(0, rulesHeight), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Règlement");
		ImGui::Separator();

		const float rulesTextHeight = ImGui::GetContentRegionAvail().y - 40;
		char rules[20000] = "";
		std::strncpy(rules, m_event.getRules().c_str(), sizeof(rules) - 1);
		if (ImGui::InputTextMultiline("##Rules", rules, sizeof(rules), ImVec2(-1, rulesTextHeight))) {
			m_event.setRules(rules);
		}

		ImGui::Spacing();

		// Boutons Import/Export
		if (ImGui::Button("Import", ImVec2(g_buttonWidth, 0))) {
			// Action import règlement
		}
		ImGui::SameLine();
		const float exportOffset = ImGui::GetContentRegionAvail().x - g_buttonWidth;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + exportOffset);
		if (ImGui::Button("Export", ImVec2(g_buttonWidth, 0))) {
			// Action export règlement
		}
	}
	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Boutons Ok/Appliquer/Annuler
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + getOffset(3));
	if (ImGui::Button("Ok", ImVec2(g_buttonWidth, 0))) {
		toCurrentEvent();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Appliquer", ImVec2(g_buttonWidth, 0))) {
		toCurrentEvent();
	}
	ImGui::SameLine();
	if (ImGui::Button("Annuler", ImVec2(g_buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
}

void EventConfigPopups::fromCurrentEvent() {
	// Load data from current event
	m_event = Application::get().getCurrentEvent();
}
void EventConfigPopups::toCurrentEvent() const {
	// Save data to current event
	Application::get().getCurrentEvent() = m_event;
}

//----------------------------------------------------------------------------------------------------------------------

GameRoundConfigPopups::GameRoundConfigPopups() = default;

GameRoundConfigPopups::~GameRoundConfigPopups() = default;

void GameRoundConfigPopups::onOpen() {
	// Initialisation des données si nécessaire
	fromCurrentEvent();
}

void GameRoundConfigPopups::onPopupUpdate() {
	// Variables statiques pour les listes et données


	// Section d'édition (GroupEdit)
	if (const float editHeight = ImGui::GetContentRegionAvail().y * 0.65f;
		ImGui::BeginChild("GroupEdit", ImVec2(0, editHeight), ImGuiChildFlags_None)) {

		// Trois colonnes : GameRounds, SubRounds, Phase Configuration
		ImGui::Columns(3, "EditColumns", false);

		// Colonne 1: Liste des Parties
		renderFirstColumn();
		ImGui::NextColumn();

		// Colonne 2: Phases de la partie
		renderSecondColumn();
		ImGui::NextColumn();

		// Colonne 3: Configuration de la phase
		renderThirdColumn();

		ImGui::Columns(1);
	}
	ImGui::EndChild();

	// Section résultats (GroupResult)
	renderResult();
	ImGui::Separator();
	ImGui::Spacing();

	// Boutons Ok/Appliquer/Annuler/Import/Export
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + getOffset(5));
	if (ImGui::Button("Ok", ImVec2(g_buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Appliquer", ImVec2(g_buttonWidth, 0))) {
		// Action apply
	}
	ImGui::SameLine();
	if (ImGui::Button("Annuler", ImVec2(g_buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Importer", ImVec2(g_buttonWidth, 0))) {
		// Action import
	}
	ImGui::SameLine();
	if (ImGui::Button("Exporter", ImVec2(g_buttonWidth, 0))) {
		// Action export
	}
}

void GameRoundConfigPopups::fromCurrentEvent() {
	// Load data from current event
	m_event = Application::get().getCurrentEvent();
}
void GameRoundConfigPopups::toCurrentEvent() const {
	// Save data to current event
	Application::get().getCurrentEvent() = m_event;
}

void GameRoundConfigPopups::renderFirstColumn() {
	static std::vector<std::string> gameRounds;
	if (ImGui::BeginChild("GroupGameRound", ImVec2(0, 0), ImGuiChildFlags_Border)) {
		ImGui::Text("Liste des Parties");
		ImGui::Separator();

		// Boutons de gestion
		if (ImGui::Button("+##AddRound")) { /* Action add round */
		}
		ImGui::SameLine();
		if (ImGui::Button("-##DelRound")) { /* Action delete round */
		}
		ImGui::SameLine();
		if (ImGui::Button("↑##UpRound")) { /* Action move up */
		}
		ImGui::SameLine();
		if (ImGui::Button("↓##DownRound")) { /* Action move down */
		}

		ImGui::Spacing();

		// Liste des parties
		if (ImGui::BeginListBox("##GameRoundList", ImVec2(-1, -1))) {
			for (size_t i = 0; i < gameRounds.size(); ++i) {
				if (const bool isSelected = m_selectedGameRound == i;
					ImGui::Selectable(gameRounds[i].c_str(), isSelected)) {
					m_selectedGameRound = i;
				}
			}
			ImGui::EndListBox();
		}
	}
	ImGui::EndChild();
}

void GameRoundConfigPopups::renderSecondColumn() {
	static std::vector<std::string> subRounds;
	static int roundNumber = 0;

	if (ImGui::BeginChild("GroupSubRound", ImVec2(0, 0), ImGuiChildFlags_Border)) {
		ImGui::Text("Phases de la partie");
		ImGui::Separator();

		// Type de partie
		ImGui::Text("Type de partie:");
		ImGui::SetNextItemWidth(-80);
		if (ImGui::Combo("##GameRoundTypes", &m_currentGameRoundType, "Normale\0Pause\0")) {
			/* Action change type */
		}
		ImGui::SameLine();
		ImGui::Text("N°");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(60);
		ImGui::InputInt("##RoundNumber", &roundNumber);

		ImGui::Spacing();

		// Boutons de gestion
		if (ImGui::Button("+##AddSubRound")) { /* Action add sub round */
		}
		ImGui::SameLine();
		if (ImGui::Button("-##DelSubRound")) { /* Action delete sub round */
		}
		ImGui::SameLine();
		if (ImGui::Button("↑##UpSubRound")) { /* Action move up */
		}
		ImGui::SameLine();
		if (ImGui::Button("↓##DownSubRound")) { /* Action move down */
		}

		ImGui::Spacing();

		// Liste des sous-parties
		if (ImGui::BeginListBox("##SubRoundList", ImVec2(-1, -1))) {
			for (size_t i = 0; i < subRounds.size(); ++i) {
				if (const bool isSelected = m_selectedSubRound == i;
					ImGui::Selectable(subRounds[i].c_str(), isSelected)) {
					m_selectedSubRound = i;
				}
			}
			ImGui::EndListBox();
		}
	}
	ImGui::EndChild();
}

void GameRoundConfigPopups::renderThirdColumn() {
	static float priceValue = 0.0f;
	static char textPrices[4096] = "";
	static bool pauseNothing = true;
	static bool pauseDiapo = false;
	static char pauseDiapoFolder[256] = "";
	static float pauseDiapoDelay = 5.0f;
	static bool previewEnabled = false;
	static bool previewFullScreen = true;

	if (ImGui::BeginChild("GroupPhase", ImVec2(0, 0), ImGuiChildFlags_Border)) {
		ImGui::Text("Configuration de la phase:");
		ImGui::Separator();

		// Onglets selon le type
		if (m_currentGameRoundType == 0) {// Partie normale
			ImGui::Text("Condition de victoire:");
			ImGui::SetNextItemWidth(-1);
			ImGui::Combo("##SubRoundTypes", &m_currentSubRoundType,
						 "Ligne\0Deux lignes\0Carton plein\0Quatre coins\0Croix\0");

			ImGui::Spacing();
			ImGui::Text("Valeur Totale");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputFloat("##PriceValue", &priceValue, 0.0f, 0.0f, "%.2f €");

			ImGui::Spacing();
			ImGui::Text("Liste des lots:");
			ImGui::InputTextMultiline("##TextPrices", textPrices, 4096, ImVec2(-1, -80));

		} else {// Pause
			if (ImGui::RadioButton("Rien", pauseNothing)) {
				pauseNothing = true;
				pauseDiapo = false;
			}
			if (ImGui::RadioButton("Diaporama", pauseDiapo)) {
				pauseNothing = false;
				pauseDiapo = true;
			}

			if (pauseDiapo) {
				ImGui::Spacing();
				ImGui::Text("Répertoire de diapo");
				ImGui::SetNextItemWidth(-80);
				ImGui::InputText("##PauseDiapoFolder", pauseDiapoFolder, 256);
				ImGui::SameLine();
				if (ImGui::Button("...##BrowseDiapo")) {
					if (const auto path = utils::FileDialog::selectFolder(); !path.empty()) {
						strncpy(pauseDiapoFolder, path.string().c_str(), 255);
					}
				}

				ImGui::Text("Délai par diapo");
				ImGui::SetNextItemWidth(-1);
				ImGui::InputFloat("##PauseDiapoDelay", &pauseDiapoDelay, 0.1f, 1.0f, "%.1f s");
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Aperçu
		ImGui::Checkbox("Aperçu", &previewEnabled);
		ImGui::SameLine();
		ImGui::Checkbox("Plein écran", &previewFullScreen);
	}
	ImGui::EndChild();
}

//TODO: refactor to use internal data and actions
//NOLINTBEGIN(readability-convert-member-functions-to-static)
void GameRoundConfigPopups::renderResult() {
	if (const float resultHeight = ImGui::GetContentRegionAvail().y - 50;
		ImGui::BeginChild("GroupResult", ImVec2(0, resultHeight), ImGuiChildFlags_Border)) {

		ImGui::Text("Résultats de la partie");
		ImGui::Separator();

		ImGui::Columns(3, "ResultColumns", false);

		std::string emptyDate = "--";
		// Colonne 1: Dates et durée
		if (ImGui::BeginChild("ResultDates", ImVec2(0, 0), ImGuiChildFlags_None)) {
			ImGui::Text("Début:");
			ImGui::InputText("##StartDate", emptyDate.data(), emptyDate.size(), ImGuiInputTextFlags_ReadOnly);
			ImGui::Text("Fin:");
			ImGui::InputText("##EndDate", emptyDate.data(), emptyDate.size(), ImGuiInputTextFlags_ReadOnly);
			ImGui::Text("Durée:");
			ImGui::InputText("##Duration", emptyDate.data(), emptyDate.size(), ImGuiInputTextFlags_ReadOnly);
			ImGui::Text("Nombre de Tirages:");
			ImGui::InputText("##NumDraws", emptyDate.data(), emptyDate.size(), ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::EndChild();
		ImGui::NextColumn();

		// Colonne 2: Tirages
		if (ImGui::BeginChild("ResultDraws", ImVec2(0, 0), ImGuiChildFlags_None)) {
			ImGui::Text("Tirages:");
			ImGui::InputTextMultiline("##TextDraws", emptyDate.data(), emptyDate.size(), ImVec2(-1, -1),
									  ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::EndChild();
		ImGui::NextColumn();

		// Colonne 3: Gagnants
		if (ImGui::BeginChild("ResultWinners", ImVec2(0, 0), ImGuiChildFlags_None)) {
			ImGui::Text("Noms des gagnants:");
			ImGui::InputTextMultiline("##ListWinners", emptyDate.data(), emptyDate.size(), ImVec2(-1, -1),
									  ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::EndChild();

		ImGui::Columns(1);
	}
	ImGui::EndChild();
	ImGui::Spacing();
}
//NOLINTEND(readability-convert-member-functions-to-static)

}// namespace evl::gui_imgui::views
