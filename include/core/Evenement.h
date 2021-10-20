/**
* @author Silmaen
* @date 20/10/202
*/
#pragma once
#include "PaquetCartons.h"
#include "Partie.h"
#include "Serializable.h"

namespace evl::core {

/**
 * @brief Classe décrivant un énénement.
 */
class Evenement: public Serializable {
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

private:
    std::string Nom;            ///< Nom de l’événement.
    std::string Lieu;           ///< Lieu de l’événement.
    std::vector<Partie> parties;///< Liste des parties de l’événement.
    //PaquetCartons* cartons;     ///< Lien vers la liste des cartons
};

}// namespace evl::core
