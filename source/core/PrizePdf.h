/**
 * @file PrizePdf.h
 * @author Silmaen
 * @date 25/11/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include <filesystem>
#include <string>

namespace evl::core {

class Event;

/**
 * @brief Build the prize list as a PDF, one page per phase, written large to be read on the day.
 *
 * Each page carries the round and the phase, then every prize in the order it is announced,
 * the text as big as the page allows. A phase too full for one page goes on to the next.
 * Only the standard Helvetica is used, so nothing is embedded and the file stays small.
 *
 * @param iEvent The event holding the rounds and the catalogue.
 * @return The bytes of the PDF, empty when there is no prize to print.
 */
[[nodiscard]] auto buildPrizePdf(const Event& iEvent) -> std::string;

/**
 * @brief Write the prize PDF to disk, atomically.
 * @param iEvent The event to print.
 * @param iPath The destination file.
 * @return True when the file was written.
 */
[[nodiscard]] auto writePrizePdf(const Event& iEvent, const std::filesystem::path& iPath) -> bool;

}// namespace evl::core
