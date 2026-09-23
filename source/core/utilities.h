/**
 * @file utilities.h
 * @author Silmaen
 * @date 03/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Settings.h"
#include <filesystem>
#include <memory>

namespace evl::core {

/**
 * @brief Initialize the utilities.
 * @param iArgc Argument count.
 * @param iArgv Argument values.
 */
void initializeUtilities(int iArgc, char* iArgv[]);

/**
 * @brief Get the executable path.
 * @return The executable path.
 */
auto getExecPath() -> std::filesystem::path;

/**
 * @brief Get the ini file path.
 * @return The ini file path.
 */
auto getConfigFile() -> std::filesystem::path;

/**
 * @brief Get the configured data location, never empty.
 *
 * The settings are backed by YAML and every non-numeric value comes back as a
 * `std::string`: reading this one as a `std::filesystem::path` throws `bad_any_cast`
 * and silently yields an empty path — which is how the rescue save stopped happening
 * at all. One accessor, so the type cannot drift apart again.
 *
 * @return The configured location, or the default beside the executable.
 */
auto getDataLocation() -> std::filesystem::path;

/**
 * @brief Load settings from file into the Settings singleton.
 */
void loadSettings();

/**
 * @brief Defines all unset settings with default values.
 */
void mergeDefaultSettings();

/**
 * @brief Save settings to file from the Settings singleton.
 */
void saveSettings();

/**
 * @brief Get the Settings singleton.
 * @return The Settings singleton.
 */
auto getSettings() -> std::shared_ptr<Settings>;

/**
 * @brief Save settings to file from the Settings singleton then destroy it.
 */
void leaveSettings();


/**
 * @brief Get the save version.
 */
auto getSaveVersion() -> uint16_t;


}// namespace evl::core
