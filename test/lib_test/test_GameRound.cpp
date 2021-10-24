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
    GameRound partie;
    partie.setType(GameRound::Type{-2});
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Type de partie inconnu");
#ifdef EVL_DEBUG
    partie.invalidStatus();
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Statut inconnu");
#endif
    partie.setType(GameRound::Type::FullCard);
    partie.startGameRound();
    partie.setType(GameRound::Type::OneQuine);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Carton plein");
}

TEST(GameRound, Type) {
    GameRound partie;
    partie.setType(GameRound::Type::None);
    EXPECT_EQ(partie.getType(), GameRound::Type::None);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Aucun type défini");
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie invalide");
    partie.setType(GameRound::Type::Inverse);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Inverse");
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie prête");
    partie.setType(GameRound::Type::TwoQuines);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Deux quines");
}

TEST(GameRound, startStop) {
    GameRound partie;
    EXPECT_EQ(partie.getType(), GameRound::Type::None);
    partie.startGameRound();// ne dois pas fonctionner
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Invalid);
    partie.setType(GameRound::Type::OneQuine);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Un quine");
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Ready);
    partie.endGameRound();// ne dois pas fonctionner
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Ready);
    partie.startGameRound();
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Started);
    partie.endGameRound();
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Result);
}

TEST(GameRound, flowPause) {
    GameRound partie;
    partie.setType(GameRound::Type::OneQuine);
    partie.startGameRound();
    partie.resumeGameRound();
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie démarrée");
    partie.pauseGameRound();
    partie.pauseGameRound();
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie en pause");
    partie.resumeGameRound();
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie démarrée");
}

TEST(GameRound, flowNumber) {
    GameRound partie;
    partie.setType(GameRound::Type::OneQuine);
    partie.endGameRound();
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie prête");
    partie.addPickedNumber(90);
    partie.startGameRound();
    partie.addPickedNumber(78);
    partie.addPickedNumber(18);
    partie.addPickedNumber(44);
    partie.closeGameRound();
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie démarrée");
    partie.endGameRound();
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie en affichage");
    partie.closeGameRound();
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie finie");
    EXPECT_EQ(partie.getDraws(), (std::vector<uint8_t>{78, 18, 44}));
}

TEST(GameRound, serialize) {
    GameRound partie;
    partie.setType(GameRound::Type::OneQuine);
    partie.startGameRound();
    partie.addPickedNumber(78);
    partie.addPickedNumber(18);
    partie.addPickedNumber(44);
    partie.endGameRound();
    partie.closeGameRound();

    fs::path tmp= fs::temp_directory_path() / "test";
    fs::create_directories(tmp);
    fs::path file= tmp / "testGameRound.sdeg";

    std::ofstream fileSave;
    fileSave.open(file, std::ios::out | std::ios::binary);
    partie.write(fileSave);
    fileSave.close();

    GameRound partie2;
    std::ifstream fileRead;
    fileRead.open(file, std::ios::in | std::ios::binary);
    partie2.read(fileRead);
    fileRead.close();

    EXPECT_EQ(partie.getStatus(), partie2.getStatus());
    EXPECT_EQ(partie.getType(), partie2.getType());
    EXPECT_EQ(partie.getStarting(), partie2.getStarting());
    EXPECT_EQ(partie.getEnding(), partie2.getEnding());
    EXPECT_EQ(partie.getDraws(), partie2.getDraws());
    fs::remove_all(tmp);
}
