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
    set(${PRJPREFIX}_GNU_MINIMAL 11.2)
    set(${PRJPREFIX}_Clang_MINIMAL 11.1)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(${PRJPREFIX}_GNU_MINIMAL 11.2)
    set(${PRJPREFIX}_Clang_MINIMAL 13.0)
    set(${PRJPREFIX}_MSVC_MINIMAL 19.65)
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(${PRJPREFIX}_GNU_MINIMAL 11.2)
    set(${PRJPREFIX}_Clang_MINIMAL 13.0)
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
else ()
    message(FATAL_ERROR "Unsupported Operating System '${CMAKE_SYSTEM_NAME}'")
endif ()

# check minimal Compiler version
if (${CMAKE_CXX_COMPILER_VERSION} LESS ${${PRJPREFIX}_${CMAKE_CXX_COMPILER_ID}_MINIMAL})
    message(FATAL_ERROR "gcovr: Too old version of ${CMAKE_CXX_COMPILER_ID}, minimum required is ${${PRJPREFIX}_${CMAKE_CXX_COMPILER_ID}_MINIMAL} (found ${CMAKE_CXX_COMPILER_VERSION})")
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
    if (NOT CMAKE_BUILD_TYPE MATCHES "Debug")
        message(FATAL_ERROR "Coverage only works on debug builds")
    endif ()
    find_program(GCOVR gcovr)
    if (GCOVR-NOTFOUND)
        message(FATAL_ERROR "unable to find gcovr: no coverage report is possible")
    else ()
        # Get gcov version
        execute_process(COMMAND ${GCOVR} --version
                OUTPUT_VARIABLE GCOVR_VERSION)
        string(REPLACE "\n" ";" GCOVR_VERSION ${GCOVR_VERSION})
        list(SUBLIST GCOVR_VERSION 0 1 GCOVR_VERSION)
        string(REPLACE " " ";" GCOVR_VERSION ${GCOVR_VERSION})
        list(SUBLIST GCOVR_VERSION 1 1 GCOVR_VERSION)

        message(STATUS "Found gcovr version ${GCOVR_VERSION} at location: ${GCOVR} ")
        if (${GCOVR_VERSION} LESS 5.0)
            message(FATAL_ERROR "gcovr: Too old version of gcovr, minimum required is 5.0")
        endif ()
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
