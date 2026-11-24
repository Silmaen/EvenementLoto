/**
 * @file MessagePopup.cpp
 * @author Silmaen
 * @date 23/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "MessagePopup.h"

#include "gui/utils/Layout.h"

#include <imgui.h>

namespace evl::gui::views {

PopupMessage::PopupMessage() = default;
PopupMessage::~PopupMessage() = default;

void PopupMessage::show(const std::string& iTitle, const std::string& iMessage, const std::string& iDetail) {
	m_title = iTitle;
	m_message = iMessage;
	m_detail = iDetail;
	open();
}

void PopupMessage::onPopupUpdate() {
	ImGui::TextWrapped("%s", m_message.c_str());
	if (!m_detail.empty()) {
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGui::TextWrapped("%s", m_detail.c_str());
		ImGui::PopStyleColor();
	}
	ImGui::Spacing();
	utils::beginDialogFooter(1);
	if (utils::dialogButton("Fermer"))
		ImGui::CloseCurrentPopup();
}

}// namespace evl::gui::views
