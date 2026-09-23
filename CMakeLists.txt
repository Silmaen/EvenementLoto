#
#  CMAKE du projet d'événement loto
#
# ---=== GENERALITIES ===---
#
cmake_minimum_required(VERSION 3.24)
set(PROJECT_PREFIX EVL)
set(PROJECT_PREFIX_LOWER evl)
# Conan acts as a CMake dependency provider: registered before project(), it only runs
# on the first find_package().
list(APPEND CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${CMAKE_CURRENT_LIST_DIR}/cmake/conan_provider.cmake")
project(EvenementLoto VERSION 0.5.0 LANGUAGES CXX C)
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
