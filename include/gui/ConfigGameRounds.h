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
public slots:

    /**
     * @brief Surcharge de la commande d’exécution.
     * @return Code de retour.
     */
    int exec() override;

private slots:

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
     * @brief Action lors du clic sur le bouton de chargement d’une partie
     */
    void actLoadGameRound();

    /**
     * @brief Action lors clic sur le bouton Avant.
     */
    void actMoveGameRoundAfter();

    /**
     * @brief Action lors clic sur le bouton Après.
     */
    void actMoveGameRoundBefore();

    /**
     * @brief Action lors clic sur le bouton Sauver.
     */
    void actSaveGameRound();

private:
    /**
     * @brief Met à jour l’affichage
     * @param loadLast Si c’est le dernier élément qui est à charger (utile lors de la création d’un nouvel élément)
     */
    void updateDisplay(bool loadLast= false);

    /**
     * @brief Charge une configuration depuis un fichier
     */
    int SaveFile();

    /**
     * @brief Affiche une boite de dialogue disant que c’est en travaux.
     * @param from Une chaine permettant de savoir d’où vient la demande.
     */
    void showNotImplemented(const QString& from);

    /**
     * @brief Renvoie la liste des types de partie.
     * @return La liste des types de partie.
     */
    static QStringList getGameRoundTypes();

    /**
     * @brief Renvoie l’id dans le composant depuis un type de partie
     * @param p Le type de partie
     * @return L’id dans le composant.
     */
    static int getIdByGameRoundType(const core::GameRound::Type& p);

    /**
     * @brief Retourne un type de partie basé sur la selection courante
     * @return Le type de partie.
     */
    core::GameRound::Type getCurrentGameRoundType();

    /**
     * @brief Renvoie la partie en cours d’édition.
     * @return La partie en cours d’édition.
     */
    core::GameRound& getCurrentGameRound();

    /**
    * @brief Renvoie la partie en cours d’édition.
    * @return La partie en cours d’édition.
    */
    int getCurrentGameRoundIndex();

    ///Lien vers la page UI.
    Ui::ConfigGameRounds* ui;

    /// L’événement en cours d’édition
    core::Event gameEvent;
};

}// namespace evl::gui
