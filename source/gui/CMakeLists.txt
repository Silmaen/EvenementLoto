file(GLOB_RECURSE
        SRCS_UI
        ${CMAKE_CURRENT_SOURCE_DIR}/*
)

add_library(${CMAKE_PROJECT_NAME}_ui ${SRCS_UI})

target_link_libraries(${CMAKE_PROJECT_NAME}_ui PUBLIC
        ${CMAKE_PROJECT_NAME}_lib
)
add_dependencies(${CMAKE_PROJECT_NAME}_ui ${CMAKE_PROJECT_NAME}_resource)

# precompiled headers
if (NOT ${PROJECT_PREFIX}_ENABLE_CLANG_TIDY)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
        target_precompile_headers(${CMAKE_PROJECT_NAME}_ui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)
    endif ()
endif ()
set_target_properties(${CMAKE_PROJECT_NAME}_ui PROPERTIES FOLDER "core_libs")

find_package(ImGui REQUIRED)
message(STATUS "Found ImGui: ${ImGui_DIR} (found version: ${ImGui_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui PRIVATE imgui::imgui)

find_package(glfw3 REQUIRED)
message(STATUS "Found glfw3: ${glfw3_DIR} (found version: ${glfw3_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui PRIVATE glfw)

find_package(stb_image REQUIRED)
message(STATUS "Found stb_image: ${stb_image_DIR} (found version: ${stb_image_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui PRIVATE stb_image::stb_image)

find_package(NanoSVG REQUIRED)
message(STATUS "Found nanosvg: ${nanosvg_DIR} (found version: ${nanosvg_VERSION})")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui PRIVATE NanoSVG::nanosvg)

find_package(nfd REQUIRED)
message(STATUS "Found nfd: ${nfd_DIR}")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui PRIVATE nfd::nfd)

target_link_vulkan(${CMAKE_PROJECT_NAME}_ui PUBLIC)
copy_shared_libraries(${CMAKE_PROJECT_NAME}_ui)
