/**
 * @author Silmaen
 * @date 19/10/2021
 */
#pragma once

#include <QAbstractButton>
#include <QDialog>

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
private slots:
    /**
     * @brief Charge une configuration depuis un fichier
     */
    void LoadFile();
    /**
     * @brief Réagit à une action sur les boutons du bas
     * @param b Le bouton du bas qui est appuyé.
     */
    void BottomButton(QAbstractButton* b);
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

private:
    Ui::ConfigCartons* ui;///< Lien vers la page UI.
};

}// namespace evl::gui
