/**
 * @file ToolBar.h
 * @author Silmaen
 * @date 15/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"

namespace evl::gui_imgui::views {

/**
 * @brief Class ToolBar.
 */
class ToolBar : public View {
public:
	/**
	 * @brief Default constructor.
	 */
	ToolBar();
	/**
	 * @brief Default destructor.
	 */
	~ToolBar() override;

	ToolBar(const ToolBar&) = delete;
	ToolBar(ToolBar&&) = delete;
	auto operator=(const ToolBar&) -> ToolBar& = delete;
	auto operator=(ToolBar&&) -> ToolBar& = delete;
	/**
	 * @brief The update function to implement in derived classes.
	 */
	void onUpdate() override;
	/**
	 * @brief Get the view name.
	 * @return The view name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "tool_bar"; }

private:
};

}// namespace evl::gui_imgui::views
