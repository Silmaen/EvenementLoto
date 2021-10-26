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

class DisplayWindow: public QMainWindow {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param evt Pointeur vers l’événement.
     * @param parent Le widget Parent.
     */
    explicit DisplayWindow(core::Event* evt, QWidget* parent= nullptr);

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

private:
    /**
     * @brief Mise à jour de l’affichage de la page de titre.
     */
    void updateEventTitlePage();

    /// Lien vers la page UI.
    Ui::DisplayWindow* ui;
    /// Lien vers le timer.
    QTimer* timer;
    /// Lien vers l’événement en cours
    core::Event* event;
};

}// namespace evl::gui
