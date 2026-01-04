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

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvgrast.h>

#include <stb_image.h>

namespace evl::gui_imgui::vulkan {

namespace {}// namespace

TextureLibrary::TextureLibrary() = default;

TextureLibrary::~TextureLibrary() { m_textureMap.clear(); }

void TextureLibrary::loadTexture(const std::filesystem::path& iTexturePath) {
	loadTexture(iTexturePath.stem().string(), iTexturePath);
}

void TextureLibrary::loadTexture(const std::string& iName, const std::filesystem::path& iTexturePath) {
	if (const auto ext = iTexturePath.extension().string(); ext == ".svg") {
		loadSvgTexture(iName, iTexturePath, 512, 512);
	} else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
		loadImageTexture(iName, iTexturePath);
	}
}

void TextureLibrary::loadImageTexture(const std::string& iName, const std::filesystem::path& iTexturePath) {
	int width{0};
	int height{0};
	int channels{0};

	unsigned char* imageData = stbi_load(iTexturePath.string().c_str(), &width, &height, &channels, 4);
	if (imageData == nullptr) {
		log_error("Failed to load texture: {} from {}", iName, iTexturePath.string());
		return;
	}

	m_textureMap[iName] =
			VulkanContext::get().loadImage(imageData, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 4);
	m_texturePaths[iName] = iTexturePath;
	stbi_image_free(imageData);
	//log_trace("Loaded texture: {} from {}", iName, iTexturePath.string());
}

void TextureLibrary::loadSvgTexture(const std::string& iName, const std::filesystem::path& iTexturePath,
									const uint32_t iWidth, const uint32_t iHeight) {
	NSVGimage* image = nsvgParseFromFile(iTexturePath.string().c_str(), "px", 96.0f);
	if (image == nullptr) {
		log_error("Failed to load SVG: {} from {}", iName, iTexturePath.string());
		return;
	}

	NSVGrasterizer* rast = nsvgCreateRasterizer();
	if (rast == nullptr) {
		log_error("Failed to create SVG rasterizer");
		nsvgDelete(image);
		return;
	}

	std::vector<unsigned char> imageData(static_cast<size_t>(iWidth * iHeight * 4));
	nsvgRasterize(rast, image, 0, 0, static_cast<float>(iWidth) / image->width, imageData.data(),
				  static_cast<int>(iWidth), static_cast<int>(iHeight), static_cast<int>(iWidth) * 4);

	m_textureMap[iName] = VulkanContext::get().loadImage(imageData.data(), iWidth, iHeight, 4);
	m_texturePaths[iName] = iTexturePath;

	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);
	log_trace("Loaded SVG texture: {} from {}", iName, iTexturePath.string());
}

auto TextureLibrary::getTextureId(const std::string& iName) const -> uint64_t {
	if (const auto it = m_textureMap.find(iName); it != m_textureMap.end()) {
		// Verify that the texture is valid
		if (VulkanContext::get().isTextureValid(it->second))
			return it->second;
	}
	return 0;
}

auto TextureLibrary::getOrLoadTextureId(const std::string& iName, const std::filesystem::path& iTexturePath)
		-> uint64_t {
	if (const auto id = getTextureId(iName); id != 0) {
		return id;
	}
	if (!m_textureMap.contains(iName)) {
		loadTexture(iName, iTexturePath);
	} else {
		if (m_texturePaths.contains(iName) && m_texturePaths.at(iName) != iTexturePath) {
			VulkanContext::get().unloadImage(m_textureMap.at(iName));
			m_textureMap.erase(iName);
			m_texturePaths.erase(iName);
			loadTexture(iName, iTexturePath);
		}
	}
	return getTextureId(iName);
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

auto TextureLibrary::getRawPixels(const std::string& iName) const -> Pixels {
	Pixels result;
	if (const auto it = m_textureMap.find(iName); it != m_textureMap.end()) {
		auto [width, height, channels] = VulkanContext::get().getImageInfo(it->second);
		result.width = width;
		result.height = height;
		result.channels = channels;
		result.data = VulkanContext::get().getImagePixels(it->second);
	} else {
		log_warn("Texture '{}' not found.", iName);
	}
	return result;
}

}// namespace evl::gui_imgui::vulkan
