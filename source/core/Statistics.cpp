/**
 * @file Statistics.cpp
 * @author argawaen
 * @date 11/11/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Statistics.h"

namespace evl::core {

void Statistics::pushRound(const GameRound& iRound) {
	// update round (only if done)
	if (iRound.getStatus() == GameRound::Status::Done) {
		const duration dur = iRound.getEnding() - iRound.getStarting();
		if (dur > roundLongest)
			roundLongest = dur;
		if (dur < roundShortest)
			roundShortest = dur;
		roundAverage = (roundAverage * m_nbRounds + dur) / (m_nbRounds + 1);
		const int nbDraw = static_cast<int>(iRound.drawsCount());
		if (nbDraw > roundMostNb)
			roundMostNb = nbDraw;
		if (nbDraw < roundLessNb || roundLessNb == 0)
			roundLessNb = nbDraw;
		roundAverageNb = (roundAverageNb * m_nbRounds + nbDraw) / (m_nbRounds + 1);
		++m_nbRounds;
	}
	if (roundShortest == duration::max())
		roundShortest = duration::zero();
	// update subrounds
	for (auto sub = iRound.beginSubRound(); sub != iRound.endSubRound(); ++sub) {
		if (sub->getStatus() != SubGameRound::Status::Done)
			break;
		const duration dur = sub->getEnding() - sub->getStarting();
		if (dur > subRoundLongest)
			subRoundLongest = dur;
		if (dur < subRoundShortest)
			subRoundShortest = dur;
		subRoundAverage = (subRoundAverage * m_nbSubRounds + dur) / (m_nbSubRounds + 1);
		const int nbDraw = static_cast<int>(sub->getDraws().size());
		if (nbDraw > subRoundMostNb)
			subRoundMostNb = nbDraw;
		if (nbDraw < subRoundLessNb || subRoundLessNb == 0)
			subRoundLessNb = nbDraw;
		subRoundAverageNb = (subRoundAverageNb * m_nbSubRounds + nbDraw) / (m_nbSubRounds + 1);
		++m_nbSubRounds;
	}
	if (subRoundShortest == duration::max())
		subRoundShortest = duration::zero();
	// update tirages
	if (m_pickCounts.size() != 90)
		m_pickCounts.resize(90);
	for (const auto draw: iRound.getAllDraws()) m_pickCounts[draw - 1]++;
	lessPickNb = *std::ranges::min_element(m_pickCounts);
	mostPickNb = *std::ranges::max_element(m_pickCounts);
	lessPickList.clear();
	mostPickList.clear();
	uint8_t idx = 1;
	for (const auto& p: m_pickCounts) {
		if (std::cmp_equal(p, lessPickNb))
			lessPickList.push_back(idx);
		if (std::cmp_equal(p, mostPickNb))
			mostPickList.push_back(idx);
		++idx;
	}
}


auto Statistics::lessPickStr() const -> std::string {
	if (lessPickList.empty())
		return "--";
	std::string res;
	for (const auto& num: lessPickList) {
		if (!res.empty())
			res += " ";
		res += std::to_string(num);
	}
	return res;
}
auto Statistics::mostPickStr() const -> std::string {
	if (mostPickList.empty())
		return "--";
	std::string res;
	for (const auto& num: mostPickList) {
		if (!res.empty())
			res += " ";
		res += std::to_string(num);
	}
	return res;
}

}// namespace evl::core
