#
# Core library
#

file(GLOB_RECURSE
        SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../basedefine.h
)
add_library(${CMAKE_PROJECT_NAME}_lib ${SRCS})
# base include dir.
target_include_directories(${CMAKE_PROJECT_NAME}_lib PUBLIC ${CMAKE_SOURCE_DIR}/source)
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC ${CMAKE_PROJECT_NAME}_Base)

# precompiled headers
if (NOT ${PROJECT_PREFIX}_ENABLE_CLANG_TIDY)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
        target_precompile_headers(${CMAKE_PROJECT_NAME}_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
    endif ()
endif ()
set_target_properties(${CMAKE_PROJECT_NAME}_lib PROPERTIES FOLDER "core_libs")

# ----==== third party ====----
# json
find_package(jsoncpp REQUIRED)
message(STATUS "Found jsoncpp: ${jsoncpp_DIR} (found version: ${jsoncpp_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC jsoncpp_static)

# YAML
find_package(yaml-cpp REQUIRED)
message(STATUS "Found yaml-cpp: ${yaml-cpp_DIR} (found version: ${yaml-cpp_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC yaml-cpp::yaml-cpp)

#spdlog
find_package(spdlog REQUIRED)
message(STATUS "Found spdlog: ${spdlog_DIR} (found version: ${spdlog_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC spdlog::spdlog)

#magic enum
find_package(magic_enum REQUIRED)
message(STATUS "Found magic_enum: ${magic_enum_DIR} (found version: ${magic_enum_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC magic_enum::magic_enum)
copy_shared_libraries(${CMAKE_PROJECT_NAME}_lib)
