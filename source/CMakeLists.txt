
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
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC ${CMAKE_PROJECT_NAME}_Base)

# ----==== third party ====----
# json

#fmt
#find_package(fmt REQUIRED)
#message(STATUS "Found fmt")
#target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC fmt::fmt)

#spdlog
find_package(spdlog REQUIRED)
message(STATUS "Found spdlog")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC spdlog::spdlog)
#target_compile_definitions(${CMAKE_PROJECT_NAME}_lib PUBLIC SPDLOG_FMT_EXTERNAL)

#magic enum
find_package(magic_enum REQUIRED)
message(STATUS "Found magic_enum")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC magic_enum::magic_enum)
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
if (CMAKE_SYSTEM_NAME MATCHES "Windows" AND CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif ()
#  Main Executable
#
