/**
 * @file MainWindow.h
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "DisplayWindow.h"
#include "VisualTheme.h"
#include "WidgetNumberGrid.h"
#include "core/Event.h"
#include "core/RandomNumberGenerator.h"
#include <QMainWindow>
#include <QSettings>
#include <QTimer>

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

    /**
     * @brief Accès direct au theme visuel
     * @return Le theme visuel
     */
    VisualTheme& getTheme() { return theme; }
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
     * @brief Fonction démarrage et fin d’une partie
     */
    void actStartStopRound();

    /**
     * @brief Fonction Tirage aléatoire
     */
    void actRandomPick();

    /**
     * @brief Fonction annulation de tirage
     */
    void actCancelPick();

    /**
     * @brief Action de changement du radio bouton de type de tirage.
     */
    void actRadioPureRandom();

    /**
     * @brief Action de changement du radio bouton de type de tirage.
     */
    void actRadioPureManual();

    /**
     * @brief Action de changement du radio bouton de type de tirage.
     */
    void actRadioBoth();

    /**
     * @brief Action de passage en mode d’affichage des règles.
     */
    void actDisplayRules();

    /**
     * @brief Action de passage en de l'affichage en plein écran.
     */
    void actChangeFullScreen();

    /**
     * @brief Action de changement d'écran d'affichage.
     */
    void actChangeScreen();

    /**
     * @brief Action d'appui sur la grille
     * @param value Quelle valeur
     */
    void actGridPushed(int value);

private:
    /**
    * @brief Mise à jour de l’affichage des horloges
    */
    void updateClocks();

    /**
     * @brief Met à jour l’affichage en fonction du contenu de l’événement courant
     */
    void updateDisplay();

    /**
    * @brief Mise à jour de l’affichage de menus.
    */
    void updateMenus();

    /**
     * @brief Mise à jour du board du bas.
     */
    void updateBottomFrame();

    /**
     * @brief Mise à jour de l’affichage des radios boutons
     */
    void updateRadioButtons();

    /**
     * @brief Mise à jour de l’affichage du bouton de start/stop
     */
    void updateStartStopButton();

    /**
     * @brief Mise à jour de l’affichage des informations de l’événement
     */
    void updateInfoEvent();

    /**
     * @brief Mise à jour de l’affichage des informations de partie
     */
    void updateInfoRound();

    /**
     * @brief Mise à jour des numéros tirés
     */
    void updateDraws();

    /**
     * @brief Mise à jour des données de statistiques
     */
    void updateStats();

    /**
     * @brief Mise à jour des commandes
     */
    void updateCommands();

    /**
     * @brief Vérifie que la fenêtre d'affichage est visible et active le bouton si elle doit l'être
     */
    void checkDisplayWindow();

    /**
     * @brief Update the list of screens
     */
    void updateScreenList();

    /**
     * @brief Sauve dans un fichier
     * @param where Chemin du fichier
     */
    void saveFile(const path& where);

    /// Lien vers la page UI.
    Ui::MainWindow* ui= nullptr;

    /// Les settings généraux.
    QSettings settings;

    /// L’événement en cours de traitement
    core::Event currentEvent;

    /// Timer interne
    QTimer* timer= nullptr;

    /// Fenêtre d’affichage sur le second écran.
    DisplayWindow* displayWindow= nullptr;

    /// Le widget de boutons
    WidgetNumberGrid* numberGrid= nullptr;

    /// Le générateur de nombre aléatoire
    core::RandomNumberGenerator rng;

    /// Le theme visuel
    VisualTheme theme;

    /// Le fichier courant
    path currentFile= path{};

    /// Simple compteur pour ralentir la sauvegarde automatique
    int autoSaveCounter= 0;

    /// Compteur de ralentissement de l'actualisation du Log
    int logUpdateCounter= 0;

    /// Variable mise à vraie au début de la procédure de mise à jour de l'affichage pour éviter les récursions infinies
    bool onUpdate= false;

    /**
     * @brief Mode de tirage des numéros
     */
    enum struct DrawMode {
        Both,     ///< À la fois manuel et tirage aléatoire par la machine
        PickOnly, ///< Seulement le tirage machine
        ManualOnly///< Seulement le tirage manuel (machine à boule)
    } currentDrawMode= DrawMode::Both;
};

}// namespace evl::gui
