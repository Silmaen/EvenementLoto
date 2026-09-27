/**
 * @file Distribution.cpp
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Distribution.h"

#include "Event.h"

namespace evl::core {

namespace {

/// Une manche pouvant recevoir des lots, et ce qu'elle pèse dans le programme.
struct Slot {
	uint32_t round = 0;///< Index de la partie.
	uint32_t subRound = 0;///< Index de la manche dans la partie.
	float weight = 0.0f;///< Poids de la manche, la plus lourde servie la première.
	bool childOnly = false;///< La manche appartient à une partie enfant.
	prizes_type prizes;///< Les articles qui lui sont attribués.
};

/// Un article du catalogue et son poids.
struct Candidate {
	std::size_t index = 0;///< Position dans le catalogue.
	float score = 0.0f;///< Poids de l'article, le plus lourd placé le premier.
	bool childFriendly = true;///< L'article a sa place dans une partie enfant.
};

/**
 * @brief Recense les manches que la répartition peut garnir.
 *
 * Une pause n'a pas de lot, et une manche déjà entamée n'est plus modifiable : sa
 * garniture est de l'histoire. Le poids superpose les deux montées voulues — la place
 * de la manche dans sa partie, et la place de la partie dans l'après-midi.
 */
auto collectSlots(const Event& iEvent, const DistributionSettings& iSettings) -> std::vector<Slot> {
	// Les parties hors pause, dans l'ordre du programme : c'est sur elles que la montée
	// se mesure, pour que la dernière partie soit bien le point d'orgue.
	std::vector<uint32_t> playable;
	for (uint32_t index = 0; index < static_cast<uint32_t>(iEvent.sizeRounds()); ++index) {
		if (std::next(iEvent.beginRounds(), index)->getType() != GameRound::Type::Pause)
			playable.push_back(index);
	}

	std::vector<Slot> slots;
	for (std::size_t position = 0; position < playable.size(); ++position) {
		const auto roundIndex = playable[position];
		const auto round = std::next(iEvent.beginRounds(), roundIndex);
		const auto subCount = round->sizeSubRound();
		if (subCount == 0)
			continue;
		const float ramp =
				playable.size() > 1 ? static_cast<float>(position) / static_cast<float>(playable.size() - 1) : 1.0f;
		const float roundWeight = 1.0f + iSettings.climaxStrength * ramp;
		const bool childOnly = iSettings.respectChildRounds && round->getType() == GameRound::Type::Enfant;
		for (uint32_t sub = 0; sub < static_cast<uint32_t>(subCount); ++sub) {
			if (!std::next(round->beginSubRound(), sub)->isEditable())
				continue;
			const float subWeight = static_cast<float>(sub + 1) / static_cast<float>(subCount);
			slots.push_back({.round = roundIndex,
							 .subRound = sub,
							 .weight = roundWeight * subWeight,
							 .childOnly = childOnly,
							 .prizes = {}});
		}
	}
	std::ranges::stable_sort(
			slots, [](const Slot& iLeft, const Slot& iRight) -> bool { return iLeft.weight > iRight.weight; });
	return slots;
}

/**
 * @brief Classe les articles du catalogue, du plus marquant au plus discret.
 *
 * La valeur et l'attrait sont ramenés chacun sur l'intervalle [0, 1] avant d'être
 * mélangés : sans cela, des prix en euros écraseraient toujours une note sur cinq.
 */
auto rankCatalogue(const prizes_type& iCatalogue, const DistributionSettings& iSettings) -> std::vector<Candidate> {
	double highestValue = 0.0;
	for (const auto& prize: iCatalogue) highestValue = std::max(highestValue, prize.getValue());

	const float weight = std::clamp(iSettings.attractivenessWeight, 0.0f, 1.0f);
	std::vector<Candidate> candidates;
	candidates.reserve(iCatalogue.size());
	for (std::size_t index = 0; index < iCatalogue.size(); ++index) {
		const auto& prize = iCatalogue[index];
		const float value = highestValue > 0.0 ? static_cast<float>(prize.getValue() / highestValue) : 0.0f;
		const float appeal =
				static_cast<float>(prize.getAttractiveness()) / static_cast<float>(Prize::g_maxAttractiveness);
		candidates.push_back({.index = index,
							  .score = (1.0f - weight) * value + weight * appeal,
							  .childFriendly = prize.isChildFriendly()});
	}
	std::ranges::stable_sort(candidates, [](const Candidate& iLeft, const Candidate& iRight) -> bool {
		return iLeft.score > iRight.score;
	});
	return candidates;
}

}// namespace

auto DistributionOverview::highestSubRoundValue() const -> double {
	double highest = 0.0;
	for (const auto& entry: subRounds) highest = std::max(highest, entry.value);
	return highest;
}

