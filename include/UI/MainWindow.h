/**
* @author Silmaen
* @date 18/10/2021
*/
#pragma once

#include <QMainWindow>
#include <QSettings>
#include <core/Evenement.h>

/**
 * @brief Namespace permettant à QT d'opérer la conversion de fichier .ui
 */
namespace Ui {
/**
 * @brief La classe encapsulant le contenu du fichier UI
 */
class MainWindow;
}// namespace Ui

namespace evl::gui {

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

    /**
     * @brief Accès aux paramètres globaux.
     * @return les paramètres
     */
    [[nodiscard]] QSettings& getSettings() { return settings; }
public slots:
    /**
     * @brief Lit ou écrit le fichier de configuration de base.
     */
    void syncSettings();

private slots:
    /**
     * @brief Affichage de la page d’à propos.
     */
    void showAbout();
    /**
     * @brief Affiche la fenêtre d’aide.
     */
    void showHelp();
    /**
     * @brief Affiche la fenêtre des paramètres généraux.
     */
    void showParametresGeneraux();
    /**
     * @brief Affiche la fenêtre de configuration des cartons.
     */
    void showParametresCartons();
    /**
     * @brief Affiche la fenêtre de configuration des parties.
     */
    void showParametresParties();
    /**
     * @brief Affiche la fenêtre de configuration de l'événement.
     */
    void showParametresEvenement();
    /**
     * @brief Fonction Fichiers->Nouveau.
     */
    void fichierNew();
    /**
     * @brief Fonction Fichiers->Nouveau.
     */
    void fichierLoad();
    /**
     * @brief Fonction Fichiers->Sauver.
     */
    void fichierSave();
    /**
     * @brief Fonction Fichiers->Sauver sous.
     */
    void fichierSaveAs();
    /**
     * @brief Fonction Fichiers->Commencer.
     */
    void fichierCommencer();
    /**
     * @brief Fonction Fichiers->Terminer.
     */
    void fichierTerminer();
    /**
     * @brief Fonction Fichiers->Quitter.
     */
    void fichierQuitter();

private:
    /**
     * @brief Met à jour l’affichage en fonction du contenu de l’événement courant
     */
    void updateDisplay();

    /**
     * @brief Affiche une boite de dialogue disant que c’est en travaux.
     * @param from Une chaine permettant de savoir d’où vient la demande.
     */
    void showNotImplemented(const QString& from);

    /// Lien vers la page UI.
    Ui::MainWindow* ui;
    /// Les settings généraux.
    QSettings settings;
    /// L'événement en cours de traitement
    core::Evenement currentEvenement;
};

}// namespace elv
