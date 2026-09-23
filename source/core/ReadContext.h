/**
 * @file ReadContext.h
 * @author Silmaen
 * @date 23/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include <cstdint>

namespace evl::core {

/**
 * @brief What a reader needs to know about the file it is walking through.
 *
 * The version alone is not enough, because version 6 is ambiguous: the serialized
 * enumerations were narrowed to `: uint8_t` without the save version being bumped, so
 * two different layouts carry that same number. `wideEnums` is what tells them apart,
 * and it is decided once by `Event::read` before the rest of the tree is walked.
 */
struct ReadContext {
	/// The save version the file declares.
	uint16_t version = 0;
	/// True when the enumerations in this file are written on four bytes.
	bool wideEnums = false;
};

}// namespace evl::core
