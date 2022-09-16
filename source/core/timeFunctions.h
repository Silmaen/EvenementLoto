/**
 * @file timeFunctions.h
 * @author damien.lachouette 
 * @date 25/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "../baseDefine.h"
#include <chrono>
#include <fmt/chrono.h>
#include <iomanip>
#include <locale>

namespace evl::core {
/// Le type d'horloge
using clock= std::chrono::system_clock;
/// Définition d’un point dans le temps.
using timePoint= clock::time_point;
/// Définition d’une durée, la différence entre deux points du temps.
using duration= std::chrono::duration<double>;

/// Point de référence dans le temps.
constexpr timePoint epoch{};

/**
 * @brief Formatage d’un point dans le temps comme date du calendrier
 * @param t Le point dans le temps
 * @return Une chaine de caractères formatant le point dans le temps comme date du calendrier
 */
inline string formatCalendar(const timePoint& t) {
    return fmt::format("{:%d %B %Y}", t);
}

/**
 * @brief Formatage d’un point dans le temps comme heure d’horloge
 * @param t Le point dans le temps
 * @return Une chaine de caractères formatant le point dans le temps comme heure d’horloge
 */
inline string formatClock(const timePoint& t) {
    return fmt::format("{:%T}", t);
}

/**
 * @brief Formatage d’un point dans le temps comme heure d’horloge sans les secondes
 * @param t Le point dans le temps
 * @return Une chaine de caractères formatant le point dans le temps comme heure d’horloge sans les secondes
 */
inline string formatClockNoSecond(const timePoint& t) {
    return fmt::format("{:%R}", t);
}

/**
 * @brief Formatage d’une durée
 * @param d La durée à formater.
 * @return Une chaine de caractères formatant la durée
 */
inline string formatDuration(const duration& d) {
    auto s= std::chrono::duration_cast<std::chrono::seconds>(d).count();
    if(s < 60)
        return fmt::format("{:%Ss}", d);
    if(s < 3600)
        return fmt::format("{:%M:%S}", d);
    return fmt::format("{:%T}", d);
}

}// namespace evl::core
