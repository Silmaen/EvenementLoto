/**
 * @file Monitors.cpp
 * @author Silmaen
 * @date 23/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Monitors.h"

namespace evl::gui {

auto MonitorInfo::key() const -> std::string { return std::format("{}@{},{}", name, position.x(), position.y()); }

auto monitorHolding(const std::vector<MonitorInfo>& iMonitors, const math::vec2i& iPosition, const math::vec2i& iSize)
		-> size_t {
	size_t best = 0;
	int64_t bestArea = 0;
	for (size_t i = 0; i < iMonitors.size(); ++i) {
		const auto& monitor = iMonitors[i];
		const int left = std::max(iPosition.x(), monitor.position.x());
		const int top = std::max(iPosition.y(), monitor.position.y());
		const int right =
				std::min(iPosition.x() + iSize.x(), monitor.position.x() + static_cast<int>(monitor.size.x()));
		const int bottom =
				std::min(iPosition.y() + iSize.y(), monitor.position.y() + static_cast<int>(monitor.size.y()));
		if (right <= left || bottom <= top)
			continue;
		if (const int64_t area = static_cast<int64_t>(right - left) * (bottom - top); area > bestArea) {
			bestArea = area;
			best = i;
		}
	}
	return best;
}

auto displayCandidates(const std::vector<MonitorInfo>& iMonitors) -> std::vector<size_t> {
	std::vector<size_t> candidates;
	for (size_t i = 0; i < iMonitors.size(); ++i) {
		if (!iMonitors[i].isMainWindow)
			candidates.push_back(i);
	}
	return candidates;
}

auto resolveDisplayMonitor(const std::vector<MonitorInfo>& iMonitors, const std::string& iWanted) -> size_t {
	const auto candidates = displayCandidates(iMonitors);
	if (candidates.empty())
		return 0;
	if (iWanted.empty())
		return candidates.front();
	for (const size_t i: candidates) {
		if (iMonitors[i].key() == iWanted)
			return i;
	}
	// Same screen, rearranged in the display settings since: its position moved.
	const auto at = iWanted.rfind('@');
	const std::string wantedName = at == std::string::npos ? iWanted : iWanted.substr(0, at);
	for (const size_t i: candidates) {
		if (iMonitors[i].name == wantedName)
			return i;
	}
	return candidates.front();
}

}// namespace evl::gui
