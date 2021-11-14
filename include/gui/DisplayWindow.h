/**
* @file DisplayWindow.h
* @author Silmaen
* @date 23/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#pragma once
#include <QMainWindow>
#include <QTimer>
#include <core/Event.h>

namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class DisplayWindow;
}// namespace Ui

namespace evl::gui {

class MainWindow;

class DisplayWindow: public QMainWindow {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param evt Pointeur vers l’événement.
     * @param parent Le widget Parent.
     */
    explicit DisplayWindow(core::Event* evt, MainWindow* parent= nullptr);

    /**
     * @brief Destructeur.
     */
    ~DisplayWindow() override;

    /**
     * @brief Accès direct aux éléments de l’UI
     * @return Pointeur vers l’UI
     */
    const Ui::DisplayWindow* getUi() const { return ui; }

    /**
     * @brief Mise à jour de l’affichage.
     */
    void updateDisplay();

    /**
     * @brief Initialise l’affichage de tous les éléments non dynamiques.
     */
    void initializeDisplay();

    /**
     * @brief Redimensionnement de la grille
     */
    void resize();

private:
    /**
     * @brief Mise à jour de l’affichage de la page de titre.
     */
    void updateEventTitlePage();
    /**
     * @brief Mise à jour de l’affichage de la page de titre du round.
     */
    void updateRoundTitlePage();
    /**
     * @brief Mise à jour de l’affichage de la page de partie.
     */
    void updateRoundRunning();
    /**
     * @brief Mise à jour de l’affichage de la page des règles de l’événement.
     */
    void updateDisplayRules();

    /**
     * @brief Initialisation des numéros dans la grille.
     */
    void initializeNumberGrid();

    /**
     * @brief remise à zéro de l'affichage de la grille
     */
    void resetGrid();

    /// Lien vers la page UI.
    Ui::DisplayWindow* ui;
    /// Lien vers le timer.
    QTimer* timer;
    /// Lien vers l’événement en cours
    core::Event* event;
    /// La taille actuelle de l’image
    QSize currentSize;
    /// sauvegarde du statut
    core::Event::Status currentStatus;
    /// Lien vers la MainWindow
    MainWindow* mwd= nullptr;
};

}// namespace evl::gui
