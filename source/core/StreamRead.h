/**
 * @file StreamRead.h
 * @author Silmaen
 * @date 21/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include <cstddef>
#include <istream>
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
 * @tparam EnumType Type of the enum to read.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oValue The value read.
 * @return True on success.
 */
template<typename EnumType>
auto readEnum(std::istream& ioBs, EnumType& oValue) -> bool {
	std::underlying_type_t<EnumType> raw = 0;
	if (!readRaw(ioBs, raw))
		return false;
	const auto value = magic_enum::enum_cast<EnumType>(raw);
	if (!value.has_value()) {
		ioBs.setstate(std::ios::failbit);
		return false;
	}
	oValue = value.value();
	return true;
}

/**
 * @brief Read a length and check it against an upper bound.
 * @tparam SizeType Type of the length as stored in the file.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oLength The length read.
 * @param[in] iMaximum Largest accepted value.
 * @return True on success.
 */
template<typename SizeType>
auto readLength(std::istream& ioBs, SizeType& oLength, const std::size_t iMaximum = g_maxSerializedLength) -> bool {
	if (!readRaw(ioBs, oLength))
		return false;
	if (std::cmp_greater(oLength, iMaximum)) {
		ioBs.setstate(std::ios::failbit);
		return false;
	}
	return true;
}

/**
 * @brief Read a length-prefixed string.
 * @param[in,out] ioBs The stream to read from.
 * @param[out] oValue The string read.
 * @return True on success.
 */
inline auto readString(std::istream& ioBs, std::string& oValue) -> bool {
	std::string::size_type length = 0;
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
	typename std::vector<ValueType>::size_type count = 0;
	if (!readLength(ioBs, count))
		return false;
	oValues.resize(count);
	if (count == 0)
		return true;
	ioBs.read(reinterpret_cast<char*>(oValues.data()), static_cast<std::streamsize>(count * sizeof(ValueType)));
	return ioBs.good();
}

}// namespace evl::core
