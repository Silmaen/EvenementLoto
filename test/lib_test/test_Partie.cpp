/**
* @author Silmaen
* @date 20/10/2021
*/
#include "core/Partie.h"
#include <gtest/gtest.h>

using namespace evl::core;

TEST(Partie, Type) {
    Partie partie;
    EXPECT_EQ(partie.getType(), Partie::Type::Aucun);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Aucun type défini");
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie invalide");
    partie.setType(Partie::Type::CartonPlein);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Carton Plein");
    EXPECT_STREQ(partie.getStatusStr().c_str(), "Partie Prête");
    partie.setType(Partie::Type::Inverse);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Inverse");
}

TEST(Partie, startStop) {
    Partie partie;
    EXPECT_EQ(partie.getType(), Partie::Type::Aucun);
    partie.startPartie();// ne doit pas fonctionner
    EXPECT_EQ(partie.getStatus(), Partie::Status::Invalid);
    partie.setType(Partie::Type::UneQuine);
    EXPECT_STREQ(partie.getTypeStr().c_str(), "Une Quine");
    EXPECT_EQ(partie.getStatus(), Partie::Status::Ready);
    partie.finishPartie();// ne doit pas fonctionner
    EXPECT_EQ(partie.getStatus(), Partie::Status::Ready);
    partie.startPartie();
    EXPECT_EQ(partie.getStatus(), Partie::Status::Started);
    partie.finishPartie();
    EXPECT_EQ(partie.getStatus(), Partie::Status::Finished);
}
