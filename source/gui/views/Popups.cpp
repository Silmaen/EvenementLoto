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
#include "gui/utils/Layout.h"

#include <imgui.h>

namespace evl::gui::views {

Popup::Popup() = default;
Popup::~Popup() = default;
void Popup::onUpdate() {
	const std::string popupTitle = getPopupTitle();
	const auto [size, fitHeight] = layout();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	// Never larger than the control window, which is what keeps it inside: the main
	// window can be resized while the dialog is open, so this is redone every frame.
	const ImVec2 room{viewport->WorkSize.x * 0.96f, viewport->WorkSize.y * 0.96f};
	const ImVec2 wanted{std::min(utils::em(size.x()), room.x), std::min(utils::em(size.y()), room.y)};
	if (m_shouldOpen) {
		onOpen();
		// Centred on the control window at every opening: the position remembered from
		// the last session could be on a screen since unplugged, or under the display.
		ImGui::SetNextWindowPos(viewport->GetWorkCenter(), ImGuiCond_Always, {0.5f, 0.5f});
		if (!fitHeight)
			ImGui::SetNextWindowSize(wanted, ImGuiCond_Always);
		ImGui::OpenPopup(popupTitle.c_str());
		m_shouldOpen = false;
	}
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
	if (fitHeight) {
		ImGui::SetNextWindowSizeConstraints({wanted.x, 0.0f}, {wanted.x, room.y});
		flags |= ImGuiWindowFlags_AlwaysAutoResize;
	} else {
		ImGui::SetNextWindowSizeConstraints({std::min(utils::em(18.0f), room.x), std::min(utils::em(10.0f), room.y)},
											room);
	}
	if (ImGui::BeginPopupModal(popupTitle.c_str(), nullptr, flags)) {
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
