/**
 * @file test_Distribution.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

#include "core/Distribution.h"
#include "core/Event.h"

using namespace evl::core;

namespace {

/// An event of two full rounds and a pause, ready to be filled.
auto programme() -> Event {
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	GameRound first{GameRound::Type::OneTwoQuineFullCard};
	first.setId(1);
	event.pushGameRound(first);
	event.pushGameRound(GameRound{GameRound::Type::Pause});
	GameRound second{GameRound::Type::OneTwoQuineFullCard};
	second.setId(2);
	event.pushGameRound(second);
	return event;
}

/// Six articles of clearly increasing value, in no particular order.
auto catalogue() -> prizes_type {
	prizes_type prizes;
	for (const int step: {3, 1, 6, 2, 5, 4}) {
		prizes.emplace_back(std::format("lot {}", step), static_cast<double>(step) * 10.0);
	}
	return prizes;
}

/// The value of a sub-round of a round, by index.
auto valueOf(const Event& iEvent, const uint32_t iRound, const uint32_t iSub) -> double {
	return std::next(iEvent.beginRounds(), iRound)->getSubRound(iSub)->getValue();
}

}// namespace

TEST(Distribution, nothingToDoWithoutACatalogue) {
	auto event = programme();
	const auto result = distributePrizes(event);
	EXPECT_EQ(result.placed, 0);
	EXPECT_NE(result.summary.find("catalogue est vide"), std::string::npos);
}

TEST(Distribution, nothingToFill) {
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	event.pushGameRound(GameRound{GameRound::Type::Pause});
	event.setCatalogue(catalogue());
	const auto result = distributePrizes(event);
	EXPECT_EQ(result.placed, 0);
	EXPECT_EQ(result.leftOver, 6);
}

TEST(Distribution, everyArticleIsPlacedAndThePauseIsLeftAlone) {
	auto event = programme();
	event.setCatalogue(catalogue());
	const auto result = distributePrizes(event);
	// Six articles, six sub-rounds across the two rounds: one each.
	EXPECT_EQ(result.placed, 6);
	EXPECT_EQ(result.filledSubRounds, 6);
	EXPECT_EQ(result.leftOver, 0);
	// The catalogue is the master list and keeps everything, so the distribution can be
	// run again.
	EXPECT_EQ(event.getCatalogue().size(), 6);
	// A pause has no prize and was not given one.
	EXPECT_EQ(std::next(event.beginRounds(), 1)->sizeSubRound(), 0);
}

TEST(Distribution, valueRisesWithinARoundAndAcrossTheEvent) {
	auto event = programme();
	event.setCatalogue(catalogue());
	ASSERT_EQ(distributePrizes(event).placed, 6);

	// Within a round: quine, then two quines, then the full card.
	EXPECT_LT(valueOf(event, 0, 0), valueOf(event, 0, 1));
	EXPECT_LT(valueOf(event, 0, 1), valueOf(event, 0, 2));
	EXPECT_LT(valueOf(event, 2, 0), valueOf(event, 2, 1));
	EXPECT_LT(valueOf(event, 2, 1), valueOf(event, 2, 2));

	// Across the event: the last round is the climax, so its full card carries the best
	// prize of the afternoon.
	EXPECT_GT(valueOf(event, 2, 2), valueOf(event, 0, 2));
	EXPECT_NEAR(valueOf(event, 2, 2), 60.0, 0.001);
}

TEST(Distribution, aFlatEventKeepsTheClimbInsideEachRound) {
	auto event = programme();
	event.setCatalogue(catalogue());
	// No climb across the event: every round weighs the same, only the sub-rounds climb.
	ASSERT_EQ(distributePrizes(event, {.climaxStrength = 0.0f}).placed, 6);
	EXPECT_LT(valueOf(event, 0, 0), valueOf(event, 0, 2));
	EXPECT_LT(valueOf(event, 2, 0), valueOf(event, 2, 2));
}

TEST(Distribution, appealCanOutweighThePrice) {
	auto event = programme();
	prizes_type prizes;
	// A costly thing nobody wants, and a cheap thing everybody does.
	Prize dull{"un bon d'achat", 100.0};
	dull.setAttractiveness(0);
	Prize jambon{"un jambon", 30.0};
	jambon.setAttractiveness(5);
	prizes.push_back(dull);
	prizes.push_back(jambon);
	event.setCatalogue(prizes);

	// On price alone, the voucher takes the best slot.
	ASSERT_EQ(distributePrizes(event, {.attractivenessWeight = 0.0f}).placed, 2);
	EXPECT_NEAR(valueOf(event, 2, 2), 100.0, 0.001);

	// On appeal alone, the ham does.
	ASSERT_EQ(distributePrizes(event, {.attractivenessWeight = 1.0f}).placed, 2);
	EXPECT_NEAR(valueOf(event, 2, 2), 30.0, 0.001);
}

TEST(Distribution, aChildRoundOnlyGetsWhatItMay) {
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	event.pushGameRound(GameRound{GameRound::Type::Enfant});
	GameRound adult{GameRound::Type::OneQuine};
	adult.setId(2);
	event.pushGameRound(adult);

	Prize bottle{"une bouteille", 20.0};
	bottle.setChildFriendly(false);
	Prize toy{"un jeu de société", 15.0};
	event.setCatalogue({bottle, toy});

	ASSERT_EQ(distributePrizes(event).placed, 2);
	const auto childPrizes = std::next(event.beginRounds(), 0)->getSubRound(0)->getPrizes();
	ASSERT_EQ(childPrizes.size(), 1);
	EXPECT_STREQ(childPrizes[0].getDesignation().c_str(), "un jeu de société");

	// Switched off, the bottle can land in the children's round like anything else.
	ASSERT_EQ(distributePrizes(event, {.respectChildRounds = false}).placed, 2);
}

TEST(Distribution, heavierSlotsAccumulateTheSurplus) {
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	event.pushGameRound(GameRound{GameRound::Type::OneQuine});
	event.setCatalogue(catalogue());
	// One sub-round, six articles: they all go there, best last.
	const auto result = distributePrizes(event);
	EXPECT_EQ(result.placed, 6);
	EXPECT_EQ(result.filledSubRounds, 1);
	const auto prizes = std::next(event.beginRounds(), 0)->getSubRound(0)->getPrizes();
	ASSERT_EQ(prizes.size(), 6);
	EXPECT_LT(prizes.front().getValue(), prizes.back().getValue());
}

TEST(Distribution, runningItAgainGivesTheSameResult) {
	auto event = programme();
	event.setCatalogue(catalogue());
	ASSERT_EQ(distributePrizes(event).placed, 6);
	const auto first = valueOf(event, 2, 2);
	// Idempotent: the sub-rounds are emptied before being filled again, so a second run
	// does not pile the same articles up twice.
	ASSERT_EQ(distributePrizes(event).placed, 6);
	EXPECT_NEAR(valueOf(event, 2, 2), first, 0.001);
	EXPECT_EQ(std::next(event.beginRounds(), 2)->getSubRound(2)->getPrizes().size(), 1);
}

TEST(Distribution, aRoundUnderWayKeepsItsPrizes) {
	auto event = programme();
	event.setCatalogue(catalogue());
	ASSERT_EQ(distributePrizes(event).placed, 6);
	const auto before = valueOf(event, 0, 0);

	// The event starts: the first sub-round is no longer editable.
	event.nextState();
	event.nextState();
	ASSERT_EQ(event.getStatus(), Event::Status::GameRunning);
	ASSERT_FALSE(std::next(event.beginRounds(), 0)->getSubRound(0)->isEditable());

	// Redistributing must not touch what is being played.
	distributePrizes(event);
	EXPECT_NEAR(valueOf(event, 0, 0), before, 0.001);
}

TEST(Distribution, theCatalogueSurvivesTheFile) {
	auto event = programme();
	event.setCatalogue(catalogue());
	std::ostringstream out(std::ios::out | std::ios::binary);
	event.write(out);

	std::istringstream in(out.str(), std::ios::in | std::ios::binary);
	Event restored;
	restored.read(in, {});
	ASSERT_TRUE(in.good());
	ASSERT_EQ(restored.getCatalogue().size(), 6);
	EXPECT_NEAR(totalValue(restored.getCatalogue()), totalValue(event.getCatalogue()), 0.001);
}
