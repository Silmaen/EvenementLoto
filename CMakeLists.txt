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
    set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror")
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(CMAKE_EXE_LINKER_FLAGS "-lm -lpthread")
    else ()
        set(CMAKE_EXE_LINKER_FLAGS "-lm -lpthread -ldl -lcrypt")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /EHsc /GR /MD /TP /Zc:wchar_t /W4 /GS /Zi /Ob0 /Od /RTC1") # /wd"4251" /wd"4275" /wd"4702"
else ()
    message(FATAL_ERROR "Unsupported Compiler '${CMAKE_CXX_COMPILER_ID}'")
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
