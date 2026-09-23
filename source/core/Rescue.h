/**
 * @file Rescue.h
 * @author Silmaen
 * @date 21/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Event.h"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

namespace evl::core {

/// Name of the file holding an interrupted game.
constexpr std::string_view g_rescueFileName = "rescue.lev";

/**
 * @brief Summary of an interrupted game found on disk.
 */
struct RescueInfo {
	/// The file holding it.
	std::filesystem::path path;
	/// Seconds elapsed since it was written.
	double ageSeconds = 0.0;
	/// Name of the event.
	std::string eventName;
	/// Numbers already drawn.
	std::size_t drawCount = 0;
};

/**
 * @brief Get the directory holding the rescue files.
 * @return The configured data location, empty when unset.
 */
[[nodiscard]] auto rescueDirectory() -> std::filesystem::path;

/**
 * @brief Atomically write an event to the rescue file, keeping one past generation.
 * @param[in] iEvent The event to save.
 * @return True when the rescue file holds the event.
 */
[[nodiscard]] auto saveRescue(const Event& iEvent) -> bool;

/**
 * @brief Read a rescue file into an event.
 * @param[in] iPath The file to read.
 * @param[out] oEvent The event to fill; left untouched on failure.
 * @return True when the file was complete and consistent.
 */
[[nodiscard]] auto loadRescue(const std::filesystem::path& iPath, Event& oEvent) -> bool;

/**
 * @brief Look for a resumable interrupted game, newest generation first.
 * @return The summary of the game to propose, or nothing.
 */
[[nodiscard]] auto findRescue() -> std::optional<RescueInfo>;

/// How many archived rescue files are kept. One archiving makes up to two of them, the
/// current generation and the previous one.
constexpr std::size_t g_maxRescueArchives = 10;

/**
 * @brief Move the rescue files aside with a timestamp, then prune the old archives.
 *
 * Declining a recovery must not be irreversible: the files are kept, renamed. They are
 * not kept forever either, or the data directory grows for the rest of the machine's
 * life.
 */
void archiveRescue();

/**
 * @brief Delete the oldest archived rescue files, keeping the newest ones.
 * @param[in] iKeep How many files to keep.
 * @return How many files were deleted.
 */
auto pruneRescueArchives(std::size_t iKeep = g_maxRescueArchives) -> std::size_t;

}// namespace evl::core
