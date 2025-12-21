/**
 * @file ConfigPopups.cpp
 * @author Silmaen
 * @date 20/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "ConfigPopups.h"

#include "core/Settings.h"
#include "core/utilities.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

MainConfigPopups::MainConfigPopups() = default;

MainConfigPopups::~MainConfigPopups() = default;

void MainConfigPopups::onPopupUpdate() {
	static char dataLocation[256] = "";
	static char themeName[256] = "";
	static float mainScale = 0.020f;
	static float titleScale = 2.0f;
	static float shortTextScale = 1.4f;
	static float longTextScale = 0.6f;
	static float gridTextScale = 0.85f;
	static auto backgroundColor = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);
	static auto gridBackgroundColor = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);
	static auto textColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	static auto selectedNumberColor = ImVec4(1.0f, 0.44f, 0.0f, 1.0f);
	static bool truncatePrice = false;
	static int truncatePriceLines = 3;
	static bool fadeNumbers = true;
	static int fadeAmount = 3;
	static int fadeStrength = 0;

	// Répertoires par défaut
	if (ImGui::BeginChild("DefaultDirectories", ImVec2(0, 80), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Répertoires par défaut");
		ImGui::Separator();
		ImGui::Text("Données:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-80);
		ImGui::InputText("##DataLocation", dataLocation, 256);
		ImGui::SameLine();
		if (ImGui::Button("...##SearchFolder")) {
			// Action recherche dossier
		}
	}
	ImGui::EndChild();

	// Calculer la hauteur disponible pour la section Thème
	constexpr float buttonSectionHeight = 50.0f;// Hauteur pour les boutons Ok/Appliquer/Annuler

	// Thème
	if (const float themeHeight = ImGui::GetContentRegionAvail().y - buttonSectionHeight;
		ImGui::BeginChild("Theme", ImVec2(0, themeHeight), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Thème");
		ImGui::Separator();

		// Thème actuel
		ImGui::Text("Thème actuel:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-80);
		ImGui::InputText("##ThemeName", themeName, 256);
		ImGui::SameLine();
		if (ImGui::Button("Importer")) {
			// Action import
		}
		ImGui::SameLine();
		if (ImGui::Button("Exporter")) {
			// Action export
		}

		ImGui::Spacing();

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
			ImGui::DragFloat("##MainScale", &mainScale, 0.001f, 0.01f, 0.05f, "%.3f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle de titre");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##TitleScale", &titleScale, 0.1f, 1.0f, 5.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle texte court");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##ShortTextScale", &shortTextScale, 0.1f, 0.2f, 3.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle texte long");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##LongTextScale", &longTextScale, 0.1f, 0.2f, 3.0f, "%.1f");
			ImGui::NextColumn();

			ImGui::Text("Facteur d'échelle de la grille");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragFloat("##GridTextScale", &gridTextScale, 0.01f, 0.2f, 2.0f, "%.2f");
			ImGui::NextColumn();

			ImGui::Separator();

			// Couleurs
			ImGui::Text("Couleur de l'arrière plan");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##BackgroundColor", reinterpret_cast<float*>(&backgroundColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Couleur de l'arrière plan de la grille");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##GridBackgroundColor", reinterpret_cast<float*>(&gridBackgroundColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Tronquer lignes de lots");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::Checkbox("##TruncatePrice", &truncatePrice);
			ImGui::NextColumn();

			ImGui::Text("Nombre maximum de lignes de lots");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragInt("##TruncatePriceLines", &truncatePriceLines, 1.0f, 1, 15);
			ImGui::NextColumn();

			ImGui::Text("Couleur du texte");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##TextColor", reinterpret_cast<float*>(&textColor), ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Separator();

			// Numéros sélectionnés
			ImGui::Text("Couleur chiffre sélectionné");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::ColorEdit3("##SelectedNumberColor", reinterpret_cast<float*>(&selectedNumberColor),
							  ImGuiColorEditFlags_NoInputs);
			ImGui::NextColumn();

			ImGui::Text("Activation de la fondu de couleur");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::Checkbox("##FadeNumbers", &fadeNumbers);
			ImGui::NextColumn();

			ImGui::Text("Fondu sur combien de chiffres");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragInt("##FadeAmount", &fadeAmount, 1.0f, 0, 10);
			ImGui::NextColumn();

			ImGui::Text("Force de la fondue");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1);
			ImGui::DragInt("##FadeStrength", &fadeStrength, 4.0f, -100, 100);
			ImGui::NextColumn();

			ImGui::Columns(1);
		}
		ImGui::EndChild();

		ImGui::Spacing();
		// Boutons Reset/Restore
		constexpr float buttonWidth = 100.0f;
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float totalWidth = buttonWidth * 2 + spacing;
		const float offset = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
		if (ImGui::Button("Reset", ImVec2(buttonWidth, 0))) {
			// Action reset
		}
		ImGui::SameLine();
		if (ImGui::Button("Restorer", ImVec2(buttonWidth, 0))) {
			// Action restore
		}
	}
	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Boutons Ok/Appliquer/Annuler
	constexpr float buttonWidth = 100.0f;
	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float totalWidth = buttonWidth * 3 + spacing * 2;
	const float offset = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
	if (ImGui::Button("Ok", ImVec2(buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Appliquer", ImVec2(buttonWidth, 0))) {
		// Action apply
	}
	ImGui::SameLine();
	if (ImGui::Button("Annuler", ImVec2(buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
}

EventConfigPopups::EventConfigPopups() = default;

EventConfigPopups::~EventConfigPopups() = default;

void EventConfigPopups::onPopupUpdate() {
	// Variables locales temporaires
	static char eventName[256] = "";
	static char eventLocation[256] = "";
	static char eventLogo[256] = "";
	static char orgaName[256] = "";
	static char orgaLogo[256] = "";
	static char rules[4096] = "";

	// Configuration événement
	if (ImGui::BeginChild("EventConfig", ImVec2(0, 140), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Configuration événement");
		ImGui::Separator();

		ImGui::Columns(2, "EventColumns");
		ImGui::SetColumnWidth(0, 200);

		ImGui::Text("Nom de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1);
		ImGui::InputText("##EventName", eventName, 256);
		ImGui::NextColumn();

		ImGui::Text("Lieu de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-1);
		ImGui::InputText("##EventLocation", eventLocation, 256);
		ImGui::NextColumn();

		ImGui::Text("Logo de l'événement:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-80);
		ImGui::InputText("##EventLogo", eventLogo, 256);
		ImGui::SameLine();
		if (ImGui::Button("...##SearchEventLogo")) {
			// Action recherche logo événement
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
		ImGui::InputText("##OrgaName", orgaName, 256);
		ImGui::NextColumn();

		ImGui::Text("Logo de l'organisateur:");
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(-80);
		ImGui::InputText("##OrgaLogo", orgaLogo, 256);
		ImGui::SameLine();
		if (ImGui::Button("...##SearchOrgaLogo")) {
			// Action recherche logo organisateur
		}
		ImGui::NextColumn();

		ImGui::Columns(1);
	}
	ImGui::EndChild();

	// Calculer la hauteur disponible pour la section Règlement
	constexpr float buttonSectionHeight = 50.0f;

	// Règlement
	if (const float rulesHeight = ImGui::GetContentRegionAvail().y - buttonSectionHeight;
		ImGui::BeginChild("Rules", ImVec2(0, rulesHeight), ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("Règlement");
		ImGui::Separator();

		const float rulesTextHeight = ImGui::GetContentRegionAvail().y - 40;
		ImGui::InputTextMultiline("##Rules", rules, 4096, ImVec2(-1, rulesTextHeight));

		ImGui::Spacing();

		// Boutons Import/Export
		constexpr float buttonWidth = 100.0f;
		if (ImGui::Button("Import", ImVec2(buttonWidth, 0))) {
			// Action import règlement
		}
		ImGui::SameLine();
		const float exportOffset = ImGui::GetContentRegionAvail().x - buttonWidth;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + exportOffset);
		if (ImGui::Button("Export", ImVec2(buttonWidth, 0))) {
			// Action export règlement
		}
	}
	ImGui::EndChild();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Boutons Ok/Appliquer/Annuler
	constexpr float buttonWidth = 100.0f;
	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float totalWidth = buttonWidth * 3 + spacing * 2;
	const float offset = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
	if (ImGui::Button("Ok", ImVec2(buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Appliquer", ImVec2(buttonWidth, 0))) {
		// Action apply
	}
	ImGui::SameLine();
	if (ImGui::Button("Annuler", ImVec2(buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
}

GameRoundConfigPopups::GameRoundConfigPopups() = default;

GameRoundConfigPopups::~GameRoundConfigPopups() = default;

void GameRoundConfigPopups::onPopupUpdate() {}

}// namespace evl::gui_imgui::views
