/**
 * @file GameRound.h
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "Serializable.h"
#include "SubGameRound.h"
#include <numeric>
#include <unordered_map>
#include <vector>

namespace evl::core {

/**
 * @brief Classe définissant une partie.
 */
class GameRound : public Serializable {
public:
	// ---- définition des types ----
	/// Le type utilisé pour représenter la liste des tirages.
	using drawsType = std::vector<uint8_t>;
	using subRoundsType = std::vector<SubGameRound>;
	/**
	 * @brief Liste des types de parties connus.
	 */
	enum struct Type : uint8_t {
		Invalid,///< Type invalide (pour debug)
		OneQuine,///< Une partie standard en remplissant une ligne seulement avant reset.
		TwoQuines,///< Une partie standard en remplissant deux lignes seulement.
		FullCard,///< Une partie standard en remplissant le carton seulement.
		OneQuineFullCard,///< Une partie standard en remplissant une ligne, puis le carton.
		OneTwoQuineFullCard,///< Une partie standard en remplissant une ligne, puis deux, puis le carton.
		Enfant,///< Une partie pour les enfants, similaire à OneQuine.
		Inverse,///< Le joueur est éliminé dès qu’un se ses numéros est tiré.
		Pause,///< Une fausse 'partie' pour matérialiser une pause.
	};
	static const std::unordered_map<Type, string> TypeConvert;

	/**
	 * @brief Les status possibles de la partie
	 */
	enum struct Status : uint8_t {
		Invalid,///< Statut invalide (pour debug)
		Ready,///< La partie est prête à être jouée
		Running,///< La partie est démarrée
		PostScreen,///< La partie est en affichage de fin.
		Done///< La partie est finie
	};
	static const std::unordered_map<Status, string> StatusConvert;

	// --- constructeurs ----
	/**
	 * @brief Constructeur
	 * @param t Le type de partie
	 */
	explicit GameRound(const Type& t = Type::OneTwoQuineFullCard);

	// ---- manipulation du type de partie ----
	/**
	 * @brief Renvoie une chaine contenant le type de partie.
	 * @return Le type de partie.
	 */
	[[nodiscard]] auto getTypeStr() const -> string;

	/**
	 * @brief Renvoie le type de partie.
	 * @return Le type de partie
	 */
	[[nodiscard]] auto getType() const -> const Type& { return type; }

	/**
	 * @brief Définit le type de partie
	 * @param t Type de partie
	 */
	void setType(const Type& t);

	// ---- manipulation du statut ----
	/**
	 * @brief Renvoie une chaine contenant le type de partie.
	 * @return Le type de partie.
	 */
	[[nodiscard]] auto getStatusStr() const -> string;

	/**
	 * @brief Renvoie le type de partie.
	 * @return Le type de partie
	 */
	[[nodiscard]] auto getStatus() const -> const Status& { return status; }

	/**
	 * @brief Renvoie si le round est fini
	 * @return True si le round est fini
	 */
	[[nodiscard]] auto isFinished() const -> bool { return status == Status::Done; }
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
		type = Type::OneTwoQuineFullCard;
	}
