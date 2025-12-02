/**
 * @file StringUtils.h
 * @author Silmaen
 * @date 30/11/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include <string>

namespace evl {

/**
 * @brief Joins the elements of a range into a single string with a delimiter.
 * @tparam Iterable A range type.
 * @param iElements The range of elements to join.
 * @param iDelimiter The delimiter to insert between elements.
 * @return A string containing the joined elements.
 */
template<std::ranges::range Iterable>
auto join(const Iterable& iElements, const std::string& iDelimiter) -> std::string {
	std::string result;
	auto it = std::begin(iElements);
	auto end = std::end(iElements);
	if (it == end)
		return result;
	result = std::format("{}", *it);
	++it;
	for (; it != end; ++it) { result += std::format("{}{}", iDelimiter, *it); }
	return result;
}

}// namespace evl
