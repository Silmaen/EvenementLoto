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
#include <unordered_map>

namespace evl::core {

/**
 * @brief Classe définissant une sous partie.
 */
class SubGameRound : public Serializable {
public:
	/// Le type utilisé pour représenter la liste des tirages.
	using drawsType = std::vector<uint8_t>;
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
	static const std::unordered_map<Type, string> TypeConvert;
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
	static const std::unordered_map<Status, string> StatusConvert;

	/**
	 * @brief Constructeur Avec données
	 * @param t Le type de sous partie
	 * @param _prices Le(s) lot(s) pour cette partie
	 * @param value La valeur du lot
	 */
	explicit SubGameRound(const Type& t = Type::OneQuine, const string& _prices = "", const double value = 0) {
		define(t, _prices, value);
	}

	/**
	 * @brief Définition de la sous-partie
	 * @param t Le type de sous partie
	 * @param _prices Le(s) lot(s) pour cette partie
	 * @param value La valeur des lots
	 */
	void define(const Type& t, const string& _prices = "", const double value = 0) {
		type = t;
		prices = _prices;
		pricesValue = value;
		winner = "";
	}

	/**
	 * @brief Renvoie les lots de cette partie.
	 * @return Les lots.
	 */
	[[nodiscard]] auto getPrices() const -> const string& { return prices; }

	/**
	 * @brief Renvoie la valeur du lot à gagner.
	 * @return La valeur du lot.
	 */
	[[nodiscard]] auto getValue() const -> const double& { return pricesValue; }

	/**
	 * @brief Renvoie le type de sous-partie.
	 * @return Le type de sous-partie.
	 */
	[[nodiscard]] auto getType() const -> const Type& { return type; }

	/**
	 * @brief Renvoie le nom du type de sous-partie
	 * @return Le nom du type de sous-partie
	 */
	[[nodiscard]] auto getTypeStr() const -> string;

	/**
	 * @brief Renvoie le statut de sous-partie.
	 * @return Le statut de sous-partie.
	 */
	[[nodiscard]] auto getStatus() const -> const Status& { return status; }

	/**
	 * @brief Renvoie le nom du statut de sous-partie
	 * @return Le nom du statut de sous-partie
	 */
	[[nodiscard]] auto getStatusStr() const -> string;

	/**
	 * @brief Advance to the next status if possible
	 */
	void nextStatus();

	/**
	 * @brief Ajoute le numéro dans la liste des numéros tirés
	 * @param num Numéro à ajouter
	 */
	void addPickedNumber(const uint8_t& num) {
		if (status == Status::Running)
			draws.push_back(num);
	}

	/**
	 * @brief Supprime le dernier tirage.
	 */
	void removeLastPick() {
		if (status == Status::Running && !draws.empty())
			draws.pop_back();
	}

	/**
	 * @brief Accès à la liste des tirages
	 * @return La liste des tirages
	 */
	[[nodiscard]] auto getDraws() const -> const drawsType& { return draws; }

	/**
	 * @brief Renvoie si la liste des tirages est vide
	 * @return True si pas de tirage
	 */
	[[nodiscard]] auto emptyDraws() const -> bool { return draws.empty(); }

	/**
	 * @brief Vérifie si la sous-partie est finie.
	 * @return True si la partie est finie.
	 */
	[[nodiscard]] auto isFinished() const -> bool { return status == Status::Done; }

	/**
	 * @brief Renvoie le numéro de la grille gagnante.
	 * @return Le numéro de la grille gagnante.
	 */
	[[nodiscard]] auto getWinner() const -> const string& { return winner; }

	/**
	 * @brief Défini le numéro du gagnant
	 * @param winner_ Le numéro de la grille gagnante
	 */
	void setWinner(const string& winner_) {
		if (status == Status::Running) {
			winner = winner_;
			nextStatus();
		}
	}

	/**
	 * @brief Lecture depuis un stream
	 * @param bs Le stream d’entrée.
	 * @param file_version La version du fichier à lire
	 */
	void read(std::istream& bs, int file_version) override;

	/**
	 * @brief Écriture dans un stream.
	 * @param bs Le stream où écrire.
	 */
	void write(std::ostream& bs) const override;

	/**
	 * @brief Écriture dans un json.
	 * @return Le json à remplir
	 */
	[[nodiscard]] auto to_json() const -> Json::Value override;

	/**
	 * @brief Lecture depuis un json
	 * @param j Le json à lire
	 */
	void from_json(const Json::Value& j) override;

	// ---- accès aux timers ----
	/**
	 * @brief Accès à la date de départ
	 * @return La date de départ
	 */
	[[nodiscard]] auto getStarting() const -> const timePoint& { return start; }

	/**
	 * @brief Accès à la date de fin
	 * @return La date de fin
	 */
	[[nodiscard]] auto getEnding() const -> const timePoint& { return end; }
#ifdef EVL_DEBUG
	/**
	 * @brief define invalide Status for testing purpose
	 */
	void invalidStatus() {
		status = Status::Invalid;
		type = Type::Invalid;
	}
	/**
	 * @brief define invalide Status for testing purpose
	 */
	void restoreStatus() {
		status = Status::Ready;
		type = Type::OneQuine;
	}
#endif
private:
	/// Le type de la sous-partie
	Type type = Type::OneQuine;
	/// Le statut de la sous-partie
	Status status = Status::Ready;
	/// La valeur du lot
	double pricesValue = 0;
	/// La liste des prix pour cette sous-partie
	string prices;
	/// Le nom du gagnant
	string winner;
	/// La liste des numéros tirés.
	drawsType draws;
	/// La date et heure de début de partie
	timePoint start;

	/// La date et heure de début de partie
	timePoint end;
};

}// namespace evl::core
