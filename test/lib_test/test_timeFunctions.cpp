/**
 * @file test_timeFunctions.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

#include "core/timeFunctions.h"

TEST(timeFunction, timePoint) {
	constexpr evl::core::time_point t = evl::core::g_epoch;
	EXPECT_TRUE(t.time_since_epoch().count() == 0);
	EXPECT_STREQ(evl::core::formatCalendar(t).c_str(), "01 January 1970");

	// Implementation-defined epoch can be 1 or 0 hour.
	EXPECT_TRUE(evl::core::formatClock(t) == "01:00:00" || evl::core::formatClock(t) == "00:00:00");
	EXPECT_TRUE(evl::core::formatClockNoSecond(t) == "01:00" || evl::core::formatClockNoSecond(t) == "00:00");
}
TEST(timeFunction, duration) {
	evl::core::duration t(0);
	EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "00s");
	t = evl::core::duration(std::chrono::seconds(15));
	EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "15s");
	t = evl::core::duration(std::chrono::seconds(75));
	EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "01:15");
	t = evl::core::duration(std::chrono::seconds(675));
	EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "11:15");
	t = evl::core::duration(std::chrono::seconds(3675));
	EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "01:01:15");
	t = evl::core::duration(std::chrono::seconds(63675));
	EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "17:41:15");
}
