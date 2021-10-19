/**
* @author Silmaen
* @date 18/10/2021
*/
#include "core/random.h"
#include <gtest/gtest.h>

TEST(random, base) {
    evl::RandomNumberGenerator rng(true);// random déterministe
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

TEST(random, manual) {
    evl::RandomNumberGenerator rng;
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
