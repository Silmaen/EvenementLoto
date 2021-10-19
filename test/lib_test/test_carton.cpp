/**
* @author Silmaen
* @date 18/10/2021
*/
#include "core/carton.h"
#include <gtest/gtest.h>

TEST(carton, activation) {
    evl::Carton carton1;
    EXPECT_TRUE(carton1.isActive());
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::EnCours);
    carton1.deactivate();
    EXPECT_FALSE(carton1.isActive());
    carton1.activate();
    EXPECT_TRUE(carton1.isActive());
}

TEST(carton, string) {
    evl::Carton carton1;
    EXPECT_THROW(carton1.fromString("toto fait du; bateau"), std::exception);
    carton1.fromString("002;;1;26;43;75;80;;4;23;36;47;60;;12;24;31;78;85;;");
    EXPECT_STREQ(carton1.asString().c_str(), "2;;1;26;43;75;80;;4;23;36;47;60;;12;24;31;78;85;;");
}

TEST(carton, play) {
    evl::Carton carton1;
    carton1.fromString("002;;1;26;43;75;80;;4;23;36;47;60;;12;24;31;78;85;;");
    for(uint8_t i: {43, 75, 12, 56, 34, 60, 78, 25, 45, 80, 26, 75})
        carton1.playNumber(i);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::PresqueUneLigne);
    carton1.playNumber(1);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::UneLigne);
    for(uint8_t i: {4, 23, 12, 85, 47, 69, 78})
        carton1.playNumber(i);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::PresqueDeuxLignes);
    carton1.playNumber(36);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::DeuxLignes);
    carton1.playNumber(24);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::PresqueCartonPlein);
    carton1.playNumber(31);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::CartonPlein);
    carton1.unPlayNumber(77);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::CartonPlein);
    carton1.unPlayNumber(78);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::PresqueCartonPlein);
    carton1.resetCarton();
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::EnCours);
}

TEST(carton, generation) {
    evl::Carton carton1;
    carton1.generate(0005);
    EXPECT_EQ(carton1.getStatus(), evl::Carton::ResultCarton::EnCours);
}
