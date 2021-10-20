/**
 * @author Silmaen
 * @date 20/10/2021
 */
#pragma once
#include <QDialog>

namespace Ui {
/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class GeneralConfig;
}// namespace Ui

namespace evl::gui {

class MainWindow;

/**
 * @brief Classe D’affichage de la Fenêtre permettant de configurer les cartons.
 */
class GeneralConfig: public QDialog {
    Q_OBJECT
public:
    /**
     * @brief Constructeur.
     * @param parent Le widget Parent.
     */
    explicit GeneralConfig(MainWindow* parent= nullptr);
    /**
     * @brief Destructeur.
     */
    ~GeneralConfig() override;

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
     * @brief Réagit au clic sur la recherche d'un dossier.
     */
    void searchFolder();

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

    Ui::GeneralConfig* ui;   ///< Lien vers la page UI.
    MainWindow* mwd= nullptr;///< Lien vers la MainWindow
};

}// namespace evl::gui
