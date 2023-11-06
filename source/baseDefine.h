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

/// Version des sauvegardes.
constexpr uint16_t currentSaveVersion= 6;

/// Définition d’un chemin vers un fichier ou répertoire
using path= std::filesystem::path;

/// Définition des chaines de caractères
using string= std::string;
/// Le type définissant la taille de la chaine
using sizeType= std::string::size_type;
/// La taille mémoire d’un caractère de chaine.
constexpr auto charSize= sizeof(string::value_type);

/// Variable qui va contenir le chemin vers ce programme
static path baseExecPath;

#ifndef EVL_AUTHOR
#define EVL_AUTHOR "Damien Lachouette"
#endif
#ifndef EVL_MAJOR
#define EVL_MAJOR "0"
#endif
#ifndef EVL_MINOR
#define EVL_MINOR "xx"
#endif
#ifndef EVL_PATCH
#define EVL_PATCH "dev"
#endif
static const string EVL_VERSION   = string(EVL_MAJOR) + string(".") + string(EVL_MINOR) + string(".") + string(EVL_PATCH);
static const string EVL_AUTHOR_STR= string(EVL_AUTHOR);
static const string EVL_APP       = "Événement Loto";

}// namespace evl
