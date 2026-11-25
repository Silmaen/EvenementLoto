/**
 * @file Event.h
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "FileFormat.h"
#include "GameRound.h"

#include "Serializable.h"
#include "Statistics.h"
#include <filesystem>
#include <optional>

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
	 * @param iContext Ce que le lecteur sait du fichier parcouru.
	 */
	void read(std::istream& iBs, const ReadContext& iContext) override;

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
	 * @return True si le fichier a été écrit.
	 */
	[[nodiscard]] auto exportJSON(const std::filesystem::path& iFile) const -> bool;

	/**
	 * @brief Import des parties au format JSON
	 *
	 * L'événement n'est remplacé que si le fichier a été compris de bout en bout : un
	 * fichier illisible le laisse intact plutôt qu'à moitié écrasé.
	 *
	 * @param iFile Le fichier à importer
	 * @return True si l'événement a été remplacé.
	 */
	[[nodiscard]] auto importJSON(const std::filesystem::path& iFile) -> bool;

	/**
	 * @brief Export des parties au format YAML
	 * @param iFile Le fichier où exporter
	 * @return True si le fichier a été écrit.
	 */
	[[nodiscard]] auto exportYaml(const std::filesystem::path& iFile) const -> bool;

	/**
	 * @brief Import des parties au format YAML
	 *
	 * Comme l'import JSON : l'événement n'est remplacé qu'en cas de succès. Un YAML
	 * illisible, ou lisible mais d'une autre forme, était la seule voie par laquelle une
	 * exception remontait jusqu'à la boucle de rendu.
	 *
	 * @param iFile Le fichier à importer
	 * @return True si l'événement a été remplacé.
	 */
	[[nodiscard]] auto importYaml(const std::filesystem::path& iFile) -> bool;

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
	 * @brief Accès au chemin complet vers le logo de l'organisateur
	 * @return Chemin complet vers le logo de l'organisateur
	 */
	[[nodiscard]] auto getOrganizerLogoFull() const -> std::filesystem::path {
		if (m_organizerLogo.empty())
			return m_organizerLogo;
		return resolveAsset(getOrganizerLogo());
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
		return resolveAsset(getLogo());
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

	/**
	 * @brief L'heure à laquelle l'événement doit finir, en minutes après minuit.
	 *
	 * Une heure du jour et non une date : le programme se prépare souvent avant de savoir
	 * quel jour il sera joué. C'est la référence du retard affiché pendant le jeu.
	 * @return L'heure prévue, vide quand aucune n'est fixée.
	 */
	[[nodiscard]] auto getPlannedEnd() const -> std::optional<std::chrono::minutes> { return m_plannedEnd; }

	/**
	 * @brief Fixe l'heure de fin prévue.
	 * @param iPlannedEnd L'heure, en minutes après minuit ; vide pour n'en fixer aucune.
	 */
	void setPlannedEnd(std::optional<std::chrono::minutes> iPlannedEnd);

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
	 * @brief Insère une partie à la position demandée.
	 *
	 * La partie improvisée du jour : une partie ajoutée alors que l'événement est déjà
	 * lancé, là où elle tombe. Elle ne peut pas passer devant la partie en cours ni
	 * devant une partie déjà jouée — ce qui est derrière relève de l'histoire, pas du
	 * programme.
	 *
	 * @param iIndex La position voulue, bornée à la plage autorisée.
	 * @param iRound La partie à insérer.
	 * @return La position réellement utilisée, rien si l'insertion est refusée.
	 */
	auto insertGameRound(uint32_t iIndex, const GameRound& iRound) -> std::optional<uint32_t>;

	/**
	 * @brief Accès au catalogue des lots de l'événement.
	 *
	 * Tous les lots dont l'organisateur dispose, saisis d'un bloc et indépendamment des
	 * parties : c'est la matière que la répartition puise ensuite. Un article affecté à
	 * une manche y est recopié, de sorte qu'un fichier d'événement reste complet à lui
	 * seul.
	 *
	 * @return Le catalogue.
	 */
	[[nodiscard]] auto getCatalogue() const -> const prizes_type& { return m_catalogue; }

	/// L'emplacement d'un article : la partie et la manche qui le mettent en jeu.
	struct PrizeSlot {
		uint32_t round = 0;///< Index de la partie.
		uint32_t subRound = 0;///< Index de la manche dans la partie.

		/// Comparaison, pour savoir si un article a changé de place.
		[[nodiscard]] auto operator==(const PrizeSlot&) const -> bool = default;
	};

	/**
	 * @brief Où un article du catalogue est mis en jeu.
	 * @param iId L'identifiant de l'article.
	 * @return La manche qui le met en jeu, rien s'il n'est affecté nulle part.
	 */
	[[nodiscard]] auto findPrizeSlot(uint32_t iId) const -> std::optional<PrizeSlot>;

	/**
	 * @brief La manche que désignent un numéro de partie et un numéro de phase.
	 * @param iRound Le numéro défini de la partie (son champ « N° »), non son rang : les pauses ont leur numérotation.
	 * @param iPhase Le numéro de la phase dans la partie, à partir de 1.
	 * @return L'emplacement, rien si la partie ou la phase n'existe pas.
	 */
	[[nodiscard]] auto slotAt(uint32_t iRound, uint32_t iPhase) const -> std::optional<PrizeSlot>;

	/**
	 * @brief Déplace un article du catalogue vers une manche, ou l'en retire.
	 *
	 * L'ajustement à la main d'une répartition : l'article quitte la manche où il était
	 * et rejoint celle demandée. Une manche déjà entamée ne se laisse pas modifier, ni
	 * comme source ni comme destination — ce qui est en jeu est en jeu.
	 *
	 * @param iId L'identifiant de l'article, qui doit être au catalogue.
	 * @param iTarget La manche voulue, rien pour le laisser au catalogue sans emploi.
	 * @return True si l'article est à l'endroit demandé.
	 */
	auto assignPrize(uint32_t iId, const std::optional<PrizeSlot>& iTarget) -> bool;

	/**
	 * @brief Fixe le gagnant dont un article fait partie du set, dans une partie enfant.
	 *
	 * Le numéro est écrit au catalogue, qui reste la liste de référence, et sur la copie en
	 * jeu tant que sa manche n'est pas entamée.
	 *
	 * @param iId L'identifiant de l'article, qui doit être au catalogue.
	 * @param iSet Le numéro de gagnant.
	 * @return True si l'article a été trouvé.
	 */
	auto setPrizeSet(uint32_t iId, uint8_t iSet) -> bool;

	/**
	 * @brief Fait entrer au catalogue les lots des parties qui n'y sont pas.
	 *
	 * Deux situations, une seule réponse. Un événement d'avant le catalogue porte tous
	 * ses lots dans ses manches, répartis à la main : sans les reprendre, le catalogue
	 * resterait vide et il n'y aurait rien à répartir. Et un lot saisi directement dans
	 * le réglage des parties n'a, lui non plus, aucune raison de rester invisible au
	 * catalogue.
	 *
	 * L'opération n'enlève rien : elle ajoute ce qui manque et attribue un identifiant à
	 * l'article comme à sa copie, ce qui rend l'affectation lisible dans les deux sens.
	 * Deux articles de même désignation sont deux articles — deux bouteilles restent
	 * deux bouteilles.
	 *
	 * @return Le nombre d'articles nouvellement entrés au catalogue.
	 */
	auto adoptOrphanPrizes() -> std::size_t;

	/**
	 * @brief Vide le catalogue, et retire des manches modifiables les articles qu'il y avait placés.
	 *
	 * Une manche déjà entamée garde ses lots : ce qui est en jeu est en jeu.
	 * @return Le nombre d'articles retirés du catalogue.
	 */
	auto clearCatalogue() -> std::size_t;

	/**
	 * @brief Remplace le catalogue des lots.
	 * @param iCatalogue Le nouveau catalogue.
	 */
	void setCatalogue(const prizes_type& iCatalogue);

	/**
	 * @brief Le premier numéro encore libre, dans la numérotation demandée.
	 *
	 * Les pauses se numérotent à part : la deuxième pause de l'après-midi est « Pause 2 »
	 * même si elle arrive après la cinquième partie.
	 *
	 * @param iForPause True pour la numérotation des pauses, false pour celle des parties.
	 * @return Le plus grand numéro utilisé dans cette numérotation, plus un.
	 */
	[[nodiscard]] auto nextFreeRoundId(bool iForPause = false) const -> int;

	/**
	 * @brief Première position où une partie peut être insérée.
	 * @return Zéro tant que l'événement est éditable, sinon juste après la partie en cours.
	 */
	[[nodiscard]] auto firstInsertableIndex() const -> uint32_t;

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
	 * @brief Désigne des enfants gagnants dans la partie enfant en cours, sans les nommer.
	 * @param iCount Le nombre d'enfants qui viennent de gagner.
	 */
	void awardChildWinnersToCurrentRound(uint32_t iCount);

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
	 * @brief Copie les logos à côté du fichier de partie, dans « <nom>_data ».
	 *
	 * Après l'appel, les logos sont relatifs à ce dossier : la partie et ses images se
	 * déplacent ensemble. Un logo introuvable est laissé tel quel.
	 * @param iFile Le fichier de partie en cours d'enregistrement.
	 */
	void bundleAssets(const std::filesystem::path& iFile);

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
	 * @brief define invalid Status for testing purpose
	 */
	void invalidStatus() { m_status = Status::Invalid; }
	/**
	 * @brief define invalid Status for testing purpose
	 */
	void restoreStatusDbg() { m_status = Status::Ready; }
#endif
private:
	/// Le statut de l’événement
	Status m_status = Status::Invalid;
	/// Le précédent statut (pour pouvoir annuler)
	Status m_previousStatus = Status::Invalid;

	/// Le nom de l'organisateur (requis pour validité)
	std::string m_organizerName;
	/// Logo de l’organisateur.
	std::filesystem::path m_organizerLogo;
	/// Nom de l'événement. (requis pour validité)
	std::string m_name;
	/// Logo de l’événement.
	std::filesystem::path m_logo;
	/// Lieu de l’événement.
	std::string m_location;

	/// Les règles de l’événement
	std::string m_rules;

	/// Liste des parties de l’événement.
	rounds_type m_gameRounds;

	/// Tous les lots dont l'organisateur dispose, avant répartition.
	prizes_type m_catalogue;
	/// L'heure de fin prévue, en minutes après minuit.
	std::optional<std::chrono::minutes> m_plannedEnd;

	/// La date et heure de début de l’événement
	time_point m_start;

	/// La date et heure de fin de l'événement
	time_point m_end;

	/// Le chemin de base de l’événement
	/**
	 * @brief Tentative de lecture complète du corps, dans un contexte donné.
	 * @param[in] iFrame Le cadre déjà validé.
	 * @param[in] iContext Le contexte à essayer.
	 * @return Vrai si le corps a été lu en entier.
	 */
	auto readWith(const FileFrame& iFrame, const ReadContext& iContext) -> bool;

	/**
	 * @brief Lecture du corps du fichier, une fois son cadre validé.
	 * @param[in,out] iBs Le stream positionné sur le corps.
	 * @param[in] iContext Ce que le lecteur sait du fichier parcouru.
	 * @return Vrai si le corps a été lu en entier.
	 */
	auto readBody(std::istream& iBs, const ReadContext& iContext) -> bool;

	/**
	 * @brief Le premier identifiant d'article encore libre.
	 * @return Le plus grand identifiant connu, au catalogue comme en jeu, plus un.
	 */
	[[nodiscard]] auto nextFreePrizeId() const -> uint32_t;

	/**
	 * @brief Donne un identifiant aux articles du catalogue qui n'en ont pas.
	 *
	 * Les identifiants sont attribués ici et nulle part ailleurs : l'événement est seul
	 * à savoir lesquels sont déjà pris.
	 */
	void assignPrizeIds();

	/**
	 * @brief Lit le catalogue des lots, à partir de la version 9.
	 * @param iBs Le stream d’entrée.
	 * @param iContext Ce que le lecteur sait du fichier parcouru.
	 * @return True si la lecture a réussi.
	 */
	auto readCatalogue(std::istream& iBs, const ReadContext& iContext) -> bool;

	std::filesystem::path m_basePath;

	/**
	 * @brief Chemin complet d'une ressource stockée relativement au fichier de partie.
	 *
	 * Si elle n'est pas là où le chemin l'annonce (partie copiée sur une autre machine),
	 * le nom du fichier est cherché à côté de la partie et dans son dossier « *_data ».
	 * @param iRelative Le chemin stocké.
	 * @return Le chemin à ouvrir.
	 */
	[[nodiscard]] auto resolveAsset(const std::filesystem::path& iRelative) const -> std::filesystem::path;

	/// status change
	bool m_changed = false;

	/**
	 * @brief Si l’événement est en phase d’édition, met à jour son statut.
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
