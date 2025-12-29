/**
 * @file Theme.h
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "core/Settings.h"
#include "core/maths/vectors.h"

namespace evl::gui_imgui {

/**
 * @brief Struct Theme.
 */
struct Theme {
	math::vec4 text{0.85f, 0.85f, 0.85f, 1.0f};
	math::vec4 windowBackground{0.21f, 0.21f, 0.23f, 1.0f};
	math::vec4 childBackground{0.18f, 0.18f, 0.20f, 1.0f};
	math::vec4 backgroundPopup{0.25f, 0.25f, 0.27f, 1.0f};
	math::vec4 border{0.18f, 0.18f, 0.20f, 1.0f};

	math::vec4 frameBackground{0.15f, 0.15f, 0.17f, 1.0f};
	math::vec4 frameBackgroundHovered{0.20f, 0.20f, 0.23f, 1.0f};
	math::vec4 frameBackgroundActive{0.25f, 0.35f, 0.50f, 1.0f};

	math::vec4 titleBar{0.18f, 0.18f, 0.20f, 1.0f};
	math::vec4 titleBarActive{0.25f, 0.35f, 0.50f, 1.0f};
	math::vec4 titleBarCollapsed{0.20f, 0.20f, 0.23f, 1.0f};
	math::vec4 menubarBackground{0.18f, 0.18f, 0.20f, 1.0f};

	math::vec4 scrollbarBackground{0.12f, 0.12f, 0.14f, 0.53f};
	math::vec4 scrollbarGrab{0.35f, 0.45f, 0.60f, 1.0f};
	math::vec4 scrollbarGrabHovered{0.45f, 0.55f, 0.70f, 1.0f};
	math::vec4 scrollbarGrabActive{0.50f, 0.60f, 0.75f, 1.0f};

	math::vec4 checkMark{0.50f, 0.75f, 1.0f, 1.0f};

	math::vec4 sliderGrab{0.35f, 0.55f, 0.85f, 0.9f};
	math::vec4 sliderGrabActive{0.50f, 0.75f, 1.0f, 1.0f};

	math::vec4 button{0.20f, 0.20f, 0.23f, 1.0f};
	math::vec4 buttonHovered{0.25f, 0.35f, 0.50f, 1.0f};
	math::vec4 buttonActive{0.35f, 0.55f, 0.85f, 1.0f};

	math::vec4 groupHeader{0.18f, 0.18f, 0.20f, 1.0f};
	math::vec4 groupHeaderHovered{0.25f, 0.35f, 0.50f, 1.0f};
	math::vec4 groupHeaderActive{0.30f, 0.45f, 0.65f, 1.0f};

	math::vec4 separator{0.15f, 0.15f, 0.17f, 1.0f};
	math::vec4 separatorActive{0.35f, 0.55f, 0.85f, 1.0f};
	math::vec4 separatorHovered{0.40f, 0.60f, 0.85f, 0.8f};

	math::vec4 resizeGrip{0.35f, 0.55f, 0.85f, 0.3f};
	math::vec4 resizeGripHovered{0.40f, 0.60f, 0.85f, 0.7f};
	math::vec4 resizeGripActive{0.50f, 0.70f, 1.0f, 1.0f};

	math::vec4 tabHovered{0.25f, 0.35f, 0.50f, 1.0f};
	math::vec4 tab{0.15f, 0.15f, 0.17f, 1.0f};
	math::vec4 tabSelected{0.25f, 0.35f, 0.50f, 1.0f};
	math::vec4 tabSelectedOverline{0.35f, 0.55f, 0.85f, 1.0f};
	math::vec4 tabDimmed{0.12f, 0.12f, 0.14f, 1.0f};
	math::vec4 tabDimmedSelected{0.18f, 0.18f, 0.20f, 1.0f};
	math::vec4 tabDimmedSelectedOverline{0.25f, 0.45f, 0.70f, 1.0f};

	math::vec4 dockingPreview{0.35f, 0.55f, 0.85f, 0.7f};
	math::vec4 dockingEmptyBackground{0.18f, 0.18f, 0.20f, 1.0f};

	math::vec4 plotLines{0.35f, 0.55f, 0.85f, 1.0f};
	math::vec4 plotLinesHovered{0.45f, 0.65f, 0.95f, 1.0f};
	math::vec4 plotHistogram{0.30f, 0.50f, 0.80f, 1.0f};
	math::vec4 plotHistogramHovered{0.40f, 0.60f, 0.90f, 1.0f};

	math::vec4 tableHeaderBg{0.18f, 0.18f, 0.20f, 1.0f};
	math::vec4 tableBorderLight{0.15f, 0.15f, 0.17f, 1.0f};
	math::vec4 tableRowBg{0.18f, 0.18f, 0.20f, 1.0f};
	math::vec4 tableRowBgAlt{0.15f, 0.15f, 0.17f, 1.0f};

	math::vec4 textSelectedBg{0.25f, 0.35f, 0.50f, 1.0f};
	math::vec4 dragDropTarget{0.35f, 0.55f, 0.85f, 0.9f};

	math::vec4 navHighlight{0.35f, 0.55f, 0.85f, 1.0f};
	math::vec4 navWindowingHighlight{0.35f, 0.55f, 0.85f, 1.0f};
	math::vec4 navWindowingDimBg{0.02f, 0.02f, 0.02f, 0.5f};
	math::vec4 modalWindowDimBg{0.02f, 0.02f, 0.02f, 0.5f};

	math::vec4 highlight{0.35f, 0.55f, 0.85f, 1.0f};
	math::vec4 propertyField{0.12f, 0.12f, 0.14f, 1.0f};

	float windowRounding{7.0f};
	float frameRounding{3.0f};
	float frameBorderSize{1.0f};
	float indentSpacing{11.0f};

	float tabRounding{10.0f};
	float tabOverline{1.5f};
	float tabBorder{1.0f};

	float controlsRounding{5.0f};

	math::vec2 itemSpacing{8.0f, 4.0f};
	math::vec2 itemInnerSpacing{4.0f, 4.0f};
	math::vec2 cellPadding{4.0f, 2.0f};
	math::vec2 framePadding{4.0f, 3.0f};
	math::vec2 buttonTextAlign{0.5f, 0.5f};
	math::vec2 selectableTextAlign{0.0f, 0.5f};
	math::vec2 displayWindowPadding{19.0f, 19.0f};
	math::vec2 displaySafeAreaPadding{3.0f, 3.0f};

	float mouseCursorScale{1.0f};

	/**
	 * @brief Load theme from settings.
	 * @param iSettings The settings to load from.
	 */
	void loadFromSettings(const core::Settings& iSettings);

	/**
	 * @brief Save theme to settings.
	 * @return The settings containing the theme.
	 */
	auto saveToSettings() -> core::Settings;
};

}// namespace evl::gui_imgui
