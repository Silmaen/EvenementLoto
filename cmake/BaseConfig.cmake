#
#
#
include(Depmanager)
#
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_INSTALL_PREFIX ${PROJECT_BUILD_DIR}/Install)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(${CMAKE_PROJECT_NAME}_Base INTERFACE)
#
# ---=== Supported OS ===---
#
set(${PRJPREFIX}_GNU_MINIMAL 12)
set(${PRJPREFIX}_CLANG_MINIMAL 14)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(EXE_EXT ".exe")
    set(LIB_EXT ".dll")
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
    set(${PRJPREFIX}_PLATFORM_WINDOWS ON)
    target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PRJPREFIX}_PLATFORM_WINDOWS)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(EXE_EXT "")
    set(LIB_EXT ".so")
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
    set(${PRJPREFIX}_PLATFORM_LINUX ON)
    target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PRJPREFIX}_PLATFORM_LINUX)
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
    target_compile_options(${CMAKE_PROJECT_NAME}_Base INTERFACE
            -Werror -Wall -Wextra -pedantic
            -Wdeprecated
            -Wdeprecated-declarations
            -Wcast-align
            -Wcast-qual
            -Wno-mismatched-new-delete
    )
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${${PRJPREFIX}_CLANG_MINIMAL})
        message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} compiler version too old: ${CMAKE_CXX_COMPILER_VERSION}, need ${${PRJPREFIX}_CLANG_MINIMAL}")
    endif ()
    message(STATUS "Using Clang compiler")
    target_compile_options(${CMAKE_PROJECT_NAME}_Base INTERFACE
            -Werror -Weverything -pedantic
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-c++20-compat
            -Wno-padded
            -Wno-used-but-marked-unused
            -Wno-exit-time-destructors
            -Wno-global-constructors
    )
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL 17)
        target_compile_options(${CMAKE_PROJECT_NAME}_Base INTERFACE
                -Wno-unsafe-buffer-usage
        )
    endif ()
else ()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif ()

target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE "${PRJPREFIX}_MAJOR=\"${CMAKE_PROJECT_VERSION_MAJOR}\"")
target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE "${PRJPREFIX}_MINOR=\"${CMAKE_PROJECT_VERSION_MINOR}\"")
target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE "${PRJPREFIX}_PATCH=\"${CMAKE_PROJECT_VERSION_PATCH}\"")
target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PRJPREFIX}_AUTHOR="Silmaen")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PRJPREFIX}_DEBUG)
endif ()

if (${PRJPREFIX}_COVERAGE)
    include(cmake/CoverageConfig.cmake)
else ()
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        message(STATUS "No Coverage configured. Set ${PRJPREFIX}_COVERAGE=on to enable it.")
    endif ()
endif ()

include(cmake/DocumentationConfig.cmake)
