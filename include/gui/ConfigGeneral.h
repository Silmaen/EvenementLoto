/**
 * @file ConfigGeneral.h
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once
#include <QDialog>

namespace Ui {

/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class ConfigGeneral;

}// namespace Ui

namespace evl::gui {

class MainWindow;

/**
 * @brief Classe D’affichage de la Fenêtre permettant de configurer les cartons.
 */
class ConfigGeneral: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit ConfigGeneral(MainWindow* parent= nullptr);

    /**
     * @brief Destructeur.
     */
    ~ConfigGeneral() override;

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
     * @brief Réagit au clic sur la recherche d’un dossier.
     */
    void actSearchFolder();

private:
    /**
     * @brief Charge une configuration depuis un fichier
     */
    void SaveFile();

    /**
     * @brief Affiche une boite de dialogue disant que c’est en travaux.
     * @param from Une chaine permettant de savoir d’où vient la demande.
     */
    void showNotImplemented(const QString& from);

    /// Lien vers la page UI.
    Ui::ConfigGeneral* ui;

    /// Lien vers la MainWindow
    MainWindow* mwd= nullptr;
};

}// namespace evl::gui