/**
 * @file Event.h
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "GameRound.h"
#include "Serializable.h"
#include "Statistics.h"
#include <filesystem>

namespace evl::core {

/**
 * @brief Classe décrivant un événement.
 */
class Event final : public Serializable {
public:
	// ---- définition des types ----
	/// Le type de la liste des rounds
	using rounds_type = std::vector<GameRound>;
	/**
	 * @brief Liste des statuts possibles.
	 */
	enum struct Status : uint8_t {
		Invalid,///< N'est pas valide
		MissingParties,///< L’événement est bien défini, mais il n’a pas de parties définies
		Ready,///< Prêt à être utilisé
		EventStarting,///< Écran de début d’événement
		GameRunning,///< Est en cours de jeu
		DisplayRules,///< En affichage des règles de l’événement
		EventEnding,///< Écran de fin d’événement
		Finished///< Est totalement fini
	};
	/**
	 * @brief Renvoie une chaine contenant le statut.
	 * @return Le statut.
	 */
	[[nodiscard]] auto getStatusStr() const -> std::string;

	// ---- Serialisation ----
	/**
	 * @brief Lecture depuis un stream
	 * @param iBs Le stream d’entrée.
	 * @param iFileVersion La version du fichier à lire
	 */
	void read(std::istream& iBs, int iFileVersion) override;

	/**
	 * @brief Écriture dans un stream.
	 * @param oBs Le stream où écrire.
	 */
	void write(std::ostream& oBs) const override;

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

	/**
	 * @brief Export des parties au format JSON
	 * @param iFile Le fichier où exporter
	 */
	void exportJSON(const std::filesystem::path& iFile) const;

	/**
	 * @brief Import des parties au format JSON
	 * @param iFile Le fichier à importer
	 */
	void importJSON(const std::filesystem::path& iFile);

	/**
	 * @brief Export des parties au format YAML
	 * @param iFile Le fichier où exporter
	 */
	void exportYaml(const std::filesystem::path& iFile) const;

	/**
	 * @brief Import des parties au format YAML
	 * @param iFile Le fichier à importer
	 */
	void importYaml(const std::filesystem::path& iFile);

	// ---- manipulation du statut ----
	/**
	 * @brief Accès au statut de l’événement.
	 * @return Le statut de l’événement.
	 */
	[[nodiscard]] auto getStatus() const -> const Status& { return m_status; }

	// ---- manipulation des Données propres ----
	/**
	 * @brief Accès au nom de l’organisateur
	 * @return Le nom de l’organisateur
	 */
	[[nodiscard]] auto getOrganizerName() const -> const std::string& { return m_organizerName; }

	/**
	 * @brief Definition du nom de l’organisateur.
	 * @param iName Le nom de l’organisateur.
	 */
	void setOrganizerName(const std::string& iName);

	/**
	 * @brief Accès au nom de l’événement
	 * @return Le nom de l’événement
	 */
	[[nodiscard]] auto getName() const -> const std::string& { return m_name; }

	/**
	 * @brief Definition du nom de l’événement.
	 * @param iName Le nom de l’événement.
	 */
	void setName(const std::string& iName);

	/**
	 * @brief Accès au lieu de l’événement
	 * @return Le lieu de l’événement
	 */
	[[nodiscard]] auto getLocation() const -> const std::string& { return m_location; }

	/**
	 * @brief Definition du lieu de l’événement.
	 * @param iLocation Le lieu de l’événement.
	 */
	void setLocation(const std::string& iLocation);

	/**
	 * @brief Accès au logo de l’organisateur
	 * @return Le logo de l’organisateur
	 */
	[[nodiscard]] auto getOrganizerLogo() const -> const std::filesystem::path& { return m_organizerLogo; }

	/**
	 * @brief Accès au chemin complet vers le log de l’organisateur
	 * @return Chemin complet vers le log de l’organisateur
	 */
	[[nodiscard]] auto getOrganizerLogoFull() const -> std::filesystem::path {
		if (m_organizerLogo.empty())
			return m_organizerLogo;
		return getBasePath() / getOrganizerLogo();
	}
	/**
	 * @brief Definition du logo de l’organisateur.
	 * @param iLogo Le logo de l’organisateur.
	 */
	void setOrganizerLogo(const std::filesystem::path& iLogo);

