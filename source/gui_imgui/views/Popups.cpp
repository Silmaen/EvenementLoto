/**
 * @file Popups.cpp
 * @author Silmaen
 * @date 17/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Popups.h"

#include "baseDefine.h"

#include <imgui.h>

namespace evl::gui_imgui::views {

Popup::Popup() = default;
Popup::~Popup() = default;
void Popup::onUpdate() {
	const std::string popupTitle = getPopupTitle();
	if (m_shouldOpen) {
		ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
		ImGui::OpenPopup(popupTitle.c_str());
		m_shouldOpen = false;
	}
	if (ImGui::BeginPopupModal(popupTitle.c_str(), nullptr, ImGuiWindowFlags_None)) {
		onPopupUpdate();
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}


}// namespace evl::gui_imgui::views
