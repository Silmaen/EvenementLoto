#
#  CMAKE du projet d'événement loto
#
# ---=== GENERALITIES ===---
#
cmake_minimum_required(VERSION 3.24)
set(PROJECT_PREFIX EVL)
set(PROJECT_PREFIX_LOWER evl)
project(EvenementLoto VERSION 0.4.1 LANGUAGES CXX)
#
# ---=== Options ===---
#
option(${PROJECT_PREFIX}_TESTING "To build the tests" ON)
option(${PROJECT_PREFIX}_PACKAGING "To enable packaging" ON)
#
# ---=== Base Configs ===---
#
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")
include(BaseConfig)
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
#
# ---=== Packaging ===---
#
if (${PROJECT_PREFIX}_PACKAGING)
    include(CPackConfig.cmake)
    include(CPack)
endif ()
#
# ---=== End ===---
#
