/**
 * @file Statistics.h
 * @author argawaen
 * @date 11/11/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "GameRound.h"
#include "timeFunctions.h"

namespace evl::core {

/**
 * @brief Class Statistics
 */
class Statistics {
public:
    /**
     * @brief Default constructor.
     */
    Statistics()= default;
    /**
     * @brief Default copy constructor
     */
    Statistics(const Statistics&)= default;
    /**
     * @brief Default move constructor
     */
    Statistics(Statistics&&)= default;
    /**
     * @brief Default copy assignation
     * @return this
     */
    Statistics& operator=(const Statistics&)= default;
    /**
     * @brief Default move assignation
     * @return this
     */
    Statistics& operator=(Statistics&&)= default;
    /**
     * @brief Destructor.
     */
    ~Statistics()= default;

    void pushRound(const GameRound& round);

    using pickList= std::vector<uint8_t>;

    int lessPickNb= 0;
    pickList lessPickList;

    int mostPickNb= 0;
    pickList mostPickList;

    int roundLessNb      = 0;
    int roundMostNb      = 0;
    double roundAverageNb= 0.0;

    int subRoundLessNb      = 0;
    int subRoundMostNb      = 0;
    double subRoundAverageNb= 0.0;

    duration roundLongest    = duration::zero();
    duration roundShortest   = duration::max();
    duration roundAverage    = duration::zero();
    duration subRoundLongest = duration::zero();
    duration subRoundShortest= duration::max();
    duration subRoundAverage = duration::zero();

private:
    int nbRounds   = 0;
    int nbSubRounds= 0;
    pickList pickCounts;
};

}// namespace evl::core
