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

//NOLINTBEGIN(cert-msc32-c,cert-msc51-cpp)
RandomNumberGenerator::RandomNumberGenerator(const bool iDebug) {
	if (iDebug) {
		m_rng.seed(1234);
	} else {
		m_rng.seed(static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));
	}
}
//NOLINTEND(cert-msc32-c,cert-msc51-cpp)

auto RandomNumberGenerator::addPick(const uint8_t& iNumber) -> bool {
	if (std::ranges::find(m_alreadyPicked, iNumber) != m_alreadyPicked.end())
		return false;
	m_alreadyPicked.push_back(iNumber);
	return true;
}

auto RandomNumberGenerator::pick() -> uint8_t {
	if (m_alreadyPicked.size() >= 90)
		return 255;
	auto n = static_cast<uint8_t>(m_dist(m_rng));
	while (std::ranges::find(m_alreadyPicked, n) != m_alreadyPicked.end()) n = static_cast<uint8_t>(m_dist(m_rng));
	m_alreadyPicked.push_back(n);
	return n;
}

void RandomNumberGenerator::popNum() {
	if (!m_alreadyPicked.empty())
		m_alreadyPicked.pop_back();
}

}// namespace evl::core
