/**
 * @file vkData.h
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include <vulkan/vulkan.h>

namespace evl::gui_imgui::vulkan {

/**
 * @brief Struct VkData.
 */
struct VkData {
	/// Allocation callbacks.
	VkAllocationCallbacks* allocator = nullptr;
	/// Vulkan instance.
	VkInstance instance = VK_NULL_HANDLE;
	/// Physical device.
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	/// Logical device.
	VkDevice device = VK_NULL_HANDLE;
	/// Queue family index.
	uint32_t queueFamily = static_cast<uint32_t>(-1);
	/// Queue.
	VkQueue queue = VK_NULL_HANDLE;
	/// Pipeline cache.
	VkPipelineCache pipelineCache = VK_NULL_HANDLE;
	/// Descriptor pool.
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
#ifdef MVI_DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
	/// Debug report callback.
	VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
#endif
};

}// namespace mvi::core::vulkan
