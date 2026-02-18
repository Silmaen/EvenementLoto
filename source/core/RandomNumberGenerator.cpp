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
	rebuildRemaining();
}
//NOLINTEND(cert-msc32-c,cert-msc51-cpp)

void RandomNumberGenerator::resetPick() {
	m_alreadyPicked.clear();
	rebuildRemaining();
}

auto RandomNumberGenerator::addPick(const uint8_t& iNumber) -> bool {
	if (std::ranges::find(m_alreadyPicked, iNumber) != m_alreadyPicked.end())
		return false;
	m_alreadyPicked.push_back(iNumber);
	std::erase(m_remaining, iNumber);
	return true;
}

auto RandomNumberGenerator::pick() -> uint8_t {
	if (m_remaining.empty())
		return 255;
	std::uniform_int_distribution<size_t> dist(0, m_remaining.size() - 1);
	const auto idx = dist(m_rng);
	const auto n = m_remaining[idx];
	m_alreadyPicked.push_back(n);
	// Swap-and-pop for O(1) removal
	m_remaining[idx] = m_remaining.back();
	m_remaining.pop_back();
	return n;
}

void RandomNumberGenerator::popNum() {
	if (!m_alreadyPicked.empty()) {
		m_remaining.push_back(m_alreadyPicked.back());
		m_alreadyPicked.pop_back();
	}
}

void RandomNumberGenerator::rebuildRemaining() {
	m_remaining.clear();
	m_remaining.reserve(g_maxNumber);
	for (uint8_t i = 1; i <= g_maxNumber; ++i) {
		if (std::ranges::find(m_alreadyPicked, i) == m_alreadyPicked.end())
			m_remaining.push_back(i);
	}
}

}// namespace evl::core
