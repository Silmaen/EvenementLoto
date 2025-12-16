/**
 * @file TextureLibrary.cpp
 * @author Silmaen
 * @date 15/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "TextureLibrary.h"

#define STB_IMAGE_IMPLEMENTATION
#include "VulkanContext.h"
#include "core/Log.h"


#include <backends/imgui_impl_vulkan.h>
#include <stb_image.h>
#include <vulkan/vulkan_core.h>

namespace evl::gui_imgui::vulkan {

namespace {}// namespace

TextureLibrary::TextureLibrary() = default;

TextureLibrary::~TextureLibrary() { m_textureMap.clear(); }

void TextureLibrary::loadTexture(const std::filesystem::path& iTexturePath) {
	loadTexture(iTexturePath.stem().string(), iTexturePath);
}

void TextureLibrary::loadTexture(const std::string& iName, const std::filesystem::path& iTexturePath) {
	int width{0};
	int height{0};
	int channels{0};

	unsigned char* imageData = stbi_load(iTexturePath.c_str(), &width, &height, &channels, 4);
	if (imageData == nullptr) {
		log_error("Failed to load texture: {} from {}", iName, iTexturePath.string());
		return;
	}

	m_textureMap[iName] = VulkanContext::get().loadImage(imageData, width, height);

	stbi_image_free(imageData);
	log_trace("Loaded texture: {} from {}", iName, iTexturePath.string());
}


[[nodiscard]] auto TextureLibrary::getTextureId(const std::string& iName) const -> uint64_t {
	if (const auto it = m_textureMap.find(iName); it != m_textureMap.end()) {
		// Verify that the texture is valid
		if (VulkanContext::get().isTextureValid(it->second))
			return it->second;
	}
	return 0;
}

void TextureLibrary::loadFolder(const std::filesystem::path& iFolderPath) {
	if (!exists(iFolderPath) || !is_directory(iFolderPath)) {
		log_warn("Texture folder '{}' does not exist or is not a directory.", iFolderPath.string());
		return;
	}

	for (const auto& entry: std::filesystem::directory_iterator(iFolderPath)) {
		if (entry.is_regular_file()) {
			if (const auto ext = entry.path().extension().string();
				ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
				loadTexture(entry.path());
			}
		}
	}
}

}// namespace evl::gui_imgui::vulkan
