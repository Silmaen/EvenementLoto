/**
 * @file ConfigGameRounds.h
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "../core/Event.h"
#include <QDialog>

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

    /**
     * @brief Accès extérieur aux éléments d’interface.
     * @return Pointeur vers la classe interne d’interface.
     */
    const Ui::ConfigGameRounds* getUi() const { return ui; }
public slots:

    /**
     * @brief Surcharge de la commande d’exécution.
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
     * @brief Action lors du click sur le bouton d’import de partie.
     */
    void actImportRounds();

    /**
     * @brief Action lors du click sur le bouton d’export de partie.
     */
    void actExportRounds();

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
     * @brief Changement de type de partie
     * @param newIndex Index du nouveau type de partie
     */
    void actChangeGameRoundType(int newIndex);

    /**
     * @brief Changement du numéro de la partie
     * @param index Nouveau numéro de partie
     */
    void actChangeRoundNumber(int index);

    /**
     * @brief Changement du type de sous-partie
     * @param newIndex Nouvel index de type de sous-partie
     */
    void actChangeSubGameRoundType(int newIndex);

    /**
     * @brief Action à la fin de l’édition des lots.
     */
    void actEndEditingPrice();

    /**
     * @brief Création d’une nouvelle sous-partie
     */
    void actCreateSubGameRound();

    /**
     * @brief Suppression d’une sous-partie
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
     * @brief Action lors du changement de partie
     */
    void actChangeSelectedGameRound();

    /**
     * @brief Action lors du changement de sous-partie
     */
    void actChangeSelectedSubGameRound();

    /**
     * @brief Action lors du changement de valeur de lot
     */
    void actChangePriceValue();

private:
    /**
     * @brief Met à jour l’affichage
     */
    void updateDisplay();

    /**
     * @brief Met à jour l’affichage de la partie d’édition
     */
    void updateDisplayEdits();
    /**
     * @brief Met à jour l’affichage de la partie d’édition
     */
    void updateDisplayRoundList();

    /**
     * @brief Mets à jour l’affichage de la partie d’affichage de résultat
     */
    void updateDisplayResults();

    /**
     * @brief Renvoie la liste des types de partie
     * @return La liste des types de partie
     */
    static QStringList getRoundTypes();

    /**
     * @brief Renvoie la liste des conditions de victoire en fonction du type de partie
     * @param t Le type de partie.
     * @return La liste des conditions de victoires.
     */
    static QStringList getVictoryType(const core::GameRound::Type& t);

    /**
     * @brief Renvoie l’index dans le menu déroulant en fonction de son remplissage
     * @param t Le type de partie
     * @param st Le type de phase
     * @return L'index/
     */
    static int getVictoryIndex(const core::GameRound::Type& t, const core::SubGameRound::Type& st);

    ///Lien vers la page UI.
    Ui::ConfigGameRounds* ui;

    /// L’événement en cours d’édition
    core::Event gameEvent;

    /// Variable mise à vrai lors de la mise à jour de l’affichage pour éviter les récursions infinies.
    bool onUpdate= false;
};

}// namespace evl::gui
