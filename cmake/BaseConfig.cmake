#
# ---=== Supported OS ===---
#
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(EXE_EXT ".exe")
    set(LIB_EXT ".exe")
    set(${PRJPREFIX}_GNU_MINIMAL 12)
    set(${PRJPREFIX}_CLANG_MINIMAL 14)
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(${PRJPREFIX}_GNU_MINIMAL 12.0)
    set(${PRJPREFIX}_CLANG_MINIMAL 14.0)
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
else ()
    message(FATAL_ERROR "Unsupported Operating System '${CMAKE_SYSTEM_NAME}'")
endif ()

#
# ---=== Supported Compiler ===----
#
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${${PRJPREFIX}_GNU_MINIMAL})
        message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} compiler version too old: ${CMAKE_CXX_COMPILER_VERSION}, need ${${PRJPREFIX}_GNU_MINIMAL}")
    endif ()
    message(STATUS "Using GNU compiler")
    add_compile_options(
            -Werror -Wall -Wextra -pedantic
            -Wdeprecated
            -Wdeprecated-declarations
            -Wcast-align
            -Wcast-qual
    )
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${${PRJPREFIX}_CLANG_MINIMAL})
        message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} compiler version too old: ${CMAKE_CXX_COMPILER_VERSION}, need ${${PRJPREFIX}_CLANG_MINIMAL}")
    endif ()
    message(STATUS "Using Clang compiler")
    add_compile_options(
            -Werror -Weverything -pedantic
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-c++20-compat
            -Wno-padded
            -Wno-used-but-marked-unused
            -Wno-exit-time-destructors
            -Wno-global-constructors
    )
else ()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif ()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions(${PRJPREFIX}_DEBUG)
endif ()

if (${PRJPREFIX}_COVERAGE)
    include(cmake/CoverageConfig.cmake)
else ()
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        message(STATUS "No Coverage configured. Set ${PRJPREFIX}_COVERAGE=on to enable it.")
    endif ()
endif ()

include(cmake/DocumentationConfig.cmake)
