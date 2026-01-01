/**
 * @file Rendering.h
 * @author Silmaen
 * @date 26/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "core/Event.h"
#include "core/maths/vectors.h"


#include <string>

namespace evl::gui_imgui::utils {

/**
 * @brief Options for action buttons.
 */
struct ActionButtonOptions {
	bool showLabel = false;///< Show label next to icon.
	bool disabled = false;///< Disable the button.
	bool sameLine = true;///< Place the button on the same line as the previous one.
	bool setDisabled = false;///< Set the disabled state according to 'disabled' field.
};

/**
 * @brief Define an action button item in ImGui.
 * @param iLabel The button label.
 * @param iActionName The action name.
 * @param iOptions The action button options.
 */
void defineActionButtonItem(const std::string& iLabel, const std::string& iActionName,
							const ActionButtonOptions& iOptions = {});

/**
 * @brief Get the next step string for an event.
 * @param iEvent The event.
 * @return The next step string.
 */
auto getNextStepStr(const core::Event& iEvent) -> std::string;

/**
 * @brief Options for text adaptation.
 */
struct TextAdaptOptions {
	bool autoRegion = true;///< Automatically use the available region.
	math::vec2 contentSize = {0.0f, 0.0f};///< Content size to fit into if autoRegion is false.
	bool vCenter = true;///< Vertically center the text.
	bool hCenter = true;///< Horizontally center the text.
	bool drawText = false;///< Draw the text after adaptation.
	std::string textAdapt;///< The text to adapt can be different thant the text to render.
};

/**
 * @brief Adapt text size to fit in the available region.
 * @param iText The text to adapt.
 * @param iOptions The text adaptation options.
 */
void adaptTextToRegion(const std::string& iText, const TextAdaptOptions& iOptions = {});

}// namespace evl::gui_imgui::utils
