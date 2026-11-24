/**
 * @file Layout.cpp
 * @author Silmaen
 * @date 23/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Layout.h"

#include <imgui.h>

namespace evl::gui::utils {

auto em(const float iCount) -> float { return iCount * ImGui::GetFontSize(); }

auto dialogButtonWidth() -> float { return em(6.0f); }

auto dialogFooterHeight() -> float {
	const auto& style = ImGui::GetStyle();
	return ImGui::GetFrameHeight() + style.ItemSpacing.y * 2.0f + style.SeparatorSize;
}

void beginDialogFooter(const int iButtonCount) {
	ImGui::Separator();
	const auto count = static_cast<float>(iButtonCount);
	const float row = dialogButtonWidth() * count + ImGui::GetStyle().ItemSpacing.x * (count - 1.0f);
	if (const float offset = ImGui::GetContentRegionAvail().x - row; offset > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
}

auto dialogButton(const char* iLabel) -> bool { return ImGui::Button(iLabel, {dialogButtonWidth(), 0.0f}); }

}// namespace evl::gui::utils
