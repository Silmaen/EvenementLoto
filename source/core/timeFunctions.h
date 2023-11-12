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
 * @brief Tronque le temp ou la durée à la seonde près.
 * @tparam T Template pour pouvoir appliquer cette fonction à une durée ou un point de temps.
 * @param t L'objet à tronquer.
 * @return Le temps tronqué.
 */
template<class T>
constexpr inline T floorMinutes(const T& t) {
    return std::chrono::floor<std::chrono::minutes>(t);
}
constexpr inline uint32_t getSeconds(const timePoint& t) {
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(t - floorMinutes(t)).count());
}

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
    return fmt::format("{:%R}:{:02d}", floorMinutes(t), getSeconds(t));
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
    int32_t s= static_cast<int32_t>(std::chrono::duration_cast<std::chrono::seconds>(d).count());
    int32_t m= (s / 60) % 60;
    int32_t h= s / 3600;
    s        = s % 60;
    if(h > 0)
        return fmt::format("{:02d}:{:02d}:{:02d}", h, m, s);
    if(m > 0)
        return fmt::format("{:02d}:{:02d}", m, s);
    return fmt::format("{:02d}s", s);
}

}// namespace evl::core
