/**
 * @file RescuePopup.cpp
 * @author Silmaen
 * @date 22/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "RescuePopup.h"

#include "core/Log.h"
#include "gui/Application.h"

#include "gui/utils/Layout.h"

#include <imgui.h>

namespace evl::gui::views {

namespace {

auto ageText(const double iSeconds) -> std::string {
	if (iSeconds < 60.0)
		return "il y a moins d'une minute";
	const auto minutes = static_cast<int>(iSeconds / 60.0);
	if (minutes < 60)
		return std::format("il y a {} minute{}", minutes, minutes > 1 ? "s" : "");
	const auto hours = minutes / 60;
	return std::format("il y a {} h {:02} min", hours, minutes % 60);
}

}// namespace

PopupRescue::PopupRescue() = default;
PopupRescue::~PopupRescue() = default;

void PopupRescue::propose(const core::RescueInfo& iInfo) {
	m_info = iInfo;
	open();
}

void PopupRescue::onPopupUpdate() {
	ImGui::TextWrapped("Une partie interrompue a été retrouvée. Elle a été enregistrée automatiquement %s.",
					   ageText(m_info.ageSeconds).c_str());
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text("Événement : %s", m_info.eventName.empty() ? "(sans nom)" : m_info.eventName.c_str());
	ImGui::Text("Numéros déjà tirés : %zu", m_info.drawCount);
	ImGui::Spacing();
	// A path is one long word: wrapped, it stays inside the dialog instead of running off it.
	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
	ImGui::TextWrapped("%s", m_info.path.string().c_str());
	ImGui::PopStyleColor();
	ImGui::Spacing();
	ImGui::TextDisabled("« Ignorer » conserve le fichier, renommé.");
	ImGui::Spacing();

	utils::beginDialogFooter(2);
	if (ImGui::Button("Reprendre", {utils::dialogButtonWidth(), 0.0f})) {
		auto& app = Application::get();
		if (core::loadRescue(m_info.path, app.getCurrentEvent())) {
			app.getCurrentFile().clear();
			log_info("Partie interrompue reprise depuis '{}'.", m_info.path.string());
		} else {
			log_error("Échec de la reprise depuis '{}'.", m_info.path.string());
		}
		ImGui::CloseCurrentPopup();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Reprendre la partie là où elle s'est arrêtée.");
	ImGui::SameLine();
	if (utils::dialogButton("Ignorer")) {
		// Archived rather than deleted: declining by mistake must not be final.
		core::archiveRescue();
		ImGui::CloseCurrentPopup();
	}
}

}// namespace evl::gui::views
