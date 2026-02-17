/**
 * @file RandomNumberGenerator.h
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include <cstdint>
#include <random>
#include <vector>

/**
 * @brief Namespace pour les fonctions centrales du programme.
 */
namespace evl::core {

/// Maximum bingo number (1 to g_maxNumber inclusive).
constexpr uint8_t g_maxNumber = 90;

/**
 * @brief Class permettant de tirer aléatoirement des nombres entre 1 et 90 (inclus).
 */
class RandomNumberGenerator {
public:
	/**
	 * @brief Constructeur de base.
	 * @param iDebug Si mis à vrai, utilise une seed déterministe.
	 */
	explicit RandomNumberGenerator(bool iDebug = false);

	/**
	 * @brief Remet à zéro la liste des numéros déjà tiré.
	 */
	void resetPick();

	/**
	 * @brief Ajoute manuellement un numéro à la liste des numéros déjà tirés.
	 * @param iNumber Le numéro qu'il faut ajouter.
	 * @return False si le numéro est déjà dans la liste.
	 */
	auto addPick(const uint8_t& iNumber) -> bool;

	/**
	 * @brief Tire au sort un numéro non déjà tiré.
	 * @return Le numéro tiré.
	 */
	[[nodiscard]] auto pick() -> uint8_t;

	/**
	 * @brief Renvoie la liste des numéros tirés.
	 * @return La liste des numéros tirés.
	 */
	[[nodiscard]] auto getPicked() const -> const std::vector<uint8_t>& { return m_alreadyPicked; }

	/**
	 * @brief Retire le dernier numéro tiré de la liste.
	 */
	void popNum();

private:
	/// Liste des numéros déjà tirés.
	std::vector<uint8_t> m_alreadyPicked;
	/// Numéros restants disponibles pour le tirage.
	std::vector<uint8_t> m_remaining;
	/// Le générateur de nombres aléatoires.
	std::mt19937 m_rng;

	/// @brief Rebuild remaining numbers from alreadyPicked.
	void rebuildRemaining();
};

}// namespace evl::core
