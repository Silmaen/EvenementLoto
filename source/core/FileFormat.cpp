/**
 * @file FileFormat.cpp
 * @author Silmaen
 * @date 22/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "FileFormat.h"

#include "StreamWrite.h"

#include <cstring>

namespace evl::core {

namespace {

/// Reversed IEEE 802.3 polynomial.
constexpr uint32_t g_polynomial = 0xEDB88320U;
/// Size of the header a framed file opens with: the magic then the version.
constexpr std::size_t g_headerSize = g_fileMagic.size() + sizeof(uint16_t);
/// Size of the checksum a framed file ends with.
constexpr std::size_t g_checksumSize = sizeof(uint32_t);

}// namespace

auto crc32(const std::string_view iBytes) -> uint32_t {
	uint32_t value = 0xFFFFFFFFU;
	for (const char byte: iBytes) {
		value ^= static_cast<uint8_t>(byte);
		for (int bit = 0; bit < 8; ++bit) value = ((value & 1U) != 0U) ? (value >> 1U) ^ g_polynomial : value >> 1U;
	}
	return ~value;
}

auto readFrame(std::istream& ioBs) -> FileFrame {
	FileFrame frame;
	frame.body.assign(std::istreambuf_iterator<char>(ioBs), std::istreambuf_iterator<char>());

	if (!frame.body.starts_with(g_fileMagic)) {
		// An older file: no magic, no checksum, and its first field is the version the
		// reader is about to consume itself.
		frame.valid = frame.body.size() >= sizeof(uint16_t);
		return frame;
	}
	frame.framed = true;
	if (frame.body.size() < g_headerSize + g_checksumSize)
		return frame;

	const std::string_view content{frame.body};
	const auto payload = content.substr(0, frame.body.size() - g_checksumSize);
	uint32_t stored = 0;
	std::memcpy(&stored, frame.body.data() + frame.body.size() - g_checksumSize, g_checksumSize);
	if (stored != crc32(payload))
		return frame;

	std::memcpy(&frame.version, frame.body.data() + g_fileMagic.size(), sizeof(uint16_t));
	frame.body = std::string{payload.substr(g_headerSize)};
	frame.valid = true;
	return frame;
}

void writeFrame(std::ostream& oBs, const uint16_t iVersion, const std::string_view iBody) {
	std::string payload;
	payload.reserve(g_headerSize + iBody.size());
	payload.append(g_fileMagic);
	payload.resize(g_headerSize);
	std::memcpy(payload.data() + g_fileMagic.size(), &iVersion, sizeof(iVersion));
	payload.append(iBody);

	oBs.write(payload.data(), static_cast<std::streamsize>(payload.size()));
	writeRaw(oBs, crc32(payload));
}

}// namespace evl::core
