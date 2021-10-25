/**
 * @file MainWindow.h
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "DisplayWindow.h"
#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <core/Event.h>

/**
 * @brief Namespace permettant à QT d’opérer la conversion de fichier .ui
 */
namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class MainWindow;
}// namespace Ui

namespace evl::gui {

/**
 * @brief Classe définissant les opérations de la page principale
 */
class MainWindow: public QMainWindow {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit MainWindow(QWidget* parent= nullptr);

    /**
     * @brief Destructeur.
     */
    ~MainWindow() override;

    /**
     * @brief Accès aux paramètres globaux.
     * @return Les paramètres.
     */
    [[nodiscard]] QSettings& getSettings() { return settings; }
public slots:

    /**
     * @brief Lit ou écrit le fichier de configuration de base.
     */
    void syncSettings();

private slots:

    /**
     * @brief Affichage de la page d’à propos.
     */
    void actShowAbout();

    /**
     * @brief Affiche la fenêtre d’aide.
     */
    void actShowHelp();

    /**
     * @brief Affiche la fenêtre des paramètres généraux.
     */
    void actShowGlobalParameters();

    /**
     * @brief Affiche la fenêtre de configuration des cartons.
     */
    void actShowCardPackParameters();

    /**
     * @brief Affiche la fenêtre de configuration des parties.
     */
    void actShowGameRoundsParameters();

    /**
     * @brief Affiche la fenêtre de configuration de l’événement.
     */
    void actShowEventParameters();

    /**
     * @brief Fonction Fichiers->Nouveau.
     */
    void actNewFile();

    /**
     * @brief Fonction Fichiers->Charger.
     */
    void actLoadFile();

    /**
     * @brief Fonction Fichiers->Sauver.
     */
    void actSaveFile();

    /**
     * @brief Fonction : fichiers sauver sous.
     */
    void actSaveAsFile();

    /**
     * @brief Fonction Fichiers->Commencer.
     */
    void actStartEvent();

    /**
     * @brief Fonction Fichiers->Terminer.
     */
    void actEndEvent();

    /**
     * @brief Fonction Fichiers->Quitter.
     */
    void actQuit();

    /**
     * @brief Fonction démarrage et fin d'une partie
     */
    void actStartStopRound();

    /**
     * @brief Fonction pause et reprise d'une partie
     */
    void actPauseResumeRound();

    /**
     * @brief Fonction Tirage aléatoire
     */
    void actRandomPick();

    /**
     * @brief Fonction annulation de tirage
     */
    void actCancelPick();

    /**
     * @brief action de changement du radio bouton de type de tirage.
     */
    void actRadioPureRandom();

    /**
     * @brief action de changement du radio bouton de type de tirage.
     */
    void actRadioPureManual();

    /**
     * @brief action de changement du radio bouton de type de tirage.
     */
    void actRadioBoth();

private:
    /**
     * @brief Met à jour l’affichage en fonction du contenu de l’événement courant
     */
    void updateDisplay();
    /**
     * @brief Mise à jour de l’affichage, spéciale in game.
     */
    void updateInGameDisplay();

    /**
     * @brief Affiche une boite de dialogue disant que c’est en travaux.
     * @param from Une chaine permettant de savoir d’où vient la demande.
     */
    void showNotImplemented(const QString& from);

    /// Lien vers la page UI.
    Ui::MainWindow* ui;

    /// Les settings généraux.
    QSettings settings;

    /// L’événement en cours de traitement
    core::Event currentEvent;

    /// Timer interne
    QTimer* timer;

    /// Fenêtre d’affichage sur le second écran.
    DisplayWindow* displayWindow= nullptr;
};

}// namespace evl::gui
