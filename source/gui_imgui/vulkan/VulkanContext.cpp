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

auto isExtensionAvailable(const std::vector<VkExtensionProperties>& iProperties, const char* iExtension) -> bool {
	for (const auto& [extensionName, specVersion]: iProperties)
		if (strcmp(extensionName, iExtension) == 0)
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

auto findMemoryType(const VkData& iVkData, const uint32_t iTypeFilter, const VkMemoryPropertyFlags iProperties)
		-> uint32_t {
	VkPhysicalDeviceMemoryProperties memProperties{};
	vkGetPhysicalDeviceMemoryProperties(iVkData.physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((iTypeFilter & (1 << i)) != 0 &&
			(memProperties.memoryTypes[i].propertyFlags & iProperties) == iProperties) {
			return i;
		}
	}

	log_error("Failed to find suitable memory type");
	return 0;
}

void createBuffer(const VkData& iVkData, const VkDeviceSize iSize, VkBufferUsageFlags iUsage,
				  VkMemoryPropertyFlags iProperties, VkBuffer& oBuffer, VkDeviceMemory& oBufferMemory) {
	const VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
										.pNext = nullptr,
										.flags = 0,
										.size = iSize,
										.usage = iUsage,
										.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
										.queueFamilyIndexCount = 0,
										.pQueueFamilyIndices = nullptr};

	VulkanContext::checkVkResult(vkCreateBuffer(iVkData.device, &bufferInfo, iVkData.allocator, &oBuffer), __FILE__,  __LINE__);

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(iVkData.device, oBuffer, &memRequirements);

	const VkMemoryAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
										 .pNext = nullptr,
										 .allocationSize = memRequirements.size,
										 .memoryTypeIndex =
												 findMemoryType(iVkData, memRequirements.memoryTypeBits, iProperties)};

	VulkanContext::checkVkResult(vkAllocateMemory(iVkData.device, &allocInfo, iVkData.allocator, &oBufferMemory), __FILE__,  __LINE__);
	vkBindBufferMemory(iVkData.device, oBuffer, oBufferMemory, 0);
}

void createImage(const VkData& iVkData, const uint32_t iWidth, const uint32_t iHeight, const VkFormat iFormat,
				 const VkImageTiling iTiling, const VkImageUsageFlags iUsage, const VkMemoryPropertyFlags iProperties,
				 VkImage& oImage, VkDeviceMemory& oImageMemory) {
	const VkImageCreateInfo imageInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
									  .pNext = nullptr,
									  .flags = 0,
									  .imageType = VK_IMAGE_TYPE_2D,
									  .format = iFormat,
									  .extent = {.width = iWidth, .height = iHeight, .depth = 1},
									  .mipLevels = 1,
									  .arrayLayers = 1,
									  .samples = VK_SAMPLE_COUNT_1_BIT,
									  .tiling = iTiling,
									  .usage = iUsage,
									  .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
									  .queueFamilyIndexCount = 0,
									  .pQueueFamilyIndices = nullptr,
									  .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};


	VulkanContext::checkVkResult(vkCreateImage(iVkData.device, &imageInfo, iVkData.allocator, &oImage),__FILE__,  __LINE__);

	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(iVkData.device, oImage, &memRequirements);

	const VkMemoryAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
										 .pNext = nullptr,
										 .allocationSize = memRequirements.size,
										 .memoryTypeIndex =
												 findMemoryType(iVkData, memRequirements.memoryTypeBits, iProperties)};

	VulkanContext::checkVkResult(vkAllocateMemory(iVkData.device, &allocInfo, iVkData.allocator, &oImageMemory), __FILE__,  __LINE__);
	vkBindImageMemory(iVkData.device, oImage, oImageMemory, 0);
}

auto createImageView(const VkData& iVkData, VkImage iImage, const VkFormat iFormat) -> VkImageView {
	const VkImageViewCreateInfo viewInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
										 .pNext = nullptr,
										 .flags = 0,
										 .image = iImage,
										 .viewType = VK_IMAGE_VIEW_TYPE_2D,
										 .format = iFormat,
										 .components = {},
										 .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
															  .baseMipLevel = 0,
															  .levelCount = 1,
															  .baseArrayLayer = 0,
															  .layerCount = 1}};

	VkImageView imageView = VK_NULL_HANDLE;
	VulkanContext::checkVkResult(vkCreateImageView(iVkData.device, &viewInfo, iVkData.allocator, &imageView), __FILE__,  __LINE__);
	return imageView;
}

