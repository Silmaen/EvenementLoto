/**
* @file DisplayView.h
 * @author Silmaen
 * @date 22/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "View.h"
#include "core/Event.h"
#include "core/Log.h"
#include "core/maths/vectors.h"

namespace evl::gui_imgui::views {

/**
 * @brief Class DisplayView - Full screen display for game rounds.
 */
class DisplayView final : public View {
public:
	/**
	 * @brief Default constructor.
	 */
	explicit DisplayView(core::Event&);
	/**
	 * @brief Default destructor.
	 */
	~DisplayView() override;

	DisplayView(const DisplayView&) = delete;
	DisplayView(DisplayView&&) = delete;
	auto operator=(const DisplayView&) -> DisplayView& = delete;
	auto operator=(DisplayView&&) -> DisplayView& = delete;

	/**
	 * @brief The update function to implement in derived classes.
	 */
	void onUpdate() override;

	/**
	 * @brief Get the view name.
	 * @return The view name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "display_window"; }

	/**
	 * @brief Set fullscreen mode.
	 * @param iFullscreen True to set fullscreen mode.
	 */
	void setFullscreen(const bool iFullscreen) { m_fullscreen = iFullscreen; }
	/**
	 * @brief Check if fullscreen mode is enabled.
	 * @return True if fullscreen mode is enabled.
	 */
	[[nodiscard]] auto isFullscreen() const -> bool { return m_fullscreen; }
	/**
	 * @brief Set monitor ID.
	 * @param iMonitor The new monitor ID.
	 */
	void setMonitorNumber(const size_t& iMonitor) {
		log_info("Setting monitor ID to {}", iMonitor);
		m_monitorId = iMonitor;
	}

	/**
	 * @brief Set preview mode.
	 * @param iPreview True to set preview mode.
	 */
	void setPreviewMode(const bool iPreview) {
		log_info("Setting preview mode to {}", iPreview);
		m_previewMode = iPreview;
	}

	/**
	 * @brief Set preview event to render.
	 * @param iEvent The event to render.
	 * @param iRound The round to render.
	 * @param iSubRound The sub-round to render.
	 */
	void setEventToRender(const core::Event& iEvent, const size_t iRound = 0, const size_t iSubRound = 0) {
		m_currentEvent = iEvent;
		m_previewRound = iRound;
		m_previewSubRound = iSubRound;
	}

private:
	void renderRoundReady() const;
	void renderRoundRunning() const;
	void renderRoundEnd() const;
	void renderEventPause() const;
	void renderEventEnd() const;
	void renderEventRules() const;
	void renderEventStart() const;

	void applyCommonStyle() const;

	core::Event& m_currentEvent;
	size_t m_monitorId = 0;
	bool m_fullscreen = true;
	bool m_lastFullscreen = false;
	bool m_previewMode = false;
	size_t m_previewRound = 0;
	size_t m_previewSubRound = 0;
	bool m_customStyle = true;
};

}// namespace evl::gui_imgui::views