	/**
	 * @brief Accès au logo de l’événement
	 * @return Le logo de l’événement
	 */
	[[nodiscard]] auto getLogo() const -> const std::filesystem::path& { return m_logo; }

	/**
	 * @brief Accès au chemin complet vers le logo de l’événement
	 * @return Chemin complet vers le logo de l’événement
	 */
	[[nodiscard]] auto getLogoFull() const -> std::filesystem::path {
		if (m_logo.empty())
			return m_logo;
		return getBasePath() / getLogo();
	}

	/**
	 * @brief Definition du logo de l’événement.
	 * @param iLogo Le logo de l’événement.
	 */
	void setLogo(const std::filesystem::path& iLogo);

	/**
	 * @brief Accès aux règles de l’événement
	 * @return Les règles.
	 */
	[[nodiscard]] auto getRules() const -> const std::string& { return m_rules; }

	/**
	 * @brief Définit les règles de l’événement.
	 * @param iNewRules Les règles.
	 */
	void setRules(const std::string& iNewRules);

	// ----- Manipulation des rounds ----
	/**
	 * @brief Renvoie l’itérateur constant de début de parties.
	 * @return L’itérateur constant de début de parties.
	 */
	[[nodiscard]] auto beginRounds() const -> rounds_type::const_iterator { return m_gameRounds.cbegin(); }
	/**
	 * @brief Renvoie l’itérateur de fin de parties.
	 * @return L’itérateur de fin de parties.
	 */
	[[nodiscard]] auto endRounds() const -> rounds_type::const_iterator { return m_gameRounds.cend(); }

	/**
	 * @brief Renvoie le nombre de parties.
	 * @return Le nombre de parties.
	 */
	[[nodiscard]] auto sizeRounds() const -> rounds_type::size_type { return m_gameRounds.size(); }
	/**
	 * @brief Renvoie un itérateur vers la partie à l’index donné
	 * @param iIndex L’index de la partie
	 * @return La partie
	 */
	auto getGameRound(const uint32_t& iIndex) -> rounds_type::iterator;

	/**
	 * @brief Vérifie si la partie courante est la dernière de la liste.
	 * @return True si la partie courante est la dernière de la liste.
	 */
	[[nodiscard]] auto isCurrentGameRoundLast() const -> bool {
		const auto it = getCurrentCGameRound();
		return (it != m_gameRounds.cend()) && (std::next(it) == m_gameRounds.cend());
	}

	/**
	 * @brief Cherche la première partie non terminée de la liste.
	 * @return La première partie non terminée de la liste.
	 */
	[[nodiscard]] auto getCurrentCGameRound() const -> rounds_type::const_iterator;

	/**
	 * @brief Cherche la première partie non terminée de la liste.
	 * @return La première partie non terminée de la liste.
	 */
	auto getCurrentGameRound() -> rounds_type::iterator;
	/**
	 * @brief Renvoie l’index de la partie courante
	 * @return Index de la partie courante
	 */
	[[nodiscard]] auto getCurrentGameRoundIndex() const -> int;


	/**
	 * @brief Cherche la première partie non terminée de la liste.
	 * @return La première partie non terminée de la liste.
	 */
	[[nodiscard]] auto getNextCGameRound() const -> rounds_type::const_iterator;

	/**
	 * @brief Cherche la première partie non terminée de la liste.
	 * @return La première partie non terminée de la liste.
	 */
	auto getNextGameRound() -> rounds_type::iterator;

	/**
	 * @brief Ajoute une partie au jeu
	 * @param iRound La partie qu'il faut ajouter.
	 */
	void pushGameRound(const GameRound& iRound);

	/**
	 * @brief Supprime un round basé sur son index
	 * @param iIndex L’index du round à supprimer.
	 */
	void deleteRoundByIndex(const uint16_t& iIndex);

