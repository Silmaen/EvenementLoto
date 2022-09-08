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
add_custom_target(Package
        DEPENDS ${CMAKE_PROJECT_NAME})
add_custom_command(
        TARGET Package POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/sources/${CMAKE_PROJECT_NAME}${EXE_EXT} ${PROJECT_BINARY_DIR}/bin/${CMAKE_PROJECT_NAME}${EXE_EXT})
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(QT_INSTALL_PATH ${${PRJPREFIX}_QT_DIR})
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows.dll")
        add_custom_command(TARGET Package POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "${PROJECT_BINARY_DIR}/bin/plugins/platforms/")
        add_custom_command(TARGET Package POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows.dll"
                "${PROJECT_BINARY_DIR}/bin/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET Package POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${PROJECT_BINARY_DIR}/sources/Qt6${QT_LIB}.dll"
                "${PROJECT_BINARY_DIR}/bin")
    endforeach (QT_LIB)
    cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH COMPILER_PATH)
    foreach (BASE_LIB winpthread-1 stdc++-6 gcc_s_seh-1)
        if (EXISTS ${COMPILER_PATH}/lib${BASE_LIB}.dll)
            add_custom_command(TARGET Package POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    "${COMPILER_PATH}/lib${BASE_LIB}.dll"
                    "${PROJECT_BINARY_DIR}/bin")
        endif ()
    endforeach ()
endif ()
add_custom_command(
        TARGET Package POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data ${PROJECT_BINARY_DIR}/bin/data)

