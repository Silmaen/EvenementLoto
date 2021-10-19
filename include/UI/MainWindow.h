/**
* @author Silmaen
* @date 18/10/2021
*/
#pragma once

#include <QMainWindow>

/**
 * @brief Namespace permettant à QT d'opérer la conversion de fichier .ui
 */
namespace Ui {
/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class MainWindow;
}// namespace Ui

namespace evl {

/**
 * @brief Classe définissant les opérations de la page principale
 */
class MainWindow: public QMainWindow {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit MainWindow(QWidget* parent= nullptr);
    /**
     * @brief Destructeur.
     */
    ~MainWindow() override;
private slots:
    /**
     * @brief Affichage de la page d'à propos.
     */
    void showAbout();

private:
    Ui::MainWindow* ui;///< Lien vers la page UI.
};

}// namespace elv
