/**
 * @file Monitors.h
 * @author Silmaen
 * @date 23/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "core/maths/vectors.h"

#include <string>
#include <vector>

namespace evl::gui {

/**
 * @brief Struct MonitorInfo.
 */
struct MonitorInfo {
	/// Monitor name.
	std::string name;
	/// Position.
	math::vec2i position{0, 0};
	/// Size.
	math::vec2ui size{0, 0};
	/// Physical size.
	math::vec2i physicalSize{0, 0};
	/// Work area position.
	math::vec2i workAreaPosition{0, 0};
	/// Work area size.
	math::vec2i workAreaSize{0, 0};

	/// Is main window.
	bool isMainWindow = false;

	/**
	 * @brief What identifies this monitor from one start to the next.
	 *
	 * The name alone is not enough: two screens of the same model may carry the same
	 * one on some platforms, and the index changes as soon as one is unplugged.
	 * @return The key stored in the settings.
	 */
	[[nodiscard]] auto key() const -> std::string;
};

/**
 * @brief The monitor holding the larger part of a window.
 *
 * The top-left corner is not enough: a window straddling two screens, or whose corner
 * sits a few pixels off its own, would be placed on the wrong one.
 * @param iMonitors The monitors.
 * @param iPosition The window position.
 * @param iSize The window size.
 * @return The index of that monitor, 0 when the window touches none.
 */
[[nodiscard]] auto monitorHolding(const std::vector<MonitorInfo>& iMonitors, const math::vec2i& iPosition,
								  const math::vec2i& iSize) -> size_t;

/**
 * @brief The monitors the players' display may be sent to: all but the control window's.
 * @param iMonitors The monitors.
 * @return Their indexes, in the platform order.
 */
[[nodiscard]] auto displayCandidates(const std::vector<MonitorInfo>& iMonitors) -> std::vector<size_t>;

/**
 * @brief The monitor the players' display goes to.
 *
 * The one asked for when it is still there and not the control window's, then one of
 * the same name, then the first candidate. Never the control window's screen while
 * there is another one: a full-screen display would hide the controls.
 * @param iMonitors The monitors.
 * @param iWanted The key of the monitor asked for, possibly empty.
 * @return Its index; 0 when there are no monitors at all.
 */
[[nodiscard]] auto resolveDisplayMonitor(const std::vector<MonitorInfo>& iMonitors, const std::string& iWanted)
		-> size_t;

}// namespace evl::gui
