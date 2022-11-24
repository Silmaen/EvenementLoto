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

namespace evl::core {

/**
 * @brief Classe décrivant un événement.
 */
class Event: public Serializable {
public:
    // ---- définition des types ----
    /// Le type de la liste des rounds
    using roundsType= std::vector<GameRound>;
    /**
     * @brief Liste des statuts possibles.
     */
    enum struct Status {
        Invalid,       ///< N'est pas valide
        MissingParties,///< L’événement est bien défini, mais il n’a pas de parties définies
        Ready,         ///< Prêt à être utilisé
        EventStarting, ///< Écran de début d’événement
        GameRunning,   ///< Est en cours de jeu
        DisplayRules,  ///< En affichage des règles de l’événement
        EventEnding,   ///< Écran de fin d’événement
        Finished       ///< Est totalement fini
    };
    static const std::unordered_map<Status, string> StatusConvert;
    /**
     * @brief Renvoie une chaine contenant le statut.
     * @return Le statut.
     */
    [[nodiscard]] string getStatusStr() const;

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
    json to_json() const override;

    /**
     * @brief Lecture depuis un json
     * @param j Le json à lire
     */
    void from_json(const json& j) override;

    /**
     * @brief Export des parties au format JSON
     * @param file Le fichier où exporter
     */
    void exportJSON(const path& file) const;

    /**
     * @brief Import des parties au format JSON
     * @param file Le fichier à importer
     */
    void importJSON(const path& file);

    // ---- manipulation du statut ----
    /**
     * @brief Accès au statut de l’événement.
     * @return Le statut de l’événement.
     */
    const Status& getStatus() const { return status; }

    // ---- manipulation des Données propres ----
    /**
     * @brief Accès au nom de l’organisateur
     * @return Le nom de l’organisateur
     */
    const string& getOrganizerName() const { return organizerName; }

    /**
     * @brief Definition du nom de l’organisateur.
     * @param name Le nom de l’organisateur.
     */
    void setOrganizerName(const string& name);

    /**
     * @brief Accès au nom de l’événement
     * @return Le nom de l’événement
     */
    const string& getName() const { return name; }

    /**
     * @brief Definition du nom de l’événement.
     * @param name Le nom de l’événement.
     */
    void setName(const string& name);

    /**
     * @brief Accès au lieu de l’événement
     * @return Le lieu de l’événement
     */
    const string& getLocation() const { return location; }

    /**
     * @brief Definition du lieu de l’événement.
     * @param location Le lieu de l’événement.
     */
    void setLocation(const std::string& location);

    /**
     * @brief Accès au logo de l’organisateur
     * @return Le logo de l’organisateur
     */
    const path& getOrganizerLogo() const { return organizerLogo; }

    /**
     * @brief Accès au chemin complet vers le log de l’organisateur
     * @return Chemin complet vers le log de l’organisateur
     */
    path getOrganizerLogoFull() const {
        if(organizerLogo.empty())
            return organizerLogo;
        return getBasePath() / getOrganizerLogo();
    }
    /**
     * @brief Definition du logo de l’organisateur.
     * @param logo Le logo de l’organisateur.
     */
    void setOrganizerLogo(const path& logo);

    /**
     * @brief Accès au logo de l’événement
     * @return Le logo de l’événement
     */
    const path& getLogo() const { return logo; }

    /**
     * @brief Accès au chemin complet vers le logo de l’événement
     * @return Chemin complet vers le logo de l’événement
     */
    path getLogoFull() const {
        if(logo.empty())
            return logo;
        return getBasePath() / getLogo();
    }

    /**
     * @brief Definition du logo de l’événement.
     * @param logo Le logo de l’événement.
     */
    void setLogo(const path& logo);

    /**
     * @brief Accès aux règles de l’événement
     * @return Les règles.
     */
    const string& getRules() const { return rules; }

    /**
     * @brief Définit les règles de l’événement.
     * @param newRules Les règles.
     */
    void setRules(const string& newRules);

    // ----- Manipulation des rounds ----
    /**
     * @brief Renvoie l’itérateur constant de début de parties.
     * @return L’itérateur constant de début de parties.
     */
    roundsType::const_iterator beginRounds() const { return gameRounds.cbegin(); }
    /**
     * @brief Renvoie l’itérateur de fin de parties.
     * @return L’itérateur de fin de parties.
     */
    roundsType::const_iterator endRounds() const { return gameRounds.cend(); }

