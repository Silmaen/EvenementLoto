/**
 * @file Popups.cpp
 * @author Silmaen
 * @date 17/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "HelpPopups.h"

#include "baseDefine.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

PopupAide::PopupAide() = default;
PopupAide::~PopupAide() = default;

void PopupAide::onPopupUpdate() {
	ImGui::Text("L'aide n'est pas encore disponible.");
	const float windowWidth = ImGui::GetWindowSize().x;
	constexpr float buttonWidth = 120.0f;
	ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
	if (ImGui::Button("Fermer", ImVec2(buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
}


PopupAbout::PopupAbout() = default;
PopupAbout::~PopupAbout() = default;

void PopupAbout::onPopupUpdate() {
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);// Police par défaut ou titre si disponible
	const std::string textTitle = std::format("Application de Loto Associatif");
	const float windowWidth = ImGui::GetWindowSize().x;
	const float textWidth = ImGui::CalcTextSize(textTitle.c_str()).x;
	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	ImGui::Text("%s", textTitle.c_str());
	ImGui::PopFont();

	// Version et copyright centrés
	const std::string version = std::format("Version {}", EVL_VERSION);
	const float versionWidth = ImGui::CalcTextSize(version.c_str()).x;
	ImGui::SetCursorPosX((windowWidth - versionWidth) * 0.5f);
	ImGui::TextDisabled("%s", version.c_str());

	ImGui::Spacing();
	const char* copyright = "Copyright © 2026 Silmaen.\nTous droits réservés.";
	const float copyrightWidth = ImGui::CalcTextSize("Copyright © 2026 Silmaen.").x;
	ImGui::SetCursorPosX((windowWidth - copyrightWidth) * 0.5f);
	ImGui::TextWrapped("%s", copyright);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// Licence dans une zone défilante
	constexpr auto licence = "Copyright (c) 2020-2026 Damien Lachouette\n\n"
							 "Permission is hereby granted, free of charge, to any person obtaining "
							 "a copy of this software and associated documentation files (the "
							 "\"Software\"), to deal in the Software without restriction, including "
							 "without limitation the rights to use, copy, modify, merge, publish, "
							 "distribute, sublicense, and/or sell copies of the Software, and to "
							 "permit persons to whom the Software is furnished to do so, subject to "
							 "the following conditions:\n\n"
							 "The above copyright notice and this permission notice shall be "
							 "included in all copies or substantial portions of the Software.\n\n"
							 "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, "
							 "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
							 "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND "
							 "NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE "
							 "LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION "
							 "OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION "
							 "WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.";

	ImGui::BeginChild("LicenceText", ImVec2(500, 200), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::TextWrapped("%s", licence);
	ImGui::EndChild();

	ImGui::Spacing();

	// Bouton Fermer centré
	constexpr float buttonWidth = 120.0f;
	ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
	if (ImGui::Button("Fermer", ImVec2(buttonWidth, 0))) {
		ImGui::CloseCurrentPopup();
	}
}


}// namespace evl::gui_imgui::views
