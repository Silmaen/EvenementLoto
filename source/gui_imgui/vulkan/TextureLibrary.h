/**
 * @file TextureLibrary.h
 * @author Silmaen
 * @date 15/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

namespace evl::gui_imgui::vulkan {

/**
 * @brief Class TextureLibrary.
 */
class TextureLibrary final {
public:
	/**
	 * @brief Default constructor.
	 */
	TextureLibrary();
	/**
	 * @brief Default destructor.
	 */
	~TextureLibrary();

	TextureLibrary(const TextureLibrary&) = delete;
	TextureLibrary(TextureLibrary&&) = delete;
	auto operator=(const TextureLibrary&) -> TextureLibrary& = delete;
	auto operator=(TextureLibrary&&) -> TextureLibrary& = delete;

	/**
	 * @brief Load all textures from a folder.
	 * @param iFolderPath The folder path.
	 */
	void loadFolder(const std::filesystem::path& iFolderPath);

	/**
	 * @brief Load a texture from file.
	 * @param iTexturePath The texture file path.
	 */
	void loadTexture(const std::filesystem::path& iTexturePath);
	/**
	 * @brief Load a texture from file with a given name.
	 * @param iName The texture name.
	 * @param iTexturePath The texture file path.
	 */
	void loadTexture(const std::string& iName, const std::filesystem::path& iTexturePath);
	/**
	 * @brief Get a texture ID by name.
	 * @param iName The texture name.
	 * @return The texture ID.
	 */
	[[nodiscard]] auto getTextureId(const std::string& iName) const -> uint64_t;

	/**
	 * @brief Get or load a texture ID by name.
	 * @param iName The texture name.
	 * @param iTexturePath The texture file path.
	 * @return The texture ID.
	 */
	[[nodiscard]] auto getOrLoadTextureId(const std::string& iName, const std::filesystem::path& iTexturePath)
			-> uint64_t;

	struct Pixels {
		std::vector<uint8_t> data;
		uint32_t width{0};
		uint32_t height{0};
		uint32_t channels{0};
	};
	auto getRawPixels(const std::string& iName) const -> Pixels;

private:
	/// Texture map.
	std::unordered_map<std::string, uint64_t> m_textureMap;
	std::unordered_map<std::string, std::filesystem::path> m_texturePaths;

	/**
	 * @brief Load a texture from SVG file.
	 * @param iName The texture name.
	 * @param iTexturePath The SVG file path.
	 * @param iWidth The output image width in pixels.
	 * @param iHeight The output image height in pixels.
	 */
	void loadSvgTexture(const std::string& iName, const std::filesystem::path& iTexturePath, uint32_t iWidth,
						uint32_t iHeight);
	/**
	 * @brief Load a texture from SVG file.
	 * @param iName The texture name.
	 * @param iTexturePath The SVG file path.
	 */
	void loadImageTexture(const std::string& iName, const std::filesystem::path& iTexturePath);
};

}// namespace evl::gui_imgui::vulkan
