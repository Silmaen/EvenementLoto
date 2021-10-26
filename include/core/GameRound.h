/**
 * @file GameRound.h
 * @author Silmaen
 * @date 20/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "Serializable.h"
#include "timeFunctions.h"
#include <chrono>
#include <vector>

namespace evl::core {

/**
 * @brief Classe définissant une partie.
 */
class GameRound: public Serializable {
public:

    /**
     * @brief Liste des types de parties connus.
     */
    enum struct Type {
        None,     ///< la partie n'a aucun type
        OneQuine, ///< Le joueur gagne dès qu’il complète une ligne.
        TwoQuines,///< Le joueur gagne dès qu’il complète deux lignes.
        FullCard, ///< Le joueur gagne dès qu’il complète son carton.
        Inverse   ///< Le joueur est éliminé dès qu’un se ses numéros est tiré.
    };

    /**
     * @brief Renvoie une chaine contenant le type de partie.
     * @return Le type de partie.
     */
    [[nodiscard]] std::string getTypeStr() const;

    /**
     * @brief Renvoie le type de partie.
     * @return Le type de partie
     */
    [[nodiscard]] const Type& getType() const { return type; }

    /**
     * @brief Définit le type de partie
     * @param t
     */
    void setType(const Type& t) {
        if(!isEditable())
            return;
        type= t;
        updateStatus();
    }

    /**
     * @brief Les status possibles de la partie
     */
    enum struct Status {
        Invalid,///< Le statut d’erreur
        Ready,  ///< La partie est prête à être jouée
        Started,///< La partie est démarrée
        Paused, ///< Est en pause
        Result, ///< Est en affichage de résultat.
        Finished///< La partie est finie
    };
    /**
     * @brief Renvoie une chaine contenant le type de partie.
     * @return Le type de partie.
     */
    [[nodiscard]] std::string getStatusStr() const;

    /**
     * @brief Renvoie le type de partie.
     * @return Le type de partie
     */
    [[nodiscard]] const Status& getStatus() const { return status; }

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
    void restoreStatus() {
        updateStatus();
    }
#endif

    /**
     * @brief Débute la partie.
     */
    void startGameRound();

    /**
     * @brief Pause la Partie
     */
    void pauseGameRound();
    /**
     * @brief Reprend la Partie
     */
    void resumeGameRound();
    /**
     * @brief Termine la Partie
     */
    void endGameRound();
    /**
     * @brief Close la Partie
     */
    void closeGameRound();

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
     * @brief Accès à la date de départ
     * @return La date de départ
     */
    const timePoint& getStarting() const { return start; }

    /**
     * @brief Accès à la date de fin
     * @return La date de fin
     */
    const timePoint& getEnding() const { return end; }

    /**
     * @brief Ajoute le numéro dans la liste des numéros tirés
     * @param num
     */
    void addPickedNumber(const uint8_t& num);

    /**
     * @brief Accès au tirage
     * @return Les tirages.
     */
    const std::vector<uint8_t>& getDraws() const { return Draws; }

    /**
     * @brief Supprime le dernier tirage.
     */
    void removeLastPick();

private:
    /**
     * @brief Met à jour le statut.
     */
    void updateStatus();

    /**
     * @brief Determine si la partie peut être éditée.
     * @return True si la partie est éditable
     */
    [[nodiscard]] bool isEditable() const;

    /// Le type de partie.
    Type type= Type::None;

    /// Le type actuel de la partie.
    Status status= Status::Invalid;

    /// La date et heure de début de partie
    timePoint start{};
    /// Si la partie est en pause
    bool paused= false;
    /// La date et heure de début de partie
    timePoint end{};

    /// La liste des numéros tirés.
    std::vector<uint8_t> Draws;
};

}// namespace evl::core
