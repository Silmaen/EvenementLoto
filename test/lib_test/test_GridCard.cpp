/**
* @author Silmaen
* @date 18/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "core/GridCard.h"
#include <gtest/gtest.h>

using namespace evl::core;

TEST(GridCard, activation) {
    GridCard carton1;
    carton1.from_json(carton1.to_json());
    EXPECT_TRUE(carton1.isActive());
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::InGame);
    carton1.deactivate();
    EXPECT_FALSE(carton1.isActive());
    carton1.activate();
    EXPECT_TRUE(carton1.isActive());
}

TEST(GridCard, string) {
    GridCard carton1;
    EXPECT_THROW(carton1.fromString("toto fait du; bateau"), std::exception);
    carton1.fromString("002;;1;26;43;75;80;;4;23;36;47;60;;12;24;31;78;85;;");
    EXPECT_STREQ(carton1.asString().c_str(), "2;;1;26;43;75;80;;4;23;36;47;60;;12;24;31;78;85;;");
}

TEST(GridCard, play) {
    GridCard carton1;
    carton1.fromString("002;;1;26;43;75;80;;4;23;36;47;60;;12;24;31;78;85;;");
    for(uint8_t i: std::vector<uint8_t>{43U, 75U, 12U, 56U, 34U, 60U, 78U, 25U, 45U, 80U, 26U, 75U})
        carton1.playNumber(i);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::AlmostOneLine);
    carton1.playNumber(1);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::OneLine);
    for(uint8_t i: std::vector<uint8_t>{4U, 23U, 12U, 85U, 47U, 69U, 78U})
        carton1.playNumber(i);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::AlmostTwoLines);
    carton1.playNumber(36);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::TwoLines);
    carton1.playNumber(24);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::AlmostFull);
    carton1.playNumber(31);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::Full);
    carton1.unPlayNumber(77);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::Full);
    carton1.unPlayNumber(78);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::AlmostFull);
    carton1.reset();
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::InGame);
}

TEST(GridCard, generation) {
    GridCard carton1;
    carton1.generate(0005);
    EXPECT_EQ(carton1.getStatus(), GridCard::Status::InGame);
}
