#
# Third parties we have to compile ourselves: ImGui ships its backends and its
# std::string helper as sources only, and the single-header libraries need one
# translation unit to instantiate them.
#
find_package(imgui REQUIRED)
message(STATUS "Found imgui: ${imgui_DIR} (found version: ${imgui_VERSION})")
find_package(glfw3 REQUIRED)
message(STATUS "Found glfw3: ${glfw3_DIR} (found version: ${glfw3_VERSION})")
find_package(stb REQUIRED)
message(STATUS "Found stb: ${stb_DIR} (found version: ${stb_VERSION})")
find_package(nanosvg REQUIRED)
message(STATUS "Found nanosvg: ${nanosvg_DIR} (found version: ${nanosvg_VERSION})")

set(imguiBindingsDir ${CMAKE_BINARY_DIR}/conan/imgui_bindings)

add_library(${CMAKE_PROJECT_NAME}_third_party STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/implementations.cpp
        ${imguiBindingsDir}/backends/imgui_impl_glfw.cpp
        ${imguiBindingsDir}/backends/imgui_impl_vulkan.cpp
        ${imguiBindingsDir}/imgui_stdlib.cpp
)
target_include_directories(${CMAKE_PROJECT_NAME}_third_party SYSTEM PUBLIC ${imguiBindingsDir})
target_link_libraries(${CMAKE_PROJECT_NAME}_third_party PUBLIC
        imgui::imgui
        glfw
        stb::stb
        nanosvg::nanosvg
)
target_link_vulkan(${CMAKE_PROJECT_NAME}_third_party PUBLIC)
set_target_properties(${CMAKE_PROJECT_NAME}_third_party PROPERTIES FOLDER "third_party" CXX_CLANG_TIDY "")
