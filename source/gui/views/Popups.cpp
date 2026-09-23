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
#include "gui/utils/FileDialog.h"

#include <imgui.h>

namespace evl::gui::views {

Popup::Popup() = default;
Popup::~Popup() = default;
void Popup::onUpdate() {
	const std::string popupTitle = getPopupTitle();
	if (m_shouldOpen) {
		onOpen();
		ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
		ImGui::OpenPopup(popupTitle.c_str());
		m_shouldOpen = false;
	}
	if (ImGui::BeginPopupModal(popupTitle.c_str(), nullptr, ImGuiWindowFlags_None)) {
		{
			// A file request made from here is drawn from here too, a few lines below:
			// a modal opened at the root level would dismiss this one.
			const utils::FileDialog::OwnerScope owner{this};
			onPopupUpdate();
		}
		ImGui::SetItemDefaultFocus();
		utils::FileDialog::draw(this);
		ImGui::EndPopup();
	}
}


}// namespace evl::gui::views
