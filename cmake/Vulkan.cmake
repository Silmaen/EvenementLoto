function(target_link_vulkan target_name INCLUDE_TYPE)
    find_package(VulkanHeaders REQUIRED)
    find_package(VulkanLoader REQUIRED)
    get_filename_component(VULKAN_DIR ${VulkanLoader_DIR} DIRECTORY)
    get_filename_component(VULKAN_DIR ${VULKAN_DIR} DIRECTORY)
    message(STATUS "Found Vulkan version ${VulkanLoader_VERSION} @ ${VULKAN_DIR}")
    # Headers must come before the system ones, which are older than the SDK.
    target_link_libraries(${target_name} ${INCLUDE_TYPE} Vulkan::Headers Vulkan::Loader)
endfunction()
