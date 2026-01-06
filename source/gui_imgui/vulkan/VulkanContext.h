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
 * @brief Struct ImageInfo.
 */
struct ImageInfo {
	/// Image width.
	uint32_t width = 0;
	/// Image height.
	uint32_t height = 0;
	/// Number of channels.
	uint32_t channels = 4;
};

/**
 * @brief Struct TextureData.
 */
struct TextureData {
	/// Image handle.
	VkImage image = VK_NULL_HANDLE;
	/// Image memory.
	VkDeviceMemory memory = VK_NULL_HANDLE;
	/// Image view.
	VkImageView imageView = VK_NULL_HANDLE;
	/// Sampler.
	VkSampler sampler = VK_NULL_HANDLE;
	/// Descriptor set.
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	/// Image info.
	ImageInfo info;
};


/**
 * @brief Class VulkanContext.
 */
class VulkanContext final {
public:
	/**
	 * @brief Default destructor.
	 */
	~VulkanContext();

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
	 * @param[in] iErr The VkResult to check.
	 * @param[in] iFile The calling file.
	 * @param[in] iLine The calling line.
	 */
	static void checkVkResult(VkResult iErr, const char* iFile = __FILE__, int iLine = __LINE__);

	/**
	 * @brief Frame render function.
	 * @param[in,out] iWd The window data.
	 * @param[in] iDrawData The draw data.
	 * @param[out] oRebuildSwapChain Swap chain rebuild flag.
	 */
	void frameRender(void* iWd, void* iDrawData, bool& oRebuildSwapChain) const;

	/**
	 * @brief Set required instance extensions.
	 * @param iInstanceExtensions The extensions list.
	 */
	void init(const std::vector<const char*>& iInstanceExtensions);

	/**
	 * @brief Reset the Vulkan context.
	 */
	void reset();

	/**
	 * @brief Access to VulkanContext instance.
	 * @return The VulkanContext instance.
	 */
	static auto get() -> VulkanContext& {
		static VulkanContext instance;
		return instance;
	}

	/**
	 * @brief Load an image from memory.
	 * @param iImageData The image data.
	 * @param iWidth The image width.
	 * @param iHeight The image height.
	 * @param iChannels The number of channels.
	 * @return The image ID.
	 */
	[[nodiscard]] auto loadImage(const unsigned char* iImageData, uint32_t iWidth, uint32_t iHeight, uint32_t iChannels)
			-> uint64_t;

	/**
	 * @brief Check if a texture ID is valid.
	 * @param iTextureId The texture ID.
	 * @return True if the texture ID is valid.
	 */
	[[nodiscard]] auto isTextureValid(const uint64_t iTextureId) const -> bool {
		return m_textures.contains(iTextureId);
	}

	/**
	 * @brief Get image information.
	 * @param iTextureId The texture ID.
	 * @return The image information.
	 */
	[[nodiscard]] auto getImageInfo(uint64_t iTextureId) const -> ImageInfo;

	/**
	 * @brief Get image pixel data.
	 * @param iTextureId The texture ID.
	 * @return The image pixel data.
	 */
	[[nodiscard]] auto getImagePixels(uint64_t iTextureId) const -> std::vector<unsigned char>;

	/**
	 * @brief Unload an image.
	 * @param iTextureId The texture ID.
	 */
	void unloadImage(uint64_t iTextureId);

private:
	/**
	 * @brief Default constructor.
	 */
	explicit VulkanContext();
	/// Vulkan data.
	VkData m_data;
	/// Loaded textures.
	std::unordered_map<uint64_t, TextureData> m_textures;
};

}// namespace evl::gui_imgui::vulkan
