
#
# ----==== Base Library ====----
#
file(GLOB_RECURSE
        SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/basedefine.h
)
add_library(${CMAKE_PROJECT_NAME}_lib ${SRCS})
target_include_directories(${CMAKE_PROJECT_NAME}_lib PUBLIC .)
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
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PRIVATE spdlog::spdlog)

#magic enum
find_package(magic_enum REQUIRED)
message(STATUS "Found magic_enum: ${magic_enum_DIR} (found version: ${magic_enum_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_lib PUBLIC magic_enum::magic_enum)
#
# ----====  UI Libraries ====----
#
#  QT dependency
#
include(../cmake/qt.cmake)
#
#  QT UI lib
#
file(GLOB_RECURSE
        SRCS_UI_QT
        ${CMAKE_CURRENT_SOURCE_DIR}/gui_qt/*
)
add_library(${CMAKE_PROJECT_NAME}_ui_qt ${SRCS_UI_QT})
target_link_libraries(${CMAKE_PROJECT_NAME}_ui_qt
        ${CMAKE_PROJECT_NAME}_lib
)
# precompiled headers
if (NOT ${PROJECT_PREFIX}_ENABLE_CLANG_TIDY)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
        target_precompile_headers(${CMAKE_PROJECT_NAME}_ui_qt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
    endif ()
endif ()
set_target_properties(${CMAKE_PROJECT_NAME}_ui_qt PROPERTIES FOLDER "core_libs")
qt_attach_to_target(${CMAKE_PROJECT_NAME}_ui_qt)
#  QT UI Library
#

#
#  ImGUI UI lib
#
file(GLOB_RECURSE
        SRCS_UI_IMGUI
        ${CMAKE_CURRENT_SOURCE_DIR}/gui_imgui/*
)
add_library(${CMAKE_PROJECT_NAME}_ui_imgui ${SRCS_UI_IMGUI})
target_link_libraries(${CMAKE_PROJECT_NAME}_ui_imgui PUBLIC
        ${CMAKE_PROJECT_NAME}_lib
)
# precompiled headers
if (NOT ${PROJECT_PREFIX}_ENABLE_CLANG_TIDY)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
        target_precompile_headers(${CMAKE_PROJECT_NAME}_ui_imgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
    endif ()
endif ()
set_target_properties(${CMAKE_PROJECT_NAME}_ui_imgui PROPERTIES FOLDER "core_libs")

find_package(ImGui REQUIRED)
message(STATUS "Found ImGui: ${ImGui_DIR} (found version: ${ImGui_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui_imgui PRIVATE imgui::imgui)

find_package(glfw3 REQUIRED)
message(STATUS "Found glfw3: ${glfw3_DIR} (found version: ${glfw3_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui_imgui PRIVATE glfw)

find_package(stb_image REQUIRED)
message(STATUS "Found stb_image: ${stb_image_DIR} (found version: ${stb_image_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui_imgui PRIVATE stb_image::stb_image)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}_ui_imgui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/darkicons"
        "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/resources/darkicons"
        COMMENT "Copying Icon resources..."
        VERBATIM
)

#  ImGUI UI Library
#
target_link_vulkan(${CMAKE_PROJECT_NAME}_ui_imgui PUBLIC)
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
        ${CMAKE_PROJECT_NAME}_ui_qt
        ${CMAKE_PROJECT_NAME}_ui_imgui
)
if (CMAKE_SYSTEM_NAME MATCHES "Windows" AND CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif ()
#  Main Executable
#
