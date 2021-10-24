/**
 * @file Event.h
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "CardPack.h"
#include "GameRound.h"
#include "Serializable.h"
#include <filesystem>

namespace evl::core {

/**
 * @brief Classe décrivant un événement.
 */
class Event: public Serializable {
public:
    /**
     * @brief Lecture depuis un stream
     * @param bs Le stream d’entrée.
     */
    void read(std::istream& bs) override;

    /**
     * @brief Écriture dans un stream.
     * @param bs Le stream où écrire.
     */
    void write(std::ostream& bs) const override;

    /**
     * @brief Liste des statuts possibles.
     */
    enum struct Status {
        Invalid,       ///< N'est pas valide
        MissingParties,///< L’événement est bien défini, mais il n’a pas de parties définies
        Ready,         ///< Prêt à être utilisé
        EventStarted,  ///< Écran de début d’événement
        Paused,        ///< Est en pause
        GamePaused,    ///< Est en pause en cours de partie
        GameStart,     ///< Est en début de partie
        GameRunning,   ///< Est en cours de partie
        GameFinished,  ///< Est en fin de partie
        Finished       ///< Est fini
    };

    /**
     * @brief Accès au statut de l’événement.
     * @return Le statut de l’événement.
     */
    const Status& getStatus() const { return status; }

    /**
     * @brief Accès au nom de l’organisateur
     * @return Le nom de l’organisateur
     */
    const std::string& getOrganizerName() const { return organizerName; }

    /**
     * @brief Definition du nom de l’organisateur.
     * @param name Le nom de l’organisateur.
     */
    void setOrganizerName(const std::string& name);

    /**
     * @brief Accès au nom de l’événement
     * @return Le nom de l’événement
     */
    const std::string& getName() const { return name; }

    /**
     * @brief Definition du nom de l’événement.
     * @param name Le nom de l’événement.
     */
    void setName(const std::string& name);

    /**
     * @brief Accès au lieu de l’événement
     * @return Le lieu de l’événement
     */
    const std::string& getLocation() const { return location; }

    /**
     * @brief Definition du lieu de l’événement.
     * @param location Le lieu de l’événement.
     */
    void setLocation(const std::string& location);

    /**
     * @brief Accès au logo de l’organisateur
     * @return Le logo de l’organisateur
     */
    const std::filesystem::path& getOrganizerLogo() const { return organizerLogo; }

    /**
     * @brief Definition du logo de l’organisateur.
     * @param logo Le logo de l’organisateur.
     */
    void setOrganizerLogo(const std::filesystem::path& logo);

    /**
     * @brief Accès au logo de l’événement
     * @return Le logo de l’événement
     */
    const std::filesystem::path& getLogo() const { return logo; }

    /**
     * @brief Definition du logo de l’événement.
     * @param logo Le logo de l’événement.
     */
    void setLogo(const std::filesystem::path& logo);

    /**
     * @brief Accès aux parties
     * @return Les parties.
     */
    const std::vector<GameRound>& getGameRounds() const { return gameRounds; }

    /// Itérateur sur les parties
    using itGameround= std::vector<GameRound>::iterator;

    /**
     * @brief Renvoie l’itérateur de fin de parties.
     * @return L’itérateur de fin de parties.
     */
    itGameround getGREnd() { return gameRounds.end(); }

    /**
     * @brief Renvoie un itérateur vers la partie à l’index donné
     * @param idx L’index de la partie
     * @return La partie
     */
    itGameround getGameRound(const uint16_t& idx);

    /**
     * @brief Cherche la première partie non terminée de la liste.
     * @return La première partie non terminée de la liste.
     */
    itGameround findFirstNotFinished();

    /**
     * @brief Ajoute une partie au jeu
     * @param round La partie à ajouter.
     */
    void pushGameRound(const GameRound& round);

    /**
     * @brief Supprime un round basé sur son index
     * @param idx L'index du round à supprimer.
     */
    void deleteRoundByIndex(const uint16_t& idx);

    /**
     * @brief Interverti deux rounds
     * @param idx Indice du premier round à échanger.
     * @param idx2 Indice du second round à échanger.
     */
    void swapRoundByIndex(const uint16_t& idx, const uint16_t& idx2);

    /**
     * @brief Démarre le round actuel.
     */
    void startCurrentRound();

    /**
     * @brief Termine la partie en cours.
     */
    void endCurrentRound();

    /**
     * @brief Clos la partie en cours.
     */
    void closeCurrentRound();

    /**
     * @brief Démarre l’événement
     */
    void startEvent();

    /**
     * @brief Termine l’événement.
     */
    void stopEvent();

    /**
     * @brief Pause l’événement
     */
    void pauseEvent();

    /**
     * @brief Reprend l’événement.
     */
    void resumeEvent();

    /**
     * @brief Vérifie s’il est possible d’éditer l’événement
     * @return True si éditable.
     */
    bool isEditable() const;

private:
    /**
     * @brief Si l'événement est en phase d'édition, met à jour son statuT.
     */
    void checkValidConfig();
    /**
     * @brief Met à jour le statut de l’événement.
     */
    void updateStatus();

    /// Le statut de l’événement
    Status status= Status::Invalid;
    /// Si l’événement est en pause
    bool paused= true;

    /// Le nom de l’organisateur (requit pour validité)
    std::string organizerName;
    /// Logo de l’organisateur.
    std::filesystem::path organizerLogo;
    /// Nom de l’événement. (requit pour validité)
    std::string name;
    /// Logo de l’événement.
    std::filesystem::path logo;
    /// Lieu de l’événement.
    std::string location;

    /// Liste des parties de l’événement.
    std::vector<GameRound> gameRounds;

    /// La date et heure de début de l’événement
    std::chrono::system_clock::time_point start{};

    /// La date et heure de début de l’événement
    std::chrono::system_clock::time_point end{};

    /// Lien vers la liste des cartons
    //[[maybe_unused]] CardPack* cartons= nullptr;
};

}// namespace evl::core
