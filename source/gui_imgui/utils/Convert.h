/**
 * @file Convert.h
 * @author Silmaen
 * @date 22/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "core/maths/vectors.h"
#include <ImGui/imgui.h>

namespace owl::gui_imgui::utils {
/**
 * @brief Convert ImVec4 to vec4.
 * @param iColor The ImVec4 color.
 * @return The vec4 color.
 */
inline auto imVec4ToVec4(const ImVec4& iColor) -> math::vec4 { return {iColor.x, iColor.y, iColor.z, iColor.w}; }

/**
 * @brief Convert vec4 to ImVec4.
 * @param iColor The vec4 color.
 * @return The ImVec4 color.
 */
inline auto vec4ToImVec4(const math::vec4& iColor) -> ImVec4 {
	return {iColor.x(), iColor.y(), iColor.z(), iColor.w()};
}

}// namespace owl::gui_imgui::utils
