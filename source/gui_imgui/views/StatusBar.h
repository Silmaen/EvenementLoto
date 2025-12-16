/**
 * @file StatusBar.h
 * @author Silmaen
 * @date 15/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"

namespace evl::gui_imgui::views {

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
	 * @brief De fault destructor.
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

private:
};

}// namespace evl::gui_imgui::views
