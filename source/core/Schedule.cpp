/**
 * @file Schedule.cpp
 * @author Silmaen
 * @date 23/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Schedule.h"

namespace evl::core {

namespace {

/// What is left of a planned length once part of it has gone by.
auto leftOf(const duration iPlanned, const duration iElapsed) -> duration {
	return std::max(iPlanned - iElapsed, duration::zero());
}

}// namespace

auto estimateSchedule(const Event& iEvent, const duration iPauseLength, const time_point iNow) -> Schedule {
	Schedule schedule;
	uint32_t played = 0;
	size_t playedDraws = 0;
	duration total = duration::zero();
	for (auto round = iEvent.beginRounds(); round != iEvent.endRounds(); ++round) {
		for (auto sub = round->beginSubRound(); sub != round->endSubRound(); ++sub) {
			if (sub->isFinished()) {
				total += sub->getEnding() - sub->getStarting();
				playedDraws += sub->getDraws().size();
				++played;
			} else if (sub->getStatus() == SubGameRound::Status::Running && !sub->getDraws().empty()) {
				schedule.currentDrawInterval =
						duration{iNow - sub->getStarting()} / static_cast<double>(sub->getDraws().size());
			}
		}
	}
	if (played > 0)
		schedule.averageSubRound = total / static_cast<double>(played);
	if (playedDraws > 0)
		schedule.drawInterval = total / static_cast<double>(playedDraws);

	for (auto round = iEvent.beginRounds(); round != iEvent.endRounds(); ++round) {
		if (round->isFinished())
			continue;
		if (round->getType() == GameRound::Type::Pause) {
			switch (round->getStatus()) {
				case GameRound::Status::Ready:
					schedule.remaining += iPauseLength;
					++schedule.pausesLeft;
					break;
				case GameRound::Status::Running:
					schedule.remaining += leftOf(iPauseLength, iNow - round->getStarting());
					++schedule.pausesLeft;
					break;
				case GameRound::Status::PostScreen:
				case GameRound::Status::Done:
				case GameRound::Status::Invalid:
					break;
			}
			continue;
		}
		for (auto sub = round->beginSubRound(); sub != round->endSubRound(); ++sub) {
			if (sub->isFinished())
				continue;
			++schedule.subRoundsLeft;
			const bool started = sub->getStatus() == SubGameRound::Status::PreScreen ||
								 sub->getStatus() == SubGameRound::Status::Running;
			schedule.remaining +=
					started ? leftOf(schedule.averageSubRound, iNow - sub->getStarting()) : schedule.averageSubRound;
		}
	}
	if (played > 0)
		schedule.estimatedEnd = iNow + std::chrono::duration_cast<clock::duration>(schedule.remaining);
	schedule.plannedEnd = plannedEndTime(iEvent, iNow);
	if (schedule.estimatedEnd.has_value() && schedule.plannedEnd.has_value())
		schedule.delay = duration{schedule.estimatedEnd.value() - schedule.plannedEnd.value()};
	return schedule;
}

auto tallyPrizes(const Event& iEvent) -> PrizeTally {
	PrizeTally tally;
	for (auto round = iEvent.beginRounds(); round != iEvent.endRounds(); ++round) {
		for (auto sub = round->beginSubRound(); sub != round->endSubRound(); ++sub) {
			if (!sub->isFinished())
				tally.remaining += sub->getValue();
			else if (sub->getWinner().empty())
				tally.unclaimed += sub->getValue();
			else
				tally.given += sub->getValue();
		}
	}
	return tally;
}

auto plannedEndTime(const Event& iEvent, const time_point iNow) -> std::optional<time_point> {
	const auto planned = iEvent.getPlannedEnd();
	if (!planned.has_value())
		return std::nullopt;
	// The occurrence of that time of day nearest to now, yesterday, today or tomorrow: a
	// programme already late still compares with today's, and one planned to end after
	// midnight with tomorrow's. Earliest of an ambiguous time, the one after a skipped
	// one: a change of clocks must not make the estimate throw.
	const auto* zone = std::chrono::current_zone();
	const auto today = std::chrono::floor<std::chrono::days>(zone->to_local(iNow));
	std::optional<time_point> nearest;
	for (const int offset: {-1, 0, 1}) {
		const auto candidate = std::chrono::time_point_cast<clock::duration>(
				zone->to_sys(today + std::chrono::days{offset} + planned.value(), std::chrono::choose::earliest));
		if (!nearest.has_value() || std::chrono::abs(candidate - iNow) < std::chrono::abs(nearest.value() - iNow))
			nearest = candidate;
	}
	return nearest;
}

auto pauseResumesAt(const GameRound& iPause, const duration iPauseLength) -> std::optional<time_point> {
	if (iPause.getType() != GameRound::Type::Pause || iPause.getStatus() != GameRound::Status::Running ||
		iPauseLength <= duration::zero())
		return std::nullopt;
	return iPause.getStarting() + std::chrono::duration_cast<clock::duration>(iPauseLength);
}

}// namespace evl::core
