/**
 * @file ConfigEvent.h
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
class ConfigEvent;

}// namespace Ui

namespace evl::gui {

class MainWindow;

/**
 * @brief Classe D’affichage de la Fenêtre permettant de configurer les cartons.
 */
class ConfigEvent: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit ConfigEvent(MainWindow* parent= nullptr);

    /**
     * @brief Destructeur.
     */
    ~ConfigEvent() override;

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
     * @brief Action de recherche de logo d’organisateur
     */
    void actSearchOrgaLogo();

    /**
     * @brief Action de recherche de logo d’événement.
     */
    void actSearchLogo();

    /**
     * @brief Action d’import des règles
     */
    void actImportRules();

    /**
     * @brief Action d’export des règles
     */
    void actExportRules();

private:
    /**
     * @brief Met à jour l’affichage
     */
    void updateDisplay();

    /**
     * @brief Charge une configuration depuis un fichier
     */
    void SaveFile();

    /// Lien vers la page UI.
    Ui::ConfigEvent* ui;

    /// L’événement en cours d’édition
    core::Event gameEvent;
};

}// namespace evl::gui
