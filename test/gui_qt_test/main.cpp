/**
 * @file main.cpp
 * @author Silmaen
 * @date 25/09/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

auto main(int iArgc, char** iArgv) -> int {
	evl::Log::init(g_logLv);
	::testing::InitGoogleTest(&iArgc, iArgv);
	const auto ret = RUN_ALL_TESTS();
	evl::Log::invalidate();
	return ret;
}
