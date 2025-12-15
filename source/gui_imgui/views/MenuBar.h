/**
 * @file MenuBar.h
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"


namespace evl::gui_imgui::views {

/**
 * @brief Class MenuBar.
 */
class MenuBar final : public View {
public:
	/**
	 * @brief Default constructor.
	 */
	MenuBar();
	/**
	 * @brief Default destructor.
	 */
	~MenuBar() override;

	MenuBar(const MenuBar&) = delete;
	MenuBar(MenuBar&&) = delete;
	auto operator=(const MenuBar&) -> MenuBar& = delete;
	auto operator=(MenuBar&&) -> MenuBar& = delete;

	/**
	 * @brief Update function called each frame.
	 */
	void onUpdate() override;
	/**
	 * @brief The update function to implement in derived classes.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "MenuBar"; }

private:
};

}// namespace evl::gui_imgui::views