auto createTextureSampler(const VkData& iVkData) -> VkSampler {
	constexpr VkSamplerCreateInfo samplerInfo{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
											  .pNext = nullptr,
											  .flags = 0,
											  .magFilter = VK_FILTER_LINEAR,
											  .minFilter = VK_FILTER_LINEAR,
											  .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
											  .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
											  .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
											  .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
											  .mipLodBias = 0,
											  .anisotropyEnable = VK_FALSE,
											  .maxAnisotropy = 0,
											  .compareEnable = VK_FALSE,
											  .compareOp = VK_COMPARE_OP_ALWAYS,
											  .minLod = 0,
											  .maxLod = 0,
											  .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
											  .unnormalizedCoordinates = VK_FALSE};

	VkSampler sampler = VK_NULL_HANDLE;
	VulkanContext::checkVkResult(vkCreateSampler(iVkData.device, &samplerInfo, iVkData.allocator, &sampler), __FILE__,  __LINE__);
	return sampler;
}

auto beginSingleTimeCommands(const VkData& iVkData) -> VkCommandBuffer {
	const VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
												.pNext = nullptr,
												.commandPool = iVkData.commandPool,
												.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
												.commandBufferCount = 1};

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	vkAllocateCommandBuffers(iVkData.device, &allocInfo, &commandBuffer);

	constexpr VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
												 .pNext = nullptr,
												 .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
												 .pInheritanceInfo = nullptr};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void endSingleTimeCommands(const VkData& iVkData, VkCommandBuffer iCommandBuffer) {
	vkEndCommandBuffer(iCommandBuffer);

	const VkSubmitInfo submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
								  .pNext = nullptr,
								  .waitSemaphoreCount = 0,
								  .pWaitSemaphores = nullptr,
								  .pWaitDstStageMask = nullptr,
								  .commandBufferCount = 1,
								  .pCommandBuffers = &iCommandBuffer,
								  .signalSemaphoreCount = 0,
								  .pSignalSemaphores = nullptr};

	VulkanContext::checkVkResult(vkQueueSubmit(iVkData.queue, 1, &submitInfo, VK_NULL_HANDLE), __FILE__,  __LINE__);
	VulkanContext::checkVkResult(vkQueueWaitIdle(iVkData.queue), __FILE__,  __LINE__);

	vkFreeCommandBuffers(iVkData.device, iVkData.commandPool, 1, &iCommandBuffer);
}

