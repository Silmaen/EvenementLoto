/**
 * @file test_Report.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

#include "core/Event.h"
#include "core/Report.h"

using namespace evl::core;

namespace {

/// An event played to the end of its first round, with prizes, a donor and a winner.
auto playedEvent() -> Event {
	Event event;
	event.setName("Loto du Sou");
	event.setOrganizerName("Sou des écoles");
	event.setLocation("Salle des fêtes");

	GameRound round{GameRound::Type::OneQuine};
	round.setId(1);
	Prize jambon{"un jambon", 45.0};
	jambon.setDonor("la charcuterie");
	round.getSubRound(0)->setPrizes({jambon, Prize{"une bouteille", 12.0}});
	event.pushGameRound(round);
	event.pushGameRound(GameRound{GameRound::Type::Pause});
	GameRound second{GameRound::Type::FullCard};
	second.setId(2);
	event.pushGameRound(second);

	event.nextState();// EventStarting
	event.nextState();// GameRunning
	// A sub-round with prizes opens on the announcement screen: one more step is what
	// starts the draws.
	while (event.getStatus() == Event::Status::GameRunning &&
		   event.getCurrentCGameRound()->getCurrentSubRound()->getStatus() != SubGameRound::Status::Running) {
		event.nextState();
	}
	event.getCurrentGameRound()->addPickedNumber(12);
	event.addWinnerToCurrentRound("carton 42");
	return event;
}

}// namespace

TEST(Report, describesTheEvent) {
	const auto report = buildReport(playedEvent());
	EXPECT_NE(report.find("# Rapport de fin d'événement"), std::string::npos);
	EXPECT_NE(report.find("Loto du Sou"), std::string::npos);
	EXPECT_NE(report.find("Sou des écoles"), std::string::npos);
	EXPECT_NE(report.find("Salle des fêtes"), std::string::npos);
}

TEST(Report, countsWhatWasPlayed) {
	const auto report = buildReport(playedEvent());
	// Two rounds, the pause not being one of them. The first is played but its
	// post-round screen is still up, so it is not over yet.
	EXPECT_NE(report.find("| Parties terminées | 0 sur 2 |"), std::string::npos);
	EXPECT_NE(report.find("| Manches jouées | 1 |"), std::string::npos);
	EXPECT_NE(report.find("| Numéros tirés | 1 |"), std::string::npos);
	EXPECT_NE(report.find("| Valeur totale des lots | 57.00 € |"), std::string::npos);
	EXPECT_NE(report.find("| Gagnants enregistrés | 1 |"), std::string::npos);
	EXPECT_NE(report.find("| Manches sans gagnant | 0 |"), std::string::npos);
}

TEST(Report, listsWinnersAndDonors) {
	const auto report = buildReport(playedEvent());
	EXPECT_NE(report.find("carton 42"), std::string::npos);
	EXPECT_NE(report.find("## Donateurs"), std::string::npos);
	EXPECT_NE(report.find("* la charcuterie — un jambon (45.00 €)"), std::string::npos);
	// The round that never ran says so instead of pretending nobody claimed it.
	EXPECT_NE(report.find("manche non jouée"), std::string::npos);
}

TEST(Report, leavesOutPausesAndEmptySections) {
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	event.pushGameRound(GameRound{GameRound::Type::Pause});
	const auto report = buildReport(event);
	// A pause is not a game, so there is nothing to report.
	EXPECT_NE(report.find("Aucune partie à rapporter."), std::string::npos);
	// And no donor was written down, so no one is thanked for nothing.
	EXPECT_EQ(report.find("## Donateurs"), std::string::npos);
}

TEST(Report, aDesignationCannotBreakTheTable) {
	Event event;
	event.setName("Loto");
	event.setOrganizerName("Amicale");
	GameRound round{GameRound::Type::OneQuine};
	round.getSubRound(0)->setPrizes({Prize{"un lot | douteux\net long", 5.0}});
	event.pushGameRound(round);
	const auto report = buildReport(event);
	// The pipe is escaped and the line break folded, so the row stays a row.
	EXPECT_NE(report.find("un lot \\| douteux et long"), std::string::npos);
}
