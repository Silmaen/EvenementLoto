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
#include "gui/Application.h"
#include "gui/utils/FileDialog.h"
#include "gui/utils/Layout.h"
#include "gui/utils/Rendering.h"

#include <cstring>
#include <imgui.h>
#include <imgui_stdlib.h>


namespace evl::gui::views {

namespace {

/// Colonne où commencent les champs, pour que les libellés s'alignent d'une page à l'autre.
auto labelWidth() -> float { return utils::em(10.0f); }

/// La place laissée à droite d'un champ pour son bouton « ... ».
auto browseRoom() -> float { return utils::em(2.5f); }

/// La hauteur que le pied de dialogue retire au contenu.
auto footerRoom() -> float { return utils::dialogFooterHeight() + ImGui::GetStyle().ItemSpacing.y; }

}// namespace

MainConfigPopups::MainConfigPopups() {
	settingsToData();
	dataToSettings();
}

MainConfigPopups::~MainConfigPopups() = default;

void MainConfigPopups::onOpen() { settingsToData(); }

void MainConfigPopups::onPopupUpdate() {
	// Des onglets plutôt que des groupes empilés à hauteur fixe : chaque page prend la
	// place qu'il lui faut, rien n'est coupé, et les réglages se rangent par sujet — ce
	// qui vaut pour l'outil, ce qui vaut pour son apparence, ce qui vaut pour l'écran
	// des joueurs.
	if (ImGui::BeginChild("ConfigPages", {0, ImGui::GetContentRegionAvail().y - footerRoom()}, ImGuiChildFlags_None)) {
		if (ImGui::BeginTabBar("ConfigTabs")) {
			if (ImGui::BeginTabItem("Général")) {
				renderGeneralTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Apparence")) {
				renderAppearanceTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Affichage joueurs")) {
				renderPlayerDisplayTab();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::EndChild();

	utils::beginDialogFooter(5);
	if (utils::dialogButton("Défauts"))
		m_data = {};
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Revenir aux valeurs d'origine de tous les réglages.");
	ImGui::SameLine();
	if (utils::dialogButton("Restaurer"))
		settingsToData();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Oublier les modifications en cours.");
	ImGui::SameLine();
	if (utils::dialogButton("Ok")) {
		dataToSettings();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (utils::dialogButton("Appliquer"))
		dataToSettings();
	ImGui::SameLine();
	if (utils::dialogButton("Annuler"))
		ImGui::CloseCurrentPopup();
}

void MainConfigPopups::renderGeneralTab() {
	ImGui::SeparatorText("Répertoires par défaut");
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Données");
	ImGui::SameLine(labelWidth());
	ImGui::SetNextItemWidth(-browseRoom());
	std::string originalPath = m_data.dataLocation.string();
	if (ImGui::InputText("##DataLocation", &originalPath))
		m_data.dataLocation = originalPath;
	ImGui::SameLine();
	if (ImGui::Button("...##SearchFolder")) {
		utils::FileDialog::selectFolder(
				[this](const std::filesystem::path& iPath) -> void { m_data.dataLocation = iPath; });
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Parcourir...");

	ImGui::SeparatorText("Tirage");
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Délai de réactivation");
	ImGui::SameLine(labelWidth());
	ImGui::SetNextItemWidth(utils::em(7.0f));
	ImGui::DragFloat("##DrawDelay", &m_data.drawDelay, 0.1f, 0.0f, 30.0f, "%.1f s");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Temps pendant lequel les commandes de tirage restent bloquées après un\n"
						  "numéro, pour donner le tempo. Zéro désactive le blocage.");

	ImGui::SeparatorText("Programme");
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Durée d'une pause");
	ImGui::SameLine(labelWidth());
	ImGui::SetNextItemWidth(utils::em(7.0f));
	ImGui::DragFloat("##PauseDuration", &m_data.pauseDuration, 0.5f, 0.0f, 120.0f, "%.0f min");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Sert à estimer l'heure de fin, et à annoncer aux joueurs l'heure de reprise.\n"
						  "Zéro : aucune heure de reprise n'est affichée.");

#ifdef EVL_PLATFORM_LINUX
	// Le choix n'a de sens que sous Linux, et seulement tant que Wayland ne permet pas
	// d'envoyer l'affichage sur un second écran : il est donc présenté avec sa
	// conséquence, pas comme une préférence anodine.
	ImGui::SeparatorText("Serveur d'affichage");
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Serveur demandé");
	ImGui::SameLine(labelWidth());
	ImGui::SetNextItemWidth(utils::em(7.0f));
	{
		constexpr std::array<const char*, 3> values{{"x11", "wayland", "auto"}};
		int selected = 0;
		for (size_t i = 0; i < values.size(); ++i) {
			if (m_data.displayServer == values[i])
				selected = static_cast<int>(i);
		}
		if (ImGui::Combo("##DisplayServer", &selected, "X11\0Wayland\0Automatique\0\0"))
			m_data.displayServer = values[static_cast<size_t>(selected)];
	}
	ImGui::TextWrapped("Sous X11, XWayland compris, la fenêtre d'affichage peut être envoyée en plein écran sur le "
					   "vidéoprojecteur. Le protocole Wayland ne le permet pas : une application n'y choisit pas où "
					   "ses fenêtres s'ouvrent. X11 reste donc le choix par défaut.");
	ImGui::TextDisabled("Prend effet au prochain démarrage.");
#endif
}

void MainConfigPopups::renderAppearanceTab() {
	ImGui::SeparatorText("Habillage des fenêtres");
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Habillage");
	ImGui::SameLine(labelWidth());
	ImGui::SetNextItemWidth(utils::em(10.0f));
	{
		int selected = static_cast<int>(m_data.themePreset);
		std::string names;
		for (const auto& candidate: magic_enum::enum_values<Theme::Preset>()) {
			names += Theme::presetName(candidate);
			names += '\0';
		}
		names += '\0';
		if (ImGui::Combo("##ThemePreset", &selected, names.c_str()))
			m_data.themePreset = static_cast<Theme::Preset>(selected);
	}
	ImGui::SameLine();
	if (ImGui::Button("Réappliquer"))
		m_data.resetTheme = true;
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Revenir aux couleurs de l'habillage, en oubliant les retouches.");
	ImGui::TextDisabled("Pris en compte avec « Appliquer » ou « Ok ».");

	ImGui::SeparatorText("Police d'interface");
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Fichier");
	ImGui::SameLine(labelWidth());
	ImGui::SetNextItemWidth(-(browseRoom() + ImGui::CalcTextSize("Défaut").x + ImGui::GetStyle().FramePadding.x * 2.0f +
							  ImGui::GetStyle().ItemSpacing.x));
	std::string fontPath = m_data.fontPath.string();
	if (ImGui::InputTextWithHint("##FontPath", "police embarquée", &fontPath))
		m_data.fontPath = fontPath;
	ImGui::SameLine();
	if (ImGui::Button("...##SearchFont")) {
		utils::FileDialog::openFile("Police|ttf,otf,ttc",
									[this](const std::filesystem::path& iPath) -> void { m_data.fontPath = iPath; });
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Parcourir...");
	ImGui::SameLine();
	if (ImGui::Button("Défaut##Font"))
		m_data.fontPath.clear();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Revenir à la police embarquée.");
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Taille");
	ImGui::SameLine(labelWidth());
	ImGui::SetNextItemWidth(utils::em(7.0f));
	ImGui::DragFloat("##FontSize", &m_data.fontSize, 0.5f, 10.0f, 48.0f, "%.0f px");
}

void MainConfigPopups::renderPlayerDisplayTab() {
	const auto style = ImGui::GetStyle();
	ImGui::TextWrapped("Ce qui suit ne concerne que l'écran vu par les joueurs.");
	ImGui::Spacing();

	// Measured before the columns open: inside them the region is the first column's,
	// and taking 12 em off it at every frame squeezed the labels to nothing.
	const float labelsWidth = std::max(ImGui::GetContentRegionAvail().x - utils::em(12.0f), utils::em(8.0f));
	ImGui::Columns(2, "ConfigColumns");
	ImGui::SetColumnWidth(0, labelsWidth);

	// Échelles
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Facteur d'échelle de titre");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::DragFloat("##TitleScale", &m_data.titleScale, 0.1f, 1.0f, 10.0f, "%.1f");
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Échelle texte grille");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::DragFloat("##GridTextScale", &m_data.gridTextScale, 0.1f, 0.3f, 1.5f, "%.1f");
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Facteur d'échelle prix");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::DragFloat("##ValueScale", &m_data.valueScale, 0.1f, 0.5f, 10.0f, "%.1f");
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Facteur d'échelle texte lots");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::DragFloat("##PriceTextScale", &m_data.priceTextScale, 0.1f, 0.5f, 10.0f, "%.1f");
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Facteur d'échelle horloges");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::DragFloat("##TimeScale", &m_data.timeScale, 0.1f, 0.5f, 5.0f, "%.1f");
	ImGui::NextColumn();

	ImGui::Separator();

	// Couleurs
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Couleur de l'arrière plan");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::ColorEdit3("##BackgroundColor", reinterpret_cast<float*>(&m_data.backgroundColor),
					  ImGuiColorEditFlags_NoInputs);
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Couleur du texte");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::ColorEdit3("##TextColor", reinterpret_cast<float*>(&m_data.textColor), ImGuiColorEditFlags_NoInputs);
	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Arrière plan de la grille");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::ColorEdit3("##GridBackgroundColor", reinterpret_cast<float*>(&m_data.gridBackgroundColor),
					  ImGuiColorEditFlags_NoInputs);
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Espacement de la grille");
	ImGui::NextColumn();
	ImGui::BeginGroup();
	const float gridSize = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) * 0.5f;
	ImGui::SetNextItemWidth(gridSize);
	ImGui::DragFloat("##GridSpaceX", &m_data.gridSpace.x(), 0.1f, 0.0f, 10.f, "%.1f");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(gridSize);
	ImGui::DragFloat("##GridSpaceY", &m_data.gridSpace.y(), 0.1f, 0.0f, 10.f, "%.1f");

	ImGui::EndGroup();

	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Tronquer lignes de lots");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::Checkbox("##TruncatePrice", &m_data.truncatePrice);
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Max lignes de lots");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::DragInt("##TruncatePriceLines", &m_data.truncatePriceLines, 1.0f, 1, 15);
	ImGui::NextColumn();

	ImGui::Separator();

	// Numéros sélectionnés
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Couleur chiffre sélectionné");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::ColorEdit3("##SelectedNumberColor", reinterpret_cast<float*>(&m_data.selectedNumberColor),
					  ImGuiColorEditFlags_NoInputs);
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Fondu de couleur");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::Checkbox("##FadeNumbers", &m_data.fadeNumbers);
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Nombre de chiffres fondus");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::DragInt("##FadeAmount", &m_data.fadeAmount, 1.0f, 0, 10);
	ImGui::NextColumn();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Force du fondu");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::DragFloat("##FadeStrength", &m_data.fadeStrength, 0.1f, -2, 2, "%.1f");
	ImGui::NextColumn();

	ImGui::Columns(1);
}

void MainConfigPopups::dataToSettings() {
	core::Settings settings;
	settings.setValue("draw_delay", m_data.drawDelay);
	settings.setValue("pause_duration", m_data.pauseDuration);
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
#ifdef EVL_PLATFORM_LINUX
	// Ailleurs il n'y a rien à choisir, et écrire la clé donnerait l'illusion du
	// contraire au prochain démarrage.
	settings.setValue("display_server", m_data.displayServer);
#endif
	core::getSettings()->include(settings, "gui");
	core::getSettings()->setValue("general/data_location", std::string{m_data.dataLocation.string()});
	// La police n'est pas qu'un réglage : elle demande la reconstruction de l'atlas, qui
	// aura lieu au début de l'image suivante.
	Application::get().setFont(m_data.fontPath, m_data.fontSize);
	// The look changes here and nowhere else: choosing in the list only prepares it. Left
	// alone when nothing was asked, so the colours edited since are not reset.
	if (const auto current = Application::get().getTheme().preset; m_data.resetTheme || m_data.themePreset != current) {
		Application::get().setTheme(Theme::fromPreset(m_data.themePreset));
		log_info("Habillage appliqué : {}", Theme::presetName(m_data.themePreset));
	}
	m_data.resetTheme = false;
}

void MainConfigPopups::settingsToData() {
	const auto settings = core::getSettings()->extract("gui");
	const Data defaults{};

	m_data.dataLocation =
			core::getSettings()->getValue<std::string>("general/data_location", defaults.dataLocation.string());
	m_data.displayServer = settings.getValue<std::string>("display_server", defaults.displayServer);
	m_data.fontPath = settings.getValue<std::string>("font_path", defaults.fontPath.string());
	m_data.fontSize = settings.getValue<float>("font_size", defaults.fontSize);
	m_data.themePreset = Application::get().getTheme().preset;
	m_data.resetTheme = false;
	m_data.drawDelay = settings.getValue<float>("draw_delay", defaults.drawDelay);
	m_data.pauseDuration = settings.getValue<float>("pause_duration", defaults.pauseDuration);
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
	const bool readOnly = (tags & ImGuiInputTextFlags_ReadOnly) != 0;
	const auto field = [](const char* iLabel) -> void {
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(iLabel);
		ImGui::SameLine(labelWidth());
	};

	ImGui::SeparatorText("Événement");
	field("Nom");
	ImGui::SetNextItemWidth(-1);
	if (std::string eventName = m_event.getName(); ImGui::InputText("##EventName", &eventName, tags))
		m_event.setName(eventName);
	field("Lieu");
	ImGui::SetNextItemWidth(-1);
	if (std::string eventLocation = m_event.getLocation(); ImGui::InputText("##EventLocation", &eventLocation, tags))
		m_event.setLocation(eventLocation);
	field("Logo");
	ImGui::SetNextItemWidth(-browseRoom());
	if (std::string eventLogo = m_event.getLogo().string(); ImGui::InputText("##EventLogo", &eventLogo, tags))
		m_event.setLogo(eventLogo);
	ImGui::SameLine();
	if (ImGui::Button("...##SearchEventLogo", {-1, 0}) && !readOnly) {
		utils::FileDialog::openFile(utils::g_imageFilter,
									[this](const std::filesystem::path& iPath) -> void { m_event.setLogo(iPath); });
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Parcourir...");

	field("Fin prévue");
	{
		// Une heure du jour : c'est la référence du retard affiché pendant le jeu.
		// Still adjustable once the event has started: a programme that slips is
		// exactly when one wants to set a new target.
		auto planned = m_event.getPlannedEnd();
		bool hasPlanned = planned.has_value();
		const bool frozen = m_event.isFinished();
		if (frozen)
			ImGui::BeginDisabled();
		if (ImGui::Checkbox("##HasPlannedEnd", &hasPlanned))
			m_event.setPlannedEnd(hasPlanned ? std::optional{std::chrono::minutes{18 * 60}} : std::nullopt);
		if (hasPlanned && planned.has_value()) {
			int hours = static_cast<int>(planned->count() / 60);
			int minutes = static_cast<int>(planned->count() % 60);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(utils::em(4.5f));
			bool changed = ImGui::DragInt("##PlannedHour", &hours, 0.1f, 0, 23, "%02d h");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(utils::em(4.5f));
			changed |= ImGui::DragInt("##PlannedMinute", &minutes, 0.2f, 0, 59, "%02d min");
			if (changed)
				m_event.setPlannedEnd(std::chrono::minutes{std::clamp(hours, 0, 23) * 60 + std::clamp(minutes, 0, 59)});
		} else {
			ImGui::SameLine();
			ImGui::TextDisabled("aucune : pas de retard calculé");
		}
		if (frozen)
			ImGui::EndDisabled();
	}

	ImGui::SeparatorText("Organisateur");
	field("Nom");
	ImGui::SetNextItemWidth(-1);
	if (std::string orgaName = m_event.getOrganizerName(); ImGui::InputText("##OrgaName", &orgaName, tags))
		m_event.setOrganizerName(orgaName);
	field("Logo");
	ImGui::SetNextItemWidth(-browseRoom());
	if (std::string orgaLogo = m_event.getOrganizerLogo().string(); ImGui::InputText("##OrgaLogo", &orgaLogo, tags))
		m_event.setOrganizerLogo(orgaLogo);
	ImGui::SameLine();
	if (ImGui::Button("...##SearchOrgaLogo", {-1, 0}) && !readOnly) {
		utils::FileDialog::openFile(utils::g_imageFilter, [this](const std::filesystem::path& iPath) -> void {
			m_event.setOrganizerLogo(iPath.string());
		});
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Parcourir...");

	ImGui::SeparatorText("Règlement");
	// Le règlement prend toute la place qui reste : c'est le seul texte long de la page.
	const float rulesHeight = std::max(
			ImGui::GetContentRegionAvail().y - footerRoom() - ImGui::GetFrameHeightWithSpacing(), utils::em(4.0f));
	if (std::string rules = m_event.getRules();
		ImGui::InputTextMultiline("##Rules", &rules, {-1, rulesHeight}, tags | ImGuiInputTextFlags_WordWrap)) {
		m_event.setRules(rules);
	}
	if (utils::dialogButton("Importer##Rules") && !readOnly) {
		utils::FileDialog::openFile(utils::g_yamlFilter, [this](const std::filesystem::path& iPath) -> void {
			try {
				if (YAML::Node node = YAML::LoadFile(iPath.string()); node["rules"]) {
					m_event.setRules(node["rules"].as<std::string>());
				} else {
					log_warn("No 'rules' field found in file: {}", iPath.string());
				}
			} catch (const YAML::Exception& e) { log_error("Failed to load YAML file: {}", e.what()); }
		});
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Lire le règlement depuis un fichier YAML.");
	ImGui::SameLine();
	if (utils::dialogButton("Exporter##Rules")) {
		utils::FileDialog::saveFile(utils::g_yamlFilter, [this](const std::filesystem::path& iPath) -> void {
			try {
				YAML::Node node;
				node["rules"] = m_event.getRules();
				if (std::ofstream fout(iPath); !fout.is_open()) {
					log_error("Failed to open file for writing: {}", iPath.string());
				} else {
					fout << node;
					fout.close();
				}
			} catch (const YAML::Exception& e) { log_error("Failed to save YAML file: {}", e.what()); }
		});
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Enregistrer le règlement dans un fichier YAML.");

	utils::beginDialogFooter(3);
	if (utils::dialogButton("Ok")) {
		toCurrentEvent();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (utils::dialogButton("Appliquer"))
		toCurrentEvent();
	ImGui::SameLine();
	if (utils::dialogButton("Annuler"))
		ImGui::CloseCurrentPopup();
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

	static float s_edit_ratio = 0.72f;
	const ImVec2 contentAvail{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - footerRoom()};
	const ImGuiStyle& style = ImGui::GetStyle();

	// Splitter sizing
	const float splitterThickness = style.ItemSpacing.y;
	const float minEditHeight = utils::em(8.0f);
	const float minResultHeight = utils::em(6.0f);
	float editHeight = contentAvail.y * s_edit_ratio;
	float resultHeight = contentAvail.y - editHeight - splitterThickness - style.ItemSpacing.y * 2.0f;

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
			// Each column carries its own title: a header row would only repeat it.
			ImGui::TableSetupColumn("Parties", ImGuiTableColumnFlags_WidthStretch, 1.0f);
			ImGui::TableSetupColumn("Phases", ImGuiTableColumnFlags_WidthStretch, 0.9f);
			ImGui::TableSetupColumn("Phase", ImGuiTableColumnFlags_WidthStretch, 1.6f);

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
	if (ImGui::BeginChild("GroupResult", ImVec2(0, std::max(resultHeight, minResultHeight)), ImGuiChildFlags_Borders))
		renderResult();
	ImGui::EndChild();

	utils::beginDialogFooter(5);
	if (utils::dialogButton("Importer")) {
		utils::FileDialog::openFile(utils::g_yamlFilter, [this](const std::filesystem::path& iPath) -> void {
			if (m_event.importYaml(iPath)) {
				m_selectedGameRound = 0;
				m_selectedSubRound = 0;
				return;
			}
			Application::get().tell("Import", "Ce fichier n'est pas un événement lisible.", iPath.string());
		});
	}
	ImGui::SameLine();
	if (utils::dialogButton("Exporter")) {
		utils::FileDialog::saveFile(utils::g_yamlFilter, [this](const std::filesystem::path& iPath) -> void {
			if (!m_event.exportYaml(iPath))
				Application::get().tell("Export", "L'événement n'a pas pu être exporté.", iPath.string());
		});
	}
	ImGui::SameLine();
	if (utils::dialogButton("Ok")) {
		toCurrentEvent();
		onClose();
	}
	ImGui::SameLine();
	if (utils::dialogButton("Appliquer"))
		toCurrentEvent();
	ImGui::SameLine();
	if (utils::dialogButton("Annuler"))
		onClose();
}

void GameRoundConfigPopups::fromCurrentEvent() {
	// Load data from current event
	m_event = Application::get().getCurrentEvent();
}

void GameRoundConfigPopups::toCurrentEvent() const {
	// Save data to current event
	Application::get().getCurrentEvent() = m_event;
	// Un lot saisi ici est un lot dont on dispose : il rejoint le catalogue, faute de
	// quoi il n'apparaîtrait ni dans la liste ni dans la répartition.
	Application::get().getCurrentEvent().adoptOrphanPrizes();
}

void GameRoundConfigPopups::renderFirstColumn() {
	if (ImGui::BeginChild("GroupGameRound", ImVec2(0, 0), ImGuiChildFlags_Borders)) {
		ImGui::TextUnformatted("Parties");
		ImGui::Separator();

		// Boutons de gestion
		if (ImGui::Button("+##AddRound")) { /* Action add round */
			addGameRound();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Ajouter une partie");
		ImGui::SameLine();
		if (ImGui::Button("-##DelRound")) { /* Action delete round */
			deleteGameRound();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Supprimer la partie");
		ImGui::SameLine();
		if (ImGui::Button("^##UpRound")) { /* Action move up */
			moveGameRoundUp();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Monter la partie");
		ImGui::SameLine();
		if (ImGui::Button("v##DownRound")) { /* Action move down */
			moveGameRoundDown();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Descendre la partie");

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
		ImGui::TextUnformatted("Phases de la partie");
		ImGui::Separator();

		// Type de partie
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Type");
		ImGui::SameLine(utils::em(4.0f));
		ImGui::SetNextItemWidth(-1);
		int currentGameRoundType = static_cast<int>(currentRound->getType()) - 1;// skipping display of id 0 == Invalid
		if (ImGui::Combo("##GameRoundTypes", &currentGameRoundType, getRoundTypes().c_str())) {
			const bool wasPause = currentRound->isPause();
			currentRound->setType(static_cast<core::GameRound::Type>(currentGameRoundType + 1));
			// Passer d'une partie à une pause, ou l'inverse, change de numérotation : le
			// numéro d'avant appartient à l'autre série et vaudrait doublon. Il est
			// d'abord rendu — la partie n'a plus de numéro dans aucune des deux séries —
			// pour ne pas se compter elle-même dans le calcul du suivant.
			if (currentRound->isPause() != wasPause) {
				currentRound->setId(0);
				currentRound->setId(m_event.nextFreeRoundId(currentRound->isPause()));
			}
		}
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("N°");
		ImGui::SameLine(utils::em(4.0f));
		ImGui::SetNextItemWidth(-1);
		int id = currentRound->getId();
		if (ImGui::InputInt("##RoundNumber", &id, 1, 100, ImGuiInputTextFlags_CharsDecimal)) {
			currentRound->setId(id);
		}
		bool hideNumber = currentRound->isNumberHidden();
		if (ImGui::Checkbox("Numéro caché aux joueurs", &hideNumber))
			currentRound->setNumberHidden(hideNumber);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Pour une partie hors numérotation, la partie enfant par exemple : le numéro reste\n"
							  "celui que désigne un tableur importé, mais l'écran des joueurs ne l'affiche pas.");
		bool showPrizes = currentRound->isShowingPrizes();
		if (ImGui::Checkbox("Afficher les lots aux joueurs", &showPrizes))
			currentRound->setShowPrizes(showPrizes);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Décoché, la page des lots s'affiche comme d'habitude, mais sans être remplie.");

		if (currentRound->getType() == core::GameRound::Type::Enfant) {
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Gagnants");
			ImGui::SameLine(utils::em(4.0f));
			ImGui::SetNextItemWidth(-1);
			int winners = static_cast<int>(currentRound->getSubRound(0)->getWinnerTarget());
			if (ImGui::InputInt("##ChildWinners", &winners, 1, 5))
				currentRound->setChildWinnerCount(static_cast<uint32_t>(std::max(winners, 1)));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Une seule phase, plusieurs gagnants : le premier enfant qui remplit une ligne gagne\n"
								  "le premier lot et sort du jeu, puis le deuxième le lot suivant, et ainsi de suite.\n"
								  "Les noms ne sont pas notés : prévoir autant de lots que de gagnants.");
		}

		ImGui::Spacing();

		// Liste des sous-parties
		if (ImGui::BeginListBox("##SubRoundList", ImVec2(-1, -1))) {
			for (size_t i = 0; i < currentRound->sizeSubRound(); ++i) {
				const std::string name = currentRound->getType() == core::GameRound::Type::Enfant
												 ? currentRound->getSubRoundLabel(i)
												 : std::format("Phase {} : {}", i + 1, currentRound->getSubRoundLabel(i));
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
		ImGui::TextUnformatted("Phase sélectionnée");
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

			ImGui::TextDisabled("Condition de victoire");
			ImGui::SetNextItemWidth(-1);
			{
				std::string subRoundName = subRound->getTypeStr();
				ImGui::InputText("##SubRoundName", &subRoundName, ImGuiInputTextFlags_ReadOnly);
			}
			const bool editable = subRound->isEditable();
			ImGui::Spacing();
			ImGui::Text("Valeur totale : %.2f €", subRound->getValue());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Somme des valeurs des articles ci-dessous.");

			ImGui::Spacing();
			ImGui::TextDisabled("Lots à gagner");
			auto prizes = subRound->getPrizes();
			if (utils::renderPrizeList(
						prizes,
						{.editable = editable,
						 .size = {0, -(ImGui::GetFrameHeightWithSpacing() + utils::em(1.5f))},
						 .withSets = currentRound->getType() == core::GameRound::Type::Enfant}) &&
				editable)
				subRound->setPrizes(prizes);
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
				ImGui::SetNextItemWidth(-browseRoom());
				auto [folderPath, delay] = currentRound->getDiapo();
				auto f_path = folderPath.string();
				if (ImGui::InputText("##PauseDiapoFolder", &f_path)) {
					currentRound->setDiapo(f_path, delay);
				}
				ImGui::SameLine();
				if (ImGui::Button("...##BrowseDiapo")) {
					utils::FileDialog::selectFolder([this, delay](const std::filesystem::path& iPath) -> void {
						// The round is resolved again: the iterator held this frame would
						// be stale by the time the organizer has picked a directory.
						if (const auto round = m_event.getGameRound(static_cast<uint32_t>(m_selectedGameRound));
							round != m_event.endRounds())
							round->setDiapo(iPath.string(), delay);
					});
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Parcourir...");

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
	ImGui::TextUnformatted("Résultats de la partie");
	ImGui::Separator();
	if (m_selectedGameRound >= m_event.sizeRounds()) {
		ImGui::TextDisabled("Aucune partie sélectionnée");
		return;
	}
	const auto gr = m_event.getGameRound(static_cast<uint32_t>(m_selectedGameRound));
	if (!gr->isFinished()) {
		ImGui::TextDisabled("La partie n'a pas encore été jouée.");
		return;
	}
	if (!ImGui::BeginTable("ResultColumns", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV,
						   {-1, -1}))
		return;
	ImGui::TableSetupColumn("Chiffres", ImGuiTableColumnFlags_WidthStretch, 1.0f);
	ImGui::TableSetupColumn("Tirages", ImGuiTableColumnFlags_WidthStretch, 1.2f);
	ImGui::TableSetupColumn("Gagnants", ImGuiTableColumnFlags_WidthStretch, 1.2f);
	ImGui::TableNextRow();

	ImGui::TableSetColumnIndex(0);
	const auto figure = [](const char* iLabel, const std::string& iValue) -> void {
		ImGui::TextDisabled("%s", iLabel);
		ImGui::SameLine(utils::em(5.0f));
		ImGui::TextUnformatted(iValue.c_str());
	};
	figure("Début", std::format("{}", gr->getStarting()));
	figure("Fin", std::format("{}", gr->getEnding()));
	figure("Durée", std::format("{}", gr->getEnding() - gr->getStarting()));
	figure("Tirages", std::format("{}", gr->getAllDraws().size()));

	ImGui::TableSetColumnIndex(1);
	ImGui::TextDisabled("Tirages");
	std::string drawStr = gr->getDrawStr();
	ImGui::InputTextMultiline("##TextDraws", &drawStr, {-1, -1},
							  ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_WordWrap);

	ImGui::TableSetColumnIndex(2);
	ImGui::TextDisabled("Gagnants");
	std::string winnerStr = gr->getWinnerStr();
	ImGui::InputTextMultiline("##ListWinners", &winnerStr, {-1, -1}, ImGuiInputTextFlags_ReadOnly);
	ImGui::EndTable();
}

void GameRoundConfigPopups::addGameRound() {
	// Numérotée d'office : saisir le numéro à la main à chaque ajout était une corvée,
	// et un doublon passait sans bruit.
	core::GameRound round;
	round.setId(m_event.nextFreeRoundId(round.isPause()));
	m_event.pushGameRound(round);
	// La nouvelle partie devient la sélection : c'est celle qu'on vient d'ajouter que
	// l'on veut régler.
	m_selectedGameRound = m_event.sizeRounds() - 1;
	m_selectedSubRound = 0;
}

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


}// namespace evl::gui::views
