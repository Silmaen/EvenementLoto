/**
 * @file EnumLabel.h
 * @author Silmaen
 * @date 21/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

namespace evl::core {

/// Returned when an enum value has no known label.
constexpr std::string_view g_unknownLabel = "inconnu";

/**
 * @brief Look up the display label of an enum value in a constexpr table.
 *
 * Tables are constexpr arrays instead of static maps: no allocation, so no exception
 * during static initialization, and an unknown value never throws.
 *
 * @tparam EnumType The enum type to convert.
 * @tparam Size Number of entries in the table.
 * @param[in] iLabels The table to search.
 * @param[in] iValue The value to look for.
 * @return The matching label, or g_unknownLabel.
 */
template<typename EnumType, std::size_t Size>
[[nodiscard]] constexpr auto enumLabel(const std::array<std::pair<EnumType, std::string_view>, Size>& iLabels,
									   const EnumType& iValue) -> std::string_view {
	for (const auto& [value, label]: iLabels) {
		if (value == iValue)
			return label;
	}
	return g_unknownLabel;
}

/**
 * @brief Look up the enum value matching a display label.
 * @tparam EnumType The enum type to convert to.
 * @tparam Size Number of entries in the table.
 * @param[in] iLabels The table to search.
 * @param[in] iLabel The label to look for.
 * @param[in] iDefault Returned when no entry matches.
 * @return The matching value, or iDefault.
 */
template<typename EnumType, std::size_t Size>
[[nodiscard]] constexpr auto enumFromLabel(const std::array<std::pair<EnumType, std::string_view>, Size>& iLabels,
										   const std::string_view& iLabel, const EnumType& iDefault) -> EnumType {
	for (const auto& [value, label]: iLabels) {
		if (label == iLabel)
			return value;
	}
	return iDefault;
}

}// namespace evl::core
