/**
 * @file pch.h
 * @author Silmaen
 * @date 01/12/2025
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#if !defined(__clang__) or __clang_major__ > 15
#include <ranges>
#endif
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <streambuf>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// third party
#include <magic_enum/magic_enum.hpp>
