/**
 * @file View.h
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "gui_imgui/event/Event.h"


#include <string>

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
	void show() { m_showWindows = true; }
	/**
	 * @brief Hide the view.
	 */
	void hide() { m_showWindows = false; }
	/**
	 * @brief Check if the view is visible.
	 * @return True if visible.
	 */
	[[nodiscard]] auto isVisible() const -> bool { return m_showWindows; }

	/**
	 * @brief The update function to implement in derived classes.
	 */
	virtual void onUpdate() = 0;

	/**
	 * @brief Access to visibility flag.
	 * @return Reference to visibility flag.
	 */
	auto visibility() -> bool& { return m_showWindows; }

	/**
	 * @brief Get the view name.
	 * @return The view name.
	 */
	[[nodiscard]] virtual auto getName() const -> std::string = 0;

	/**
	 * @brief Event handler.
	 * @param[in,out] ioEvent The Event to react.
	 */
	virtual void onEvent([[maybe_unused]] event::Event& ioEvent) {}

private:
	/// Show windows flag.
	bool m_showWindows = true;
};

}// namespace evl::gui_imgui::views
