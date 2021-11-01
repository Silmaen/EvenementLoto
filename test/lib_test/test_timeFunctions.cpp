/**
 * @file test_timeFunctions.cpp
 * @author Silmaen
 * @date 26/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "core/timeFunctions.h"
#include <gtest/gtest.h>

TEST(timeFunction, timePoint) {
    evl::core::timePoint t= evl::core::epoch;
    EXPECT_STREQ(evl::core::formatCalendar(t).c_str(), "01 January 1970");
    EXPECT_STREQ(evl::core::formatClock(t).c_str(), "01:00:00");
    EXPECT_STREQ(evl::core::formatClockNoSecond(t).c_str(), "01:00");
}
TEST(timeFunction, duration) {
    evl::core::duration t(0);
    EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "00");
    t= evl::core::duration(std::chrono::seconds(15));
    EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "15");
    t= evl::core::duration(std::chrono::seconds(75));
    EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "01:15");
    t= evl::core::duration(std::chrono::seconds(675));
    EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "11:15");
    t= evl::core::duration(std::chrono::seconds(3675));
    EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "01:01:15");
    t= evl::core::duration(std::chrono::seconds(63675));
    EXPECT_STREQ(evl::core::formatDuration(t).c_str(), "17:41:15");
}
