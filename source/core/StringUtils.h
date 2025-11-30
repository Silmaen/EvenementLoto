/**
 * @file StringUtils.h
 * @author Silmaen
 * @date 30/11/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

namespace evl {

/**
 * @brief Joins the elements of a range into a single string with a delimiter.
 * @tparam Iterable A range type.
 * @param elements The range of elements to join.
 * @param delimiter The delimiter to insert between elements.
 * @return A string containing the joined elements.
 */
template<std::ranges::range Iterable>
std::string join(const Iterable& elements, const std::string& delimiter) {
    std::string result;
    auto it = std::begin(elements);
    auto end = std::end(elements);
    if (it == end) return result;
    result = std::format("{}", *it);
    ++it;
    for (; it != end; ++it) {
        result += std::format("{}{}", delimiter, *it);
    }
    return result;
}

}// namespace evl
