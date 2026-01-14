/**
 * @file GameRound.h
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "Log.h"
#include "Serializable.h"
#include "SubGameRound.h"
#include <filesystem>
#include <numeric>
#include <vector>

namespace evl::core {

/**
 * @brief Classe définissant une partie.
 */
class GameRound final : public Serializable {
public:
	// ---- définition des types ----
	/// Le type utilisé pour représenter la liste des tirages.
	using draws_type = std::vector<uint8_t>;
	using sub_rounds_type = std::vector<SubGameRound>;
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

	// --- constructeurs ----
	/**
	 * @brief Constructeur
	 * @param iType Le type de partie
	 */
	explicit GameRound(const Type& iType = Type::OneTwoQuineFullCard);

	// ---- manipulation du type de partie ----
	/**
	 * @brief Renvoie une chaine contenant le type de partie.
	 * @return Le type de partie.
	 */
	[[nodiscard]] auto getTypeStr() const -> std::string;

	/**
	 * @brief Renvoie le type de partie.
	 * @return Le type de partie
	 */
	[[nodiscard]] auto getType() const -> const Type& { return m_type; }

	/**
	 * @brief Définit le type de partie
	 * @param iType Type de partie
	 */
	void setType(const Type& iType);

	// ---- manipulation du statut ----
	/**
	 * @brief Renvoie une chaine contenant le type de partie.
	 * @return Le type de partie.
	 */
	[[nodiscard]] auto getStatusStr() const -> std::string;

	/**
	 * @brief Renvoie le type de partie.
	 * @return Le type de partie
	 */
	[[nodiscard]] auto getStatus() const -> const Status& { return m_status; }

	/**
	 * @brief Renvoie si le round est fini
	 * @return True si le round est fini
	 */
	[[nodiscard]] auto isFinished() const -> bool { return m_status == Status::Done; }
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
		m_type = Type::OneTwoQuineFullCard;
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
	[[nodiscard]] auto getStateString() const -> std::string;

	/**
	 * @brief Ajoute le numéro dans la liste des numéros tirés
	 * @param iNumber Numéro à ajouter
	 */
	void addPickedNumber(const uint8_t& iNumber);

	/**
	 * @brief Supprime le dernier tirage.
	 */
	void removeLastPick();

	/**
	 * @brief donne un gagnant
	 * @param iWinner Le nom du gagnant
	 */
	void addWinner(const std::string& iWinner);

	/**
	 * @brief Renvoie la liste des gagnants de sous-partie
	 * @return Les gagnants
	 */
	[[nodiscard]] auto getWinnerStr() const -> std::string;

	// ---- Serialisation ----
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
	[[nodiscard]] auto getCurrentSubRound() const -> sub_rounds_type::const_iterator;
	/**
	 * @brief Accès à la sous-partie courante
	 * @return Itérateur constant sur la sous-partie courante
	 */
	auto getSubRound(uint32_t iIndex) -> sub_rounds_type::iterator;
	/**
	 * @brief Accès à la sous-partie courante
	 * @return Itérateur constant sur la sous-partie courante
	 */
	[[nodiscard]] auto getSubRound(uint32_t iIndex) const -> sub_rounds_type::const_iterator;
	/**
	 * @brief Renvoie un itérateur constant sur le début de la liste des subround.
	 * @return Itérateur constant sur le début de la liste des subround.
	 */
	[[nodiscard]] auto beginSubRound() const -> sub_rounds_type::const_iterator { return m_subGames.cbegin(); }
	/**
	 * @brief Renvoie un itérateur constant sur la fin de la liste des subround.
	 * @return Itérateur constant sur la fin de la liste des subround.
	 */
	[[nodiscard]] auto endSubRound() const -> sub_rounds_type::const_iterator { return m_subGames.cend(); }
	/**
	 * @brief Renvoie la taille de la liste des subround.
	 * @return La taille de la fin de la liste des subround.
	 */
	[[nodiscard]] auto sizeSubRound() const -> sub_rounds_type::size_type { return m_subGames.size(); }

	/**
	 * @brief Défini le numéro de partie
	 * @param iId Le numéro de partie
	 */
	void setId(const int iId) {
		if (!isEditable()) {
			log_warn("Impossible de modifier l'identifiant d'une partie non éditable");
			return;
		}
		m_id = iId;
	}

	/**
	 * @brief Renvoie le numéro d'affichage de la partie
	 * @return Le numéro d'affichage
	 */
	[[nodiscard]] auto getId() const -> const int& { return m_id; }

	/**
	 * @brief Affichage de nom spécial
	 * @return Nom du round
	 */
	[[nodiscard]] auto getName() const -> std::string;

	// ----------- Draws management -----------------
	/**
	 * @brief Renvoie les tirages sous forme de chaine de caractère
	 * @return Les tirages
	 */
	[[nodiscard]] auto getDrawStr() const -> std::string;

	/**
	 * @brief Renvoie la liste complète des tirages
	 * @return Liste des tirages
	 */
	[[nodiscard]] auto getAllDraws() const -> draws_type;

	/**
	 * @brief Renvoie si des tirages sont présents
	 * @return true si aucun tirage
	 */
	[[nodiscard]] auto emptyDraws() const -> bool {
		return m_subGames.empty() || m_subGames.front().getDraws().empty();
	}

	/**
	 * @brief Renvoie le dernier numéro tiré qui est annulable (255 sinon)
	 * @return Dernier numéro tiré
	 */
	[[nodiscard]] auto getLastCancelableDraw() const -> uint8_t {
		if (m_status != Status::Running)
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
	[[nodiscard]] auto drawsCount() const -> draws_type::size_type {
		return std::accumulate(
				m_subGames.begin(), m_subGames.end(), 0ULL,
				[](draws_type::size_type iAccu, const auto& iItem) -> auto { return iAccu + iItem.getDraws().size(); });
	}

	/**
	 * @brief Défini le diaporama pour la pause
	 * @param iPath Le chemin du diaporama
	 * @param iDelai Le délai entre chaque image
	 */
	void setDiapo(const std::string& iPath, double iDelai);

	/**
	 * @brief Renvoie le diaporama et son délai
	 * @return Le chemin du diaporama et le délai
	 */
	[[nodiscard]] auto getDiapo() const -> std::tuple<std::filesystem::path, double>;

	/**
	 * @brief Check if the round has diaporama
	 * @return true if there is a diaporama
	 */
	[[nodiscard]] auto hasDiapo() const -> bool {
		return m_type == Type::Pause && (!m_diapoPath.empty() || m_diapoDelay > 0);
	}

	/**
	 * @brief Accès à la sous-partie courante
	 * @return Pointeur vers la sous-partie courante
	 */
	auto getCurrentSubRound() -> sub_rounds_type::iterator;
	/**
	 * @brief Determine si la partie peut être éditée.
	 * @return True si la partie est éditable
	 */
	[[nodiscard]] auto isEditable() const -> bool;

private:
	/// Le numéro de la partie (à ne pas afficher si négatif)
	int m_id = 0;

	/// Le type de partie.
	Type m_type = Type::OneTwoQuineFullCard;

	/// Le type actuel de la partie.
	Status m_status = Status::Ready;

	/// La date et heure de début de partie
	time_point m_start;

	/// La date et heure de début de partie
	time_point m_end;

	/// La liste des
	sub_rounds_type m_subGames;

	/// Diaporama Path (only in pause)
	std::filesystem::path m_diapoPath;

	/// Diaporama delay (only in pause)
	double m_diapoDelay = 0;
	// ---------------- private functions ----------------
};

}// namespace evl::core
