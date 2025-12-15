/**
 * @file defines.h
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

// clang-format off
// Get which compiler...
#if defined(__clang__) && defined(_MSC_VER)
#define EVL_COMPILER clang_cl
#define EVL_COMPILER_CLANG_CL
#elifdef __clang__
#define EVL_COMPILER clang
#define EVL_COMPILER_CLANG
#elifdef _MSC_VER
#define EVL_COMPILER msvc
#define EVL_COMPILER_MSVC
#elifdef __GNUC__
#define EVL_COMPILER gcc
#define EVL_COMPILER_GCC
#else
#define EVL_COMPILER unknown
#endif

// Check supported compiler.
#if !defined(EVL_COMPILER_GCC) && !defined(EVL_COMPILER_CLANG)
#error("unsupported compiler: EVL_COMPILER")
#endif

#if defined(EVL_COMPILER_MSVC) || defined(EVL_COMPILER_CLANG_CL)
#define EVL_DO_PRAGMA(arg) __Pragma(#arg)
#elif defined(EVL_COMPILER_GCC) || defined(EVL_COMPILER_CLANG)
#define EVL_DO_PRAGMA(arg) _Pragma(#arg)
#else
#define EVL_DO_PRAGMA(arg)
#endif

#ifdef EVL_COMPILER_CLANG
#define EVL_DIAG_POP EVL_DO_PRAGMA(clang diagnostic pop)
#define EVL_DIAG_PUSH EVL_DO_PRAGMA(clang diagnostic push)
#define EVL_DIAG_DISABLE_CLANG(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#if __clang_major__ > 15
#define EVL_DIAG_DISABLE_CLANG16(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#else
#define EVL_DIAG_DISABLE_CLANG16(diag)
#endif
#if __clang_major__ > 16
#define EVL_DIAG_DISABLE_CLANG17(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#else
#define EVL_DIAG_DISABLE_CLANG17(diag)
#endif
#if __clang_major__ > 17
#define EVL_DIAG_DISABLE_CLANG18(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#else
#define EVL_DIAG_DISABLE_CLANG18(diag)
#endif
#if __clang_major__ > 18
#define EVL_DIAG_DISABLE_CLANG19(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#else
#define EVL_DIAG_DISABLE_CLANG19(diag)
#endif
#if __clang_major__ > 19
#define EVL_DIAG_DISABLE_CLANG20(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#else
#define EVL_DIAG_DISABLE_CLANG20(diag)
#endif
#if __clang_major__ > 20
#define EVL_DIAG_DISABLE_CLANG21(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#else
#define EVL_DIAG_DISABLE_CLANG21(diag)
#endif
#if __clang_major__ > 21
#define EVL_DIAG_DISABLE_CLANG22(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#else
#define EVL_DIAG_DISABLE_CLANG22(diag)
#endif
#if __clang_major__ > 22
#define EVL_DIAG_DISABLE_CLANG23(diag) EVL_DO_PRAGMA(clang diagnostic ignored diag)
#else
#define EVL_DIAG_DISABLE_CLANG23(diag)
#endif
#define EVL_DIAG_DISABLE_GCC(diag)
#define EVL_DIAG_DISABLE_CLANG_CL(diag)
#define EVL_DIAG_DISABLE_MSVC(diag)
#elifdef EVL_COMPILER_GCC
#define EVL_DIAG_POP EVL_DO_PRAGMA(GCC diagnostic pop)
#define EVL_DIAG_PUSH EVL_DO_PRAGMA(GCC diagnostic push)
#define EVL_DIAG_DISABLE_CLANG(diag)
#define EVL_DIAG_DISABLE_CLANG16(diag)
#define EVL_DIAG_DISABLE_CLANG17(diag)
#define EVL_DIAG_DISABLE_CLANG18(diag)
#define EVL_DIAG_DISABLE_CLANG19(diag)
#define EVL_DIAG_DISABLE_CLANG20(diag)
#define EVL_DIAG_DISABLE_CLANG21(diag)
#define EVL_DIAG_DISABLE_CLANG22(diag)
#define EVL_DIAG_DISABLE_CLANG23(diag)
#define EVL_DIAG_DISABLE_GCC(diag) EVL_DO_PRAGMA(GCC diagnostic ignored diag)
#define EVL_DIAG_DISABLE_CLANG_CL(diag)
#define EVL_DIAG_DISABLE_MSVC(diag)
#elifdef EVL_COMPILER_CLANG_CL
#define EVL_DIAG_POP
#define EVL_DIAG_PUSH
#define EVL_DIAG_DISABLE_CLANG(diag)
#define EVL_DIAG_DISABLE_GCC(diag)
#define EVL_DIAG_DISABLE_CLANG_CL(diag)
#define EVL_DIAG_DISABLE_CLANG16(diag)
#define EVL_DIAG_DISABLE_CLANG17(diag)
#define EVL_DIAG_DISABLE_CLANG18(diag)
#define EVL_DIAG_DISABLE_CLANG19(diag)
#define EVL_DIAG_DISABLE_CLANG20(diag)
#define EVL_DIAG_DISABLE_CLANG21(diag)
#define EVL_DIAG_DISABLE_CLANG22(diag)
#define EVL_DIAG_DISABLE_CLANG23(diag)
#define EVL_DIAG_DISABLE_MSVC(diag)
#elifdef EVL_COMPILER_MSVC
#define EVL_DIAG_POP EVL_DO_PRAGMA(warning(pop))
#define EVL_DIAG_PUSH EVL_DO_PRAGMA(warning(push))
#define EVL_DIAG_DISABLE_CLANG(diag)
#define EVL_DIAG_DISABLE_CLANG16(diag)
#define EVL_DIAG_DISABLE_CLANG17(diag)
#define EVL_DIAG_DISABLE_CLANG18(diag)
#define EVL_DIAG_DISABLE_CLANG19(diag)
#define EVL_DIAG_DISABLE_CLANG20(diag)
#define EVL_DIAG_DISABLE_GCC(diag)
#define EVL_DIAG_DISABLE_CLANG_CL(diag)
#define EVL_DIAG_DISABLE_MSVC(diag) EVL_DO_PRAGMA(warning(disable : diag))
#else
#define EVL_DIAG_POP
#define EVL_DIAG_PUSH
#define EVL_DIAG_DISABLE_CLANG(diag)
#define EVL_DIAG_DISABLE_CLANG16(diag)
#define EVL_DIAG_DISABLE_CLANG17(diag)
#define EVL_DIAG_DISABLE_CLANG18(diag)
#define EVL_DIAG_DISABLE_CLANG19(diag)
#define EVL_DIAG_DISABLE_CLANG20(diag)
#define EVL_DIAG_DISABLE_CLANG21(diag)
#define EVL_DIAG_DISABLE_CLANG22(diag)
#define EVL_DIAG_DISABLE_CLANG23(diag)
#define EVL_DIAG_DISABLE_GCC(diag)
#define EVL_DIAG_DISABLE_CLANG_CL(diag)
#define EVL_DIAG_DISABLE_MSVC(diag)
#endif

// clang-format on
