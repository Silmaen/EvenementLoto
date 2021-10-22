/**
* @author Silmaen
* @date 20/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "core/GameRound.h"
#include <gtest/gtest.h>

using namespace evl::core;

TEST(GameRound, Type) {
    GameRound partie;
    EXPECT_EQ(partie.getType(), GameRound::Type::None);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Aucun type défini");
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie invalide");
    partie.setType(GameRound::Type::FullCard);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Carton Plein");
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie Prête");
    partie.setType(GameRound::Type::Inverse);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Inverse");
}

TEST(GameRound, startStop) {
    GameRound partie;
    EXPECT_EQ(partie.getType(), GameRound::Type::None);
    partie.startGameRound();// ne doit pas fonctionner
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Invalid);
    partie.setType(GameRound::Type::OneQuine);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Une Quine");
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Ready);
    partie.endGameRound();// ne doit pas fonctionner
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Ready);
    partie.startGameRound();
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Started);
    partie.endGameRound();
    EXPECT_EQ(partie.getStatus(), GameRound::Status::Finished);
}
