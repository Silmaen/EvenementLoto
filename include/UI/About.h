/**
 * @author Silmaen
 * @date 18/10/2021
 */
#pragma once

#include <QDialog>

namespace Ui {
/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class About;
}// namespace Ui

namespace evl::gui {

/**
 * @brief Classe D'affichage de la boite de dialogue d'à propos.
 */
class About: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit About(QWidget* parent= nullptr);
    /**
     * @brief Destructeur.
     */
    ~About() override;

private:
    Ui::About* ui;///< Lien vers la page UI.
};

}// namespace evl
