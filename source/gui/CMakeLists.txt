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

target_link_libraries(${CMAKE_PROJECT_NAME}_ui PUBLIC ${CMAKE_PROJECT_NAME}_third_party)

find_package(nfd REQUIRED)
message(STATUS "Found nfd: ${nfd_DIR}")
target_link_libraries(${CMAKE_PROJECT_NAME}_ui PRIVATE nfd::nfd)

target_link_vulkan(${CMAKE_PROJECT_NAME}_ui PUBLIC)
copy_shared_libraries(${CMAKE_PROJECT_NAME}_ui)
