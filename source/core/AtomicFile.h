/**
 * @file AtomicFile.h
 * @author Silmaen
 * @date 21/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include <filesystem>
#include <functional>
#include <ostream>

namespace evl::core {

/**
 * @brief Write a file through a temporary copy, then move it into place.
 *
 * The destination is only replaced once the content has been fully written and flushed:
 * a crash or a power loss during the write leaves the previous version intact. Never
 * throws, so it is safe to call from a shutdown path.
 *
 * @param[in] iPath The file to write.
 * @param[in] iWriter Callback filling the stream.
 * @param[in] iKeepPrevious Move the current file aside, suffixed with '.1', before
 *            replacing it.
 * @return True when the destination holds the new content.
 */
[[nodiscard]] auto writeFileAtomically(const std::filesystem::path& iPath,
									   const std::function<void(std::ostream&)>& iWriter,
									   bool iKeepPrevious = false) noexcept -> bool;

}// namespace evl::core
