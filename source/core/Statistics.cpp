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

void Statistics::pushRound(const GameRound& round) {
    // update round (only if done)
    if(round.getStatus() == GameRound::Status::Done) {
        const duration dur= round.getEnding() - round.getStarting();
        if(dur > roundLongest) roundLongest= dur;
        if(dur < roundShortest) roundShortest= dur;
        roundAverage    = (roundAverage * nbRounds + dur) / (nbRounds + 1);
        const int nbDraw= static_cast<int>(round.drawsCount());
        if(nbDraw > roundMostNb) roundMostNb= nbDraw;
        if(nbDraw < roundLessNb || roundLessNb == 0) roundLessNb= nbDraw;
        roundAverageNb= (roundAverageNb * nbRounds + nbDraw) / (nbRounds + 1);
        ++nbRounds;
    }
    if(roundShortest == duration::max()) roundShortest= duration::zero();
    // update subrounds
    for(auto sub= round.beginSubRound(); sub != round.endSubRound(); ++sub) {
        if(sub->getStatus() != SubGameRound::Status::Done)
            break;
        duration dur= sub->getEnding() - sub->getStarting();
        if(dur > subRoundLongest) subRoundLongest= dur;
        if(dur < subRoundShortest) subRoundShortest= dur;
        subRoundAverage = (subRoundAverage * nbSubRounds + dur) / (nbSubRounds + 1);
        const int nbDraw= static_cast<int>(sub->getDraws().size());
        if(nbDraw > subRoundMostNb) subRoundMostNb= nbDraw;
        if(nbDraw < subRoundLessNb || subRoundLessNb == 0) subRoundLessNb= nbDraw;
        subRoundAverageNb= (subRoundAverageNb * nbSubRounds + nbDraw) / (nbSubRounds + 1);
        ++nbSubRounds;
    }
    if(subRoundShortest == duration::max()) subRoundShortest= duration::zero();
    // update tirages
    if(pickCounts.size() != 90)
        pickCounts.resize(90);
    for(const auto draw: round.getAllDraws())
        pickCounts[draw - 1]++;
    lessPickNb= *std::ranges::min_element(pickCounts);
    mostPickNb= *std::ranges::max_element(pickCounts);
    lessPickList.clear();
    mostPickList.clear();
    uint8_t idx= 1;
    for(const auto& p: pickCounts) {
        if(std::cmp_equal(p, lessPickNb))
            lessPickList.push_back(idx);
        if(std::cmp_equal(p, mostPickNb))
            mostPickList.push_back(idx);
        ++idx;
    }
}

}// namespace evl::core
