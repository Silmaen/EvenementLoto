/**
 * @author Silmaen
 * @date 18/10/2021
 */
#pragma once
#include <random>
#include <vector>

/**
 * @brief Namespace pour les fonctions centrales du programme.
 */
namespace evl::core {

/**
 * @brief Class permettant de tirer aléatoirement des nombres entre 1 et 90 (inclus).
 */
class RandomNumberGenerator {
public:
    /**
     * @brief Constructeur de base.
     * @param debug Si mis à vrai, utilise une seed déterministe.
     */
    RandomNumberGenerator(bool debug= false) {
        if(debug)
            std::srand(1234);
        else
            std::srand(time(nullptr));
    }

    /**
     * @brief Remet à zéro la liste des numéros déjà tiré.
     */
    void resetPick() { alreadyPicked.clear(); }

    /**
     * @brief Ajoute manuellement un numéro à la liste des numéros déjà tirés.
     * @param num Le numéro à ajouter.
     * @return False si le numéro est déjà dans la liste.
     */
    [[nodiscard]] bool addPick(const uint8_t& num);

    /**
     * @brief Tire au sort un numéro non déjà tiré.
     * @return Le numéro tiré.
     */
    [[nodiscard]] uint8_t pick();

    /**
     * @brief Renvoie la liste des numéros tirés.
     * @return La liste des numéros tirés.
     */
    [[nodiscard]] const std::vector<uint8_t>& getPicked() const { return alreadyPicked; }

    /**
     * @brief Retire le dernier numéro tiré de la liste.
     */
    void popNum() {
        if(!alreadyPicked.empty()) alreadyPicked.pop_back();
    }

    /**
     * @brief Génère une ligne de grille de loto.
     *
     * Les chiffres tirés sont ajouté à la liste d’exclusion pour l’éventuel tirage de la ligne suivante.
     * @return Liste des numéros tirés.
     */
    std::vector<uint8_t> generateLine();

private:
    std::vector<uint8_t> alreadyPicked;///< Liste des numéros déjà tirer.
};

}// namespace evl