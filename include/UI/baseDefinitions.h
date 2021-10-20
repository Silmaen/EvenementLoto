/**
 * @author Silmaen,
 * @date 20/10/2021
 */

#pragma once
#include <filesystem>

/**
 * @brief Namespace des fonctions d’affichage.
 */
namespace evl::gui {

/// Variable qui va contenir le chemin vers ce programme
static std::filesystem::path baseExecPath;
/// Nom du fichier de configuration générale de base.
inline std::filesystem::path iniFile= "config.ini";

/**
 * @brief Construit le chemin vers le fichier de configuration initiale
 * @return Le chemin complet vers le fichier de configuration.
 */
inline std::filesystem::path getIniFile() { return baseExecPath / iniFile; }

// forward declarations
class MainWindow;
class ConfigurationParties;
class EvenementConfig;
class GeneralConfig;
class ConfigCartons;
class About;

}// namespace evl::gui
