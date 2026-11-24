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
#include "gui/utils/Layout.h"

#include <imgui.h>

namespace evl::gui::views {

PopupReport::PopupReport() = default;
PopupReport::~PopupReport() = default;

void PopupReport::onOpen() { m_report = core::buildReport(Application::get().getCurrentEvent()); }

void PopupReport::onPopupUpdate() {
	if (ImGui::BeginChild(
				"ReportText",
				{0, ImGui::GetContentRegionAvail().y - utils::dialogFooterHeight() - ImGui::GetStyle().ItemSpacing.y},
				ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar)) {
		// Shown as it is written: the text on screen and the file on disk are the same.
		ImGui::TextUnformatted(m_report.c_str());
	}
	ImGui::EndChild();

	utils::beginDialogFooter(3);
	if (utils::dialogButton("Enregistrer...")) {
		utils::FileDialog::saveFile("Rapport|md,txt", [report = m_report](const std::filesystem::path& iPath) -> void {
			if (core::writeFileAtomically(iPath, [&report](std::ostream& oBs) -> void { oBs << report; })) {
				log_info("Rapport enregistré dans '{}'.", iPath.string());
				return;
			}
			Application::get().tell("Rapport", "Le rapport n'a pas pu être enregistré.", iPath.string());
		});
	}
	ImGui::SameLine();
	if (utils::dialogButton("Copier")) {
		ImGui::SetClipboardText(m_report.c_str());
		log_info("Rapport copié dans le presse-papier.");
	}
	ImGui::SameLine();
	if (utils::dialogButton("Fermer"))
		ImGui::CloseCurrentPopup();
}

}// namespace evl::gui::views
