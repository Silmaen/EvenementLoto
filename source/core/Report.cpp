/**
 * @file Report.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Report.h"

#include "Event.h"

namespace evl::core {

namespace {

/// A euro amount, always with its two decimals.
auto euros(const double iValue) -> std::string { return std::format("{:.2f} €", iValue); }

/// A markdown table cell, the pipes escaped so a designation cannot break the table.
auto cell(std::string iText) -> std::string {
	for (auto pos = iText.find('|'); pos != std::string::npos; pos = iText.find('|', pos + 2))
		iText.replace(pos, 1, "\\|");
	// A line break inside a cell would end the row.
	std::ranges::replace(iText, '\n', ' ');
	if (iText.empty())
		return " ";
	return iText;
}

/// A duration, or a dash when the round never ran.
auto elapsed(const time_point& iStart, const time_point& iEnd) -> std::string {
	if (iStart == g_epoch || iEnd == g_epoch || iEnd < iStart)
		return "--";
	return formatDuration(iEnd - iStart);
}

/// The header of the report: who, where, when.
void appendHeader(std::string& ioReport, const Event& iEvent) {
	ioReport += "# Rapport de fin d'événement\n\n";
	ioReport += std::format("## {}\n\n", iEvent.getName());
	ioReport += std::format("* Organisateur : {}\n", iEvent.getOrganizerName());
	if (!iEvent.getLocation().empty())
		ioReport += std::format("* Lieu : {}\n", iEvent.getLocation());
	if (iEvent.getStarting() != g_epoch) {
		ioReport += std::format("* Date : {}\n", formatCalendar(iEvent.getStarting()));
		ioReport += std::format("* Début : {}\n", formatClockNoSecond(iEvent.getStarting()));
	}
	if (iEvent.getEnding() != g_epoch)
		ioReport += std::format("* Fin : {}\n", formatClockNoSecond(iEvent.getEnding()));
	ioReport += std::format("* Durée : {}\n\n", elapsed(iEvent.getStarting(), iEvent.getEnding()));
}

/// What the whole afternoon amounted to.
struct Totals {
	std::size_t rounds = 0;///< Rounds that are not pauses.
	std::size_t finishedRounds = 0;///< Of those, the ones that went all the way to Done.
	std::size_t subRounds = 0;///< Sub-rounds that were played.
	std::size_t draws = 0;///< Numbers drawn.
	std::size_t winners = 0;///< Sub-rounds that ended with a name.
	std::size_t unclaimed = 0;///< Sub-rounds that ended without one.
	double value = 0.0;///< Total value of the prizes put in play.
};

auto computeTotals(const Event& iEvent) -> Totals {
	Totals totals;
	for (auto round = iEvent.beginRounds(); round != iEvent.endRounds(); ++round) {
		if (round->getType() == GameRound::Type::Pause)
			continue;
		++totals.rounds;
		if (round->isFinished())
			++totals.finishedRounds;
		totals.draws += round->drawsCount();
		for (auto sub = round->beginSubRound(); sub != round->endSubRound(); ++sub) {
			totals.value += sub->getValue();
			if (!sub->isFinished())
				continue;
			++totals.subRounds;
			if (sub->getWinner().empty())
				++totals.unclaimed;
			else
				++totals.winners;
		}
	}
	return totals;
}

void appendSummary(std::string& ioReport, const Totals& iTotals) {
	ioReport += "## Résumé\n\n";
	ioReport += "| Élément | Valeur |\n|---|---|\n";
	ioReport += std::format("| Parties terminées | {} sur {} |\n", iTotals.finishedRounds, iTotals.rounds);
	ioReport += std::format("| Manches jouées | {} |\n", iTotals.subRounds);
	ioReport += std::format("| Numéros tirés | {} |\n", iTotals.draws);
	ioReport += std::format("| Valeur totale des lots | {} |\n", euros(iTotals.value));
	ioReport += std::format("| Gagnants enregistrés | {} |\n", iTotals.winners);
	ioReport += std::format("| Manches sans gagnant | {} |\n\n", iTotals.unclaimed);
}

void appendRounds(std::string& ioReport, const Event& iEvent) {
	ioReport += "## Parties\n\n";
	bool any = false;
	for (auto round = iEvent.beginRounds(); round != iEvent.endRounds(); ++round) {
		if (round->getType() == GameRound::Type::Pause)
			continue;
		any = true;
		ioReport += std::format("### {}\n\n", round->getName());
		ioReport += std::format("* Type : {}\n", round->getTypeStr());
		ioReport += std::format("* État : {}\n", round->getStatusStr());
		if (round->getStarting() != g_epoch)
			ioReport += std::format("* Début : {}\n", formatClockNoSecond(round->getStarting()));
		ioReport += std::format("* Durée : {}\n", elapsed(round->getStarting(), round->getEnding()));
		ioReport += std::format("* Numéros tirés : {}\n\n", round->drawsCount());

		ioReport += "| Manche | Lots | Valeur | Gagnant |\n|---|---|---|---|\n";
		for (auto sub = round->beginSubRound(); sub != round->endSubRound(); ++sub) {
			std::string winner = sub->getWinner();
			if (!sub->isFinished())
				winner = "manche non jouée";
			else if (winner.empty())
				winner = "sans gagnant";
			ioReport += std::format("| {} | {} | {} | {} |\n", cell(sub->getTypeStr()), cell(sub->getPrices()),
									cell(euros(sub->getValue())), cell(winner));
		}
		ioReport += "\n";
	}
	if (!any)
		ioReport += "Aucune partie à rapporter.\n\n";
}

void appendDonors(std::string& ioReport, const Event& iEvent) {
	std::string body;
	for (auto round = iEvent.beginRounds(); round != iEvent.endRounds(); ++round) {
		if (round->getType() == GameRound::Type::Pause)
			continue;
		for (auto sub = round->beginSubRound(); sub != round->endSubRound(); ++sub) {
			for (const auto& prize: sub->getPrizes()) {
				if (prize.getDonor().empty())
					continue;
				body += std::format("* {} — {}", prize.getDonor(), prize.getDesignation());
				if (prize.getValue() > 0.0)
					body += std::format(" ({})", euros(prize.getValue()));
				body += "\n";
			}
		}
	}
	if (body.empty())
		return;
	// Nothing to thank anybody for if no donor was written down: the section only
	// appears when it has something to say.
	ioReport += "## Donateurs\n\n";
	ioReport += body;
	ioReport += "\n";
}

void appendStatistics(std::string& ioReport, const Event& iEvent) {
	const auto stats = iEvent.getStats();
	ioReport += "## Statistiques\n\n";
	ioReport += std::format("* Numéros les plus sortis : {} ({} fois)\n", stats.mostPickStr(), stats.mostPickNb);
	ioReport += std::format("* Numéros les moins sortis : {} ({} fois)\n", stats.lessPickStr(), stats.lessPickNb);
	ioReport += std::format("* Partie la plus longue : {}\n", formatDuration(stats.roundLongest));
	ioReport += std::format("* Partie la plus courte : {}\n", formatDuration(stats.roundShortest));
	ioReport += std::format("* Durée moyenne d'une partie : {}\n", formatDuration(stats.roundAverage));
	ioReport += std::format("* Manche la plus longue : {}\n", formatDuration(stats.subRoundLongest));
	ioReport += std::format("* Manche la plus courte : {}\n", formatDuration(stats.subRoundShortest));
	ioReport += std::format("* Durée moyenne d'une manche : {}\n", formatDuration(stats.subRoundAverage));
	ioReport += std::format("* Tirages par partie : {} au plus, {} au moins, {:.1f} en moyenne\n", stats.roundMostNb,
							stats.roundLessNb, stats.roundAverageNb);
	ioReport += std::format("* Tirages par manche : {} au plus, {} au moins, {:.1f} en moyenne\n", stats.subRoundMostNb,
							stats.subRoundLessNb, stats.subRoundAverageNb);
}

}// namespace

auto buildReport(const Event& iEvent) -> std::string {
	std::string report;
	appendHeader(report, iEvent);
	appendSummary(report, computeTotals(iEvent));
	appendRounds(report, iEvent);
	appendDonors(report, iEvent);
	appendStatistics(report, iEvent);
	return report;
}

}// namespace evl::core
