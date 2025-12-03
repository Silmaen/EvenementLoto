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
constexpr uint16_t g_currentSaveVersion = 6;

/// Variable qui va contenir le chemin vers ce programme
static std::filesystem::path g_baseExecPath;

/// Indique si on utilise ImGui ou Qt pour l'interface.
static bool g_useImGui = false;

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
static const std::string EVL_VERSION = std::format("{}.{}.{}", EVL_MAJOR, EVL_MINOR, EVL_PATCH);
static const std::string EVL_AUTHOR_STR = std::format("{}", EVL_AUTHOR);
static const std::string EVL_APP = "Événement Loto";

}// namespace evl
