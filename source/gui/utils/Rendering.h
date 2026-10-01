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

namespace evl::gui::utils {

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
 * @brief Draw an editable list of prize articles.
 *
 * One row per article: what it is, who gave it, what it is worth, how much it makes
 * people want it, and whether it can be put in play in a children's round. Shared by
 * the round configuration and by the improvised round, which fill the very same list.
 *
 * @param[in,out] ioPrizes The articles to edit.
 * @param[in] iEditable False once the round has started, the list is then read only.
 * @param[in] iSize The region for the list, the ImGui child convention for zero and
 *            negative values applying.
 * @return True when the list changed this frame.
 */
auto renderPrizeList(core::prizes_type& ioPrizes, bool iEditable, const math::vec2& iSize = {0, -80}) -> bool;

/**
 * @brief Options for text adaptation.
 */
struct TextAdaptOptions {
	bool autoRegion = true;///< Automatically use the available region.
	math::vec2 contentSize = {0.0f, 0.0f};///< Content size to fit into if autoRegion is false.
	bool vCenter = true;///< Vertically center the text.
	bool hCenter = true;///< Horizontally center the text.
	bool drawText = false;///< Draw the text after adaptation.
	std::string textAdapt;///< The text to adapt can be different than the text to render.
};

/**
 * @brief Adapt text size to fit in the available region.
 * @param iText The text to adapt.
 * @param iOptions The text adaptation options.
 */
void adaptTextToRegion(const std::string& iText, const TextAdaptOptions& iOptions = {});

}// namespace evl::gui::utils
