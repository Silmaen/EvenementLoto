/**
 * @file test_Log.cpp
 * @author Silmaen
 * @date 03/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

using namespace evl;

TEST(Log, InitAndInvalidate) {
	// should already be uninitiated by main
	EXPECT_TRUE(Log::initiated());
	EXPECT_EQ(Log::getVerbosityLevel(), g_logLv);
	Log::init(Log::Level::Debug);
	EXPECT_TRUE(Log::initiated());
	// should keep the same level if re-initialized
	EXPECT_EQ(Log::getVerbosityLevel(), g_logLv);

	Log::setVerbosityLevel(Log::Level::Critical);
	EXPECT_EQ(Log::getVerbosityLevel(), Log::Level::Critical);

	Log::invalidate();
	EXPECT_FALSE(Log::initiated());

	Log::init(Log::Level::Debug);
	EXPECT_TRUE(Log::initiated());
	EXPECT_EQ(Log::getVerbosityLevel(), Log::Level::Debug);
	// restore test global level for other tests
	Log::setVerbosityLevel(g_logLv);
}
