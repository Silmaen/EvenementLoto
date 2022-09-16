/**
* @author Silmaen
* @date 27/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "core/SubGameRound.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
namespace fs= std::filesystem;
using namespace evl::core;

TEST(SubGameRound, types) {
    SubGameRound partie(SubGameRound::Type::OneQuine, "");
    EXPECT_STREQ(partie.getTypeStr().c_str(), "simple quine");
    SubGameRound partie2(SubGameRound::Type::FullCard, "");
    EXPECT_STREQ(partie2.getTypeStr().c_str(), "carton plein");
    SubGameRound partie3(SubGameRound::Type::Inverse, "");
    EXPECT_STREQ(partie3.getTypeStr().c_str(), "inverse");
    SubGameRound partie4(SubGameRound::Type{-1}, "une moto");
    EXPECT_STREQ(partie4.getTypeStr().c_str(), "Type Inconnu");
}

TEST(SubGameRound, serialize) {
    SubGameRound partie(SubGameRound::Type::TwoQuines, "une moto", 152.12);
    EXPECT_FALSE(partie.isFinished());
    partie.setWinner("mr X");
    EXPECT_TRUE(partie.isFinished());
    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "testSubGameRound.sdeg";

    std::ofstream fileSave;
    fileSave.open(file, std::ios::out | std::ios::binary);
    partie.write(fileSave);
    fileSave.close();

    SubGameRound partie2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    partie2.read(fileRead, evl::currentSaveVersion);
    fileRead.close();

    EXPECT_EQ(partie2.getType(), SubGameRound::Type::TwoQuines);
    EXPECT_STREQ(partie2.getTypeStr().c_str(), "double quine");
    EXPECT_STREQ(partie2.getWinner().c_str(), "mr X");
    EXPECT_STREQ(partie2.getPrices().c_str(), "une moto");
    EXPECT_NEAR(partie2.getValue(), 152.12, 0.001);
    fs::remove_all(tmp);
}
