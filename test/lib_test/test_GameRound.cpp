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
    EXPECT_STREQ(gr.getStatusStr().c_str(), "prêt");
    EXPECT_STREQ(gr.getTypeStr().c_str(), "Inverse");
#ifdef EVL_DEBUG
    gr.invalidStatus();
    EXPECT_STREQ(gr.getStatusStr().c_str(), "inconnu");
    EXPECT_STREQ(gr.getTypeStr().c_str(), "inconnu");
    gr.restoreStatus();
#endif
}

TEST(GameRound, Id) {
    GameRound gr{GameRound::Type::Enfant};
    EXPECT_EQ(gr.getID(), 0);
    gr.setID(665);
    EXPECT_EQ(gr.getID(), 665);
}

TEST(GameRound, Name) {
    GameRound gr;
    EXPECT_STREQ(gr.getName().c_str(), "Partie");
    gr.setID(665);
    gr.setType(GameRound::Type::Inverse);
    EXPECT_STREQ(gr.getName().c_str(), "Partie 665 Inverse");
}

TEST(GameRound, SimpleFlow) {
    GameRound gr{GameRound::Type::OneQuine};
    auto sub= gr.getSubRound(0);
    sub->define(SubGameRound::Type::OneQuine, "un canard en plastique", 1523);
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Ready);
    EXPECT_EQ(sub->getStatus(), SubGameRound::Status::Ready);
    EXPECT_EQ(gr.getLastCancelableDraw(), 255);
    gr.nextStatus();
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Running);
    EXPECT_EQ(gr.getLastCancelableDraw(), 255);
    EXPECT_EQ(sub->getStatus(), SubGameRound::Status::PreScreen);
    gr.nextStatus();
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Running);
    EXPECT_EQ(sub->getStatus(), SubGameRound::Status::Running);
    gr.addPickedNumber(39);
    gr.addPickedNumber(12);
    gr.addWinner("toto");
    EXPECT_EQ(gr.getStatus(), GameRound::Status::PostScreen);
    EXPECT_EQ(sub->getStatus(), SubGameRound::Status::Done);
    gr.nextStatus();
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Done);
    EXPECT_EQ(sub->getStatus(), SubGameRound::Status::Done);
}

TEST(GameRound, MultipleFlow) {
    GameRound gr{GameRound::Type::OneQuineFullCard};
    auto sub1= gr.getSubRound(0);
    auto sub2= gr.getSubRound(1);
    sub1->define(SubGameRound::Type::OneQuine, "un canard en plastique", 523);
    sub2->define(SubGameRound::Type::FullCard, "deux canards en plastique", 40);
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Ready);
    EXPECT_EQ(sub1->getStatus(), SubGameRound::Status::Ready);
    EXPECT_EQ(sub2->getStatus(), SubGameRound::Status::Ready);
    gr.nextStatus();
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Running);
    EXPECT_EQ(sub1->getStatus(), SubGameRound::Status::PreScreen);
    EXPECT_EQ(sub2->getStatus(), SubGameRound::Status::Ready);
    gr.nextStatus();
    gr.addPickedNumber(39);
    gr.addPickedNumber(12);
    gr.addWinner("toto");
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Running);
    EXPECT_EQ(sub1->getStatus(), SubGameRound::Status::Done);
    EXPECT_EQ(sub2->getStatus(), SubGameRound::Status::PreScreen);
    EXPECT_EQ(gr.getLastCancelableDraw(), 255);
    gr.nextStatus();
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Running);
    EXPECT_EQ(sub1->getStatus(), SubGameRound::Status::Done);
    EXPECT_EQ(sub2->getStatus(), SubGameRound::Status::Running);
    gr.addPickedNumber(21);
    gr.addPickedNumber(6);
    EXPECT_EQ(gr.getLastCancelableDraw(), 6);
    gr.addWinner("titi");
    EXPECT_EQ(gr.getStatus(), GameRound::Status::PostScreen);
    EXPECT_EQ(sub1->getStatus(), SubGameRound::Status::Done);
    EXPECT_EQ(sub2->getStatus(), SubGameRound::Status::Done);
    gr.nextStatus();
    EXPECT_EQ(gr.getStatus(), GameRound::Status::Done);
    EXPECT_EQ(sub1->getStatus(), SubGameRound::Status::Done);
    EXPECT_EQ(sub2->getStatus(), SubGameRound::Status::Done);
}

TEST(GameRound, Type) {
    GameRound gr{GameRound::Type::Enfant};
    EXPECT_NE(gr.beginSubRound(), gr.endSubRound());
    gr.addWinner("586");
    gr.nextStatus();
    gr.setType(GameRound::Type::OneTwoQuineFullCard);
    EXPECT_STREQ(gr.getTypeStr().c_str(), "Enfant");
    gr.addWinner("1586");
    EXPECT_EQ(gr.getStatus(), GameRound::Status::PostScreen);
    EXPECT_TRUE(gr.isCurrentSubRoundLast());
    gr.addWinner("2586");
    EXPECT_STREQ(gr.beginSubRound()->getWinner().c_str(), "1586");
}

