/**
 * @file ConfigPopups.cpp
 * @author Silmaen
 * @date 20/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "ConfigPopups.h"

#include "DisplayView.h"
#include "core/Log.h"
#include "core/Settings.h"
#include "core/maths/vectors.h"
#include "core/utilities.h"
#include "gui_imgui/Application.h"
#include "gui_imgui/utils/FileDialog.h"

#include <cstring>
#include <imgui.h>
#include <imgui_stdlib.h>


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

MainConfigPopups::MainConfigPopups() {
	settingsToData();
	dataToSettings();
}

MainConfigPopups::~MainConfigPopups() = default;

void MainConfigPopups::onOpen() { settingsToData(); }

void MainConfigPopups::onPopupUpdate() {
	// Répertoires par défaut
	const auto style = ImGui::GetStyle();
	if (ImGui::BeginChild("DefaultDirectories", ImVec2(0, 80), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Répertoires par défaut");
		ImGui::Separator();
		ImGui::Text("Données:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-80);
		std::string originalPath = m_data.dataLocation.string();
		if (ImGui::InputText("##DataLocation", &originalPath)) {
			m_data.dataLocation = originalPath;
		}
		ImGui::SameLine();
		if (ImGui::Button("...##SearchFolder")) {
			if (const auto path = utils::FileDialog::selectFolder(); !path.empty()) {
				m_data.dataLocation = path;
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
		if (ImGui::BeginChild("Customization", ImVec2(0, ImGui::GetContentRegionAvail().y - g_buttonSectionHeight),
							  ImGuiWindowFlags_NoTitleBar)) {
			ImGui::Text("Personnalisation");
			ImGui::Separator();

			ImGui::Columns(2, "ConfigColumns");
			ImGui::SetColumnWidth(1, 250);

			// Échelles
			ImGui::Text("Facteur d'échelle de titre");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##TitleScale", &m_data.titleScale, 0.1f, 1.0f, 10.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Echelle texte grille");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##GridTextScale", &m_data.gridTextScale, 0.1f, 0.3f, 1.5f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle prix");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##ValueScale", &m_data.valueScale, 0.1f, 0.5f, 10.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle texte lots");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##PriceTextScale", &m_data.priceTextScale, 0.1f, 0.5f, 10.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle horloges");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##TimeScale", &m_data.timeScale, 0.1f, 0.5f, 2.5f, "%.1f");
			ImGui::NextColumn();

			ImGui::Separator();

			// Couleurs
			ImGui::Text("Couleur de l'arrière plan");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##BackgroundColor", reinterpret_cast<float*>(&m_data.backgroundColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Couleur du texte");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##TextColor", reinterpret_cast<float*>(&m_data.textColor), ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Separator();

			ImGui::Text("Arrière plan de la grille");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##GridBackgroundColor", reinterpret_cast<float*>(&m_data.gridBackgroundColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Espacement de la grille");
			ImGui::NextColumn();
			ImGui::BeginGroup();
			const float gridSize = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) * 0.5f;
			ImGui::SetNextItemWidth(gridSize);
			ImGui::DragFloat("##GridSpaceX", &m_data.gridSpace.x(), 0.1f, 0.1f, 10.f, "%.1f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(gridSize);
			ImGui::DragFloat("##GridSpaceY", &m_data.gridSpace.y(), 0.1f, 0.1f, 10.f, "%.1f");

			ImGui::EndGroup();

			ImGui::NextColumn();

			ImGui::Separator();

			ImGui::Text("Tronquer lignes de lots");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::Checkbox("##TruncatePrice", &m_data.truncatePrice);
			ImGui::NextColumn();

			ImGui::Text("Max lignes de lots");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragInt("##TruncatePriceLines", &m_data.truncatePriceLines, 1.0f, 1, 15);
			ImGui::NextColumn();

			ImGui::Separator();

			// Numéros sélectionnés
			ImGui::Text("Couleur chiffre sélectionné");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##SelectedNumberColor", reinterpret_cast<float*>(&m_data.selectedNumberColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Fondu de couleur");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::Checkbox("##FadeNumbers", &m_data.fadeNumbers);
			ImGui::NextColumn();

			ImGui::Text("Nombre de chiffres fondu");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragInt("##FadeAmount", &m_data.fadeAmount, 1.0f, 0, 10);
			ImGui::NextColumn();

			ImGui::Text("Force de la fondue");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##FadeStrength", &m_data.fadeStrength, 0.1f, -2, 2, "%.1f");
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
	settings.setValue("title_scale", m_data.titleScale);
	settings.setValue("time_scale", m_data.timeScale);
	settings.setValue("value_scale", m_data.valueScale);
	settings.setValue("prices_scale", m_data.priceTextScale);
	settings.setValue("grid_text_scale", m_data.gridTextScale);
	settings.setValue("background_color", m_data.backgroundColor);
	settings.setValue("grid_background_color", m_data.gridBackgroundColor);
	settings.setValue("grid_button_spacing", m_data.gridSpace);
	settings.setValue("text_color", m_data.textColor);
	settings.setValue("selected_number_color", m_data.selectedNumberColor);
	settings.setValue("truncate_price", m_data.truncatePrice);
	settings.setValue("truncate_price_lines", m_data.truncatePriceLines);
	settings.setValue("fade_numbers", m_data.fadeNumbers);
	settings.setValue("fade_amount", m_data.fadeAmount);
	settings.setValue("fade_strength", m_data.fadeStrength);
	core::getSettings()->include(settings, "gui");
	core::getSettings()->setValue("general/data_location", std::string{m_data.dataLocation.string()});
}

void MainConfigPopups::settingsToData() {
	const auto settings = core::getSettings()->extract("gui");
	const Data defaults{};

	m_data.dataLocation =
			core::getSettings()->getValue<std::string>("general/data_location", defaults.dataLocation.string());
	m_data.titleScale = settings.getValue<float>("title_scale", defaults.titleScale);
	m_data.timeScale = settings.getValue<float>("time_scale", defaults.timeScale);
	m_data.valueScale = settings.getValue<float>("value_scale", defaults.valueScale);
	m_data.priceTextScale = settings.getValue<float>("prices_scale", defaults.priceTextScale);
	m_data.gridTextScale = settings.getValue<float>("grid_text_scale", defaults.gridTextScale);
	m_data.backgroundColor = settings.getValue<math::vec4>("background_color", defaults.backgroundColor);
	m_data.gridBackgroundColor = settings.getValue<math::vec4>("grid_background_color", defaults.gridBackgroundColor);
	m_data.gridSpace = settings.getValue<math::vec2>("grid_button_spacing", defaults.gridSpace);
	m_data.textColor = settings.getValue<math::vec4>("text_color", defaults.textColor);
	m_data.selectedNumberColor = settings.getValue<math::vec4>("selected_number_color", defaults.selectedNumberColor);
	m_data.truncatePrice = settings.getValue<bool>("truncate_price", defaults.truncatePrice);
	m_data.truncatePriceLines = settings.getValue<int>("truncate_price_lines", defaults.truncatePriceLines);
	m_data.fadeNumbers = settings.getValue<bool>("fade_numbers", defaults.fadeNumbers);
	m_data.fadeAmount = settings.getValue<int>("fade_amount", defaults.fadeAmount);
	m_data.fadeStrength = settings.getValue<float>("fade_strength", defaults.fadeStrength);
}

//----------------------------------------------------------------------------------------------------------------------

EventConfigPopups::EventConfigPopups() = default;

EventConfigPopups::~EventConfigPopups() = default;

void EventConfigPopups::onOpen() {
	// Load data from current event
	fromCurrentEvent();
}

void EventConfigPopups::onPopupUpdate() {

	ImGuiInputTextFlags tags = ImGuiInputTextFlags_None;
	if (!m_event.isEditable())
		tags |= ImGuiInputTextFlags_ReadOnly;
	// Configuration événement
	if (ImGui::BeginChild("EventConfig", ImVec2(0, 140), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Configuration événement");
		ImGui::Separator();

		ImGui::Columns(2, "EventColumns");
		ImGui::SetColumnWidth(0, 200);

		ImGui::Text("Nom de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1);
		std::string eventName = m_event.getName();
		if (ImGui::InputText("##EventName", &eventName, tags)) {
			m_event.setName(eventName);
		}
		ImGui::NextColumn();

		ImGui::Text("Lieu de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1);
		std::string eventLocation = m_event.getLocation();
		if (ImGui::InputText("##EventLocation", &eventLocation, tags)) {
			m_event.setLocation(eventLocation);
		}
		ImGui::NextColumn();

		ImGui::Text("Logo de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-80);
		std::string eventLogo = m_event.getLogo().string();
		if (ImGui::InputText("##EventLogo", &eventLogo, tags)) {
			m_event.setLogo(eventLogo);
		}
		ImGui::SameLine();
		if (ImGui::Button("...##SearchEventLogo") && (tags & ImGuiInputTextFlags_ReadOnly) == 0) {
			if (const auto path = utils::FileDialog::openFile(utils::g_imageFilter); !path.empty()) {
				m_event.setLogo(path);
			}
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
		std::string orgaName = m_event.getOrganizerName();
		if (ImGui::InputText("##OrgaName", &orgaName, tags)) {
			m_event.setOrganizerName(orgaName);
		}
		ImGui::NextColumn();

		ImGui::Text("Logo de l'organisateur:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-80);
		std::string orgaLogo = m_event.getOrganizerLogo().string();
		if (ImGui::InputText("##OrgaLogo", &orgaLogo, tags)) {
			m_event.setOrganizerLogo(orgaLogo);
		}
		ImGui::SameLine();
		if (ImGui::Button("...##SearchOrgaLogo") && (tags & ImGuiInputTextFlags_ReadOnly) == 0) {
			if (const auto path = utils::FileDialog::openFile(utils::g_imageFilter); !path.empty()) {
				m_event.setOrganizerLogo(path.string());
			}
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
		std::string rules = m_event.getRules();
		if (ImGui::InputTextMultiline("##Rules", &rules, ImVec2(-1, rulesTextHeight),
									  tags | ImGuiInputTextFlags_WordWrap)) {
			m_event.setRules(rules);
		}

		ImGui::Spacing();

		// Boutons Import/Export
		if (ImGui::Button("Import", ImVec2(g_buttonWidth, 0)) && (tags & ImGuiInputTextFlags_ReadOnly) == 0) {
			if (const auto path = utils::FileDialog::openFile(utils::g_yamlFilter); !path.empty()) {
				try {
					if (YAML::Node node = YAML::LoadFile(path.string()); node["rules"]) {
						m_event.setRules(node["rules"].as<std::string>());
					} else {
						log_warn("No 'rules' field found in file: {}", path.string());
					}
				} catch (const YAML::Exception& e) { log_error("Failed to load YAML file: {}", e.what()); }
			}
		}
		ImGui::SameLine();
		const float exportOffset = ImGui::GetContentRegionAvail().x - g_buttonWidth;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + exportOffset);
		if (ImGui::Button("Export", ImVec2(g_buttonWidth, 0)) && (tags & ImGuiInputTextFlags_ReadOnly) == 0) {
			if (const auto path = utils::FileDialog::saveFile(utils::g_yamlFilter); !path.empty()) {
				try {
					YAML::Node node;
					node["rules"] = m_event.getRules();
					if (std::ofstream fout(path); !fout.is_open()) {
						log_error("Failed to open file for writing: {}", path.string());
					} else {
						fout << node;
						fout.close();
					}
				} catch (const YAML::Exception& e) { log_error("Failed to save YAML file: {}", e.what()); }
			}
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

namespace {

auto getRoundTypes() -> std::string {
	std::string result;
	for (const auto& name: magic_enum::enum_values<core::GameRound::Type>()) {
		if (name == core::GameRound::Type::Invalid)
			continue;
		if (!result.empty()) {
			result += '\0';
		}
		const core::GameRound round(name);
		result += round.getTypeStr();
	}
	result += '\0';// Double null terminator for ImGui
	return result;
}

}// namespace

GameRoundConfigPopups::GameRoundConfigPopups() = default;

GameRoundConfigPopups::~GameRoundConfigPopups() = default;

void GameRoundConfigPopups::onOpen() {
	// Initialisation des données si nécessaire
	fromCurrentEvent();
}

void GameRoundConfigPopups::onClose() {
	// Réinitialisation des sélections
	log_info("Closing GameRoundConfigPopups, disabling preview display.");
	auto& app = Application::get();
	app.setDisplayPreview(false);
	m_selectedGameRound = 0;
	m_selectedSubRound = 0;
	ImGui::CloseCurrentPopup();
}

void GameRoundConfigPopups::onPopupUpdate() {
	// Variables statiques pour les listes et données

	static float s_edit_ratio = 0.65f;
	const ImVec2 contentAvail = ImGui::GetContentRegionAvail();
	const ImGuiStyle& style = ImGui::GetStyle();

	// Splitter sizing
	constexpr float splitterThickness = 6.0f;
	constexpr float minEditHeight = 100.0f;
	constexpr float minResultHeight = 100.0f;
	float editHeight = contentAvail.y * s_edit_ratio;
	float resultHeight = contentAvail.y - editHeight - splitterThickness - style.WindowPadding.y;

	// Clamp to minimums
	if (editHeight < minEditHeight) {
		editHeight = minEditHeight;
		resultHeight = contentAvail.y - editHeight - splitterThickness;
	}
	if (resultHeight < minResultHeight) {
		resultHeight = minResultHeight;
		editHeight = contentAvail.y - resultHeight - splitterThickness;
	}
	s_edit_ratio = contentAvail.y > 0.0f ? editHeight / contentAvail.y : s_edit_ratio;

	// Section d'édition (GroupEdit)
	if (ImGui::BeginChild("GroupEdit", ImVec2(0, editHeight), ImGuiChildFlags_None)) {
		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp |
											   ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoSavedSettings;

		if (ImGui::BeginTable("EditColumnsTable", 3, tableFlags, ImVec2(-1, -1))) {
			// Optional: set initial stretch weights
			ImGui::TableSetupColumn("GameRounds", ImGuiTableColumnFlags_WidthStretch, 1.0f);
			ImGui::TableSetupColumn("SubRounds", ImGuiTableColumnFlags_WidthStretch, 1.1f);
			ImGui::TableSetupColumn("PhaseConfig", ImGuiTableColumnFlags_WidthStretch, 1.4f);
			ImGui::TableHeadersRow();

			// Column 1
			// Column 1
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			renderFirstColumn();

			// Column 2
			ImGui::TableSetColumnIndex(1);
			renderSecondColumn();

			// Column 3
			ImGui::TableSetColumnIndex(2);
			renderThirdColumn();

			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
	// --- Splitter between edit and result sections ---
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY());
		const ImVec2 splitterPos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton("##EditResultSplitter", ImVec2(contentAvail.x, splitterThickness));
		const bool splitterActive = ImGui::IsItemActive();
		const bool splitterHovered = ImGui::IsItemHovered();

		// Draw splitter handle
		const ImU32 splitterColor =
				ImGui::GetColorU32(splitterHovered ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
		ImGui::GetWindowDrawList()->AddRectFilled(
				splitterPos, ImVec2(splitterPos.x + contentAvail.x, splitterPos.y + splitterThickness), splitterColor);

		// Drag behavior
		if (splitterActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
			const float delta = ImGui::GetIO().MouseDelta.y;
			editHeight =
					std::clamp(editHeight + delta, minEditHeight, contentAvail.y - minResultHeight - splitterThickness);
			s_edit_ratio = contentAvail.y > 0.0f ? editHeight / contentAvail.y : s_edit_ratio;
		}
	}
	// Section résultats (GroupResult)
	if (ImGui::BeginChild("GroupResult", ImVec2(0, resultHeight), ImGuiChildFlags_Borders)) {
		renderResult();
		ImGui::Separator();
		ImGui::Spacing();

		// Boutons Ok/Appliquer/Annuler/Import/Export
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + getOffset(5));
		if (ImGui::Button("Ok", ImVec2(g_buttonWidth, 0))) {
			toCurrentEvent();
			onClose();
		}
		ImGui::SameLine();
		if (ImGui::Button("Appliquer", ImVec2(g_buttonWidth, 0))) {
			toCurrentEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Annuler", ImVec2(g_buttonWidth, 0))) {
			onClose();
		}
		ImGui::SameLine();
		if (ImGui::Button("Importer", ImVec2(g_buttonWidth, 0))) {
			// Action import
			if (const auto path = utils::FileDialog::openFile(utils::g_yamlFilter); !path.empty()) {
				m_event.importYaml(path);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Exporter", ImVec2(g_buttonWidth, 0))) {
			// Action export
			if (const auto path = utils::FileDialog::saveFile(utils::g_yamlFilter); !path.empty()) {
				m_event.exportYaml(path);
			}
		}
	}
	ImGui::EndChild();
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
	if (ImGui::BeginChild("GroupGameRound", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
		ImGui::Text("Liste des Parties");
		ImGui::Separator();

		// Boutons de gestion
		if (ImGui::Button("+##AddRound")) { /* Action add round */
			addGameRound();
		}
		ImGui::SameLine();
		if (ImGui::Button("-##DelRound")) { /* Action delete round */
			deleteGameRound();
		}
		ImGui::SameLine();
		if (ImGui::Button("^##UpRound")) { /* Action move up */
			moveGameRoundUp();
		}
		ImGui::SameLine();
		if (ImGui::Button("v##DownRound")) { /* Action move down */
			moveGameRoundDown();
		}

		ImGui::Spacing();

		// Liste des parties
		if (ImGui::BeginListBox("##GameRoundList", ImVec2(-1, -1))) {
			for (uint32_t i = 0; i < m_event.sizeRounds(); ++i) {
				const auto round = m_event.getGameRound(i);
				const std::string roundLabel = std::format("{} : {}", round->getName(), round->getTypeStr());
				if (ImGui::Selectable(roundLabel.c_str(), m_selectedGameRound == i)) {
					m_selectedGameRound = i;
					log_info("Selected game round {}", i);
				}
			}
			ImGui::EndListBox();
		}
	}
	ImGui::EndChild();
}

