/**
 * @file implementations.cpp
 * @author Silmaen
 * @date 21/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

// Single translation unit instantiating the header-only third parties, kept out of the
// project sources so that neither the warnings nor clang-tidy apply to them.

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvgrast.h>
