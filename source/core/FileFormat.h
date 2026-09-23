/**
 * @file FileFormat.h
 * @author Silmaen
 * @date 22/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>

namespace evl::core {

/// Four bytes opening a save file, so that anything that is not one is rejected before
/// a single field is interpreted.
constexpr std::string_view g_fileMagic = "EVL1";

/// First save version to carry the magic and the trailing checksum. Anything below is
/// read as a bare versioned body.
constexpr uint16_t g_firstFramedVersion = 7;

/**
 * @brief CRC-32, the IEEE 802.3 polynomial, computed bit by bit.
 *
 * No lookup table: a save file is a few hundred kilobytes, the difference is invisible
 * next to the disk access, and there is nothing to initialise.
 *
 * @param[in] iBytes The bytes to sum.
 * @return The checksum.
 */
[[nodiscard]] auto crc32(std::string_view iBytes) -> uint32_t;

/// What a framed file was found to contain.
struct FileFrame {
	/// The serialized body, to be parsed on its own.
	std::string body;
	/// The save version the file declares.
	uint16_t version = 0;
	/// True when the file carried a magic and a checksum that matched.
	bool framed = false;
	/// False when the file is unusable: too short, bad magic length, checksum mismatch.
	bool valid = false;
};

/**
 * @brief Read a whole stream and hand back the body to parse.
 *
 * A file opening with the magic is checked against its trailing checksum **before**
 * anything in it is interpreted, which is what lets a truncated or corrupted rescue
 * file be discarded in favour of the previous generation instead of half loaded.
 *
 * An older file carries no frame: its body is the whole content, and its version is the
 * first field of that body, left in place for the reader.
 *
 * @param[in,out] ioBs The stream to read, consumed to its end.
 * @return What was found.
 */
[[nodiscard]] auto readFrame(std::istream& ioBs) -> FileFrame;

/**
 * @brief Write a serialized body as a framed file: magic, version, body, checksum.
 * @param[in,out] oBs The stream to write to.
 * @param[in] iVersion The save version to declare.
 * @param[in] iBody The serialized body.
 */
void writeFrame(std::ostream& oBs, uint16_t iVersion, std::string_view iBody);

}// namespace evl::core
