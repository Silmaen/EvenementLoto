/**
 * @file baseDefinitions.h
 * @author Silmaen,
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <filesystem>

/**
 * @brief Namespace des fonctions d’affichage.
 */
namespace evl::gui {

/// Variable qui va contenir le chemin vers ce programme
static std::filesystem::path baseExecPath;
/// Nom du fichier de configuration générale de base.
inline std::filesystem::path iniFile= "config.ini";

/// Clé vers le dataPath.
inline QString dataPathKey= "path/data_path";

/// Listes des formats d’image supportés
inline QStringList imageFilter= {
        "Toutes les images supportées (*.png *.jpg *.jpeg *.bmp)",
        "Fichiers png (*.png)",
        "Fichiers jpg (*.jpg, *.jpeg)",
        "Fichiers bmp (*.bmp)",
};

/**
 * @brief Construit le chemin vers le fichier de configuration initiale
 * @return Le chemin complet vers le fichier de configuration.
 */
inline std::filesystem::path getIniFile() { return baseExecPath / iniFile; }

// forward declarations
class MainWindow;
class ConfigGameRounds;
class ConfigEvent;
class ConfigGeneral;
class ConfigCardPack;
class About;

/**
 * @brief Affiche une boite de dialogue disant que c’est en travaux.
 * @param from Une chaine permettant de savoir d’où vient la demande.
 */
inline void showNotImplemented(const QString& from){
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

}// namespace evl::gui
