/**
 * @file StreamWrite.h
 * @author Silmaen
 * @date 22/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

#include "timeFunctions.h"

namespace evl::core {

/**
 * @brief Write a trivially copyable value as is.
 * @tparam ValueType Type of the value to write.
 * @param[in,out] oBs The stream to write to.
 * @param[in] iValue The value to write.
 */
template<typename ValueType>
void writeRaw(std::ostream& oBs, const ValueType& iValue) {
	static_assert(std::is_trivially_copyable_v<ValueType>, "writeRaw only handles trivially copyable types");
	oBs.write(reinterpret_cast<const char*>(&iValue), sizeof(ValueType));
}

/**
 * @brief Write an enumerator, in the width its underlying type declares.
 *
 * Every serialized enumeration in this project declares `: uint8_t`, so the width is
 * already fixed and the same everywhere. This exists so that a later enumeration
 * forgetting to declare one shows up here rather than in a file nobody can read.
 *
 * @tparam EnumType Type of the enum to write.
 * @param[in,out] oBs The stream to write to.
 * @param[in] iValue The value to write.
 */
template<typename EnumType>
void writeEnum(std::ostream& oBs, const EnumType iValue) {
	using underlying = std::underlying_type_t<EnumType>;
	static_assert(sizeof(underlying) == 1, "a serialized enum must declare a fixed-width underlying type");
	writeRaw(oBs, static_cast<underlying>(iValue));
}

/**
 * @brief Write a length on a fixed width.
 *
 * `std::size_t` is eight bytes on the platforms built today and four on a 32-bit one,
 * which is exactly the kind of difference that makes a file unreadable elsewhere.
 *
 * @param[in,out] oBs The stream to write to.
 * @param[in] iLength The length to write.
 */
inline void writeLength(std::ostream& oBs, const std::size_t iLength) { writeRaw(oBs, static_cast<uint64_t>(iLength)); }

/**
 * @brief Write a length-prefixed string.
 * @param[in,out] oBs The stream to write to.
 * @param[in] iValue The string to write.
 */
inline void writeString(std::ostream& oBs, const std::string& iValue) {
	writeLength(oBs, iValue.size());
	if (!iValue.empty())
		oBs.write(iValue.data(), static_cast<std::streamsize>(iValue.size()));
}

/**
 * @brief Write a length-prefixed vector of trivially copyable values.
 * @tparam ValueType Type of the vector elements.
 * @param[in,out] oBs The stream to write to.
 * @param[in] iValues The values to write.
 */
template<typename ValueType>
void writeVector(std::ostream& oBs, const std::vector<ValueType>& iValues) {
	static_assert(std::is_trivially_copyable_v<ValueType>, "writeVector only handles trivially copyable types");
	writeLength(oBs, iValues.size());
	if (!iValues.empty())
		oBs.write(reinterpret_cast<const char*>(iValues.data()),
				  static_cast<std::streamsize>(iValues.size() * sizeof(ValueType)));
}

/**
 * @brief Write a point in time as a count of nanoseconds since the clock's epoch.
 *
 * A raw dump of a `time_point` writes whatever representation the standard library
 * chose: libstdc++ counts nanoseconds, another implementation counts something else,
 * and the file stops being interchangeable. Nanoseconds are named explicitly here —
 * which also happens to be byte for byte what the previous versions wrote, so a file
 * from before this change still reads.
 *
 * @param[in,out] oBs The stream to write to.
 * @param[in] iValue The point in time to write.
 */
inline void writeTimePoint(std::ostream& oBs, const time_point& iValue) {
	writeRaw(oBs, static_cast<int64_t>(
						  std::chrono::duration_cast<std::chrono::nanoseconds>(iValue.time_since_epoch()).count()));
}

}// namespace evl::core