#endif

	// ---- flux du jeu ----
	/**
	 * @brief Advance to the next status if possible
	 */
	void nextStatus();

	/**
	 * @brief Renvoie une chaine de caractère décrivant l'état courant
	 * @return L'état courant
	 */
	[[nodiscard]] auto getStateString() const -> string;

	/**
	 * @brief Ajoute le numéro dans la liste des numéros tirés
	 * @param num Numéro à ajouter
	 */
	void addPickedNumber(const uint8_t& num);

	/**
	 * @brief Supprime le dernier tirage.
	 */
	void removeLastPick();

	/**
	 * @brief donne un gagnant
	 * @param win Le nom du gagnant
	 */
	void addWinner(const std::string& win);

	/**
	 * @brief Renvoie la liste des gagnants de sous-partie
	 * @return Les gagnants
	 */
	[[nodiscard]] auto getWinnerStr() const -> string;

	// ---- Serialisation ----
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

	// ---- accès aux sub rounds (lecture seule) ----
	/**
	 * @brief Renvoie si la sous-partie actuelle est la dernière de la liste
	 * @return True si on est à la dernière sous-partie.
	 */
	[[nodiscard]] auto isCurrentSubRoundLast() const -> bool;
	/**
	 * @brief Accès à la sous-partie courante l’interface
	 * @return Itérateur constant sur la sous-partie courante
	 */
	[[nodiscard]] auto getCurrentSubRound() const -> subRoundsType::const_iterator;
	/**
	 * @brief Accès à la sous-partie courante
	 * @return Itérateur constant sur la sous-partie courante
	 */
	auto getSubRound(uint32_t index) -> subRoundsType::iterator;
	/**
	 * @brief Accès à la sous-partie courante
	 * @return Itérateur constant sur la sous-partie courante
	 */
	[[nodiscard]] auto getSubRound(uint32_t index) const -> subRoundsType::const_iterator;
	/**
	 * @brief Renvoie un itérateur constant sur le début de la liste des subround.
	 * @return Itérateur constant sur le début de la liste des subround.
	 */
	[[nodiscard]] auto beginSubRound() const -> subRoundsType::const_iterator { return subGames.cbegin(); }
	/**
	 * @brief Renvoie un itérateur constant sur la fin de la liste des subround.
	 * @return Itérateur constant sur la fin de la liste des subround.
	 */
	[[nodiscard]] auto endSubRound() const -> subRoundsType::const_iterator { return subGames.cend(); }
	/**
	 * @brief Renvoie la taille de la liste des subround.
	 * @return La taille de la fin de la liste des subround.
	 */
	[[nodiscard]] auto sizeSubRound() const -> subRoundsType::size_type { return subGames.size(); }

	/**
	 * @brief Défini le numéro de partie
	 * @param id Le numéro de partie
	 */
	void setID(const int id) { Id = id; }

	/**
	 * @brief Renvoie le numéro d'affichage de la partie
	 * @return Le numéro d'affichage
	 */
	[[nodiscard]] auto getID() const -> const int& { return Id; }

	/**
	 * @brief Affichage de nom spécial
	 * @return Nom du round
	 */
	[[nodiscard]] auto getName() const -> string;

	// ----------- Draws management -----------------
	/**
	 * @brief Renvoie les tirages sous forme de chaine de caractère
	 * @return Les tirages
	 */
	[[nodiscard]] auto getDrawStr() const -> string;

	/**
	 * @brief Renvoie la liste complète des tirages
	 * @return Liste des tirages
	 */
	[[nodiscard]] auto getAllDraws() const -> drawsType;

	/**
	 * @brief Renvoie si des tirages sont présents
	 * @return true si aucun tirage
	 */
	[[nodiscard]] auto emptyDraws() const -> bool { return subGames.empty() || subGames.front().getDraws().empty(); }

	/**
	 * @brief Renvoie le dernier numéro tiré qui est annulable (255 sinon)
	 * @return Dernier numéro tiré
	 */
	[[nodiscard]] auto getLastCancelableDraw() const -> uint8_t {
		if (status != Status::Running)
			return 255;
		if (emptyDraws())
			return 255;
		if (getCurrentSubRound()->emptyDraws())
			return 255;
		return getCurrentSubRound()->getDraws().back();
	}

	/**
	 * @brief Renvoie le nombre total de tirages.
	 * @return Le nombre de tirages.
	 */
	[[nodiscard]] auto drawsCount() const -> drawsType::size_type {
		return std::accumulate(
				subGames.begin(), subGames.end(), 0ULL,
				[](drawsType::size_type accu, const auto& item) -> auto { return accu + item.getDraws().size(); });
	}

	void setDiapo(const string& path, double delai);

	[[nodiscard]] auto getDiapo() const -> std::tuple<path, double>;

	/**
	 * @brief Check if the round has diaporama
	 * @return true if there is a diaporama
	 */
	[[nodiscard]] auto hasDiapo() const -> bool {
		return type == Type::Pause && (!diapoPath.empty() || diapoDelay > 0);
	}

private:
	/// Le numéro de la partie (à ne pas afficher si négatif)
	int Id = 0;

	/// Le type de partie.
	Type type = Type::OneTwoQuineFullCard;

	/// Le type actuel de la partie.
	Status status = Status::Ready;

	/// La date et heure de début de partie
	timePoint start;

	/// La date et heure de début de partie
	timePoint end;

	/// La liste des
	subRoundsType subGames;

	/// Diaporama Path (only in pause)
	path diapoPath;

	/// Diaporama delay (only in pause)
	double diapoDelay = 0;
	// ---------------- private functions ----------------

	/**
	 * @brief Accès à la sous-partie courante
	 * @return Pointeur vers la sous-partie courante
	 */
	auto getCurrentSubRound() -> subRoundsType::iterator;

	/**
	 * @brief Determine si la partie peut être éditée.
	 * @return True si la partie est éditable
	 */
	[[nodiscard]] auto isEditable() const -> bool;
};

}// namespace evl::core
