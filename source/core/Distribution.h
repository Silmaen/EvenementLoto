/**
 * @file Distribution.h
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "Prize.h"

#include <cstdint>
#include <string>
#include <vector>

namespace evl::core {

class Event;

/**
 * @brief Les réglages de la répartition automatique des lots.
 *
 * Ils existent parce que la « bonne » répartition n'est pas une vérité : elle dépend de
 * la salle, du nombre de cartons vendus et de l'habitude de l'association. Ce sont des
 * boutons, pas des constantes enfouies.
 */
struct DistributionSettings {
	/**
	 * @brief Part du poids d'un article venant de son attrait plutôt que de sa valeur.
	 *
	 * Zéro : seule la valeur compte. Un : seul l'attrait compte. Un jambon fait souvent
	 * plus d'effet qu'un objet plus cher, et c'est l'effet qui fait jouer.
	 */
	float attractivenessWeight = 0.3f;

	/**
	 * @brief Force de la montée au long de l'événement, jusqu'au climax final.
	 *
	 * Zéro : toutes les parties se valent. Un : la dernière partie emporte tout ce
	 * qu'elle peut. Entre les deux, une montée d'autant plus marquée que le nombre est
	 * grand.
	 */
	float climaxStrength = 0.6f;

	/// Réserver aux parties enfant les seuls articles marqués compatibles.
	bool respectChildRounds = true;
};

/// Ce que la répartition a fait, pour le dire à l'organisateur.
struct DistributionResult {
	/// Nombre d'articles placés.
	std::size_t placed = 0;
	/// Nombre de manches qui ont reçu au moins un article.
	std::size_t filledSubRounds = 0;
	/// Articles restés au catalogue, faute de manche où les mettre.
	std::size_t leftOver = 0;
	/// Ce qu'il faut dire, en une phrase.
	std::string summary;
};

/**
 * @brief Ce que la répartition donne à voir, manche par manche et partie par partie.
 *
 * Calculé ici plutôt que dessiné directement : les chiffres de l'histogramme sont alors
 * les mêmes que ceux du tableau, et ils se vérifient sans ouvrir de fenêtre.
 */
struct DistributionOverview {
	/// Une manche et ce qu'elle met en jeu.
	struct SubRoundEntry {
		uint32_t round = 0;///< Index de la partie.
		uint32_t subRound = 0;///< Index de la manche dans la partie.
		std::string roundName;///< Le nom de la partie.
		std::string subRoundName;///< Le type de la manche.
		double value = 0.0;///< La valeur des lots en jeu.
		std::size_t count = 0;///< Le nombre d'articles.
		bool editable = true;///< Faux pour une manche entamée, qui ne bouge plus.
	};

	/// Les manches, dans l'ordre du programme, les pauses exclues.
	std::vector<SubRoundEntry> subRounds;
	/// La valeur totale de chaque partie, dans l'ordre du programme.
	std::vector<double> roundValues;
	/// Le nom de chaque partie, dans le même ordre.
	std::vector<std::string> roundNames;
	/// La valeur des articles du catalogue qui ne sont affectés nulle part.
	double unassignedValue = 0.0;
	/// Le nombre de ces articles.
	std::size_t unassignedCount = 0;

	/**
	 * @brief La plus grande valeur de manche, pour mettre l'histogramme à l'échelle.
	 * @return La valeur la plus haute, zéro si rien n'est réparti.
	 */
	[[nodiscard]] auto highestSubRoundValue() const -> double;

	/**
	 * @brief La plus grande valeur de partie.
	 * @return La valeur la plus haute, zéro si rien n'est réparti.
	 */
	[[nodiscard]] auto highestRoundValue() const -> double;
};

/**
 * @brief Dresse l'état de la répartition d'un événement.
 * @param iEvent L'événement à examiner.
 * @return Ce qu'il y a à montrer.
 */
[[nodiscard]] auto overview(const Event& iEvent) -> DistributionOverview;

/**
 * @brief Répartit le catalogue de l'événement sur ses manches.
 *
 * Deux montées se superposent, telles que l'organisateur les décrit :
 *
 * - **au sein d'une partie**, la quine vaut moins que la double quine, qui vaut moins
 *   que le carton plein ;
 * - **au long de l'événement**, chaque partie pèse un peu plus que la précédente, la
 *   dernière étant le point d'orgue.
 *
 * Le classement des articles ne se fait pas sur le seul prix : l'attrait y entre pour
 * la part que disent les réglages. Les articles sont ensuite servis du plus lourd au
 * plus léger dans les manches classées par poids décroissant, ce qui donne à la
 * dernière manche de la dernière partie le plus beau lot.
 *
 * Seules les manches encore modifiables sont touchées : une partie déjà jouée garde ses
 * lots, et une pause n'en a pas. Les articles qu'aucune manche ne peut prendre restent
 * au catalogue.
 *
 * @param[in,out] ioEvent L'événement à garnir.
 * @param[in] iSettings Les réglages de la répartition.
 * @return Ce qui a été fait.
 */
auto distributePrizes(Event& ioEvent, const DistributionSettings& iSettings = {}) -> DistributionResult;

}// namespace evl::core
