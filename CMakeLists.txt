#
#  CMAKE du projet d'événement loto
#
cmake_minimum_required(VERSION 3.22)

#
# ---=== GENERALITIES ===---
#
project(EvenementLoto VERSION 0.3.0 LANGUAGES CXX)
set(PRJPREFIX EVL)
set(PRJPREFIX_LOWER evl)
set(${PRJPREFIX}_ROOT_DIR "${PROJECT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX ${PROJECT_BUILD_DIR}/Install)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE off)

#
# ---=== Base Configs ===---
#
include(cmake/BaseConfig.cmake)
#
# ---=== Dependencies ===---
#
add_subdirectory(external)

#
# ---=== The Program ===---
#
add_subdirectory(source)

#
# ---=== TESTING PROCEDURES ===---
#
enable_testing()
add_subdirectory(test)
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
            b2-1 double-conversion icuin69 icuuc69 pcre2-16-0 icudt69 iconv-2 pcre-1
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
