/**
* @author Silmaen
* @date 20/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "core/GameRound.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
namespace fs= std::filesystem;
using namespace evl::core;

TEST(GameRound, Invalid) {
    GameRound gr{GameRound::Type::Inverse};
    EXPECT_EQ(gr.getStarting(), epoch);
    EXPECT_EQ(gr.getEnding(), epoch);
    EXPECT_STREQ(gr.getStatusStr().c_str(), "prête");
    EXPECT_STREQ(gr.getTypeStr().c_str(), "Inverse");
#ifdef EVL_DEBUG
    gr.invalidStatus();
    EXPECT_STREQ(gr.getStatusStr().c_str(), "Statut inconnu");
    EXPECT_STREQ(gr.getTypeStr().c_str(), "inconnu");
    gr.restoreStatus();
#endif
}

TEST(GameRound, Type) {
    GameRound gr{GameRound::Type::Enfant};
    gr.closeGameRound();
    gr.addWinner(586);
    gr.startGameRound();
    gr.setType(GameRound::Type::Normal);
    EXPECT_STREQ(gr.getTypeStr().c_str(), "Enfant");
    gr.addWinner(1586);
    EXPECT_EQ(gr.getStatus(), GameRound::Status::DisplayResult);
    EXPECT_EQ(gr.getCurrentCSubRound(), gr.endSubRound());
    gr.addWinner(2586);
    EXPECT_EQ(gr.beginSubRound()->getWinner(), 1586);
}

TEST(GameRound, startStop) {
    GameRound gr{GameRound::Type::Normal};
    gr.removeLastPick();
    gr.addPickedNumber(55);
    gr.startGameRound();
    EXPECT_EQ(gr.getStatusStr(), "démarrée");
    gr.startGameRound();
    gr.removeLastPick();
    gr.addPickedNumber(60);
    gr.addPickedNumber(60);
    gr.addPickedNumber(30);
    gr.addPickedNumber(45);
    gr.removeLastPick();
    gr.addWinner(4875);
    gr.addWinner(4876);
    EXPECT_TRUE(gr.isCurrentSubRoundLast());
    gr.addWinner(4877);
    EXPECT_FALSE(gr.isCurrentSubRoundLast());
    EXPECT_EQ(gr.getStatusStr(), "en affichage");
    gr.closeGameRound();
    EXPECT_EQ(gr.getStatusStr(), "finie");
}

TEST(GameRound, draws) {
    GameRound gr{GameRound::Type::Normal};
    gr.startGameRound();
    gr.addPickedNumber(60);
    gr.addPickedNumber(30);
    gr.addPickedNumber(45);
    EXPECT_EQ(gr.sizeDraws(), 3);
    EXPECT_NE(gr.beginReverseDraws(), gr.endReverseDraws());
    EXPECT_NE(gr.beginDraws(), gr.endDraws());
    EXPECT_EQ(*gr.beginReverseDraws(), 45);
    EXPECT_EQ(*gr.beginDraws(), 60);
}

TEST(GameRound, serialize) {
    GameRound gr{GameRound::Type::Enfant};
    gr.startGameRound();
    gr.addPickedNumber(55);
    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "testGameRound.sdeg";

    std::ofstream fileSave;
    fileSave.open(file, std::ios::out | std::ios::binary);
    gr.write(fileSave);
    fileSave.close();

    GameRound gr2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    gr2.read(fileRead);
    fileRead.close();

    EXPECT_EQ(gr2.getType(), GameRound::Type::Enfant);
    fs::remove_all(tmp);
}

TEST(GameRound, TypeNormal) {
    GameRound gr{GameRound::Type::Normal};
    EXPECT_EQ(gr.sizeSubRound(), 3);
    EXPECT_FALSE(gr.isCurrentSubRoundLast());
    auto it= gr.beginSubRound();
    EXPECT_EQ(it->getType(), SubGameRound::Type::OneQuine);
    ++it;
    EXPECT_EQ(it->getType(), SubGameRound::Type::TwoQuines);
    ++it;
    EXPECT_EQ(it->getType(), SubGameRound::Type::FullCard);
    EXPECT_EQ(gr.getSubRound(0)->getType(), SubGameRound::Type::OneQuine);
}

TEST(GameRound, TypeReverse) {
    GameRound gr{GameRound::Type::Inverse};
    EXPECT_TRUE(gr.isCurrentSubRoundLast());
    EXPECT_EQ(gr.sizeSubRound(), 1);
    auto it= gr.beginSubRound();
    EXPECT_EQ(it->getType(), SubGameRound::Type::FullCard);
}

TEST(GameRound, TypeEnfant) {
    GameRound gr{GameRound::Type::Enfant};
    EXPECT_EQ(gr.sizeSubRound(), 1);
    auto it= gr.beginSubRound();
    EXPECT_EQ(it->getType(), SubGameRound::Type::OneQuine);
}
