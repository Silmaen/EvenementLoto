
#
# ----==== Base Library ====----
#
file(GLOB_RECURSE
        SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/*.h
        )
add_library(${CMAKE_PROJECT_NAME}_lib ${SRCS})
target_include_directories(${CMAKE_PROJECT_NAME}_lib PUBLIC .)

# ----==== third party ====----
# json
find_package(nlohmann_json 3.10 REQUIRED)
message(STATUS "Found nlohmann")

#fmt
find_package(fmt 8 REQUIRED)
message(STATUS "Found fmt")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC fmt)

#spdlog
find_package(spdlog REQUIRED)
message(STATUS "Found spdlog")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC spdlog)
target_compile_definitions(${CMAKE_PROJECT_NAME}_lib PUBLIC SPDLOG_FMT_EXTERNAL)

#magic enum
target_include_directories(${CMAKE_PROJECT_NAME}_lib PUBLIC ${MAGIC_ENUM_INCLUDE_DIR})
message(STATUS "Found magic_enum in ${MAGIC_ENUM_INCLUDE_DIR}")
#
# ----====  UI Library ====----
#
#  QT dependency
#
include(../cmake/qt.cmake)
#
#  UI lib
#
file(GLOB_RECURSE
        SRCS_UI
        ${CMAKE_CURRENT_SOURCE_DIR}/gui/*
        )
add_library(${CMAKE_PROJECT_NAME}_ui ${SRCS_UI})
target_link_libraries(${CMAKE_PROJECT_NAME}_ui
        ${CMAKE_PROJECT_NAME}_lib
        )
qt_attach_to_target(${CMAKE_PROJECT_NAME}_ui)
#  UI Library
#

#
# ----==== Main Executable ====----
#
file(GLOB_RECURSE
        SRCS_RESOURCE
        resources/*.qrc
        resources/*.rc
        )
add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRCS_RESOURCE})
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
            -Wno-reserved-identifier
            -Wno-missing-prototypes
            )
endif ()
target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_PROJECT_NAME}_lib
        ${CMAKE_PROJECT_NAME}_ui
        )
#  Main Executable
#
