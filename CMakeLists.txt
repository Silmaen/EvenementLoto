#
#  CMAKE du projet d'événement loto
#
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
#
# ---=== GENERALITIES ===---
#
cmake_minimum_required(VERSION 3.24)
set(PRJPREFIX EVL)
set(PRJPREFIX_LOWER evl)
project(EvenementLoto VERSION 0.3.0 LANGUAGES CXX)
#
# ---=== Options ===---
#
option(${PRJPREFIX}_DOC_ONLY "To only generate documentation" OFF)
option(${PRJPREFIX}_TESTING "To build the tests" ON)


set(${PRJPREFIX}_ROOT_DIR "${PROJECT_SOURCE_DIR}")
#
# ---=== Base Configs ===---
#
include(cmake/BaseConfig.cmake)


if (NOT ${PRJPREFIX}_DOC_ONLY)
    #
    # ---=== The Program ===---
    #
    add_subdirectory(source)

    #
    # ---=== TESTING PROCEDURES ===---
    #

    if (${PRJPREFIX}_TESTING)
        enable_testing()
        add_subdirectory(test)
    endif ()
endif ()
#
# ---=== Packaging ===---
#
include(CPack)
add_custom_target(Package
        DEPENDS ${CMAKE_PROJECT_NAME})
set(PACKAGE_BINARY ${PROJECT_BINARY_DIR}/source)

set(PACKAGE_LOCATION ${PROJECT_BINARY_DIR}/bin)
# copy main executable

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    # copy along dlls
    cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH COMPILER_PATH)
    foreach (BASE_LIB winpthread-1 stdc++-6 gcc_s_seh-1
            fmt
            winpthread-1 freetype-6 harfbuzz-0 md4c png16-16 zlib1 bz2-1 brotlidec brotlicommon glib-2.0-0 intl-8 graphite2
            b2-1 double-conversion icuin71 icudt71 icuuc71 pcre2-8-0 pcre2-16-0 iconv-2 pcre-1
    )
        if (EXISTS ${COMPILER_PATH}/lib${BASE_LIB}.dll)
            add_custom_command(TARGET Package POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${COMPILER_PATH}/lib${BASE_LIB}.dll"
                    "${PROJECT_BINARY_DIR}/bin")
        elseif (EXISTS ${COMPILER_PATH}/${BASE_LIB}.dll)
            add_custom_command(TARGET Package POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${COMPILER_PATH}/${BASE_LIB}.dll"
                    "${PROJECT_BINARY_DIR}/bin")
        endif ()
    endforeach ()
endif ()

add_custom_command(
        TARGET Package POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data ${PROJECT_BINARY_DIR}/bin/data)

install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/ DESTINATION .)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_custom_command(
            TARGET Package POST_BUILD
            COMMAND ${CMAKE_CPACK_COMMAND} -G ZIP)
else ()
    add_custom_command(
            TARGET Package POST_BUILD
            COMMAND ${CMAKE_CPACK_COMMAND} -G TGZ)
endif ()