void transitionImageLayout(const VkData& iVkData, VkImage iImage, const VkImageLayout iOldLayout,
						   const VkImageLayout iNewLayout) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(iVkData);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = iOldLayout;
	barrier.newLayout = iNewLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = iImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;

	if (iOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && iNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (iOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			   iNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (iOldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
			   iNewLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (iOldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
			   iNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		log_error("Unsupported layout transition from {} to {}", magic_enum::enum_name(iOldLayout),
				  magic_enum::enum_name(iNewLayout));
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(iVkData, commandBuffer);
}


void copyBufferToImage(const VkData& iVkData, VkBuffer iBuffer, VkImage iImage, const uint32_t iWidth,
					   const uint32_t iHeight) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(iVkData);

	const VkBufferImageCopy region{.bufferOffset = 0,
								   .bufferRowLength = 0,
								   .bufferImageHeight = 0,
								   .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
														.mipLevel = 0,
														.baseArrayLayer = 0,
														.layerCount = 1},
								   .imageOffset = {.x = 0, .y = 0, .z = 0},
								   .imageExtent = {.width = iWidth, .height = iHeight, .depth = 1}};

	vkCmdCopyBufferToImage(commandBuffer, iBuffer, iImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(iVkData, commandBuffer);
}


}// namespace

VulkanContext::VulkanContext() { m_data = {}; }

VulkanContext::~VulkanContext() { reset(); }


void VulkanContext::init(const std::vector<const char*>& iInstanceExtensions) {
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
		checkVkResult(err, __FILE__,  __LINE__);

		// Enable required extensions
		std::vector<const char*> instanceExtensions = iInstanceExtensions;
		if (isExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
			instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
		if (isExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
			instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
			create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		}
#endif

		// Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
		const std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
		create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
		create_info.ppEnabledLayerNames = layers.data();
		instanceExtensions.push_back("VK_EXT_debug_report");
#endif

		// Create Vulkan Instance
		create_info.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		create_info.ppEnabledExtensionNames = instanceExtensions.data();
		err = vkCreateInstance(&create_info, m_data.allocator, &m_data.instance);
		checkVkResult(err, __FILE__,  __LINE__);
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
		checkVkResult(err, __FILE__,  __LINE__);
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
		if (isExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
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
		checkVkResult(err, __FILE__,  __LINE__);
		vkGetDeviceQueue(m_data.device, m_data.queueFamily, 0, &m_data.queue);
	}

	// Create Descriptor Pool
	// If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
	{
		std::vector<VkDescriptorPoolSize> pool_sizes = {
				{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				 .descriptorCount = std::max(IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE, 200)},
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 0;
		for (const auto& [type, descriptorCount]: pool_sizes) pool_info.maxSets += descriptorCount;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		err = vkCreateDescriptorPool(m_data.device, &pool_info, m_data.allocator, &m_data.descriptorPool);
		checkVkResult(err, __FILE__,  __LINE__);
	}
	// Create Command Pool
	{
		const VkCommandPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
											   .pNext = nullptr,
											   .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
											   .queueFamilyIndex = m_data.queueFamily};
		err = vkCreateCommandPool(m_data.device, &poolInfo, m_data.allocator, &m_data.commandPool);
		checkVkResult(err, __FILE__,  __LINE__);
		log_info("[vulkan] Command pool created.");
	}
	log_info("[vulkan] Vulkan context initialized.");
}

void VulkanContext::reset() {
	log_trace("[vulkan] Resetting Vulkan context.");

	if (m_data.device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(m_data.device);
	}

	for (auto& [image, memory, imageView, sampler, descriptorSet, infos]: m_textures | std::views::values) {
		if (descriptorSet != VK_NULL_HANDLE)
			vkFreeDescriptorSets(m_data.device, m_data.descriptorPool, 1, &descriptorSet);
		if (sampler != VK_NULL_HANDLE)
			vkDestroySampler(m_data.device, sampler, m_data.allocator);
		if (imageView != VK_NULL_HANDLE)
			vkDestroyImageView(m_data.device, imageView, m_data.allocator);
		if (image != VK_NULL_HANDLE)
			vkDestroyImage(m_data.device, image, m_data.allocator);
		if (memory != VK_NULL_HANDLE)
			vkFreeMemory(m_data.device, memory, m_data.allocator);
	}
	m_textures.clear();
	log_trace("[vulkan] All textures destroyed.");

	if (m_data.commandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(m_data.device, m_data.commandPool, m_data.allocator);
		m_data.commandPool = VK_NULL_HANDLE;
		log_trace("[vulkan] Command pool destroyed.");
	}

	if (m_data.descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(m_data.device, m_data.descriptorPool, m_data.allocator);
		m_data.descriptorPool = VK_NULL_HANDLE;
		log_trace("[vulkan] Descriptor pool destroyed.");
	}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
	// Remove the debug report callback
	MVI_DIAG_PUSH
	MVI_DIAG_DISABLE_CLANG("-Wcast-function-type-strict")
	const auto f_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
			vkGetInstanceProcAddr(m_data.instance, "vkDestroyDebugReportCallbackEXT"));
	MVI_DIAG_POP
	f_vkDestroyDebugReportCallbackEXT(m_data.instance, m_data.debugReport, m_data.allocator);
	m_data.debugReport = VK_NULL_HANDLE;
#endif// APP_USE_VULKAN_DEBUG_REPORT

	vkDestroyDevice(m_data.device, m_data.allocator);
	m_data.device = VK_NULL_HANDLE;
	log_trace("[vulkan] Logical device destroyed.");
	vkDestroyInstance(m_data.instance, m_data.allocator);
	m_data.instance = VK_NULL_HANDLE;
	log_trace("[vulkan] Vulkan instance destroyed.");
	m_data.physicalDevice = VK_NULL_HANDLE;
	m_data.queue = VK_NULL_HANDLE;
	m_data.queueFamily = static_cast<uint32_t>(-1);
}

void VulkanContext::checkVkResult(const VkResult iErr, const char* iFile, int iLine) {
	if (iErr == VK_SUCCESS)
		return;
	log_error("[vulkan] Error({}:{}): VkResult = {}", iFile, iLine, magic_enum::enum_name(iErr));
	if (iErr < 0)
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
		checkVkResult(err, __FILE__,  __LINE__);

	const ImGui_ImplVulkanH_Frame* fd = &wd->Frames[static_cast<int>(wd->FrameIndex)];
	{
		err = vkWaitForFences(m_data.device, 1, &fd->Fence, VK_TRUE,
							  UINT64_MAX);// wait indefinitely instead of periodically checking
		checkVkResult(err, __FILE__,  __LINE__);

		err = vkResetFences(m_data.device, 1, &fd->Fence);
		checkVkResult(err, __FILE__,  __LINE__);
	}
	{
		err = vkResetCommandPool(m_data.device, fd->CommandPool, 0);
		checkVkResult(err, __FILE__,  __LINE__);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
		checkVkResult(err, __FILE__,  __LINE__);
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
		checkVkResult(err, __FILE__,  __LINE__);
		err = vkQueueSubmit(m_data.queue, 1, &info, fd->Fence);
		checkVkResult(err, __FILE__,  __LINE__);
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
		checkVkResult(err, __FILE__,  __LINE__);
	wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;// Now we can use the next set of semaphores
}

auto VulkanContext::loadImage(const unsigned char* iImageData, const uint32_t iWidth, const uint32_t iHeight,
							  const uint32_t iChannels) -> uint64_t {
	const VkDeviceSize imageSize = static_cast<VkDeviceSize>(iWidth) * static_cast<VkDeviceSize>(iHeight) * 4;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
	createBuffer(m_data, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(m_data.device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, iImageData, imageSize);
	vkUnmapMemory(m_data.device, stagingBufferMemory);

	TextureData texture{};
	createImage(m_data, iWidth, iHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				texture.image, texture.memory);

	transitionImageLayout(m_data, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(m_data, stagingBuffer, texture.image, iWidth, iHeight);
	transitionImageLayout(m_data, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(m_data.device, stagingBuffer, m_data.allocator);
	vkFreeMemory(m_data.device, stagingBufferMemory, m_data.allocator);

	texture.imageView = createImageView(m_data, texture.image, VK_FORMAT_R8G8B8A8_UNORM);
	texture.sampler = createTextureSampler(m_data);

	texture.descriptorSet =
			ImGui_ImplVulkan_AddTexture(texture.sampler, texture.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	texture.info.width = iWidth;
	texture.info.height = iHeight;
	texture.info.channels = iChannels;

	const auto textureId = reinterpret_cast<uint64_t>(texture.descriptorSet);
	m_textures[textureId] = texture;

	return textureId;
}

auto VulkanContext::getImageInfo(const uint64_t iTextureId) const -> ImageInfo {
	ImageInfo info{};
	if (const auto it = m_textures.find(iTextureId); it != m_textures.end()) {
		info = it->second.info;
	} else {
		log_error("Texture ID {} not found", iTextureId);
	}
	return info;
}

auto VulkanContext::getImagePixels(const uint64_t iTextureId) const -> std::vector<unsigned char> {
	std::vector<unsigned char> pixels;

	const auto it = m_textures.find(iTextureId);
	if (it == m_textures.end()) {
		log_error("Texture ID {} not found", iTextureId);
		return pixels;
	}

	const auto& texture = it->second;
	const auto info = getImageInfo(iTextureId);
	const VkDeviceSize imageSize = static_cast<VkDeviceSize>(info.width) * static_cast<VkDeviceSize>(info.height) * 4;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	createBuffer(m_data, imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(m_data);

	transitionImageLayout(m_data, texture.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	const VkBufferImageCopy region{.bufferOffset = 0,
								   .bufferRowLength = 0,
								   .bufferImageHeight = 0,
								   .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
														.mipLevel = 0,
														.baseArrayLayer = 0,
														.layerCount = 1},
								   .imageOffset = {.x = 0, .y = 0, .z = 0},
								   .imageExtent = {.width = info.width, .height = info.height, .depth = 1}};

	vkCmdCopyImageToBuffer(commandBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1,
						   &region);

	endSingleTimeCommands(m_data, commandBuffer);

	transitionImageLayout(m_data, texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	pixels.resize(imageSize);
	void* data = nullptr;
	vkMapMemory(m_data.device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(pixels.data(), data, imageSize);
	vkUnmapMemory(m_data.device, stagingBufferMemory);

	vkDestroyBuffer(m_data.device, stagingBuffer, m_data.allocator);
	vkFreeMemory(m_data.device, stagingBufferMemory, m_data.allocator);

	return pixels;
}


}// namespace evl::gui_imgui::vulkan
