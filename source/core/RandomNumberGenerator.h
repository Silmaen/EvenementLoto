/**
 * @file RandomNumberGenerator.h
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include <cstdint>
#include <vector>

/**
 * @brief Namespace pour les fonctions centrales du programme.
 */
namespace evl::core {

/**
 * @brief Class permettant de tirer aléatoirement des nombres entre 1 et 90 (inclus).
 */
class RandomNumberGenerator {
public:
	/**
	 * @brief Constructeur de base.
	 * @param debug Si mis à vrai, utilise une seed déterministe.
	 */
	explicit RandomNumberGenerator(bool debug = false);

	/**
	 * @brief Remet à zéro la liste des numéros déjà tiré.
	 */
	void resetPick() { alreadyPicked.clear(); }

	/**
	 * @brief Ajoute manuellement un numéro à la liste des numéros déjà tirés.
	 * @param num Le numéro qu'il faut ajouter.
	 * @return False si le numéro est déjà dans la liste.
	 */
	auto addPick(const uint8_t& num) -> bool;

	/**
	 * @brief Tire au sort un numéro non déjà tiré.
	 * @return Le numéro tiré.
	 */
	[[nodiscard]] auto pick() -> uint8_t;

	/**
	 * @brief Renvoie la liste des numéros tirés.
	 * @return La liste des numéros tirés.
	 */
	[[nodiscard]] auto getPicked() const -> const std::vector<uint8_t>& { return alreadyPicked; }

	/**
	 * @brief Retire le dernier numéro tiré de la liste.
	 */
	void popNum();

private:
	/// Liste des numéros déjà tirer.
	std::vector<uint8_t> alreadyPicked;
};

}// namespace evl::core
