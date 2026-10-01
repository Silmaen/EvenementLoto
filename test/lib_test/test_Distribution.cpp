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

#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

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

TEST(Distribution, anOldFileHandsItsManualPrizesToTheCatalogue) {
	// An event as it was before the catalogue existed: every prize sits in its
	// sub-round, placed by hand. Written in the current format it keeps its catalogue,
	// so the old layout is rebuilt here by gathering rather than by reading a fixture.
	auto event = programme();
	event.getGameRound(0)->getSubRound(0)->setPrizes({Prize{"un lot de quine", 10.0}});
	event.getGameRound(0)->getSubRound(2)->setPrizes({Prize{"un carton plein", 50.0}, Prize{"et un bouquet", 15.0}});
	event.getGameRound(2)->getSubRound(1)->setPrizes({Prize{"une double quine", 25.0}});
	ASSERT_TRUE(event.getCatalogue().empty());

	EXPECT_EQ(event.gatherCatalogueFromRounds(), 4);
	EXPECT_NEAR(totalValue(event.getCatalogue()), 100.0, 0.001);
	// Programme order, the pause skipped.
	EXPECT_STREQ(event.getCatalogue().front().getDesignation().c_str(), "un lot de quine");
	EXPECT_STREQ(event.getCatalogue().back().getDesignation().c_str(), "une double quine");

	// And that is enough for the automatic distribution to have something to chew on.
	const auto result = distributePrizes(event);
	EXPECT_EQ(result.placed, 4);
	EXPECT_EQ(result.leftOver, 0);
}

TEST(Distribution, gatheringIgnoresPausesAndEmptyArticles) {
	auto event = programme();
	event.getGameRound(0)->getSubRound(0)->setPrizes({Prize{"un lot", 10.0}, Prize{}});
	EXPECT_EQ(event.gatherCatalogueFromRounds(), 1);
	// Run twice: gathering replaces, it does not pile up.
	EXPECT_EQ(event.gatherCatalogueFromRounds(), 1);
}

TEST(Distribution, readingAPreCatalogueFileFillsTheCatalogue) {
	// Version 8 wrote prizes in the sub-rounds and knew nothing of a catalogue. Its body
	// is built here by writing a current file and relabelling it, the catalogue field
	// being the only difference and sitting at the very end of the body.
	auto event = programme();
	event.getGameRound(0)->getSubRound(0)->setPrizes({Prize{"un jambon", 45.0}});
	event.getGameRound(2)->getSubRound(2)->setPrizes({Prize{"une tondeuse", 200.0}});

	std::ostringstream out(std::ios::out | std::ios::binary);
	event.write(out);
	auto buffer = out.str();
	// Drop the trailing catalogue (its eight-byte count, empty here) and the checksum,
	// declare version 8, and re-sign.
	constexpr std::size_t bodyOffset = 4 + sizeof(uint16_t);
	ASSERT_GT(buffer.size(), bodyOffset + sizeof(uint64_t) + sizeof(uint32_t));
	auto body = buffer.substr(bodyOffset, buffer.size() - bodyOffset - sizeof(uint32_t) - sizeof(uint64_t));
	constexpr uint16_t legacyVersion = 8;
	std::string legacy(sizeof(legacyVersion), '\0');
	std::memcpy(legacy.data(), &legacyVersion, sizeof(legacyVersion));
	legacy += body;

	std::istringstream stream(legacy, std::ios::in | std::ios::binary);
	Event restored;
	restored.read(stream, {});
	ASSERT_TRUE(stream.good());
	// Nothing was written as a catalogue, yet the event arrives with one.
	ASSERT_EQ(restored.getCatalogue().size(), 2);
	EXPECT_NEAR(totalValue(restored.getCatalogue()), 245.0, 0.001);
}

TEST(Distribution, theShippedEventsArriveWithACatalogue) {
	// The real files in `data/`, versions 3, 4 and 6, with prizes placed by hand years
	// ago. This is the case the organizer will actually try the distribution on.
	const std::filesystem::path dataDir{EVL_TEST_DATA_DIR};
	ASSERT_TRUE(is_directory(dataDir)) << dataDir.string();
	std::size_t withPrizes = 0;
	for (const auto& name: {"loto_sou.lev", "loto_sou_2.lev", "super_loto.lev", "test_sou.lev"}) {
		const auto path = dataDir / name;
		if (!is_regular_file(path))
			continue;
		std::ifstream file(path, std::ios::in | std::ios::binary);
		ASSERT_TRUE(file.is_open()) << path.string();
		Event event;
		event.read(file, {});
		ASSERT_TRUE(file.good()) << name;
		// Whatever was in the rounds is now in the catalogue, ready to be redistributed.
		std::size_t inRounds = 0;
		for (auto round = event.beginRounds(); round != event.endRounds(); ++round) {
			if (round->getType() == GameRound::Type::Pause)
				continue;
			for (auto sub = round->beginSubRound(); sub != round->endSubRound(); ++sub) {
				for (const auto& prize: sub->getPrizes()) {
					if (!prize.isEmpty())
						++inRounds;
				}
			}
		}
		EXPECT_EQ(event.getCatalogue().size(), inRounds) << name;
		if (inRounds > 0)
			++withPrizes;
	}
	// At least one of the delivered events really carries prizes, otherwise this test
	// would be passing on nothing.
	EXPECT_GT(withPrizes, 0);
}

TEST(Distribution, aCatalogueWithoutValuesSaysSo) {
	// A version 3 file never stored the prize values: only the designations come back.
	// Ranking them means nothing, and the summary must not pretend otherwise.
	auto event = programme();
	event.setCatalogue({Prize{"un lot"}, Prize{"un autre lot"}});
	const auto result = distributePrizes(event);
	EXPECT_EQ(result.placed, 2);
	EXPECT_NE(result.summary.find("ordre obtenu est arbitraire"), std::string::npos);

	// One value is enough for the ranking to mean something again.
	event.setCatalogue({Prize{"un lot", 10.0}, Prize{"un autre lot"}});
	EXPECT_EQ(distributePrizes(event).summary.find("arbitraire"), std::string::npos);
}
