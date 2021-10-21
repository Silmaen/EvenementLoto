/**
 * @author Silmaen
 * @date 20/10/2021
 */
#pragma once

#include <QDialog>
#include <core/Evenement.h>

namespace Ui {
/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class ConfigurationParties;
}// namespace Ui

namespace evl::gui {

/**
 * @brief Classe D’affichage de la Fenêtre permettant de configurer les cartons.
 */
class ConfigurationParties: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit ConfigurationParties(QWidget* parent= nullptr);
    /**
     * @brief Destructeur.
     */
    ~ConfigurationParties() override;

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
     * @brief Action lors du click sur le bouton de création de partie.
     */
    void partieCreate();
    /**
     * @brief Action lors du clic sur le bouton de chargement d'une partie
     */
    void partieLoad();
    /**
     * @brief Action lors clic sur le bouton Avant.
     */
    void partieOrderAfter();
    /**
     * @brief Action lors clic sur le bouton Après.
     */
    void partieOrderBefore();
    /**
     * @brief Action lors clic sur le bouton Sauver.
     */
    void partieSave();
    /**
     * @brief Action lors d'un changement du type de partie.
     */
    void partieTypeChanged(QString);

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

    ///Lien vers la page UI.
    Ui::ConfigurationParties* ui;
    /// L’événement en cours d’édition
    core::Evenement evenement;
    /// La partie courante.
    core::Partie currentPartie;
};

}// namespace evl::gui
