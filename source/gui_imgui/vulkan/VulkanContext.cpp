/**
 * @file VulkanContext.cpp
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "VulkanContext.h"
#include "core/Log.h"
#include "core/defines.h"
#include "gui_imgui/Application.h"

#include <backends/imgui_impl_vulkan.h>

namespace evl::gui_imgui::vulkan {

namespace {

auto IsExtensionAvailable(const std::vector<VkExtensionProperties>& properties, const char* extension) -> bool {
	for (const auto& [extensionName, specVersion]: properties)
		if (strcmp(extensionName, extension) == 0)
			return true;
	return false;
}


#ifdef APP_USE_VULKAN_DEBUG_REPORT
VKAPI_ATTR auto VKAPI_CALL debug_report(VkDebugReportFlagsEXT, const VkDebugReportObjectTypeEXT objectType, uint64_t,
										size_t, int32_t, const char*, const char* pMessage, void*) -> VkBool32 {
	log_error("[vulkan] Debug report from ObjectType: %i\nMessage: %s\n", magic_enum::enum_name(objectType), pMessage);
	return VK_FALSE;
}
#endif// APP_USE_VULKAN_DEBUG_REPORT
}// namespace

VulkanContext::VulkanContext(std::vector<const char*> iInstanceExtensions) {
	VkResult err = VK_SUCCESS;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
	volkInitialize();
#endif

	// Create Vulkan Instance
	{
		VkInstanceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		// Enumerate available extensions
		uint32_t properties_count = 0;
		std::vector<VkExtensionProperties> properties;
		vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
		properties.resize(properties_count);
		err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());
		checkVkResult(err);

		// Enable required extensions
		if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
			iInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
		if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
			iInstanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
			create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		}
#endif

		// Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
		const std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
		create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
		create_info.ppEnabledLayerNames = layers.data();
		iInstanceExtensions.push_back("VK_EXT_debug_report");
#endif

		// Create Vulkan Instance
		create_info.enabledExtensionCount = static_cast<uint32_t>(iInstanceExtensions.size());
		create_info.ppEnabledExtensionNames = iInstanceExtensions.data();
		err = vkCreateInstance(&create_info, m_data.allocator, &m_data.instance);
		checkVkResult(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
		volkLoadInstance(g_Instance);
#endif

		// Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
		MVI_DIAG_PUSH
		MVI_DIAG_DISABLE_CLANG("-Wcast-function-type-strict")
		auto f_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(m_data.instance, "vkCreateDebugReportCallbackEXT"));
		MVI_DIAG_POP
		assert(f_vkCreateDebugReportCallbackEXT != nullptr);
		VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
		debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
								VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		debug_report_ci.pfnCallback = debug_report;
		debug_report_ci.pUserData = nullptr;
		err = f_vkCreateDebugReportCallbackEXT(m_data.instance, &debug_report_ci, m_data.allocator,
											   &m_data.debugReport);
		checkVkResult(err);
#endif
	}

	// Select Physical Device (GPU)
	m_data.physicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(m_data.instance);
	assert(m_data.physicalDevice != VK_NULL_HANDLE);

	// Select graphics queue family
	m_data.queueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(m_data.physicalDevice);
	assert(std::cmp_not_equal(m_data.queueFamily, -1));

	// Create Logical Device (with 1 queue)
	{
		std::vector<const char*> device_extensions;
		device_extensions.push_back("VK_KHR_swapchain");

		// Enumerate physical device extension
		uint32_t properties_count = 0;
		std::vector<VkExtensionProperties> properties;
		vkEnumerateDeviceExtensionProperties(m_data.physicalDevice, nullptr, &properties_count, nullptr);
		properties.resize(properties_count);
		vkEnumerateDeviceExtensionProperties(m_data.physicalDevice, nullptr, &properties_count, properties.data());
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
		if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
			device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

		const std::vector<float> queue_priority = {1.0f};
		VkDeviceQueueCreateInfo queue_info[1] = {};
		queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[0].queueFamilyIndex = m_data.queueFamily;
		queue_info[0].queueCount = static_cast<uint32_t>(queue_priority.size());
		queue_info[0].pQueuePriorities = queue_priority.data();
		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = std::size(queue_info);
		create_info.pQueueCreateInfos = queue_info;
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		create_info.ppEnabledExtensionNames = device_extensions.data();
		err = vkCreateDevice(m_data.physicalDevice, &create_info, m_data.allocator, &m_data.device);
		checkVkResult(err);
		vkGetDeviceQueue(m_data.device, m_data.queueFamily, 0, &m_data.queue);
	}

	// Create Descriptor Pool
	// If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
	{
		std::vector<VkDescriptorPoolSize> pool_sizes = {
				{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				 .descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 0;
		for (const auto& [type, descriptorCount]: pool_sizes) pool_info.maxSets += descriptorCount;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		err = vkCreateDescriptorPool(m_data.device, &pool_info, m_data.allocator, &m_data.descriptorPool);
		checkVkResult(err);
	}
}

VulkanContext::~VulkanContext() {

	vkDestroyDescriptorPool(m_data.device, m_data.descriptorPool, m_data.allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
	// Remove the debug report callback
	MVI_DIAG_PUSH
	MVI_DIAG_DISABLE_CLANG("-Wcast-function-type-strict")
	const auto f_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
			vkGetInstanceProcAddr(m_data.instance, "vkDestroyDebugReportCallbackEXT"));
	MVI_DIAG_POP
	f_vkDestroyDebugReportCallbackEXT(m_data.instance, m_data.debugReport, m_data.allocator);
#endif// APP_USE_VULKAN_DEBUG_REPORT

	vkDestroyDevice(m_data.device, m_data.allocator);
	vkDestroyInstance(m_data.instance, m_data.allocator);
}

void VulkanContext::checkVkResult(const VkResult err) {
	if (err == VK_SUCCESS)
		return;
	log_error("[vulkan] Error: VkResult = %d", magic_enum::enum_name(err));
	if (err < 0)
		Application::get().reportError("Vulkan encountered a fatal error.");
}


void VulkanContext::frameRender(void* iWd, void* iDrawData, bool& oRebuildSwapChain) const {
	auto* wd = static_cast<ImGui_ImplVulkanH_Window*>(iWd);
	auto* draw_data = static_cast<ImDrawData*>(iDrawData);
	VkSemaphore image_acquired_semaphore =
			wd->FrameSemaphores[static_cast<int>(wd->SemaphoreIndex)].ImageAcquiredSemaphore;
	VkSemaphore render_complete_semaphore =
			wd->FrameSemaphores[static_cast<int>(wd->SemaphoreIndex)].RenderCompleteSemaphore;
	VkResult err = vkAcquireNextImageKHR(m_data.device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore,
										 VK_NULL_HANDLE, &wd->FrameIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		oRebuildSwapChain = true;
	if (err == VK_ERROR_OUT_OF_DATE_KHR)
		return;
	if (err != VK_SUBOPTIMAL_KHR)
		checkVkResult(err);

	const ImGui_ImplVulkanH_Frame* fd = &wd->Frames[static_cast<int>(wd->FrameIndex)];
	{
		err = vkWaitForFences(m_data.device, 1, &fd->Fence, VK_TRUE,
							  UINT64_MAX);// wait indefinitely instead of periodically checking
		checkVkResult(err);

		err = vkResetFences(m_data.device, 1, &fd->Fence);
		checkVkResult(err);
	}
	{
		err = vkResetCommandPool(m_data.device, fd->CommandPool, 0);
		checkVkResult(err);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
		checkVkResult(err);
	}
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = wd->RenderPass;
		info.framebuffer = fd->Framebuffer;
		info.renderArea.extent.width = static_cast<uint32_t>(wd->Width);
		info.renderArea.extent.height = static_cast<uint32_t>(wd->Height);
		info.clearValueCount = 1;
		info.pClearValues = &wd->ClearValue;
		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

	// Submit command buffer
	vkCmdEndRenderPass(fd->CommandBuffer);
	{
		constexpr VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &image_acquired_semaphore;
		info.pWaitDstStageMask = &wait_stage;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &fd->CommandBuffer;
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &render_complete_semaphore;

		err = vkEndCommandBuffer(fd->CommandBuffer);
		checkVkResult(err);
		err = vkQueueSubmit(m_data.queue, 1, &info, fd->Fence);
		checkVkResult(err);
	}
	if (oRebuildSwapChain)
		return;
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &render_complete_semaphore;
	info.swapchainCount = 1;
	info.pSwapchains = &wd->Swapchain;
	info.pImageIndices = &wd->FrameIndex;
	err = vkQueuePresentKHR(m_data.queue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		oRebuildSwapChain = true;
	if (err == VK_ERROR_OUT_OF_DATE_KHR)
		return;
	if (err != VK_SUBOPTIMAL_KHR)
		checkVkResult(err);
	wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;// Now we can use the next set of semaphores
}

}// namespace evl::gui_imgui::vulkan
