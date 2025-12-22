/**
 * @file MainView.h
 * @author Silmaen
 * @date 22/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"

namespace evl::gui_imgui::views {

/**
 * @brief Class MainView.
 */
class MainView : public View {
public:
	/**
	 * @brief Default constructor.
	 */
	MainView();
	/**
	 * @brief Default destructor.
	 */
	~MainView() override;

	MainView(const MainView&) = delete;
	MainView(MainView&&) = delete;
	auto operator=(const MainView&) -> MainView& = delete;
	auto operator=(MainView&&) -> MainView& = delete;
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
	void renderLeftPanel();
	void renderRightPanel();
	void renderStatisticsTab();
	void renderEventInfo();
};

}// namespace evl::gui_imgui::views
