/**
 * @author Silmaen
 * @date 19/10/2021
 */
#pragma once

#include "core/PaquetCartons.h"
#include <QAbstractButton>
#include <QCheckBox>
#include <QDialog>
#include <QSpinBox>
#include <array>

namespace Ui {
/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class ConfigCartons;
}// namespace Ui

namespace evl::gui {

/**
 * @brief Classe D’affichage de la Fenêtre permettant de configurer les cartons.
 */
class ConfigCartons: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit ConfigCartons(QWidget* parent= nullptr);
    /**
     * @brief Destructeur.
     */
    ~ConfigCartons() override;

    /**
     * @brief Accès aux données internes.
     * @return Lien vers les données internes
     */
    core::PaquetCartons& getCartons() { return cartons; }

    /**
     * @brief Accès aux données internes.
     * @return Lien vers les données internes
     */
    const core::PaquetCartons& getCartons() const { return cartons; }
public slots:
    /**
     * @brief Surcharge de la commande d’exécution.
     * @return Code de retour.
     */
    int exec() override;
private slots:
    /**
     * @brief Charge une configuration depuis un fichier
     */
    void LoadFile();
    /**
     * @brief Charge une configuration depuis un fichier
     */
    void SaveFile();
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
     * @brief Sauvegarde le carton en cours dans la liste.
     */
    void SaveInList();
    /**
     * @brief Charge depuis la liste le numéro du carton.
     */
    void LoadFromList();
    /**
     * @brief Traduit la ligne brute du carton.
     */
    void LoadLine();
    /**
     * @brief Génère aléatoirement le nombre de cartons demandé.
     */
    void Generate();
    /**
     * @brief Mise à jour de l’affichage d’un carton.
     */
    void UpdateGrid();

    /**
     * @brief Met à jour l’affichage.
     */
    void UpdateDisplay();

    /**
     * @brief Met à jour le nom dans les données.
     */
    void updateName();
    /**
     * @brief Recupère le nom du fichier depuis l’UI.
     */
    void updateFileName();

private:
    /**
     * @brief Affiche une boite de dialogue disant que c’est en travaux.
     * @param from Une chaine permettant de savoir d’où vient la demande.
     */
    void showNotImplemented(const QString& from);

    Ui::ConfigCartons* ui;      ///< Lien vers la page UI.
    core::PaquetCartons cartons;///< Les données internes.
    QString path;               ///< Chemin vers le fichier

    struct cell {
        QSpinBox* spin  = nullptr;
        QCheckBox* check= nullptr;
        bool isValid() const {
            return check != nullptr && spin != nullptr;
        }
        void toggle() {
            if(!isValid()) return;
            spin->setEnabled(check->isChecked());
            if(!check->isChecked()) {
                spin->setValue(0);
            }
        }
    };
    std::array<std::array<cell, 9>, 3> lines;
};

}// namespace evl::gui
