#
#  CMAKE du projet d'événement loto
#
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
#
# ---=== GENERALITIES ===---
#
cmake_minimum_required(VERSION 3.24)
set(PROJECT_PREFIX EVL)
set(PROJECT_PREFIX_LOWER evl)
project(EvenementLoto VERSION 0.4.0 LANGUAGES CXX)
#
# ---=== Options ===---
#
option(${PROJECT_PREFIX}_DOC_ONLY "To only generate documentation" OFF)
option(${PROJECT_PREFIX}_TESTING "To build the tests" ON)
option(${PROJECT_PREFIX}_ENABLE_QT "Enable QT version of the code" OFF)

set(${PROJECT_PREFIX}_ROOT_DIR "${PROJECT_SOURCE_DIR}")
#
# ---=== Base Configs ===---
#
include(BaseConfig)


if (NOT ${PROJECT_PREFIX}_DOC_ONLY)
    #
    # ---=== Sanitizers ===---
    #
    include(Sanitizers)

    #
    # ---=== The Program ===---
    #
    add_subdirectory(source)

    #
    # ---=== TESTING PROCEDURES ===---
    #

    if (${PROJECT_PREFIX}_TESTING)
        enable_testing()
        add_subdirectory(test)
    endif ()
endif ()
#
# ---=== Packaging ===---
#
include(CPackConfig.cmake)
include(CPack)
add_custom_target(Package
        DEPENDS ${CMAKE_PROJECT_NAME})
set(PACKAGE_BINARY ${PROJECT_BINARY_DIR}/source)

set(PACKAGE_LOCATION ${PROJECT_BINARY_DIR}/bin)
# copy main executable

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    # copy along dlls
    cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH COMPILER_PATH)

    set(BASE_LIBS stdc++-6 gcc_s_seh-1 winpthread-1 zlib1 zstd)

    if (${${PROJECT_PREFIX}_ENABLE_QT})
        list(APPEND BASE_LIBS Qt6Core Qt6Gui Qt6Svg Qt6Widgets harfbuzz-0 b2-1
                double-conversion icuin74 icuuc74 pcre2-16-0 zlib1 zstd
                freetype-6 md4c png16-16 icudt74 bz2-1 brotlidec glib-2.0-0
                graphite2 intl-8 brotlicommon pcre2-8-0 iconv-)
    endif ()
    foreach (BASE_LIB ${BASE_LIBS})
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
        else ()
            message(FATAL_ERROR "${BASE_LIB} : Not found")
        endif ()
    endforeach ()
endif ()

add_custom_command(
        TARGET Package POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/document/Readme.md ${PROJECT_BINARY_DIR}/bin)
add_custom_command(
        TARGET Package POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/bin/data
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/data/base_rules.txt ${PROJECT_BINARY_DIR}/bin/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data/slides ${PROJECT_BINARY_DIR}/bin/data/slides
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data/theme ${PROJECT_BINARY_DIR}/bin/data/theme)

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

#
# ---=== End ===---
#
