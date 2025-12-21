/**
 * @file FileDialog.h
 * @author Silmaen
 * @date 20/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

namespace evl::gui_imgui::utils {

/**
 * @brief Class FileDialog.
 */
class FileDialog {
public:
	/**
	 * @brief Open a file dialog to search for an existing file.
	 * @param[in] iFilter Filter to apply during the search.
	 * @return A valid file path or null if canceled.
	 */
	static auto openFile(const std::string& iFilter) -> std::filesystem::path;

	/**
	 * @brief Open a file dialog to define a file to create.
	 * @param[in] iFilter Filter to apply during the search.
	 * @return A valid file path or null if canceled.
	 */
	static auto saveFile(const std::string& iFilter) -> std::filesystem::path;

	/**
	 * @brief Open a dialog to select a folder.
	 * @return A valid folder path or empty if canceled.
	 */
	static auto selectFolder() -> std::filesystem::path;

private:
	/// Last used folder path.
	static std::filesystem::path m_lastPath;
};

}// namespace evl::gui_imgui::utils
