/**
 * @file VulkanContext.h
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "vkData.h"
#include <vector>

namespace evl::gui_imgui::vulkan {

/**
 * @brief Class VulkanContext.
 */
class VulkanContext {
public:
	/**
	 * @brief Default constructor.
	 */
	explicit VulkanContext(std::vector<const char*> iInstanceExtensions);
	/**
	 * @brief Default destructor.
	 */
	virtual ~VulkanContext();

	VulkanContext(const VulkanContext&) = delete;
	VulkanContext(VulkanContext&&) = delete;
	auto operator=(const VulkanContext&) -> VulkanContext& = delete;
	auto operator=(VulkanContext&&) -> VulkanContext& = delete;

	/**
	 * @brief Get the Vulkan Data.
	 * @return The Vulkan data.
	 */
	[[nodiscard]] auto getVkData() const -> const VkData& { return m_data; }

	/**
	 * @brief Check VkResult and log error if any.
	 * @param[in] err The VkResult to check.
	 */
	static void checkVkResult(VkResult err);

	/**
	 * @brief Frame render function.
	 * @param[in,out] iWd The window data.
	 * @param[in] iDrawData The draw data.
	 * @param[out] oRebuildSwapChain Swap chain rebuild flag.
	 */
	void frameRender(void* iWd, void* iDrawData, bool& oRebuildSwapChain) const;

private:
	/// Vulkan data.
	VkData m_data;
};

}// namespace evl::gui_imgui::vulkan
