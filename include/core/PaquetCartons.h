/**
* @author Silmaen
* @date 19/10/202
*/
#pragma once
#include "Serializable.h"
#include "carton.h"
#include <vector>

namespace evl::core {

/**
 * @brief Classe pour gérer l’ensemble des cartons disponibles.
 */
class PaquetCartons: public Serializable {
public:
    /**
     * @brief Constructeur par défaut.
     */
    PaquetCartons();
    /**
     * @brief Écriture dans un stream.
     * @param bs Le stream où écrire.
     */
    void write(std::ostream& bs) const override;
    /**
     * @brief Lecture depuis un stream
     * @param bs Le stream d’entrée.
     */
    void read(std::istream& bs) override;

    /**
     * @brief Remet la classe à zéro.
     */
    void fullReset() {
        nom= "";
        cartons.clear();
    }
    /**
     * @brief Accès à la liste des cartons.
     * @return La liste des cartons.
     */
    const std::vector<Carton>& getCartons() const { return cartons; }
    /**
     * @brief Accès à la liste des cartons.
     * @return La liste des cartons.
     */
    std::vector<Carton>& getCartons() { return cartons; }
    /**
     * @brief Accès au nom de la liste.
     * @return Le nom de la liste.
     */
    const std::string& getNom() const { return nom; }
    /**
     * @brief Accès au nom de la liste.
     * @return Le nom de la liste.
     */
    std::string& getNom() { return nom; }
    /**
     * @brief Renvoie le nombre de cartons de la liste
     * @return Le nombre de cartons de la liste.
     */
    size_t getNumber() { return cartons.size(); }

    //---------------------------------------------------------
    /**
     * @brief Génère un certain nombre de cartons aléatoirement
     * @param number Le nombre de cartons à générer.
     * @param rng Lien éventuel vers un générateur de nombre aléatoire externe.
     */
    void generate(const uint32_t& number, RandomNumberGenerator* rng= nullptr);
    /**
     * @brief Génère une liste de cartons ayant le statut demandé
     * @param st Le status des cartons.
     * @return Liste des cartons avec le statut demandé.
     */
    std::vector<Carton> getCartonByStatus(const Carton::ResultCarton& st) const;

private:
    std::string nom;            ///< Le nom de ce paquet de cartons
    std::vector<Carton> cartons;///< La liste des cartons du paquet.
};

}// namespace evl::core
