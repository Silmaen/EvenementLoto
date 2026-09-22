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
	ImGui::TextDisabled("%s", m_info.path.string().c_str());
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Reprendre la partie")) {
		auto& app = Application::get();
		if (core::loadRescue(m_info.path, app.getCurrentEvent())) {
			app.getCurrentFile().clear();
			log_info("Partie interrompue reprise depuis '{}'.", m_info.path.string());
		} else {
			log_error("Échec de la reprise depuis '{}'.", m_info.path.string());
		}
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Ignorer")) {
		// Archived rather than deleted: declining by mistake must not be final.
		core::archiveRescue();
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("(« Ignorer » conserve le fichier, renommé)");
}

}// namespace evl::gui::views
