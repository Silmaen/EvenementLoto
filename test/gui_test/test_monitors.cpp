#include "../TestMainHelper.h"
#include "gui/Monitors.h"

#include <vector>

using evl::gui::MonitorInfo;

namespace {

auto monitor(const char* iName, const int iX, const int iY = 0) -> MonitorInfo {
	MonitorInfo info;
	info.name = iName;
	info.position = {iX, iY};
	info.size = {1920, 1080};
	info.workAreaPosition = {iX, iY};
	info.workAreaSize = {1920, 1040};
	return info;
}

/// Three screens side by side, the control window on the middle one.
auto threeScreens() -> std::vector<MonitorInfo> {
	std::vector screens{monitor("DP-1", 0), monitor("HDMI-1", 1920), monitor("DP-2", 3840)};
	screens[1].isMainWindow = true;
	return screens;
}

}// namespace

TEST(gui_Monitors, HoldingTakesTheLargestOverlap) {
	const std::vector screens{monitor("DP-1", 0), monitor("HDMI-1", 1920), monitor("DP-2", 3840)};
	// Its corner is on the first screen, most of it on the second.
	EXPECT_EQ(evl::gui::monitorHolding(screens, {1800, 100}, {1000, 800}), 1U);
	EXPECT_EQ(evl::gui::monitorHolding(screens, {3900, 0}, {200, 200}), 2U);
	// A window lost off every screen still gets one.
	EXPECT_EQ(evl::gui::monitorHolding(screens, {-5000, -5000}, {100, 100}), 0U);
	EXPECT_EQ(evl::gui::monitorHolding({}, {0, 0}, {100, 100}), 0U);
}

TEST(gui_Monitors, CandidatesLeaveOutTheControlScreen) {
	EXPECT_EQ(evl::gui::displayCandidates(threeScreens()), (std::vector<size_t>{0, 2}));
	auto single = std::vector{monitor("eDP-1", 0)};
	single[0].isMainWindow = true;
	EXPECT_TRUE(evl::gui::displayCandidates(single).empty());
}

TEST(gui_Monitors, NeverOnTheControlScreenWhileThereIsAnother) {
	const auto screens = threeScreens();
	// Nothing asked yet: a fresh install used to put the display on screen 0 whatever
	// the list showed, on top of the controls when they were there.
	EXPECT_EQ(evl::gui::resolveDisplayMonitor(screens, ""), 0U);
	EXPECT_EQ(evl::gui::resolveDisplayMonitor(screens, screens[2].key()), 2U);
	// Asked for the screen the control window has since moved to.
	EXPECT_EQ(evl::gui::resolveDisplayMonitor(screens, screens[1].key()), 0U);
	// Unplugged.
	EXPECT_EQ(evl::gui::resolveDisplayMonitor(screens, "VGA-1@0,0"), 0U);
}

TEST(gui_Monitors, TheChoiceSurvivesAReorderingOrARearrangement) {
	auto screens = threeScreens();
	const auto wanted = screens[2].key();
	// The platform lists them in another order on the next start.
	std::swap(screens[0], screens[2]);
	EXPECT_EQ(evl::gui::resolveDisplayMonitor(screens, wanted), 0U);
	// The screen was dragged elsewhere in the display settings: same name, new place.
	screens[0].position = {-1920, 0};
	EXPECT_EQ(evl::gui::resolveDisplayMonitor(screens, wanted), 0U);
}

TEST(gui_Monitors, SingleScreenFallsBackOnIt) {
	auto single = std::vector{monitor("eDP-1", 0)};
	single[0].isMainWindow = true;
	EXPECT_EQ(evl::gui::resolveDisplayMonitor(single, "DP-1@1920,0"), 0U);
	EXPECT_EQ(evl::gui::resolveDisplayMonitor({}, ""), 0U);
}
