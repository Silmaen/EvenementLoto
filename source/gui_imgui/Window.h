/**
 * @file Window.h
 * @author Silmaen
 * @date 04/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include <string>

namespace evl::gui_imgui {

/**
 * @brief Class Window.
 */
class Window {
public:
	struct Parameters {
		std::string title{"Owl ImGui Window"};
		uint32_t width{1280};
		uint32_t height{720};
		std::string iconPath;
		bool showOnCreate{true};
	};

	/**
	 * @brief Default constructor.
	 */
	Window() = delete;
	/**
	 * @brief Default destructor.
	 */
	virtual ~Window();

	Window(const Window&) = delete;
	Window(Window&&) = delete;
	auto operator=(const Window&) -> Window& = delete;
	auto operator=(Window&&) -> Window& = delete;

	/**
	 * @brief Default constructor.
	 * @param[in] iParams The window parameters.
	 */
	explicit Window(Parameters iParams);

	/**
	 * @brief Request the window to close.
	 */
	void close();

	/**
	 * @brief Show the window.
	 */
	void show();

	/**
	 * @brief Hide the window.
	 */
	void hide();

	/**
	 * @brief Access to the Native Window.
	 * @return Native window's raw pointer.
	 */
	[[nodiscard]]
	auto getNativeWindow() const -> void* {
		return m_window;
	}

	/**
	 * @brief Get the static name of the window class.
	 * @return The static name.
	 */
	[[nodiscard]]
	virtual auto getStaticName() const -> std::string = 0;

	/**
	 * @brief Function called at Update Time.
	 */
	void onUpdate();

	/**
	 * @brief Render function called each frame.
	 */
	virtual void onRender() {}

	/**
	 * @brief Check if the window is still alive.
	 * @return True if the window is alive.
	 */
	[[nodiscard]]
	auto isAlive() const -> bool { return m_window != nullptr; }
protected:
	// default event handlers
	/**
	 * @brief Event called when the window is requested to close.
	 */
	virtual void onclose() {close();}
	/**
	 * @brief Event called when the window is resized.
	 * @param iWidth New width.
	 * @param iHeight New height.
	 */
	virtual void onResize([[maybe_unused]] int iWidth, [[maybe_unused]] int iHeight) {}
	/**
	 * @brief Event called when a key is pressed.
	 * @param iKey The key code.
	 */
	virtual void onKeyPressed([[maybe_unused]] int iKey) {}
	/**
	 * @brief Event called when a key is released.
	 * @param iKey The key code.
	 */
	virtual void onKeyReleased([[maybe_unused]] int iKey) {}
	/**
	 * @brief Event called when a mouse button is pressed.
	 * @param iButton The button code.
	 */
	virtual void onMouseButtonPressed([[maybe_unused]] int iButton) {}
	/**
	 * @brief Event called when a mouse button is released.
	 * @param iButton The button code.
	 */
	virtual void onMouseButtonReleased([[maybe_unused]] int iButton) {}
	/**
	 * @brief Event called when the mouse is moved.
	 * @param iX New X position.
	 * @param iY New Y position.
	 */
	virtual void onMouseMoved([[maybe_unused]] double iX, [[maybe_unused]] double iY) {}
	/**
	 * @brief Event called when the mouse wheel is scrolled.
	 * @param iXOffset Scroll offset in X direction.
	 * @param iYOffset Scroll offset in Y direction.
	 */
	virtual void onMouseScrolled([[maybe_unused]] double iXOffset, [[maybe_unused]] double iYOffset) {}

	/// Window parameters.
	Parameters m_parameters;
	/// Native window pointer.
	void* m_window;
	/// Is window visible.
	bool m_isVisible{false};
};

}// namespace evl::gui_imgui
