/**
 * @file StreamRead.h
 * @author Silmaen
 * @date 21/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "timeFunctions.h"

#include <cstddef>
#include <cstdint>
#include <istream>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <magic_enum/magic_enum.hpp>

namespace evl::core {

/// Upper bound for a length read from a file, to reject corrupted content early.
constexpr std::size_t g_maxSerializedLength = 1UL << 20UL;
/// Upper bound for a number of objects read from a file.
constexpr std::size_t g_maxSerializedCount = 1UL << 16UL;

/**
 * @brief Read a trivially copyable value.
 *
 * The stream is the error channel: a short read leaves it in a failed state, which the
 * callers check instead of trusting whatever bytes the file contained.
 *
 * @tparam ValueType Type of the value to read.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oValue The value read.
 * @return True on success.
 */
template<typename ValueType>
auto readRaw(std::istream& ioBs, ValueType& oValue) -> bool {
	static_assert(std::is_trivially_copyable_v<ValueType>, "readRaw only handles trivially copyable types");
	ioBs.read(reinterpret_cast<char*>(&oValue), sizeof(ValueType));
	return ioBs.good();
}

/**
 * @brief Read an enum value, rejecting anything that is not a declared enumerator.
 *
 * `iWide` reads the four bytes a plain `enum` used to occupy, for the files written
 * before the serialized enumerations declared `: uint8_t`. See ReadContext.
 *
 * @tparam EnumType Type of the enum to read.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oValue The value read.
 * @param[in] iWide True to read four bytes instead of the underlying type's width.
 * @return True on success.
 */
template<typename EnumType>
auto readEnum(std::istream& ioBs, EnumType& oValue, const bool iWide = false) -> bool {
	using underlying = std::underlying_type_t<EnumType>;
	int64_t raw = 0;
	if (iWide) {
		int32_t stored = 0;
		if (!readRaw(ioBs, stored))
			return false;
		raw = stored;
	} else {
		underlying stored = 0;
		if (!readRaw(ioBs, stored))
			return false;
		raw = static_cast<int64_t>(stored);
	}
	// A four-byte field holding something the underlying type cannot even represent is
	// not a stale layout, it is a corrupted file.
	if (std::cmp_less(raw, std::numeric_limits<underlying>::min()) ||
		std::cmp_greater(raw, std::numeric_limits<underlying>::max())) {
		ioBs.setstate(std::ios::failbit);
		return false;
	}
	const auto value = magic_enum::enum_cast<EnumType>(static_cast<underlying>(raw));
	if (!value.has_value()) {
		ioBs.setstate(std::ios::failbit);
		return false;
	}
	oValue = value.value();
	return true;
}

/**
 * @brief Read a length, on the fixed width it is stored on, and bound it.
 *
 * Always eight bytes, whatever the type asked for: that is what makes the file readable
 * on a platform where `std::size_t` is not eight bytes. It is also what the 64-bit
 * builds have always written, so a file from before the fixed width was spelled out
 * still reads.
 *
 * @tparam SizeType Type the caller wants the length in.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oLength The length read.
 * @param[in] iMaximum Largest accepted value.
 * @return True on success.
 */
template<typename SizeType>
auto readLength(std::istream& ioBs, SizeType& oLength, const std::size_t iMaximum = g_maxSerializedLength) -> bool {
	uint64_t stored = 0;
	if (!readRaw(ioBs, stored))
		return false;
	if (stored > iMaximum) {
		ioBs.setstate(std::ios::failbit);
		return false;
	}
	oLength = static_cast<SizeType>(stored);
	return true;
}

/**
 * @brief Read a point in time, stored as a count of nanoseconds since the epoch.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oValue The point in time read.
 * @return True on success.
 */
inline auto readTimePoint(std::istream& ioBs, time_point& oValue) -> bool {
	int64_t stored = 0;
	if (!readRaw(ioBs, stored))
		return false;
	oValue = time_point{std::chrono::duration_cast<clock::duration>(std::chrono::nanoseconds{stored})};
	return true;
}

/**
 * @brief Read a length-prefixed string.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oValue The string read.
 * @return True on success.
 */
inline auto readString(std::istream& ioBs, std::string& oValue) -> bool {
	std::size_t length = 0;
	if (!readLength(ioBs, length))
		return false;
	oValue.resize(length);
	if (length == 0)
		return true;
	ioBs.read(oValue.data(), static_cast<std::streamsize>(length));
	return ioBs.good();
}

/**
 * @brief Read a length-prefixed vector of trivially copyable values.
 * @tparam ValueType Type of the vector elements.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oValues The values read.
 * @return True on success.
 */
template<typename ValueType>
auto readVector(std::istream& ioBs, std::vector<ValueType>& oValues) -> bool {
	static_assert(std::is_trivially_copyable_v<ValueType>, "readVector only handles trivially copyable types");
	std::size_t count = 0;
	if (!readLength(ioBs, count))
		return false;
	oValues.resize(count);
	if (count == 0)
		return true;
	ioBs.read(reinterpret_cast<char*>(oValues.data()), static_cast<std::streamsize>(count * sizeof(ValueType)));
	return ioBs.good();
}

}// namespace evl::core
