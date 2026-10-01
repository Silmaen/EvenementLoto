/**
 * @file ReportPopup.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "ReportPopup.h"

#include "core/AtomicFile.h"
#include "core/Log.h"
#include "core/Report.h"
#include "gui/Application.h"
#include "gui/utils/FileDialog.h"

#include <imgui.h>

namespace evl::gui::views {

namespace {

constexpr float g_buttonWidth = 110.0f;

}// namespace

PopupReport::PopupReport() = default;
PopupReport::~PopupReport() = default;

void PopupReport::onOpen() { m_report = core::buildReport(Application::get().getCurrentEvent()); }

void PopupReport::onPopupUpdate() {
	if (ImGui::BeginChild("ReportText", {0, ImGui::GetContentRegionAvail().y - 40}, ImGuiChildFlags_Borders,
						  ImGuiWindowFlags_HorizontalScrollbar)) {
		// Shown as it is written: the text on screen and the file on disk are the same.
		ImGui::TextUnformatted(m_report.c_str());
	}
	ImGui::EndChild();

	ImGui::Separator();
	if (ImGui::Button("Enregistrer...", {g_buttonWidth, 0})) {
		utils::FileDialog::saveFile("Rapport|md,txt", [report = m_report](const std::filesystem::path& iPath) -> void {
			if (core::writeFileAtomically(iPath, [&report](std::ostream& oBs) -> void { oBs << report; })) {
				log_info("Rapport enregistré dans '{}'.", iPath.string());
				return;
			}
			Application::get().tell("Rapport", "Le rapport n'a pas pu être enregistré.", iPath.string());
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Copier", {g_buttonWidth, 0})) {
		ImGui::SetClipboardText(m_report.c_str());
		log_info("Rapport copié dans le presse-papier.");
	}
	ImGui::SameLine();
	if (ImGui::Button("Fermer", {g_buttonWidth, 0}))
		ImGui::CloseCurrentPopup();
}

}// namespace evl::gui::views
