/**
 * @file Layout.h
 * @author Silmaen
 * @date 23/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

namespace evl::gui::utils {

/**
 * @brief A length counted in font heights.
 *
 * What lets a layout follow the font size chosen in the settings: a width written in
 * pixels was right for one size only, and cut its text at any other.
 * @param iCount The number of font heights.
 * @return The length in pixels.
 */
[[nodiscard]] auto em(float iCount) -> float;

/// The width of a dialog button, wide enough for « Appliquer ».
[[nodiscard]] auto dialogButtonWidth() -> float;

/// The height the footer of a dialog takes: a separator and one row of buttons.
[[nodiscard]] auto dialogFooterHeight() -> float;

/**
 * @brief Opens the footer of a dialog: a separator, then the cursor placed so that a
 * row of buttons of the standard width ends on the right edge.
 * @param iButtonCount How many buttons follow, separated by `SameLine()`.
 */
void beginDialogFooter(int iButtonCount);

/**
 * @brief A dialog button of the standard width.
 * @param iLabel Its label.
 * @return True when clicked.
 */
[[nodiscard]] auto dialogButton(const char* iLabel) -> bool;

}// namespace evl::gui::utils