TEST(GameRound, Types) {
    GameRound gr{GameRound::Type::OneQuine};
    EXPECT_STREQ(gr.getTypeStr().c_str(), "Simple quine");
    EXPECT_EQ(gr.sizeSubRound(), 1);
    gr.setType(GameRound::Type::TwoQuines);
    EXPECT_STREQ(gr.getTypeStr().c_str(), "Double quine");
    EXPECT_EQ(gr.sizeSubRound(), 1);
    gr.setType(GameRound::Type::FullCard);
    EXPECT_STREQ(gr.getTypeStr().c_str(), "Gros lot");
    EXPECT_EQ(gr.sizeSubRound(), 1);
    gr.setType(GameRound::Type::OneQuineFullCard);
    EXPECT_STREQ(gr.getTypeStr().c_str(), "une quine et carton");
    EXPECT_EQ(gr.sizeSubRound(), 2);
    gr.setType(GameRound::Type::OneTwoQuineFullCard);
    EXPECT_STREQ(gr.getTypeStr().c_str(), "normale");
    EXPECT_EQ(gr.sizeSubRound(), 3);
    gr.setType(GameRound::Type::Pause);
    EXPECT_STREQ(gr.getTypeStr().c_str(), "Pause");
    EXPECT_EQ(gr.sizeSubRound(), 0);
    gr.setType(GameRound::Type{-1});
}
TEST(GameRound, startStop) {
    GameRound gr{GameRound::Type::OneTwoQuineFullCard};
    EXPECT_TRUE(gr.emptyDraws());
    gr.removeLastPick();
    gr.addPickedNumber(55);
    EXPECT_TRUE(gr.emptyDraws());
    gr.nextStatus();
    EXPECT_EQ(gr.getStatusStr(), "démarré");
    gr.nextStatus();
    gr.removeLastPick();
    gr.addPickedNumber(60);
    EXPECT_FALSE(gr.emptyDraws());
    gr.addPickedNumber(60);
    gr.addPickedNumber(30);
    gr.addPickedNumber(45);
    EXPECT_EQ(gr.drawsCount(), 4);
    gr.removeLastPick();
    gr.addWinner("4875");
    gr.addWinner("4876");
    EXPECT_TRUE(gr.isCurrentSubRoundLast());
    gr.addWinner("4877");
    EXPECT_FALSE(gr.isCurrentSubRoundLast());
    EXPECT_EQ(gr.getStatusStr(), "écran de fin");
    gr.nextStatus();
    gr.nextStatus();
    EXPECT_EQ(gr.getStatusStr(), "terminé");
}

TEST(GameRound, draws) {
    GameRound gr{GameRound::Type::OneTwoQuineFullCard};
    gr.nextStatus();
    gr.addPickedNumber(60);
    gr.addPickedNumber(30);
    gr.addPickedNumber(45);
    EXPECT_EQ(gr.drawsCount(), 3);
    auto draws= gr.getAllDraws();
    EXPECT_NE(draws.begin(), draws.end());
    EXPECT_NE(draws.rbegin(), draws.rend());
    EXPECT_EQ(*draws.rbegin(), 45);
    EXPECT_EQ(*draws.begin(), 60);
}

TEST(GameRound, serialize) {
    GameRound gr{GameRound::Type::Enfant};
    gr.nextStatus();
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
    gr2.read(fileRead, evl::currentSaveVersion);
    fileRead.close();

    EXPECT_EQ(gr2.getType(), GameRound::Type::Enfant);
    fs::remove_all(tmp);
}

TEST(GameRound, TypeNormal) {
    GameRound gr{GameRound::Type::OneTwoQuineFullCard};
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

TEST(GameRound, results) {
    GameRound gr{GameRound::Type::OneTwoQuineFullCard};
    gr.nextStatus();
    // simulate draws & winners
    gr.addPickedNumber(5);
    gr.addPickedNumber(78);
    gr.addPickedNumber(45);
    gr.addPickedNumber(23);
    gr.addWinner("Mr X");
    gr.addPickedNumber(65);
    gr.addPickedNumber(12);
    EXPECT_STREQ(gr.getDrawStr().c_str(),
                 "simple quine: 5 78 45 23\ndouble quine: 65 12\n");
    EXPECT_STREQ(gr.getWinnerStr().c_str(),
                 "simple quine: Mr X\n");
    gr.addPickedNumber(14);
    gr.addPickedNumber(26);
    gr.addWinner("Mr Y");
    gr.addPickedNumber(27);
    EXPECT_STREQ(gr.getDrawStr().c_str(),
                 "simple quine: 5 78 45 23\ndouble quine: 65 12 14 26\ncarton plein: 27\n");
    EXPECT_STREQ(gr.getWinnerStr().c_str(),
                 "simple quine: Mr X\ndouble quine: Mr Y\n");
}
