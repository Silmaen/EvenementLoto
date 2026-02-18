/**
* @author Silmaen
* @date 18/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "../TestMainHelper.h"

#include "core/RandomNumberGenerator.h"

#include <set>

using namespace evl::core;

TEST(RandomNumberGenerator, base) {
	RandomNumberGenerator rng(true);// random déterministe
	EXPECT_EQ(rng.getPicked().size(), 0);
	uint8_t a = rng.pick();
	EXPECT_GE(a, 1);
	a = rng.pick();
	EXPECT_LE(a, 90);
	a = rng.pick();
	EXPECT_GE(a, 1);
	a = rng.pick();
	EXPECT_LE(a, 90);
	EXPECT_EQ(rng.getPicked().size(), 4);
}

TEST(RandomNumberGenerator, manual) {
	RandomNumberGenerator rng;
	rng.popNum();
	EXPECT_TRUE(rng.addPick(16));
	EXPECT_TRUE(rng.addPick(72));
	EXPECT_TRUE(rng.addPick(66));
	EXPECT_FALSE(rng.addPick(72));
	rng.popNum();
	rng.popNum();
	EXPECT_EQ(rng.getPicked().back(), 16);
	rng.resetPick();
	EXPECT_EQ(rng.getPicked().size(), 0);
}

TEST(RandomNumberGenerator, LargePick) {
	RandomNumberGenerator rng(true);
	uint8_t a = 0;
	for (uint8_t i = 0; i < 250; ++i) a = rng.pick();
	EXPECT_EQ(a, 255);
	EXPECT_EQ(rng.getPicked().size(), 90);
}

TEST(RandomNumberGenerator, AllNumbersUnique) {
	RandomNumberGenerator rng(true);
	std::set<uint8_t> picked;
	for (int i = 0; i < 90; ++i) {
		const uint8_t n = rng.pick();
		EXPECT_GE(n, 1);
		EXPECT_LE(n, 90);
		EXPECT_EQ(picked.count(n), 0) << "Duplicate number picked: " << static_cast<int>(n);
		picked.insert(n);
	}
	EXPECT_EQ(picked.size(), 90);
	EXPECT_EQ(rng.pick(), 255);
}

TEST(RandomNumberGenerator, PopAfterExhaustion) {
	RandomNumberGenerator rng(true);
	for (int i = 0; i < 90; ++i) std::ignore = rng.pick();
	EXPECT_EQ(rng.getPicked().size(), 90);
	rng.popNum();
	EXPECT_EQ(rng.getPicked().size(), 89);
	const uint8_t n = rng.pick();
	EXPECT_GE(n, 1);
	EXPECT_LE(n, 90);
}

TEST(RandomNumberGenerator, AddPickAndPick) {
	RandomNumberGenerator rng(true);
	EXPECT_TRUE(rng.addPick(42));
	// Pick should never return 42 now
	for (int i = 0; i < 89; ++i) {
		const uint8_t n = rng.pick();
		EXPECT_NE(n, 42) << "Picked manually added number 42";
	}
	EXPECT_EQ(rng.getPicked().size(), 90);
}
