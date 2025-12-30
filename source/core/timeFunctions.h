/**
 * @file timeFunctions.h
 * @author damien.lachouette 
 * @date 25/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include <chrono>
#include <format>
#include <string>

namespace evl::core {
/// Le type d'horloge
using clock = std::chrono::system_clock;
/// Définition d’un point dans le temps.
using time_point = clock::time_point;
/// Définition d’une durée, la différence entre deux points du temps.
using duration = std::chrono::duration<double>;

/// Point de référence dans le temps.
constexpr time_point g_epoch{};

/**
 * @brief Tronque le temp ou la durée à la seconde près.
 * @tparam T Template pour pouvoir appliquer cette fonction à une durée ou un point de temps.
 * @param iTimePoint L'objet qu'il faut tronquer.
 * @return Le temps tronqué.
 */
template<class T>
constexpr auto floorMinutes(const T& iTimePoint) -> T {
	return std::chrono::floor<std::chrono::minutes>(iTimePoint);
}
constexpr auto getSeconds(const time_point& iTimePoint) -> uint32_t {
	return static_cast<uint32_t>(
			std::chrono::duration_cast<std::chrono::seconds>(iTimePoint - floorMinutes(iTimePoint)).count());
}

/**
 * @brief Formatage d’un point dans le temps comme date du calendrier
 * @param iTimePoint Le point dans le temps
 * @return Une chaine de caractères formatant le point dans le temps comme date du calendrier
 */
inline auto formatCalendar(const time_point& iTimePoint) -> std::string {
	auto local = std::chrono::zoned_time{std::chrono::current_zone(), iTimePoint};
	return std::format("{:%d %B %Y}", local);
}

/**
 * @brief Formatage d’un point dans le temps comme heure d’horloge
 * @param iTimePoint Le point dans le temps
 * @return Une chaine de caractères formatant le point dans le temps comme heure d’horloge
 */
inline auto formatClock(const time_point& iTimePoint) -> std::string {
	auto local = std::chrono::zoned_time{std::chrono::current_zone(), iTimePoint};
	return std::format("{:%R}:{:02d}", local, getSeconds(iTimePoint));
}

/**
 * @brief Formatage d’un point dans le temps comme heure d’horloge sans les secondes
 * @param iTimePoint Le point dans le temps
 * @return Une chaine de caractères formatant le point dans le temps comme heure d’horloge sans les secondes
 */
inline auto formatClockNoSecond(const time_point& iTimePoint) -> std::string {
	auto local = std::chrono::zoned_time{std::chrono::current_zone(), iTimePoint};
	return std::format("{:%R}", local);
}

/**
 * @brief Formatage d’une durée
 * @param iDuration La durée qu'il faut formater.
 * @return Une chaine de caractères formatant la durée
 */
inline auto formatDuration(const duration& iDuration) -> std::string {
	int32_t s = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::seconds>(iDuration).count());
	int32_t m = s / 60 % 60;
	int32_t h = s / 3600;
	s = s % 60;
	if (h > 0)
		return std::format("{:02d}:{:02d}:{:02d}", h, m, s);
	if (m > 0)
		return std::format("{:02d}:{:02d}", m, s);
	return std::format("{:02d}s", s);
}

}// namespace evl::core