auto DistributionOverview::highestRoundValue() const -> double {
	double highest = 0.0;
	for (const auto& value: roundValues) highest = std::max(highest, value);
	return highest;
}

auto overview(const Event& iEvent) -> DistributionOverview {
	DistributionOverview result;
	for (uint32_t roundIndex = 0; roundIndex < static_cast<uint32_t>(iEvent.sizeRounds()); ++roundIndex) {
		const auto round = std::next(iEvent.beginRounds(), roundIndex);
		if (round->getType() == GameRound::Type::Pause)
			continue;
		double roundTotal = 0.0;
		for (uint32_t subIndex = 0; subIndex < static_cast<uint32_t>(round->sizeSubRound()); ++subIndex) {
			const auto sub = std::next(round->beginSubRound(), subIndex);
			roundTotal += sub->getValue();
			result.subRounds.push_back({.round = roundIndex,
										.subRound = subIndex,
										.roundName = round->getName(),
										.subRoundName = sub->getTypeStr(),
										.value = sub->getValue(),
										.count = sub->getPrizes().size(),
										.editable = sub->isEditable()});
		}
		result.roundValues.push_back(roundTotal);
		result.roundNames.push_back(round->getName());
	}
	for (const auto& prize: iEvent.getCatalogue()) {
		if (iEvent.findPrizeSlot(prize.getId()).has_value())
			continue;
		result.unassignedValue += prize.getValue();
		++result.unassignedCount;
	}
	return result;
}

auto distributePrizes(Event& ioEvent, const DistributionSettings& iSettings) -> DistributionResult {
	DistributionResult result;
	auto slots = collectSlots(ioEvent, iSettings);
	const auto& catalogue = ioEvent.getCatalogue();

	if (catalogue.empty()) {
		result.summary = "Le catalogue est vide : il n'y a rien à répartir.";
		return result;
	}
	if (slots.empty()) {
		result.leftOver = catalogue.size();
		result.summary = "Aucune manche ne peut recevoir de lot : tout reste au catalogue.";
		return result;
	}

	const auto candidates = rankCatalogue(catalogue, iSettings);
	std::vector<bool> placed(candidates.size(), false);

	// Servies par passes : d'abord un article dans chaque manche, la plus lourde
	// d'abord, puis un deuxième, et ainsi de suite. Les manches les plus lourdes
	// accumulent donc, ce qui est exactement ce qu'on attend d'un point d'orgue.
	std::size_t remaining = candidates.size();
	bool progress = true;
	while (remaining > 0 && progress) {
		progress = false;
		for (auto& slot: slots) {
			for (std::size_t rank = 0; rank < candidates.size(); ++rank) {
				if (placed[rank])
					continue;
				const auto& candidate = candidates[rank];
				if (slot.childOnly && !candidate.childFriendly)
					continue;
				slot.prizes.push_back(catalogue[candidate.index]);
				placed[rank] = true;
				--remaining;
				progress = true;
				break;
			}
			if (remaining == 0)
				break;
		}
	}

	for (const auto& slot: slots) {
		if (slot.prizes.empty())
			continue;
		const auto round = ioEvent.getGameRound(slot.round);
		if (round == ioEvent.endRounds())
			continue;
		// Les articles d'une manche sont rangés du plus discret au plus marquant : c'est
		// dans cet ordre qu'ils s'annoncent.
		auto ordered = slot.prizes;
		std::ranges::reverse(ordered);
		round->getSubRound(slot.subRound)->setPrizes(ordered);
		result.placed += ordered.size();
		++result.filledSubRounds;
	}
	// Les manches que la répartition pouvait garnir et n'a pas garnies sont vidées : la
	// répartition est rejouable, elle ne laisse pas la trace d'un essai précédent.
	for (const auto& slot: slots) {
		if (!slot.prizes.empty())
			continue;
		if (const auto round = ioEvent.getGameRound(slot.round); round != ioEvent.endRounds())
			round->getSubRound(slot.subRound)->setPrizes({});
	}
	result.leftOver = remaining;

	result.summary = std::format("{} lot(s) répartis sur {} manche(s)", result.placed, result.filledSubRounds);
	if (result.leftOver > 0)
		result.summary += std::format(", {} laissé(s) au catalogue faute de manche compatible", result.leftOver);
	result.summary += ".";
	// Un catalogue sans une valeur ni un attrait ne dit rien de l'ordre à suivre — c'est
	// le cas d'un vieux fichier dont le format ne portait pas les prix. Le dire vaut
	// mieux que de laisser croire au classement obtenu.
	if (std::ranges::none_of(candidates, [](const Candidate& iCandidate) -> bool { return iCandidate.score > 0.0f; })) {
		result.summary += " Aucun article ne porte de valeur ni d'attrait : l'ordre obtenu est arbitraire, "
						  "renseignez-les pour que la répartition ait un sens.";
	}
	return result;
}

}// namespace evl::core
