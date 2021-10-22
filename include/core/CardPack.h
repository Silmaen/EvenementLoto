/**
 * @file CardPack.h
 * @author Silmaen
 * @date 19/10/202
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include "GridCard.h"
#include "Serializable.h"
#include <vector>

namespace evl::core {

/**
 * @brief Classe pour gérer l’ensemble des cartons disponibles.
 */
class CardPack: public Serializable {
public:
    /**
     * @brief Constructeur par défaut.
     */
    CardPack();

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
    const std::vector<GridCard>& getGridCards() const { return cartons; }

    /**
     * @brief Accès à la liste des cartons.
     * @return La liste des cartons.
     */
    std::vector<GridCard>& getGridCards() { return cartons; }

    /**
     * @brief Accès au nom de la liste.
     * @return Le nom de la liste.
     */
    const std::string& getName() const { return nom; }

    /**
     * @brief Accès au nom de la liste.
     * @return Le nom de la liste.
     */
    std::string& getName() { return nom; }

    /**
     * @brief Renvoie le nombre de cartons de la liste
     * @return Le nombre de cartons de la liste.
     */
    size_t size() { return cartons.size(); }

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
    std::vector<GridCard> getGridCardsByStatus(const GridCard::Status& st) const;

private:
    /// Le nom de ce paquet de cartons
    std::string nom;

    /// La liste des cartons du paquet.
    std::vector<GridCard> cartons;
};

}// namespace evl::core
