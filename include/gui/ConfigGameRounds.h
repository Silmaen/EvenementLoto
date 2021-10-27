/**
 * @file ConfigGameRounds.h
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include <QDialog>
#include <core/Event.h>

namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class ConfigGameRounds;

}// namespace Ui

namespace evl::gui {

/**
 * @brief Classe D’affichage de la Fenêtre permettant de configurer les cartons.
 */
class ConfigGameRounds: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit ConfigGameRounds(QWidget* parent= nullptr);

    /**
     * @brief Destructeur.
     */
    ~ConfigGameRounds() override;

    /**
     * @brief Accès à l’événement de la boite de dialogue
     * @return L’événement.
     */
    const core::Event& getEvent() const { return gameEvent; }

    /**
     * @brief Défini l’événement à éditer.
     * @param e L’événement.
     */
    void setEvent(const core::Event& e);

    const Ui::ConfigGameRounds* getUi() const { return ui; }
public slots:

    /**
     * @brief Surcharge de la commande d’exécution.
     * @return Code de retour.
     */
    void preExec();

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
     * @brief Action lors du click sur le bouton de création de partie.
     */
    void actCreateGameRound();

    /**
     * @brief Action lors du click sur le bouton de suppression de partie.
     */
    void actDeleteGameRound();

    /**
     * @brief Action lors clic sur le bouton Avant.
     */
    void actMoveGameRoundAfter();

    /**
     * @brief Action lors clic sur le bouton Après.
     */
    void actMoveGameRoundBefore();

    /**
     * @brief
     * @param newIndex
     */
    void actChangeGameRoundType(int newIndex);

    /**
     * @brief
     * @param newIndex
     */
    void actChangeSubGameRoundType(int newIndex);

    /**
     * @brief
     */
    void actEndEditingPrice();

    /**
     * @brief
     */
    void actCreateSubGameRound();

    /**
     * @brief
     */
    void actDeleteSubGameRound();
    /**
     * @brief Action lors clic sur le bouton Avant.
     */
    void actMoveSubGameRoundAfter();

    /**
     * @brief Action lors clic sur le bouton Après.
     */
    void actMoveSubGameRoundBefore();

    /**
     * @brief
     * @param newIndex
     */
    void actChangeSelectedGameRound(int newIndex);

    /**
     * @brief
     * @param newIndex
     */
    void actChangeSelectedSubGameRound(int newIndex);

private:
    /**
     * @brief Met à jour l’affichage
     */
    void updateDisplay();

    void updateDisplayEdits();

    void updateDisplayResults();

    ///Lien vers la page UI.
    Ui::ConfigGameRounds* ui;

    /// L’événement en cours d’édition
    core::Event gameEvent;
};

}// namespace evl::gui
