/**
 * \author Silmaen
 * \date 17/10/2021
 *
 */
#pragma once
#include "baseDefine.h"
#include <array>
#include <string>

namespace evl {

/**
 * @brief Classe de base pour définir un carton.
 */
class carton {
public:
    /**
     * @brief Les données d'une case de grille.
     */
    struct GCase {
        uint8_t numero : 7 {1};     ///< Le numéro de la case.
        uint8_t checked : 1 {false};///< Si le numéro est coché.
    };
    /**
     * @brief Les différents statut du carton
     */
    enum class ResultCarton {
        EnCours,           ///< Le carton est toujours en jeu.
        PresqueUneLigne,   ///< Il ne manque qu'un numéro pour qu'une ligne soit remplie.
        UneLigne,          ///< Une ligne est entièrement remplie.
        PresqueDeuxLignes, ///< Il ne manque qu'un numéro pour que 2 lignes soient remplies.
        DeuxLignes,        ///< Deux lignes du carton sont remplies.
        PresqueCartonPlein,///< Il n'y a plus qu'un seul numéro.
        CartonPlein,       ///< Tout le carton est rempli.
        HorsJeu            ///< Le carton n'est pas en jeu.
    };
    /**
     * @brief Remet à zéro l'état de case cochée.
     */
    void resetCarton();

    /**
     * @brief Renvoie une chaine de caractère représentant le carton.
     * @return Une chaine.
     */
    std::string asString() const;

    /**
     * @brief Défini la grille à partir d'une chaine.
     * @param s Chaine de définition d'une grille.
     */
    void fromString(std::string& s);

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
    [[nodiscard]] const ResultCarton& getStatus() const { return res; }

    /**
     * @brief Sort ce carton du jeu
     */
    void deactivate() { res= ResultCarton::HorsJeu; }

    /**
     * @brief Met ce carton en jeu
     */
    void activate() {
        res= ResultCarton::EnCours;
        updateStatus();
    }

private:
    uint32_t numero= 0;                                       ///< le numéro du carton
    std::array<std::array<GCase, nb_colones>, nb_ligne> lines;///< Les données du carton.
    ResultCarton res= ResultCarton::EnCours;                  ///< l'état actuel du carton
};

}// namespace evl
