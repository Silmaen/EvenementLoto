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

TEST(SubGameRound, serialize) {
    SubGameRound partie(SubGameRound::Type::TwoQuines, "une moto");
    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "testGameRound.sdeg";

    std::ofstream fileSave;
    fileSave.open(file, std::ios::out | std::ios::binary);
    partie.write(fileSave);
    fileSave.close();

    SubGameRound partie2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    partie2.read(fileRead);
    fileRead.close();

    EXPECT_EQ(partie2.getType(), SubGameRound::Type::TwoQuines);
    EXPECT_EQ(partie2.getWinner(), 0);
    EXPECT_STREQ(partie2.getPrices().c_str(), "une moto");

    fs::remove_all(tmp);
}
