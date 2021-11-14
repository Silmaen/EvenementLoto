/**
 * @file test_CardPack.cpp
 * @author Silmaen
 * @date 19/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "core/CardPack.h"
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
namespace fs= std::filesystem;
using namespace evl;
using namespace evl::core;

TEST(CardPack, generation) {
    CardPack p;
    p.getName()= "Mon  Loto";

    p.from_json(p.to_json());
    RandomNumberGenerator rng;

    p.generate(200, &rng);

    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "test.sdeg";

    std::ofstream filesave;
    filesave.open(file, std::ios::out | std::ios::binary);
    p.write(filesave);
    filesave.close();

    CardPack p2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    p2.read(fileRead);
    fileRead.close();

    EXPECT_STREQ(p.getName().c_str(), p2.getName().c_str());
    EXPECT_EQ(p.size(), p2.size());
    EXPECT_STREQ(p.getGridCards()[5].asString().c_str(), p2.getGridCards()[5].asString().c_str());
    fs::remove_all(tmp);
}

TEST(CardPack, internalGeneration) {
    CardPack p;
    p.getName()= "Mon  Loto";
    p.generate(200);
    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "testCardPack.sdeg";

    std::ofstream filesave;
    filesave.open(file, std::ios::out | std::ios::binary);
    p.write(filesave);
    filesave.close();

    CardPack p2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    p2.read(fileRead);
    fileRead.close();

    EXPECT_STREQ(p.getName().c_str(), p2.getName().c_str());
    EXPECT_EQ(p.size(), p2.size());
    EXPECT_STREQ(p.getGridCards()[5].asString().c_str(), p2.getGridCards()[5].asString().c_str());
    fs::remove_all(tmp);
}

TEST(CardPack, Selection) {
    CardPack p;
    p.getName()= "Mon  Loto";
    p.generate(200);
    EXPECT_EQ(p.getGridCardsByStatus(GridCard::Status::OutGame).size(), 0);
    EXPECT_EQ(p.getGridCardsByStatus(GridCard::Status::InGame).size(), 200);
}

TEST(CardPack, Reset) {
    CardPack p;
    p.getName()= "Mon  Loto";
    p.generate(200);
    p.fullReset();
    EXPECT_EQ(p.size(), 0);
    EXPECT_STREQ(p.getName().c_str(), "");
}
