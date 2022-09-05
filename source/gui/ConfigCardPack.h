/**
 * @file ConfigCardPack.h
 * @author Silmaen
 * @date 19/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "../core/CardPack.h"
#include <QAbstractButton>
#include <QCheckBox>
#include <QDialog>
#include <QSpinBox>
#include <array>

namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class ConfigCardPack;

}// namespace Ui

namespace evl::gui {

/**
 * @brief Classe D’affichage de la Fenêtre permettant de configurer les cartons.
 */
class ConfigCardPack: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit ConfigCardPack(QWidget* parent= nullptr);

    /**
     * @brief Destructeur.
     */
    ~ConfigCardPack() override;

    /**
     * @brief Accès aux données internes.
     * @return Lien vers les données internes
     */
    core::CardPack& getCardPack() { return cardPack; }

    /**
     * @brief Accès aux données internes.
     * @return Lien vers les données internes
     */
    const core::CardPack& getCardPack() const { return cardPack; }

public slots:

    /**
     * @brief Surcharge de la commande d’exécution.
     */
    void preExec();

    /**
     * @brief Charge une configuration depuis un fichier
     */
    void actLoadFile();

    /**
     * @brief Charge une configuration depuis un fichier
     */
    void actSaveFile();

    /**
     * @brief Réagit à une action sur le bouton Apply.
     */
    void actApply();

    /**
     * @brief Réagit à une action sur le bouton cancel.
     */
    void actCancel();

    /**
     * @brief Réagit à une action sur le bouton ok.
     */
    void actOk();

    /**
     * @brief Sauvegarde le carton en cours dans la liste.
     */
    void actSaveInList();

    /**
     * @brief Charge depuis la liste le numéro du carton.
     */
    void actLoadFromList();

    /**
     * @brief Traduit la ligne brute du carton.
     */
    void actLoadLine();

    /**
     * @brief Génère aléatoirement le nombre de cartons demandé.
     */
    void actGenerate();

    /**
     * @brief Mise à jour de l’affichage d’un carton.
     */
    void actUpdateGrid();

    /**
     * @brief Met à jour l’affichage.
     */
    void actUpdateDisplay();

    /**
     * @brief Met à jour le nom dans les données.
     */
    void actUpdateName();

    /**
     * @brief Récupère le nom du fichier depuis l’UI.
     */
    void actUpdateFileName();

private:

    /// Lien vers la page UI.
    Ui::ConfigCardPack* ui;

    /// Les données internes.
    core::CardPack cardPack;

    /// Chemin vers le fichier
    QString path;

    /**
     * @brief Structure interne de cellule de grille.
     */
    struct cell {

        /// Lien vers le spinbox.
        QSpinBox* spin= nullptr;

        /// Lien vers la checkbox.
        QCheckBox* check= nullptr;

        /**
         * @brief Test si la cellule est valide
         * @return true si la cellule est valide.
         */
        bool isValid() const {
            return check != nullptr && spin != nullptr;
        }

        /**
         * @brief Change l’activation de la cellule.
         */
        void toggle() {
            if(!isValid()) return;
            spin->setEnabled(check->isChecked());
            if(!check->isChecked()) {
                spin->setValue(0);
            }
        }
    };

    /// Les cellules en lignes.
    std::array<std::array<cell, 9>, 3> lines;
};

}// namespace evl::gui
