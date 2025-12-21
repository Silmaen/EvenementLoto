#
#
#
include(Depmanager)
include(Utilities)
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
set(${PROJECT_PREFIX}_GNU_MINIMAL 12)
set(${PROJECT_PREFIX}_CLANG_MINIMAL 14)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(EXE_EXT ".exe")
    set(LIB_EXT ".dll")
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
    set(${PROJECT_PREFIX}_PLATFORM_WINDOWS ON)
    target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PROJECT_PREFIX}_PLATFORM_WINDOWS)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(EXE_EXT "")
    set(LIB_EXT ".so")
    message(STATUS "Detected Operating System '${CMAKE_SYSTEM_NAME}'")
    set(${PROJECT_PREFIX}_PLATFORM_LINUX ON)
    target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PROJECT_PREFIX}_PLATFORM_LINUX)
else ()
    message(FATAL_ERROR "Unsupported Operating System '${CMAKE_SYSTEM_NAME}'")
endif ()

#
# ---=== Supported Compiler ===----
#
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${${PROJECT_PREFIX}_GNU_MINIMAL})
        message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} compiler version too old: ${CMAKE_CXX_COMPILER_VERSION}, need ${${PROJECT_PREFIX}_GNU_MINIMAL}")
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
    set(${PROJECT_PREFIX}_COMPILER_GCC ON)
    set(${PROJECT_PREFIX}_COMPILER_STR "gcc")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${${PROJECT_PREFIX}_CLANG_MINIMAL})
        message(FATAL_ERROR "${CMAKE_CXX_COMPILER_ID} compiler version too old: ${CMAKE_CXX_COMPILER_VERSION}, need ${${PROJECT_PREFIX}_CLANG_MINIMAL}")
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
            -Wno-unused-macros
    )
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL 17)
        target_compile_options(${CMAKE_PROJECT_NAME}_Base INTERFACE
                -Wno-unsafe-buffer-usage
        )
    endif ()
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL 18)
        target_compile_options(${CMAKE_PROJECT_NAME}_Base INTERFACE
                -Wno-switch-default
        )
    endif ()
    set(${PROJECT_PREFIX}_COMPILER_CLANG ON)
    set(${PROJECT_PREFIX}_COMPILER_STR "clang")
    target_compile_options(${CMAKE_PROJECT_NAME}_Base INTERFACE -Wno-enum-constexpr-conversion)
else ()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif ()

get_property(${PROJECT_PREFIX}_IS_GENERATOR_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if (${${PROJECT_PREFIX}_IS_GENERATOR_MULTI_CONFIG})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG>)
else ()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif ()

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/output/install)

set(${PROJECT_PREFIX}_INSTALL_BIN "bin/${${PROJECT_PREFIX}_PLATFORM_STR}_${${PROJECT_PREFIX}_ARCH_STR}")
set(${PROJECT_PREFIX}_INSTALL_LIB "lib/${${PROJECT_PREFIX}_PLATFORM_STR}_${${PROJECT_PREFIX}_ARCH_STR}")

add_custom_target(${CMAKE_PROJECT_NAME}_SuperBase)
set_target_properties(${CMAKE_PROJECT_NAME}_SuperBase PROPERTIES FOLDER "Utils")
if (${PROJECT_PREFIX}_PLATFORM_WINDOWS)
    if (MINGW)
        cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH COMPILER_PATH)
        message(STATUS "MinGW environment detected: add dependence to dlls from ${COMPILER_PATH}")
        set(REQUIRED_LIBS libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        foreach (lib IN ITEMS ${REQUIRED_LIBS})
            if (NOT EXISTS ${COMPILER_PATH}/${lib})
                message(WARNING "Required Dll not found: ${COMPILER_PATH}/${lib}")
            else ()
                message(STATUS "Adding Dll: ${COMPILER_PATH}/${lib}")
                add_custom_command(TARGET ${CMAKE_PROJECT_NAME}_SuperBase POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COMPILER_PATH}/${lib} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                        COMMENT "Copying ${lib} into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
            endif ()
        endforeach ()
        target_compile_options(${CMAKE_PROJECT_NAME}_Base INTERFACE -U_GLIBCXX_DEBUG)
    endif ()
elseif (${PROJECT_PREFIX}_PLATFORM_LINUX)
    set(CMAKE_PLATFORM_USES_PATH_WHEN_NO_SONAME OFF)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath='$ORIGIN' -Wl,--disable-new-dtags")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath='$ORIGIN' -Wl,--disable-new-dtags")
endif ()
add_dependencies(${CMAKE_PROJECT_NAME}_Base ${CMAKE_PROJECT_NAME}_SuperBase)

target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE "${PROJECT_PREFIX}_MAJOR=\"${CMAKE_PROJECT_VERSION_MAJOR}\"")
target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE "${PROJECT_PREFIX}_MINOR=\"${CMAKE_PROJECT_VERSION_MINOR}\"")
target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE "${PROJECT_PREFIX}_PATCH=\"${CMAKE_PROJECT_VERSION_PATCH}\"")
target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PROJECT_PREFIX}_AUTHOR="Silmaen")

if (${${PROJECT_PREFIX}_IS_GENERATOR_MULTI_CONFIG})
    target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PROJECT_PREFIX}_$<IF:$<CONFIG:Debug>,DEBUG,RELEASE>)
else ()
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PROJECT_PREFIX}_DEBUG)
    else ()
        target_compile_definitions(${CMAKE_PROJECT_NAME}_Base INTERFACE ${PROJECT_PREFIX}_RELEASE)
    endif ()
endif ()

if (${PROJECT_PREFIX}_ENABLE_COVERAGE)
    include(cmake/CoverageConfig.cmake)
else ()
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        message(STATUS "No Coverage configured. Set ${PROJECT_PREFIX}_COVERAGE=on to enable it.")
    endif ()
endif ()

include(cmake/DocumentationConfig.cmake)
include(cmake/Vulkan.cmake)
