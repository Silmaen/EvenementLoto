/**
 * @file View.h
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

namespace evl::gui_imgui::views {

/**
 * @brief Class View.
 */
class View {
public:
	/**
	 * @brief Default constructor.
	 */
	View();
	/**
	 * @brief Default destructor.
	 */
	virtual ~View();

	View(const View&) = delete;
	View(View&&) = delete;
	auto operator=(const View&) -> View& = delete;
	auto operator=(View&&) -> View& = delete;

	/**
	 * @brief Update function called each frame.
	 */
	void update();

	/**
	 * @brief Show the view.
	 */
	void show() { m_show_windows = true; }
	/**
	 * @brief Hide the view.
	 */
	void hide() { m_show_windows = false; }
	/**
	 * @brief Check if the view is visible.
	 * @return True if visible.
	 */
	[[nodiscard]] auto isVisible() const -> bool { return m_show_windows; }

	/**
	 * @brief The update function to implement in derived classes.
	 */
	virtual void onUpdate() = 0;

	/**
	 * @brief Access to visibility flag.
	 * @return Reference to visibility flag.
	 */
	auto visibility() -> bool& { return m_show_windows; }

private:
	/// Show windows flag.
	bool m_show_windows = true;
};

}// namespace evl::gui_imgui::views
