/**
 * @file Schedule.h
 * @author Silmaen
 * @date 23/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "Event.h"
#include "timeFunctions.h"

#include <optional>

namespace evl::core {

/// Where the programme stands, and when it should end.
struct Schedule {
	/// When the last round should end; empty until a phase has been played, since there is
	/// nothing to measure before.
	std::optional<time_point> estimatedEnd;
	/// What is left to play, pauses included.
	duration remaining = duration::zero();
	/// The mean length of a played phase, the basis of the estimate.
	duration averageSubRound = duration::zero();
	/// Phases still to finish, the one under way included.
	uint32_t subRoundsLeft = 0;
	/// Pauses still to come or under way.
	uint32_t pausesLeft = 0;
	/// The planned end, today or on the day the event started.
	std::optional<time_point> plannedEnd;
	/// How late the estimate is on the planned end; negative when ahead. Empty without
	/// an estimate or without a planned end.
	std::optional<duration> delay;
	/// Mean time between two numbers over the phases played, checks and announcements
	/// included: the tempo of the afternoon.
	std::optional<duration> drawInterval;
	/// The same over the phase under way only.
	std::optional<duration> currentDrawInterval;
};

/// What the prizes amount to, at this point of the event.
struct PrizeTally {
	/// Won and handed out: phases over, with a winner.
	double given = 0.0;
	/// Phases over that nobody claimed.
	double unclaimed = 0.0;
	/// Still to be played for.
	double remaining = 0.0;
};

/**
 * @brief Adds up the value of the prizes already won, left unclaimed and still in play.
 * @param iEvent The event.
 * @return The three totals.
 */
[[nodiscard]] auto tallyPrizes(const Event& iEvent) -> PrizeTally;

/**
 * @brief The planned end as a time: the occurrence of the event's time of day nearest to
 * now, so that it may fall the next day for a programme ending after midnight.
 * @param iEvent The event.
 * @param iNow The present time.
 * @return The planned end, empty if none is set.
 */
[[nodiscard]] auto plannedEndTime(const Event& iEvent, time_point iNow) -> std::optional<time_point>;

/**
 * @brief Estimates the end of the event from the phases already played.
 *
 * Each phase still to play takes the mean length of those played, a phase under way only
 * what it has left of it, and each pause the planned length, the one under way only
 * what it has left.
 * @param iEvent The event.
 * @param iPauseLength The planned length of a pause.
 * @param iNow The present time.
 * @return The schedule.
 */
[[nodiscard]] auto estimateSchedule(const Event& iEvent, duration iPauseLength, time_point iNow) -> Schedule;

/**
 * @brief When a pause should end.
 * @param iPause The pause.
 * @param iPauseLength The planned length of a pause.
 * @return The planned end, empty if the round is not a pause under way or no length is planned.
 */
[[nodiscard]] auto pauseResumesAt(const GameRound& iPause, duration iPauseLength) -> std::optional<time_point>;

}// namespace evl::core
