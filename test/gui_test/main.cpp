/**
 * @file main.cpp
 * @author Silmaen
 * @date 25/09/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

auto main(int iArgc, char** iArgv) -> int {
	// Errors only: when the application ends in State::Error, the reason is the one thing
	// the test output would otherwise not say, and ctest shows it for a failed run alone.
	evl::Log::init(evl::Log::Level::Error);
	evl::core::initializeUtilities(iArgc, iArgv);
	evl::core::mergeDefaultSettings();
	::testing::InitGoogleTest(&iArgc, iArgv);
	const auto ret = RUN_ALL_TESTS();
	evl::core::leaveSettings();
	evl::Log::invalidate();
	return ret;
}
