/**
* @file DisplayWindow.h
* @author Silmaen
* @date 23/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#pragma once
#include <QDialog>
#include <QTimer>
#include <core/Event.h>

namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class DisplayWindow;
}// namespace Ui

namespace evl::gui {

class DisplayWindow: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit DisplayWindow(core::Event* evt, QWidget* parent= nullptr);

    /**
     * @brief Destructeur.
     */
    ~DisplayWindow() override;
public slots:
private slots:
    /**
     * @brief Mise à jour de l'affichage.
     */
    void updateDisplay();

private:
    /// Lien vers la page UI.
    Ui::DisplayWindow* ui;
    /// Lien vers le timer.
    QTimer* timer;
    /// Lien vers l’événement en cours
    core::Event* event;
};

}// namespace evl::gui
