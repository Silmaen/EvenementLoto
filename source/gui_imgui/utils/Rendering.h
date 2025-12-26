/**
 * @file Rendering.h
 * @author Silmaen
 * @date 26/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "core/Event.h"


#include <string>

namespace evl::gui_imgui::utils {

struct ActionButtonOptions {
	bool showLabel = false;///< Show label next to icon.
	bool disabled = false;///< Disable the button.
	bool sameLine = true;///< Place the button on the same line as the previous one.
	bool setDisabled = false;///< Set the disabled state according to 'disabled' field.
};

void defineActionButtonItem(const std::string& iLabel, const std::string& iActionName,
							const ActionButtonOptions& iOptions = {});

auto getNextStepStr(const core::Event& iEvent) -> std::string;

}// namespace evl::gui_imgui::utils
