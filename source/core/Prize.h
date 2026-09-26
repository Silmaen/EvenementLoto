/**
 * @file Prize.h
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "Serializable.h"

#include <string>

namespace evl::core {

/**
 * @brief Un lot à gagner, décrit article par article.
 *
 * Les lots étaient une simple chaine de caractères, une ligne par article, avec une
 * valeur unique pour l'ensemble. Les décrire un par un est ce qui permet de les saisir
 * une fois pour toutes puis de les répartir sur les parties : le donateur pour le
 * remercier, la valeur pour faire monter les enjeux au long de l'après-midi,
 * l'attractivité parce qu'un lot cher n'est pas toujours celui qui fait envie, et la
 * compatibilité enfant pour ne pas mettre une bouteille en jeu dans une partie enfant.
 */
class Prize final : public Serializable {
public:
	/// La note d'attractivité la plus haute.
	static constexpr uint8_t g_maxAttractiveness = 5;

	/**
	 * @brief Constructeur.
	 * @param iDesignation La désignation de l'article.
	 * @param iValue Sa valeur, en euros.
	 */
	explicit Prize(std::string iDesignation = {}, const double iValue = 0.0)
		: m_designation{std::move(iDesignation)}, m_value{iValue} {}

	/**
	 * @brief Renvoie la désignation de l'article.
	 * @return La désignation.
	 */
	[[nodiscard]] auto getDesignation() const -> const std::string& { return m_designation; }

	/**
	 * @brief Définit la désignation de l'article.
	 * @param iDesignation La désignation.
	 */
	void setDesignation(const std::string& iDesignation) { m_designation = iDesignation; }

	/**
	 * @brief Renvoie le donateur de l'article, vide s'il n'est pas connu.
	 * @return Le donateur.
	 */
	[[nodiscard]] auto getDonor() const -> const std::string& { return m_donor; }

	/**
	 * @brief Définit le donateur de l'article.
	 * @param iDonor Le donateur.
	 */
	void setDonor(const std::string& iDonor) { m_donor = iDonor; }

	/**
	 * @brief Renvoie la valeur de l'article, en euros.
	 * @return La valeur.
	 */
	[[nodiscard]] auto getValue() const -> double { return m_value; }

	/**
	 * @brief Définit la valeur de l'article.
	 * @param iValue La valeur, en euros, négative ignorée.
	 */
	void setValue(const double iValue) {
		if (iValue >= 0.0)
			m_value = iValue;
	}

	/**
	 * @brief Renvoie la note d'attractivité, de 0 (non notée) à 5.
	 * @return La note.
	 */
	[[nodiscard]] auto getAttractiveness() const -> uint8_t { return m_attractiveness; }

	/**
	 * @brief Définit la note d'attractivité.
	 * @param iAttractiveness La note, bornée à 5.
	 */
	void setAttractiveness(const uint8_t iAttractiveness) {
		m_attractiveness = std::min(iAttractiveness, g_maxAttractiveness);
	}

	/**
	 * @brief Renvoie si l'article peut être mis en jeu dans une partie enfant.
	 * @return True si compatible.
	 */
	[[nodiscard]] auto isChildFriendly() const -> bool { return m_childFriendly; }

	/**
	 * @brief Définit si l'article peut être mis en jeu dans une partie enfant.
	 * @param iChildFriendly True si compatible.
	 */
	void setChildFriendly(const bool iChildFriendly) { m_childFriendly = iChildFriendly; }

	/**
	 * @brief Renvoie si l'article est vide, donc sans rien à afficher.
	 * @return True si rien n'est renseigné.
	 */
	[[nodiscard]] auto isEmpty() const -> bool { return m_designation.empty() && m_value == 0.0; }

	/**
	 * @brief Lecture depuis un stream.
	 * @param iBs Le stream d’entrée.
	 * @param iContext Ce que le lecteur sait du fichier parcouru.
	 */
	void read(std::istream& iBs, const ReadContext& iContext) override;

	/**
	 * @brief Écriture dans un stream.
	 * @param iBs Le stream où écrire.
	 */
	void write(std::ostream& iBs) const override;

	/**
	 * @brief Écriture dans un json.
	 * @return Le json à remplir.
	 */
	[[nodiscard]] auto toJson() const -> Json::Value override;

	/**
	 * @brief Lecture depuis un json.
	 * @param iJson Le json à lire.
	 */
	void fromJson(const Json::Value& iJson) override;

	/**
	 * @brief Écriture dans un YAML node.
	 * @return Le YAML node à remplir.
	 */
	[[nodiscard]] auto toYaml() const -> YAML::Node override;

	/**
	 * @brief Lecture depuis un YAML node.
	 * @param iNode Le YAML node à lire.
	 */
	void fromYaml(const YAML::Node& iNode) override;

private:
	/// La désignation de l'article.
	std::string m_designation;
	/// Le donateur, facultatif.
	std::string m_donor;
	/// La valeur de l'article, en euros.
	double m_value = 0.0;
	/// La note d'attractivité, de 0 (non notée) à 5.
	uint8_t m_attractiveness = 0;
	/// Si l'article peut être mis en jeu dans une partie enfant.
	bool m_childFriendly = true;
};

/// La liste des articles d'un lot.
using prizes_type = std::vector<Prize>;

/**
 * @brief Somme des valeurs d'une liste d'articles.
 * @param iPrizes La liste.
 * @return La valeur totale, en euros.
 */
[[nodiscard]] auto totalValue(const prizes_type& iPrizes) -> double;

/**
 * @brief Les désignations d'une liste d'articles, une par ligne.
 * @param iPrizes La liste.
 * @return Les désignations, séparées par des retours à la ligne.
 */
[[nodiscard]] auto designations(const prizes_type& iPrizes) -> std::string;

/**
 * @brief Découpe l'ancienne chaine de lots en articles, une ligne par article.
 *
 * Les fichiers d'avant la version 8 décrivent les lots par une chaine multiligne et une
 * valeur unique. La valeur totale est portée par le premier article : la répartir
 * également inventerait des prix que personne n'a saisis, alors que le total, lui, est
 * une donnée réelle.
 *
 * @param iPrices La chaine multiligne.
 * @param iValue La valeur de l'ensemble.
 * @return La liste des articles.
 */
[[nodiscard]] auto prizesFromLegacy(const std::string& iPrices, double iValue) -> prizes_type;

}// namespace evl::core
