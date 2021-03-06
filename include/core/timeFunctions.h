/**
 * @file timeFunctions.h
 * @author damien.lachouette 
 * @date 25/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "baseDefine.h"
#include <chrono>
#include <iomanip>
#include <locale>
#include <sstream>

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
    std::stringstream oss;
    auto tt= clock::to_time_t(t);
    oss << std::put_time(std::localtime(&tt), "%d %B %Y");
    return oss.str();
}

/**
 * @brief Formatage d’un point dans le temps comme heure d’horloge
 * @param t Le point dans le temps
 * @return Une chaine de caractères formatant le point dans le temps comme heure d’horloge
 */
inline string formatClock(const timePoint& t) {
    std::stringstream oss;
    auto tt= clock::to_time_t(t);
    oss << std::put_time(std::localtime(&tt), "%H:%M:%S");
    return oss.str();
}

/**
 * @brief Formatage d’un point dans le temps comme heure d’horloge sans les secondes
 * @param t Le point dans le temps
 * @return Une chaine de caractères formatant le point dans le temps comme heure d’horloge sans les secondes
 */
inline string formatClockNoSecond(const timePoint& t) {
    std::stringstream oss;
    auto tt= clock::to_time_t(t);
    oss << std::put_time(std::localtime(&tt), "%H:%M");
    return oss.str();
}

/**
 * @brief Formatage d’une durée
 * @param d La durée à formater.
 * @return Une chaine de caractères formatant la durée
 */
inline string formatDuration(const duration& d) {
    std::stringstream oss;
    auto h= std::chrono::duration_cast<std::chrono::hours>(d).count();
    if(h != 0) {
        if(h < 10) oss << "0";
        oss << h << ":";
    }
    auto m= std::chrono::duration_cast<std::chrono::minutes>(d).count() % 60;
    if(m != 0) {
        if(m < 10) oss << "0";
        oss << m << ":";
    }
    auto s= std::chrono::duration_cast<std::chrono::seconds>(d).count() % 60;
    if(s < 10) oss << "0";
    oss << s;
    return oss.str();
}

}// namespace evl::core
