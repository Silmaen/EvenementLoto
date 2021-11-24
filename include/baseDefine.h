/**
 * @file baseDefine.h
 * @author Silmaen
 * @date 17/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

/**
 * @brief namespace de base.
 */
namespace evl {

/// Nombre de lignes sur un carton
constexpr uint8_t nb_ligne= 3;
/// Nombre de colonnes sur un carton
constexpr uint8_t nb_colones= 5;

/// Version des sauvegardes
constexpr uint16_t currentSaveVersion= 3;

/// Définition d’un chemin vers un fichier ou répertoire
using path= std::filesystem::path;

/// Définition des chaines de caractères
using string= std::string;
/// Le type définissant la taille de la chaine
using sizeType= std::string::size_type;
/// La taille mémoire d’un caractère de chaine.
constexpr auto charSize= sizeof(string::value_type);

}// namespace evl
