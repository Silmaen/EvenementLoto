/**
* @file SubGameRound.h
* @author Silmaen
* @date 26/10/202
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#pragma once

#include "Serializable.h"
#include "timeFunctions.h"

namespace evl::core {

/**
 * @brief Classe définissant une sous partie.
 */
class SubGameRound final : public Serializable {
public:
	/// Le type utilisé pour représenter la liste des tirages.
	using draws_type = std::vector<uint8_t>;
	/**
	 * @brief Liste des types de sous-parties connus.
	 */
	enum struct Type : uint8_t {
		Invalid,///< Type invalide (pour debug)
		OneQuine,///< Le joueur gagne dès qu’il complète une ligne.
		TwoQuines,///< Le joueur gagne dès qu’il complète deux lignes.
		FullCard,///< Le joueur gagne dès qu’il complète son carton.
		Inverse,///< Le joueur est éliminé dès qu’un se ses numéros est tiré.
	};
	/**
	 * @brief Liste des statuts de sous-parties connus.
	 */
	enum struct Status : uint8_t {
		Invalid,///< Statut invalide (pour debug)
		Ready,///< Sous partie prê^te à jouer.
		PreScreen,///< Sous-partie en affichage.
		Running,///< Sous partie en cours.
		Done,///< Sous partie finie.
	};

	/**
	 * @brief Constructeur Avec données
	 * @param iType Le type de sous partie
	 * @param iPrices Le(s) lot(s) pour cette partie
	 * @param iValue La valeur du lot
	 */
	explicit SubGameRound(const Type& iType = Type::OneQuine, const std::string& iPrices = "",
						  const double iValue = 0) {
		define(iType, iPrices, iValue);
	}

	/**
	 * @brief Définition de la sous-partie
	 * @param iType Le type de sous partie
	 * @param iPrices Le(s) lot(s) pour cette partie
	 * @param iValue La valeur des lots
	 */
	void define(const Type& iType, const std::string& iPrices = "", const double iValue = 0) {
		m_type = iType;
		m_prices = iPrices;
		m_pricesValue = iValue;
		m_winner = "";
	}

	/**
	 * @brief Renvoie les lots de cette partie.
	 * @return Les lots.
	 */
	[[nodiscard]] auto getPrices() const -> const std::string& { return m_prices; }

	/**
	 * @brief Renvoie la valeur du lot à gagner.
	 * @return La valeur du lot.
	 */
	[[nodiscard]] auto getValue() const -> const double& { return m_pricesValue; }

	/**
	 * @brief Renvoie le type de sous-partie.
	 * @return Le type de sous-partie.
	 */
	[[nodiscard]] auto getType() const -> const Type& { return m_type; }

	/**
	 * @brief Renvoie le nom du type de sous-partie
	 * @return Le nom du type de sous-partie
	 */
	[[nodiscard]] auto getTypeStr() const -> std::string;

	/**
	 * @brief Renvoie le statut de sous-partie.
	 * @return Le statut de sous-partie.
	 */
	[[nodiscard]] auto getStatus() const -> const Status& { return m_status; }

	/**
	 * @brief Renvoie le nom du statut de sous-partie
	 * @return Le nom du statut de sous-partie
	 */
	[[nodiscard]] auto getStatusStr() const -> std::string;

	/**
	 * @brief Advance to the next status if possible
	 */
	void nextStatus();

	/**
	 * @brief Ajoute le numéro dans la liste des numéros tirés
	 * @param iNumber Numéro à ajouter
	 */
	void addPickedNumber(const uint8_t& iNumber) {
		if (m_status == Status::Running)
			m_draws.push_back(iNumber);
	}

	/**
	 * @brief Supprime le dernier tirage.
	 */
	void removeLastPick() {
		if (m_status == Status::Running && !m_draws.empty())
			m_draws.pop_back();
	}

	/**
	 * @brief Accès à la liste des tirages
	 * @return La liste des tirages
	 */
	[[nodiscard]] auto getDraws() const -> const draws_type& { return m_draws; }

	/**
	 * @brief Renvoie si la liste des tirages est vide
	 * @return True si pas de tirage
	 */
	[[nodiscard]] auto emptyDraws() const -> bool { return m_draws.empty(); }

	/**
	 * @brief Vérifie si la sous-partie est finie.
	 * @return True si la partie est finie.
	 */
	[[nodiscard]] auto isFinished() const -> bool { return m_status == Status::Done; }

	/**
	 * @brief Renvoie le numéro de la grille gagnante.
	 * @return Le numéro de la grille gagnante.
	 */
	[[nodiscard]] auto getWinner() const -> const std::string& { return m_winner; }

	/**
	 * @brief Défini le numéro du gagnant
	 * @param iWinner Le numéro de la grille gagnante
	 */
	void setWinner(const std::string& iWinner) {
		if (m_status == Status::Running) {
			m_winner = iWinner;
			nextStatus();
		}
	}

	/**
	 * @brief Lecture depuis un stream
	 * @param iBs Le stream d’entrée.
	 * @param iFileVersion La version du fichier à lire
	 */
	void read(std::istream& iBs, int iFileVersion) override;

	/**
	 * @brief Écriture dans un stream.
	 * @param iBs Le stream où écrire.
	 */
	void write(std::ostream& iBs) const override;

	/**
	 * @brief Écriture dans un json.
	 * @return Le json à remplir
	 */
	[[nodiscard]] auto toJson() const -> Json::Value override;

	/**
	 * @brief Lecture depuis un json
	 * @param iJson Le json à lire
	 */
	void fromJson(const Json::Value& iJson) override;

	/**
	 * @brief Écriture dans un YAML node.
	 * @return Le YAML node à remplir
	 */
	[[nodiscard]] auto toYaml() const -> YAML::Node override;

	/**
	 * @brief Lecture depuis un YAML node
	 * @param iNode Le YAML node à lire
	 */
	void fromYaml(const YAML::Node& iNode) override;

	// ---- accès aux timers ----
	/**
	 * @brief Accès à la date de départ
	 * @return La date de départ
	 */
	[[nodiscard]] auto getStarting() const -> const time_point& { return m_start; }

	/**
	 * @brief Accès à la date de fin
	 * @return La date de fin
	 */
	[[nodiscard]] auto getEnding() const -> const time_point& { return m_end; }
#ifdef EVL_DEBUG
	/**
	 * @brief define invalide Status for testing purpose
	 */
	void invalidStatus() {
		m_status = Status::Invalid;
		m_type = Type::Invalid;
	}
	/**
	 * @brief define invalide Status for testing purpose
	 */
	void restoreStatus() {
		m_status = Status::Ready;
		m_type = Type::OneQuine;
	}
#endif
private:
	/// Le type de la sous-partie
	Type m_type = Type::OneQuine;
	/// Le statut de la sous-partie
	Status m_status = Status::Ready;
	/// La valeur du lot
	double m_pricesValue = 0;
	/// La liste des prix pour cette sous-partie
	std::string m_prices;
	/// Le nom du gagnant
	std::string m_winner;
	/// La liste des numéros tirés.
	draws_type m_draws;
	/// La date et heure de début de partie
	time_point m_start;

	/// La date et heure de début de partie
	time_point m_end;
};

}// namespace evl::core
