/**
* @author Silmaen
* @date 18/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "core/RandomNumberGenerator.h"
#include <gtest/gtest.h>

using namespace evl::core;

TEST(RandomNumberGenerator, base) {
    RandomNumberGenerator rng(true);// random déterministe
    EXPECT_EQ(rng.getPicked().size(), 0);
    uint8_t a= rng.pick();
    EXPECT_GE(a, 1);
    a= rng.pick();
    EXPECT_LE(a, 90);
    a= rng.pick();
    EXPECT_GE(a, 1);
    a= rng.pick();
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
    uint8_t a;
    for(uint8_t i= 0; i < 250; ++i)
        a= rng.pick();
    EXPECT_EQ(a, 255);
    EXPECT_EQ(rng.getPicked().size(), 90);
}
