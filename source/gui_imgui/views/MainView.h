/**
 * @file MainView.h
 * @author Silmaen
 * @date 22/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"
#include "core/Event.h"
#include "core/maths/vectors.h"

namespace evl::gui_imgui::views {

/**
 * @brief Class MainView.
 */
class MainView final : public View {
public:
	/**
	 * @brief Default constructor.
	 */
	MainView(core::Event&);
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
	void renderLeftPanel() const;
	void renderRightPanel() const;
	void renderStatisticsTab() const;
	void renderEventInfo() const;
	void renderDrawnNumbersTab() const;
	void renderCommandsTab() const;
	void renderBottomPanel();

	core::Event& m_currentEvent;

	enum struct DrawMode : uint8_t { Random, Manual, Both };
	DrawMode m_drawMode = DrawMode::Both;

	// internat size variables
	float m_topBottomSplit = 0.7f;
	float m_leftRightSplit = 0.7f;
	math::vec2 m_lastSize = {0.0f, 0.0f};
	int m_selectedScreen = 0;
};

}// namespace evl::gui_imgui::views
