/**
 * @file GridCard.h
 * @author Silmaen
 * @date 17/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "RandomNumberGenerator.h"
#include "Serializable.h"
#include <array>
#include <ostream>
#include <string>

namespace evl::core {

/**
 * @brief Classe de base pour définir un carton.
 */
class GridCard: public Serializable {
public:
    /**
     * @brief Les données d’une case de grille.
     */
    struct GCase {
        /// Le numéro de la case.
        uint8_t number : 7 {1};
        /// Si le numéro est coché.
        uint8_t checked : 1 {false};
    };
    /**
     * @brief Les différents statut du carton
     */
    enum struct Status {
        InGame,        ///< Le carton est toujours en jeu.
        AlmostOneLine, ///< Il ne manque qu’un numéro pour qu’une ligne soit remplie.
        OneLine,       ///< Une ligne est entièrement remplie.
        AlmostTwoLines,///< Il ne manque qu’un numéro pour que deux lignes soient remplies.
        TwoLines,      ///< Deux lignes du carton sont remplies.
        AlmostFull,    ///< Il n'y a plus qu'un seul numéro.
        Full,          ///< Tout le carton est rempli.
        OutGame        ///< Le carton n’est pas en jeu.
    };

    /**
     * @brief Remet à zéro l’état de case cochée.
     */
    void reset();

    /**
     * @brief Vérifie si le carton est actif.
     * @return Vrai si le carton est actif.
     */
    bool isActive() const { return res != Status::OutGame; }

    /**
     * @brief Renvoie une chaine de caractère représentant le carton.
     * @return Une chaine.
     */
    std::string asString() const;

    /**
     * @brief Défini la grille à partir d’une chaine.
     * @param s Chaine de définition d’une grille.
     */
    void fromString(const std::string& s);

    /**
     * @brief Mets à jour le carton avec le numéro joué.
     * @param num Le numéro joué.
     */
    void playNumber(const uint8_t num);

    /**
     * @brief Mets à jour le carton en enlevant le numéro joué.
     * @param num Le numéro joué.
     */
    void unPlayNumber(const uint8_t num);

    /**
     * @brief Mets à jour le status du carton
     */
    void updateStatus();

    /**
     * @brief Renvoie le status du carton.
     * @return Le statut du carton.
     */
    [[nodiscard]] const Status& getStatus() const { return res; }

    /**
     * @brief Sort ce carton du jeu
     */
    void deactivate();

    /**
     * @brief Met ce carton en jeu
     */
    void activate();

    /**
     * @brief Génère aléatoirement une grille, celle-ci est active par défaut.
     * @param num Le numéro de la grille
     * @param rng Lien éventuel vers un générateur de nombre aléatoire externe.
     */
    void generate(const uint32_t& num, RandomNumberGenerator* rng= nullptr);

    /**
     * @brief Écriture dans un stream.
     * @param os Le stream où écrire.
     */
    void write(std::ostream& os) const override;

    /**
     * @brief Lecture depuis un stream
     * @param is Le stream d’entrée.
     */
    void read(std::istream& is) override;

    /**
     * @brief Écriture dans un json.
     * @return Le json à remplir
     */
    json to_json() const override { return json{}; }

    /**
     * @brief Lecture depuis un json
     * @param j Le json à lire
     */
    void from_json(const json&) override {}

private:
    /// Le numéro du carton
    uint32_t IdNumber= 0;

    /// Les données du carton.
    std::array<std::array<GCase, nb_colones>, nb_ligne> lines;

    /// L’état actuel du carton
    Status res= Status::InGame;
};

}// namespace evl::core
