/**
* @file DisplayWindow.h
* @author Silmaen
* @date 23/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#pragma once
#include "../core/Event.h"
#include <QMainWindow>
#include <QTimer>
#include <spdlog/spdlog.h>

namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class DisplayWindow;
}// namespace Ui

namespace evl::gui {

class MainWindow;

/**
 * @brief Fenêtre d’affichage vue par les joueurs
 */
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
    void initializeDisplay() const;

    /**
     * @brief Redimensionnement de la grille
     */
    void resize();

    /**
     * @brief Modes de fonctionnement
     */
    enum struct Mode : uint8_t {
        Game,  /// in game
        Preview/// in preview
    };
    void setMode(const Mode& newMode) {
        mode= newMode;
    }
    void setRoundIndex(uint32_t rIndex, uint32_t srIndex) {
        spdlog::trace("setting round {} sub {}", rIndex, srIndex);
        roundIndex   = rIndex;
        subRoundIndex= srIndex;
    }

private:
    /**
     * @brief Mise à jour de l’affichage de la page de titre.
     */
    void updateEventTitlePage() const;

    /**
     * @brief Mise à jour de l’affichage de la page de titre du round.
     */
    void updateRoundTitlePage() const;

    /**
     * @brief Mise à jour de l’affichage de la page de partie.
     */
    void updateRoundRunning() const;

    /**
     * @brief Mise à jour de l’affichage de la page des règles de l’événement.
     */
    void updateDisplayRules() const;

    void updatePauseScreen() const;
    void updateRoundEndingPage();

    /**
     * @brief Initialisation des numéros dans la grille.
     */
    void initializeNumberGrid() const;

    /**
     * @brief Mise à jour les couleurs des fenêtres
     */
    void updateColors();

    /**
     * @brief Remise à zéro de l'affichage de la grille
     */
    void resetGrid() const;

    enum struct Page : uint8_t {
        MainTitle,
        EventStarting,
        PricesDisplay,
        NumberDisplay,
        PauseDisplay,
        RulesDisplay,
        EventEnding,
        RoundEnding
    };
    static const std::unordered_map<Page, int> PageIndex;

    void setPage(const Page& newPage);

    /// Lien vers la page UI.
    Ui::DisplayWindow* ui= nullptr;
    /// Lien vers le timer.
    QTimer* timer= nullptr;
    /// Lien vers l’événement en cours
    core::Event* event= nullptr;
    /// La taille actuelle de l’image
    QSize currentSize;
    /// Lien vers la MainWindow
    MainWindow* mwd= nullptr;

    Mode mode             = Mode::Game;
    uint32_t roundIndex   = 0;
    uint32_t subRoundIndex= 0;
};

}// namespace evl::gui
