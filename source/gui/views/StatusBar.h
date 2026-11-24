/**
 * @file StatusBar.h
 * @author Silmaen
 * @date 15/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"

namespace evl::gui::views {

/**
 * @brief Class StatusBar.
 */
class StatusBar : public View {
public:
	/**
	 * @brief Default constructor.
	 */
	StatusBar();
	/**
	 * @brief Default destructor.
	 */
	~StatusBar() override;

	StatusBar(const StatusBar&) = delete;
	StatusBar(StatusBar&&) = delete;
	auto operator=(const StatusBar&) -> StatusBar& = delete;
	auto operator=(StatusBar&&) -> StatusBar& = delete;
	/**
	 * @brief The update function to implement in derived classes.
	 */
	void onUpdate() override;
	/**
	 * @brief Get the view name.
	 * @return The view name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "status_bar"; }

	/**
	 * @brief The height of the bar: one line of text and the window padding around it.
	 * @return The height in pixels.
	 */
	[[nodiscard]] static auto height() -> float;

private:
};

}// namespace evl::gui::views
