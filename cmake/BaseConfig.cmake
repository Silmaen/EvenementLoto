#
# ---=== Supported OS ===---
#
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(EXE_EXT ".exe")
    set(LIB_EXT ".dll")
    set(${PRJPREFIX}_GNU_MINIMAL 12)
    set(${PRJPREFIX}_CLANG_MINIMAL 14)
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(EXE_EXT "")
    set(LIB_EXT ".so")
    set(${PRJPREFIX}_GNU_MINIMAL 11.2)
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

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_compile_definitions(${PRJPREFIX}_MAJOR="${CMAKE_PROJECT_VERSION_MAJOR}")
add_compile_definitions(${PRJPREFIX}_MINOR="${CMAKE_PROJECT_VERSION_MINOR}")
add_compile_definitions(${PRJPREFIX}_PATCH="${CMAKE_PROJECT_VERSION_PATCH}")
add_compile_definitions(${PRJPREFIX}_AUTHOR="Damien Lachouette")

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