void GameRoundConfigPopups::renderSecondColumn() {
	const auto currentRound = m_event.getGameRound(static_cast<uint32_t>(m_selectedGameRound));

	if (currentRound == m_event.endRounds()) {
		if (ImGui::BeginChild("GroupSubRound", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
			ImGui::Text("Aucune partie sélectionnée");
		}
		ImGui::EndChild();
		return;
	}
	if (ImGui::BeginChild("GroupSubRound", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
		ImGui::Text("Phases de la partie");
		ImGui::Separator();

		// Type de partie
		ImGui::Text("Type de partie:");
		ImGui::SetNextItemWidth(-140);
		int currentGameRoundType = static_cast<int>(currentRound->getType()) - 1;// skipping display of id 0 == Invalid
		if (ImGui::Combo("##GameRoundTypes", &currentGameRoundType, getRoundTypes().c_str())) {
			currentRound->setType(static_cast<core::GameRound::Type>(currentGameRoundType + 1));
		}
		ImGui::SameLine();
		ImGui::Text("N°");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		int id = currentRound->getId();
		if (ImGui::InputInt("##RoundNumber", &id, 1, 100, ImGuiInputTextFlags_CharsDecimal)) {
			currentRound->setId(id);
		}

		ImGui::Spacing();

		// Liste des sous-parties
		if (ImGui::BeginListBox("##SubRoundList", ImVec2(-1, -1))) {
			for (size_t i = 0; i < currentRound->sizeSubRound(); ++i) {
				const auto subRound = currentRound->getSubRound(static_cast<uint32_t>(i));
				const std::string name = std::format("Phase {} : {}", i, subRound->getTypeStr());
				if (const bool isSelected = m_selectedSubRound == i; ImGui::Selectable(name.c_str(), isSelected)) {
					m_selectedSubRound = i;
				}
			}
			ImGui::EndListBox();
		}
	}
	ImGui::EndChild();
}

void GameRoundConfigPopups::renderThirdColumn() {

	auto& app = Application::get();

	static bool previewFullScreen = true;

	// Validate selected game round first
	if (m_selectedGameRound >= m_event.sizeRounds()) {
		if (ImGui::BeginChild("GroupPhase", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
			ImGui::Text("Aucune partie sélectionnée");
		}
		ImGui::EndChild();
		return;
	}

	const auto currentRound = m_event.getGameRound(static_cast<uint32_t>(m_selectedGameRound));
	if (currentRound == m_event.endRounds()) {
		if (ImGui::BeginChild("GroupPhase", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
			ImGui::Text("Partie invalide sélectionnée");
		}
		ImGui::EndChild();
		return;
	}

	if (ImGui::BeginChild("GroupPhase", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_None)) {
		ImGui::Text("Configuration de la phase:");
		ImGui::Separator();

		// Onglets selon le type
		if (currentRound->getType() != core::GameRound::Type::Pause) {
			// Validate sub-round selection
			if (m_selectedSubRound >= currentRound->sizeSubRound()) {
				ImGui::Text("Aucune phase sélectionnée");
				ImGui::EndChild();
				return;
			}
			// Partie normale
			const auto subRound = currentRound->getSubRound(static_cast<uint32_t>(m_selectedSubRound));

			ImGui::Text("Condition de victoire:");
			ImGui::SetNextItemWidth(-1);
			{
				std::string subRoundName = subRound->getTypeStr();
				ImGui::InputText("##SubRoundName", &subRoundName, ImGuiInputTextFlags_ReadOnly);
			}
			ImGuiInputFlags flags = ImGuiInputTextFlags_None;
			if (!subRound->isEditable())
				flags |= ImGuiInputTextFlags_ReadOnly;
			ImGui::Spacing();
			ImGui::Text("Valeur Totale");
			ImGui::SetNextItemWidth(-1);
			auto priceValue = static_cast<float>(subRound->getValue());
			if (ImGui::InputFloat("##PriceValue", &priceValue, 0.0f, 0.0f, "%.2f €", flags)) {
				subRound->define(subRound->getType(), subRound->getPrices(), static_cast<double>(priceValue));
			}

			ImGui::Spacing();
			ImGui::Text("Liste des lots:");
			std::string prices = subRound->getPrices();
			if (ImGui::InputTextMultiline("##TextPrices", &prices, ImVec2(-1, -80), flags)) {
				subRound->define(subRound->getType(), prices, subRound->getValue());
			}
		} else {
			// Pause
			bool pauseDiapo = currentRound->hasDiapo();
			if (const bool pauseNothing = !pauseDiapo; ImGui::RadioButton("Rien", pauseNothing)) {
				pauseDiapo = false;
				currentRound->setDiapo("", 0.0);
			}
			if (ImGui::RadioButton("Diaporama", pauseDiapo)) {
				pauseDiapo = true;
				if (!currentRound->hasDiapo()) {
					currentRound->setDiapo("", 5.0);
				}
			}

			if (pauseDiapo) {
				ImGui::Spacing();
				ImGui::Text("Répertoire de diapo");
				ImGui::SetNextItemWidth(-80);
				auto [folderPath, delay] = currentRound->getDiapo();
				auto f_path = folderPath.string();
				if (ImGui::InputText("##PauseDiapoFolder", &f_path)) {
					currentRound->setDiapo(folderPath.string(), delay);
				}
				ImGui::SameLine();
				if (ImGui::Button("...##BrowseDiapo")) {
					if (const auto path = utils::FileDialog::selectFolder(); !path.empty()) {
						currentRound->setDiapo(path.string(), delay);
					}
				}

				ImGui::Text("Délai par diapo");
				ImGui::SetNextItemWidth(-1);
				auto p_delay = static_cast<float>(delay);
				if (ImGui::InputFloat("##PauseDiapoDelay", &p_delay, 0.1f, 1.0f, "%.1f s")) {
					currentRound->setDiapo(folderPath.string(), static_cast<double>(p_delay));
				}
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Aperçu
		if (m_event.getStarting() == core::clock::time_point{}) {
			bool previewEnabled = app.getDisplayPreview();
			if (ImGui::Checkbox("Aperçu", &previewEnabled)) {
				// Toggle preview
				app.setDisplayPreview(previewEnabled);
			}
			ImGui::SameLine();
			ImGui::Checkbox("Plein écran", &previewFullScreen);
			if (previewEnabled) {
				const auto dv = std::static_pointer_cast<DisplayView>(app.getView("display_window"));
				dv->setFullscreen(previewFullScreen);
				dv->setEventToRender(m_event, m_selectedGameRound, m_selectedSubRound);
			}
		}
	}
	ImGui::EndChild();
}

void GameRoundConfigPopups::renderResult() {
	if (const float resultHeight = ImGui::GetContentRegionAvail().y - 50;
		ImGui::BeginChild("GroupResult", ImVec2(0, resultHeight), ImGuiChildFlags_Borders)) {

		ImGui::Text("Résultats de la partie");
		ImGui::Separator();
		// Validate selected game round
		if (m_selectedGameRound >= m_event.sizeRounds()) {
			ImGui::Text("Aucune partie sélectionnée");
			ImGui::EndChild();
			ImGui::Spacing();
			return;
		}
		const auto gr = m_event.getGameRound(static_cast<uint32_t>(m_selectedGameRound));

		ImGui::Columns(3, "ResultColumns", false);

		std::string start_date = "--";
		std::string end_date = "--";
		std::string duration = "--";
		std::string num_draws = "--";
		if (gr->isFinished()) {
			start_date = std::format("{}", gr->getStarting());
			end_date = std::format("{}", gr->getEnding());
			duration = std::format("{}", gr->getEnding() - gr->getStarting());
			num_draws = std::format("{}", gr->getAllDraws().size());
		}
		// Colonne 1: Dates et durée
		if (ImGui::BeginChild("ResultDates", ImVec2(0, 0), ImGuiChildFlags_None)) {
			ImGui::Text("Début:");
			ImGui::SameLine();
			ImGui::InputText("##StartDate", &start_date, ImGuiInputTextFlags_ReadOnly);
			ImGui::Text("Fin:");
			ImGui::SameLine();
			ImGui::InputText("##EndDate", &end_date, ImGuiInputTextFlags_ReadOnly);
			ImGui::Text("Durée:");
			ImGui::SameLine();
			ImGui::InputText("##Duration", &duration, ImGuiInputTextFlags_ReadOnly);
			ImGui::Text("Nombre de Tirages:");
			ImGui::SameLine();
			ImGui::InputText("##NumDraws", &num_draws, ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::EndChild();
		ImGui::NextColumn();

		// Colonne 2: Tirages
		if (ImGui::BeginChild("ResultDraws", ImVec2(0, 0), ImGuiChildFlags_None)) {
			ImGui::Text("Tirages:");
			std::string drawStr = gr->getDrawStr();
			ImGui::InputTextMultiline("##TextDraws", &drawStr, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::EndChild();
		ImGui::NextColumn();

		// Colonne 3: Gagnants
		if (ImGui::BeginChild("ResultWinners", ImVec2(0, 0), ImGuiChildFlags_None)) {
			ImGui::Text("Noms des gagnants:");
			std::string winnerStr = gr->getWinnerStr();
			ImGui::InputTextMultiline("##ListWinners", &winnerStr, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::EndChild();

		ImGui::Columns(1);
	}
	ImGui::EndChild();
	ImGui::Spacing();
}

void GameRoundConfigPopups::addGameRound() { m_event.pushGameRound(core::GameRound()); }

void GameRoundConfigPopups::deleteGameRound() {
	if (m_event.sizeRounds() == 0) {
		return;
	}
	m_event.deleteRoundByIndex(static_cast<uint16_t>(m_selectedGameRound));
	if (m_selectedGameRound >= m_event.sizeRounds()) {
		m_selectedGameRound = m_event.sizeRounds() - 1;
	}
}

void GameRoundConfigPopups::moveGameRoundUp() {
	if (m_event.sizeRounds() < 2 || m_selectedGameRound == 0) {
		return;
	}
	m_event.swapRoundByIndex(static_cast<uint16_t>(m_selectedGameRound),
							 static_cast<uint16_t>(m_selectedGameRound - 1));
	m_selectedGameRound--;
}

void GameRoundConfigPopups::moveGameRoundDown() {
	if (m_event.sizeRounds() < 2 || m_selectedGameRound >= m_event.sizeRounds() - 1) {
		return;
	}
	m_event.swapRoundByIndex(static_cast<uint16_t>(m_selectedGameRound),
							 static_cast<uint16_t>(m_selectedGameRound + 1));
	m_selectedGameRound++;
}

}// namespace evl::gui_imgui::views
