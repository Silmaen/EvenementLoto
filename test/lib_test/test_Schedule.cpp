/**
 * @file test_Schedule.cpp
 * @author Silmaen
 * @date 23/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"

#include "core/Schedule.h"

using namespace evl::core;
using namespace std::chrono_literals;

namespace {

/// One quine, a pause, then a full card: two phases and a pause to play.
auto programme() -> Event {
	Event event;
	event.setName("Loto");
	event.setOrganizerName("APE");
	event.pushGameRound(GameRound{GameRound::Type::OneQuine});
	event.pushGameRound(GameRound{GameRound::Type::Pause});
	event.pushGameRound(GameRound{GameRound::Type::FullCard});
	event.nextState();
	event.nextState();
	return event;
}

/// The phase under way is being played. Checked against end() first: a round showing its
/// end screen has no current phase, and reading it ran off the vector under ASan.
auto phaseRunning(const Event& iEvent) -> bool {
	const auto round = iEvent.getCurrentCGameRound();
	if (round == iEvent.endRounds())
		return false;
	const auto sub = round->getCurrentSubRound();
	return sub != round->endSubRound() && sub->getStatus() == SubGameRound::Status::Running;
}

}// namespace

TEST(Schedule, NoEstimateBeforeAPhaseIsPlayed) {
	const auto event = programme();
	const auto schedule = estimateSchedule(event, 15min, clock::now());
	// Nothing has been measured yet: an estimate would be a guess dressed as a figure.
	EXPECT_FALSE(schedule.estimatedEnd.has_value());
	EXPECT_EQ(schedule.subRoundsLeft, 2U);
	EXPECT_EQ(schedule.pausesLeft, 1U);
	EXPECT_GE(schedule.remaining, duration{15min});
}

TEST(Schedule, EstimatesOnceAPhaseIsPlayed) {
	auto event = programme();
	while (!phaseRunning(event)) event.nextState();
	event.getCurrentGameRound()->addPickedNumber(12);
	event.addWinnerToCurrentRound("Mme Y");
	const auto now = clock::now();
	const auto schedule = estimateSchedule(event, 15min, now);
	ASSERT_TRUE(schedule.estimatedEnd.has_value());
	EXPECT_EQ(schedule.subRoundsLeft, 1U);
	EXPECT_EQ(schedule.pausesLeft, 1U);
	// The pause still to come is in it, at its planned length.
	EXPECT_GE(schedule.estimatedEnd.value() - now, 15min);
}

TEST(Schedule, APauseUnderWayCountsWhatIsLeftOfIt) {
	auto event = programme();
	while (event.getCurrentCGameRound()->getType() != GameRound::Type::Pause) {
		if (phaseRunning(event))
			event.addWinnerToCurrentRound("Mme Y");
		else
			event.nextState();
	}
	const auto pause = event.getCurrentCGameRound();
	ASSERT_EQ(pause->getStatus(), GameRound::Status::Running);
	const auto resumes = pauseResumesAt(*pause, 15min);
	ASSERT_TRUE(resumes.has_value());
	EXPECT_EQ(resumes.value() - pause->getStarting(), 15min);
	// An hour later the pause has overrun: nothing is left of it, never a negative time.
	const auto late = estimateSchedule(event, 15min, pause->getStarting() + 1h);
	EXPECT_EQ(late.pausesLeft, 1U);
	EXPECT_LT(late.remaining, duration{1min});
	// No planned length, no resumption time to show.
	EXPECT_FALSE(pauseResumesAt(*pause, 0min).has_value());
}

TEST(Schedule, DelayIsMeasuredOnThePlannedEnd) {
	auto event = programme();
	while (!phaseRunning(event)) event.nextState();
	event.getCurrentGameRound()->addPickedNumber(12);
	event.addWinnerToCurrentRound("Mme Y");
	const auto now = clock::now();
	// No planned end: no delay to speak of.
	EXPECT_FALSE(estimateSchedule(event, 15min, now).delay.has_value());

	// Planned half an hour ago: the programme is late by at least that much, and the
	// planned end is today's, not tomorrow's — which is what an event started after its
	// planned end once got, showing « -23 h ».
	const auto* zone = std::chrono::current_zone();
	const auto local = zone->to_local(now - 30min);
	const auto timeOfDay =
			std::chrono::floor<std::chrono::minutes>(local - std::chrono::floor<std::chrono::days>(local));
	event.setPlannedEnd(timeOfDay);
	const auto schedule = estimateSchedule(event, 15min, now);
	ASSERT_TRUE(schedule.plannedEnd.has_value());
	ASSERT_TRUE(schedule.delay.has_value());
	EXPECT_EQ(schedule.delay.value(), duration{schedule.estimatedEnd.value() - schedule.plannedEnd.value()});
	EXPECT_GE(schedule.delay.value(), duration{29min});
	EXPECT_LT(schedule.delay.value(), duration{12h});
}

TEST(Schedule, PlannedEndIsATimeOfDay) {
	Event event;
	event.setPlannedEnd(std::chrono::minutes{24 * 60});
	EXPECT_FALSE(event.getPlannedEnd().has_value());
	event.setPlannedEnd(std::chrono::minutes{-5});
	EXPECT_FALSE(event.getPlannedEnd().has_value());
	event.setPlannedEnd(std::chrono::minutes{18 * 60});
	EXPECT_EQ(event.getPlannedEnd(), std::chrono::minutes{18 * 60});
	event.setPlannedEnd(std::nullopt);
	EXPECT_FALSE(event.getPlannedEnd().has_value());
}

TEST(Schedule, DrawRhythmComesFromThePhasesPlayed) {
	auto event = programme();
	while (!phaseRunning(event)) event.nextState();
	EXPECT_FALSE(estimateSchedule(event, 15min, clock::now()).drawInterval.has_value());
	event.getCurrentGameRound()->addPickedNumber(12);
	event.getCurrentGameRound()->addPickedNumber(45);
	// Under way: the phase's own tempo is known, the afternoon's not yet.
	const auto current = estimateSchedule(event, 15min, clock::now() + 1min);
	ASSERT_TRUE(current.currentDrawInterval.has_value());
	EXPECT_GE(current.currentDrawInterval.value(), duration{30s});
	event.addWinnerToCurrentRound("Mme Y");
	EXPECT_TRUE(estimateSchedule(event, 15min, clock::now()).drawInterval.has_value());
}

TEST(Schedule, PrizesAreTalliedByOutcome) {
	Event event;
	event.setName("Loto");
	event.setOrganizerName("APE");
	GameRound round{GameRound::Type::OneQuine};
	round.getSubRound(0)->setPrizes({Prize{"un jambon", 45.0}});
	event.pushGameRound(round);
	GameRound second{GameRound::Type::OneQuine};
	second.getSubRound(0)->setPrizes({Prize{"une tondeuse", 200.0}});
	event.pushGameRound(second);
	GameRound third{GameRound::Type::OneQuine};
	third.getSubRound(0)->setPrizes({Prize{"un panier", 60.0}});
	event.pushGameRound(third);
	event.nextState();
	event.nextState();
	EXPECT_DOUBLE_EQ(tallyPrizes(event).remaining, 305.0);

	const auto settle = [&event](const std::string& iWinner) -> void {
		while (!phaseRunning(event)) event.nextState();
		event.addWinnerToCurrentRound(iWinner);
	};
	settle("Mme Y");
	settle({});// « Passer »: nobody claimed it
	const auto tally = tallyPrizes(event);
	EXPECT_DOUBLE_EQ(tally.given, 45.0);
	EXPECT_DOUBLE_EQ(tally.unclaimed, 200.0);
	EXPECT_DOUBLE_EQ(tally.remaining, 60.0);
}
