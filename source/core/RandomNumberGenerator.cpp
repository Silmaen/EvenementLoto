/**
 * @file RandomNumberGenerator.cpp
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "../baseDefine.h"
#include "RandomNumberGenerator.h"

namespace evl::core {

RandomNumberGenerator::RandomNumberGenerator(const bool debug) {
	if (debug) {
		//NOLINTBEGIN
		std::srand(1234);
		//NOLINTEND
	} else
		std::srand(static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
}

auto RandomNumberGenerator::addPick(const uint8_t& num) -> bool {
	if (std::ranges::find(alreadyPicked, num) != alreadyPicked.end())
		return false;
	alreadyPicked.push_back(num);
	return true;
}

auto RandomNumberGenerator::pick() -> uint8_t {
	if (alreadyPicked.size() >= 90)
		return 255;
	auto n = static_cast<uint8_t>(std::rand() % 90 + 1);
	while (std::ranges::find(alreadyPicked, n) != alreadyPicked.end()) n = static_cast<uint8_t>(std::rand() % 90 + 1);
	alreadyPicked.push_back(n);
	return n;
}

void RandomNumberGenerator::popNum() {
	if (!alreadyPicked.empty())
		alreadyPicked.pop_back();
}

}// namespace evl::core
