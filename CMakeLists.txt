#
#  CMAKE du projet d'événement loto
#
cmake_minimum_required(VERSION 3.15)

#
# ---=== GENERALITIES ===---
#
project(EvenementLoto VERSION 0.0.1 LANGUAGES CXX)
set(PRJPREFIX EVL)
set(PRJPREFIX_LOWER evl)
set(${PRJPREFIX}_ROOT_DIR "${PROJECT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX ${PROJECT_BUILD_DIR}/Install)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# ---=== Supported OS ===---
#
if (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
else ()
    message(FATAL_ERROR "Unsupported Operating System '${CMAKE_SYSTEM_NAME}'")
endif ()

#
# ---=== base flags ===---
#
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wall -Wextra -pedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -pedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /WX)
else ()
    message(FATAL_ERROR "Unsupported Compiler '${CMAKE_CXX_COMPILER_ID}'")
endif ()

if (${PRJPREFIX}_COVERAGE AND ${PRJPREFIX}_PROFILING)
    message(FATAL_ERROR "Incompatible option activated at the same time: ${PRJPREFIX}_COVERAGE and ${PRJPREFIX}_PROFILING")
endif ()

if (${PRJPREFIX}_COVERAGE)
    find_program(GCOVR gcovr)
    if (GCOVR-NOTFOUND)
        message(FATAL_ERROR "unable to find gcovr: no coverage report is possible")
    else ()
        message(STATUS "Found GCOVR at location: ${GCOVR}")
    endif ()
    # options for coverage
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(--coverage)
        link_libraries(gcov)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(--coverage)
        add_link_options(--coverage)
        get_filename_component(COMPILER_PATH ${CMAKE_CXX_COMPILER} DIRECTORY)
        set(${PRJPREFIX}_GCOV "${COMPILER_PATH}/llvm-cov gcov")
        if (WIN32)
            string(REPLACE "/" "\\\\" ${PRJPREFIX}_GCOV ${${PRJPREFIX}_GCOV})
        endif ()
    else ()
        message(FATAL_ERROR "Unsupported Compiler '${CMAKE_CXX_COMPILER_ID}' for coverage")
    endif ()
endif ()

if (${PRJPREFIX}_PROFILING)
    #options for profiling
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-pg)

    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # todo
    else ()
        message(FATAL_ERROR "Unsupported Compiler '${CMAKE_CXX_COMPILER_ID}' for profiling")
    endif ()
endif ()

#
# ---=== Defines ===---
#
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions(${PRJPREFIX}_DEBUG)
else ()
    add_compile_definitions(${PRJPREFIX}_RELEASE)
endif ()
option(${PRJPREFIX}_VERBOSE_TEST "if the tests should be verbose or not" False)
if (${PRJPREFIX}_VERBOSE_TEST)
    add_compile_definitions(${PRJPREFIX}_VERBOSE_TEST)
endif ()

#
# ---=== Third parties ===---
#
add_subdirectory(third_party)

#
# ---=== The Program ===---
#
add_subdirectory(sources)

#
# ---=== TESTING PROCEDURES ===---
#
enable_testing()
add_subdirectory(test)

#
# ---=== documentation ===---
#
find_package(Doxygen
        REQUIRED dot)
if (DOXYGEN_FOUND)
    message(STATUS "found doxygen version: ${DOXYGEN_VERSION}")
    add_custom_target(documentation
            COMMAND ${DOXYGEN_EXECUTABLE}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Generating API documentation"
            VERBATIM)
else (DOXYGEN_FOUND)
    message(ERROR no doxygen found)
endif (DOXYGEN_FOUND)
