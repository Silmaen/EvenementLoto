/**
 * @file MainWindow.h
 * @author Silmaen
 * @date 05/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Window.h"

namespace evl::gui_imgui {

/**
 * @brief Class MainWindow.
 */
class MainWindow : public Window {
public:
	/**
	 * @brief Default constructor.
	 */
	MainWindow() = delete;
	/**
	 * @brief Default destructor.
	 */
	~MainWindow() override;
	MainWindow(const MainWindow&) = delete;
	MainWindow(MainWindow&&) = delete;
	auto operator=(const MainWindow&) -> MainWindow& = delete;
	auto operator=(MainWindow&&) -> MainWindow& = delete;

	/**
	 * @brief Default constructor.
	 * @param[in] iParams The window parameters.
	 */
	explicit MainWindow(Parameters iParams);
	/**
	 * @brief Get the static name of the window class.
	 * @return The static name.
	 */
	[[nodiscard]]
	auto getStaticName() const -> std::string override {
		return "MainWindow";
	}

private:
	void onRender() override;
};

}// namespace evl::gui_imgui
