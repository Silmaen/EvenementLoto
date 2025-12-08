/**
 * @file MainWindow.h
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Theme.h"

#include "core/maths/vectors.h"
#include <array>

namespace evl::gui_imgui {

/**
 * @brief Struct MainWindowOptions.
 */
struct MainWindowOptions {
	/// Window title.
	std::string title;
	/// Window size.
	math::vec2ui size{1280, 800};
	/// Icon path.
	std::filesystem::path iconPath;
};

/**
 * @brief Class MainWindow.
 */
class MainWindow final {
public:
	/**
	 * @brief Default constructor.
	 */
	MainWindow();
	/**
	 * @brief Default destructor.
	 */
	~MainWindow();

	MainWindow(const MainWindow&) = delete;
	MainWindow(MainWindow&&) = delete;
	auto operator=(const MainWindow&) -> MainWindow& = delete;
	auto operator=(MainWindow&&) -> MainWindow& = delete;

	/**
	 * @brief Detect if the window should close.
	 * @return True if the window should close.
	 */
	[[nodiscard]] auto shouldClose() const -> bool;

	/**
	 * @brief Initialize the window.
	 * @param iOptions The window options.
	 */
	void init(const MainWindowOptions& iOptions);

	/**
	 * @brief Close the window.
	 */
	void close();

	/**
	 * @brief Start a new frame.
	 */
	void newFrame();
	/**
	 * @brief Render the window.
	 * @param iClearColor The clear color.
	 */
	void render(const math::vec4& iClearColor = {0.45f, 0.55f, 0.60f, 1.00f});

	/**
	 * @brief Set the theme.
	 * @param iTheme The theme to set.
	 */
	void setTheme(const Theme& iTheme);

private:
	/// Window options.
	MainWindowOptions m_options{};
	/// Native window pointer.
	void* m_window{};
	/// Swap chain rebuild flag.
	bool m_swapChainRebuild = false;
	/// Current theme.
	Theme m_currentTheme{};
	/// Fonts loaded flag.
	bool m_fontsLoaded = false;
	/// Minimum image count.
	uint32_t m_minImageCount = 2;
	/// Vulkan window setup done flag.
	bool m_windowSetupDone = false;
	/// Setup Vulkan window.
	void setupVulkanWindow(int width, int height);
	/// Cleanup Vulkan window.
	void cleanupVulkanWindow();
};

}// namespace evl::gui_imgui
