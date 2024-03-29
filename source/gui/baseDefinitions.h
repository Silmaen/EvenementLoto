/**
 * @file baseDefinitions.h
 * @author Silmaen,
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "../baseDefine.h"
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QTime>

/**
 * @brief Namespace des fonctions d’affichage.
 */
namespace evl::gui {

/// Nom du fichier de configuration générale de base.
inline path iniFile= "config.ini";

/**
 * @brief Construit le chemin vers le fichier de configuration initiale
 * @return Le chemin complet vers le fichier de configuration.
 */
inline path getIniFile() { return baseExecPath / iniFile; }

// forward declarations
class MainWindow;
class ConfigGameRounds;
class ConfigEvent;
class ConfigGeneral;
class About;

/**
 * @brief Affiche une boite de dialogue disant que c’est en travaux.
 * @param from Une chaine permettant de savoir d’où vient la demande.
 */
void showNotImplemented(const QString& from);

QImage loadImage(const path& from);

/// Namespace regroupant les clées des paramètres.
namespace settings {

/// Clé vers le dataPath.
inline QString dataPathKey= "path/data_path";
/// Valeur par défaut du dataPath.
inline QString dataPathDefault= "";

}// namespace settings

}// namespace evl::gui