    /**
     * @brief Renvoie le nombre de parties.
     * @return Le nombre de parties.
     */
    roundsType::size_type sizeRounds() const { return gameRounds.size(); }
    /**
     * @brief Renvoie un itérateur vers la partie à l’index donné
     * @param idx L’index de la partie
     * @return La partie
     */
    roundsType::iterator getGameRound(const uint32_t& idx);

    /**
     * @brief Cherche la première partie non terminée de la liste.
     * @return La première partie non terminée de la liste.
     */
    roundsType::const_iterator getCurrentCGameRound() const;

    /**
     * @brief Cherche la première partie non terminée de la liste.
     * @return La première partie non terminée de la liste.
     */
    roundsType::iterator getCurrentGameRound();
    /**
     * @brief Renvoie l’index de la partie courante
     * @return Index de la partie courante
     */
    int getCurrentGameRoundIndex() const;

    /**
     * @brief Ajoute une partie au jeu
     * @param round La partie à ajouter.
     */
    void pushGameRound(const GameRound& round);

    /**
     * @brief Supprime un round basé sur son index
     * @param idx L’index du round à supprimer.
     */
    void deleteRoundByIndex(const uint16_t& idx);

    /**
     * @brief Interverti deux rounds
     * @param idx Indice du premier round à échanger.
     * @param idx2 Indice du second round à échanger.
     */
    void swapRoundByIndex(const uint16_t& idx, const uint16_t& idx2);

    // ----- Flow de l'événement -----

    /**
     * @brief Démarre l’événement
     */
    void nextState();

    /**
     * @brief Renvoie une chaine de caractère décrivant l'état courant
     * @return L'état courant
     */
    string getStateString() const;

    bool checkStateChanged() {
        bool ch= changed;
        changed= false;
        return ch;
    }
    /**
     * @brief Termine la partie en cours.
     * @param win Le numéro de la grille à ajouter
     */
    void addWinnerToCurrentRound(const std::string& win);

    /**
     * @brief Passe l’événement en mode d’affichage des règles.
     * resumeEvent() permet de retourner au statut précédent.
     */
    void displayRules();

    /**
     * @brief Vérifie s’il est possible d’éditer l’événement
     * @return True si éditable.
     */
    bool isEditable() const;

    // ---- accès aux timers ----
    /**
     * @brief Accès à la date de départ
     * @return La date de départ
     */
    const timePoint& getStarting() const { return start; }

    /**
     * @brief Accès à la date de fin
     * @return La date de fin
     */
    const timePoint& getEnding() const { return end; }

    // ---- accès au chemin ----

    /**
     * @brief Récupération du chemin de base vers l’événement
     * @return Le chemin de base de l’événement
     */
    const path& getBasePath() const { return basePath; }

    /**
     * @brief Definition du chemin de base vers l’événement
     * @param p Le chemin vers l’événement
     */
    void setBasePath(const path& p);

    /**
     * @brief Renvoie le nombre de tirages et la liste des numéros les moins tirés
     * @return Les statistiques
     */
    Statistics getStats(bool withoutChild= true) const;

#ifdef EVL_DEBUG
    /**
     * @brief define invalide Status for testing purpose
     */
    void invalidStatus() {
        status= Status{-1};
    }
    /**
     * @brief define invalide Status for testing purpose
     */
    void restoreStatusDbg() {
        status= Status::Ready;
    }
#endif
private:
    /// Le statut de l’événement
    Status status= Status::Invalid;
    /// Le précédent statut (pour pouvoir annuler)
    Status previousStatus= Status::Invalid;

    /// Le nom de l’organisateur (requit pour validité)
    string organizerName;
    /// Logo de l’organisateur.
    path organizerLogo;
    /// Nom de l’événement. (requit pour validité)
    string name;
    /// Logo de l’événement.
    path logo;
    /// Lieu de l’événement.
    string location;

    /// Les règles de l’événement
    string rules;

    /// Liste des parties de l’événement.
    roundsType gameRounds;

    /// La date et heure de début de l’événement
    timePoint start{};

    /// La date et heure de début de l’événement
    timePoint end{};

    /// Le chemin de base de l’événement
    path basePath;

    /// status change
    bool changed= false;

    /**
     * @brief Si l’événement est en phase d’édition, met à jour son statuT.
     */
    void checkValidConfig();

    /**
     * @brief Bascule à un nouveau statut et sauvegarde le précédent statut
     * @param newStatus Le nouveau statut à adopter
     */
    void changeStatus(const Status& newStatus);

    /**
     * @brief Remet l’événement au statut précédent
     */
    void restoreStatus();
};

}// namespace evl::core
