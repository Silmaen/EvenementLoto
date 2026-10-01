/**
 * @file test_Prize.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

#include "core/Prize.h"
#include "core/SubGameRound.h"
#include "core/utilities.h"

#include <sstream>

using namespace evl::core;

TEST(Prize, boundsAreEnforced) {
	Prize prize{"un panier garni", 25.0};
	EXPECT_STREQ(prize.getDesignation().c_str(), "un panier garni");
	EXPECT_NEAR(prize.getValue(), 25.0, 0.001);
	EXPECT_TRUE(prize.isChildFriendly());
	EXPECT_EQ(prize.getAttractiveness(), 0);

	// A negative price is a typing slip, not a discount.
	prize.setValue(-10.0);
	EXPECT_NEAR(prize.getValue(), 25.0, 0.001);
	// The scale the interface draws stops at five.
	prize.setAttractiveness(42);
	EXPECT_EQ(prize.getAttractiveness(), Prize::g_maxAttractiveness);
}

TEST(Prize, emptiness) {
	EXPECT_TRUE(Prize{}.isEmpty());
	EXPECT_FALSE(Prize{"une bouteille"}.isEmpty());
	EXPECT_FALSE(Prize({}, 12.0).isEmpty());
}

TEST(Prize, listHelpers) {
	const prizes_type prizes{Prize{"une moto", 1500.0}, Prize{"un casque", 120.0}, Prize{}};
	EXPECT_NEAR(totalValue(prizes), 1620.0, 0.001);
	// The empty article contributes no line: nothing to show on the screen.
	EXPECT_STREQ(designations(prizes).c_str(), "une moto\nun casque");
	EXPECT_TRUE(designations({}).empty());
	EXPECT_NEAR(totalValue({}), 0.0, 0.001);
}

TEST(Prize, legacyStringBecomesArticles) {
	// The whole value goes on the first article: splitting it equally would invent
	// prices nobody ever entered, while the total is a figure that was really given.
	const auto prizes = prizesFromLegacy("un canard en plastique\ndes chaussettes sales", 30.0);
	ASSERT_EQ(prizes.size(), 2);
	EXPECT_STREQ(prizes[0].getDesignation().c_str(), "un canard en plastique");
	EXPECT_STREQ(prizes[1].getDesignation().c_str(), "des chaussettes sales");
	EXPECT_NEAR(totalValue(prizes), 30.0, 0.001);
	EXPECT_NEAR(prizes[1].getValue(), 0.0, 0.001);

	// No designation but a value: the lot exists, it just has no name yet.
	const auto valueOnly = prizesFromLegacy("", 42.0);
	ASSERT_EQ(valueOnly.size(), 1);
	EXPECT_NEAR(valueOnly[0].getValue(), 42.0, 0.001);

	// Nothing at all stays nothing at all.
	EXPECT_TRUE(prizesFromLegacy("", 0.0).empty());
	// Blank lines are not articles.
	EXPECT_EQ(prizesFromLegacy("\n\nun lot\n\n", 0.0).size(), 1);
}

TEST(Prize, serializeRoundTrip) {
	Prize prize{"une moto", 1500.0};
	prize.setDonor("le garage du coin");
	prize.setAttractiveness(5);
	prize.setChildFriendly(false);

	prize.setId(7);

	std::ostringstream out(std::ios::out | std::ios::binary);
	prize.write(out);
	std::istringstream in(out.str(), std::ios::in | std::ios::binary);
	Prize restored;
	// La version doit être dite : un article écrit aujourd'hui porte un identifiant, que
	// le lecteur ne va chercher que s'il sait à quelle version il a affaire.
	restored.read(in, {.version = getSaveVersion()});
	EXPECT_TRUE(in.good());
	EXPECT_EQ(restored.getId(), 7U);
	EXPECT_STREQ(restored.getDesignation().c_str(), "une moto");
	EXPECT_STREQ(restored.getDonor().c_str(), "le garage du coin");
	EXPECT_NEAR(restored.getValue(), 1500.0, 0.001);
	EXPECT_EQ(restored.getAttractiveness(), 5);
	EXPECT_FALSE(restored.isChildFriendly());
}

TEST(Prize, jsonAndYamlRoundTrip) {
	Prize prize{"un jambon", 45.0};
	prize.setDonor("la charcuterie");
	prize.setAttractiveness(3);
	prize.setChildFriendly(false);

	Prize fromJson;
	fromJson.fromJson(prize.toJson());
	EXPECT_STREQ(fromJson.getDesignation().c_str(), "un jambon");
	EXPECT_STREQ(fromJson.getDonor().c_str(), "la charcuterie");
	EXPECT_NEAR(fromJson.getValue(), 45.0, 0.001);
	EXPECT_EQ(fromJson.getAttractiveness(), 3);
	EXPECT_FALSE(fromJson.isChildFriendly());

	Prize fromYaml;
	fromYaml.fromYaml(prize.toYaml());
	EXPECT_STREQ(fromYaml.getDesignation().c_str(), "un jambon");
	EXPECT_NEAR(fromYaml.getValue(), 45.0, 0.001);
	EXPECT_EQ(fromYaml.getAttractiveness(), 3);
	EXPECT_FALSE(fromYaml.isChildFriendly());
}

TEST(Prize, subRoundExposesTheList) {
	SubGameRound sub{SubGameRound::Type::OneQuine, "un lot\nun autre", 60.0};
	ASSERT_EQ(sub.getPrizes().size(), 2);
	EXPECT_NEAR(sub.getValue(), 60.0, 0.001);
	EXPECT_STREQ(sub.getPrices().c_str(), "un lot\nun autre");

	prizes_type replacement{Prize{"une tondeuse", 200.0}};
	sub.setPrizes(replacement);
	EXPECT_NEAR(sub.getValue(), 200.0, 0.001);

	// Once the round is under way the list is frozen.
	sub.nextStatus();
	sub.nextStatus();
	sub.setPrizes({});
	EXPECT_EQ(sub.getPrizes().size(), 1);
}

TEST(Prize, anArticleWithoutAnIdentifierStillReads) {
	// Un article écrit avant la version 10 n'a pas d'identifiant : le lecteur ne doit
	// pas aller en chercher un, sous peine de décaler tout le reste.
	Prize prize{"un lot ancien", 12.0};
	std::ostringstream out(std::ios::out | std::ios::binary);
	// Écrit à la main dans la disposition d'avant, sans identifiant.
	const auto body = [&prize]() -> std::string {
		std::ostringstream stream(std::ios::out | std::ios::binary);
		prize.write(stream);
		// Le premier champ est l'identifiant, sur quatre octets : le retirer redonne
		// exactement la disposition de la version 9.
		return stream.str().substr(sizeof(uint32_t));
	}();
	out << body;

	std::istringstream in(out.str(), std::ios::in | std::ios::binary);
	Prize restored;
	restored.read(in, {.version = 9});
	EXPECT_TRUE(in.good());
	EXPECT_EQ(restored.getId(), 0U);
	EXPECT_STREQ(restored.getDesignation().c_str(), "un lot ancien");
	EXPECT_NEAR(restored.getValue(), 12.0, 0.001);
}