	/**
	 * @brief Interverti deux rounds
	 * @param iIndex Indice du premier round à échanger.
	 * @param iIndex2 Indice du second round à échanger.
	 */
	void swapRoundByIndex(const uint16_t& iIndex, const uint16_t& iIndex2);

	// ----- Flow de l'événement -----

	/**
	 * @brief Démarre l’événement
	 */
	void nextState();

	/**
	 * @brief Renvoie une chaine de caractère décrivant l'état courant
	 * @return L'état courant
	 */
	[[nodiscard]] auto getStateString() const -> std::string;

	auto checkStateChanged() -> bool {
		const bool ch = m_changed;
		m_changed = false;
		return ch;
	}
	/**
	 * @brief Termine la partie en cours.
	 * @param iWin Le numéro de la grille à ajouter
	 */
	void addWinnerToCurrentRound(const std::string& iWin);

	/**
	 * @brief Passe l’événement en mode d’affichage des règles.
	 * resumeEvent() permet de retourner au statut précédent.
	 */
	void displayRules();

	/**
	 * @brief Vérifie s’il est possible d’éditer l’événement
	 * @return True si éditable.
	 */
	[[nodiscard]] auto isEditable() const -> bool;

	/**
	 * @brief Vérifie si l’événement est fini.
	 * @return True si l’événement est fini.
	 */
	[[nodiscard]] auto isFinished() const -> bool { return m_status == Status::Finished; }

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

	// ---- accès au chemin ----

	/**
	 * @brief Récupération du chemin de base vers l’événement
	 * @return Le chemin de base de l’événement
	 */
	[[nodiscard]] auto getBasePath() const -> const std::filesystem::path& { return m_basePath; }

	/**
	 * @brief Definition du chemin de base vers l’événement
	 * @param iBasePath Le chemin vers l’événement
	 */
	void setBasePath(const std::filesystem::path& iBasePath);

	/**
	 * @brief Renvoie le nombre de tirages et la liste des numéros les moins tirés
	 * @return Les statistiques
	 */
	[[nodiscard]] auto getStats(bool iWithoutChild = true) const -> Statistics;

	/**
	 * @brief Renvoie la progression de l’événement entre 0.0 et 1.0
	 * @return La progression de l’événement
	 */
	[[nodiscard]] auto getProgression() const -> float;

	/**
	 * @brief Vérifie si l’événement attend un tirage de numéro.
	 * @return True si l’événement attend un tirage de numéro.
	 */
	[[nodiscard]] auto canDraw() const -> bool;

#ifdef EVL_DEBUG
	/**
	 * @brief define invalide Status for testing purpose
	 */
	void invalidStatus() { m_status = Status::Invalid; }
	/**
	 * @brief define invalide Status for testing purpose
	 */
	void restoreStatusDbg() { m_status = Status::Ready; }
#endif
private:
	/// Le statut de l’événement
	Status m_status = Status::Invalid;
	/// Le précédent statut (pour pouvoir annuler)
	Status m_previousStatus = Status::Invalid;

	/// Le nom de l’organisateur (requit pour validité)
	std::string m_organizerName;
	/// Logo de l’organisateur.
	std::filesystem::path m_organizerLogo;
	/// Nom de l’événement. (requit pour validité)
	std::string m_name;
	/// Logo de l’événement.
	std::filesystem::path m_logo;
	/// Lieu de l’événement.
	std::string m_location;

	/// Les règles de l’événement
	std::string m_rules;

	/// Liste des parties de l’événement.
	rounds_type m_gameRounds;

	/// La date et heure de début de l’événement
	time_point m_start;

	/// La date et heure de début de l’événement
	time_point m_end;

	/// Le chemin de base de l’événement
	std::filesystem::path m_basePath;

	/// status change
	bool m_changed = false;

	/**
	 * @brief Si l’événement est en phase d’édition, met à jour son statuT.
	 */
	void checkValidConfig();

	/**
	 * @brief Bascule à un nouveau statut et sauvegarde le précédent statut
	 * @param iNewStatus Le nouveau statut à adopter
	 */
	void changeStatus(const Status& iNewStatus);

	/**
	 * @brief Remet l’événement au statut précédent
	 */
	void restoreStatus();
};

}// namespace evl::core
