/**
 * @author Silmaen
 * @date 20/10/2021
 */
#pragma once

#include "core/Evenement.h"
#include <QDialog>

namespace Ui {
/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class EvenementConfig;
}// namespace Ui

namespace evl::gui {

class MainWindow;
/**
 * @brief Classe D’affichage de la Fenêtre permettant de configurer les cartons.
 */
class EvenementConfig: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit EvenementConfig(MainWindow* parent= nullptr);
    /**
     * @brief Destructeur.
     */
    ~EvenementConfig() override;

    /**
     * @brief Accès à l’événement de la boite de dialogue
     * @return L’événement.
     */
    const core::Evenement& getEvenement() const { return evenement; }
    /**
     * @brief Défini l’événement à éditer.
     * @param e L’événement.
     */
    void setEvenement(const core::Evenement& e);
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
     * @brief Action de recherche de logo d’organisateur
     */
    void actSearchOrgaLogo();

    /**
     * @brief Action de recherche de logo d'événement.
     */
    void actSearchLogo();

private:
    /**
     * @brief Met à jour l’affichage
     */
    void updateDisplay();
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
    Ui::EvenementConfig* ui;
    /// L’évenement en cours d'édition
    core::Evenement evenement;
    /// Lien vers la MainWindow
    MainWindow* mwd= nullptr;
};

}// namespace evl::gui
