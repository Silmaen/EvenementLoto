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

namespace evl::gui::views {

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
	 * @brief Draw the display content, scaled down, inside the current window.
	 *
	 * What the presenter tab shows: the very same content as the projector, fitted in
	 * the region it is given and keeping the projector's aspect ratio, so the organizer
	 * reads on the control screen what the room is looking at.
	 *
	 * @param iSize The region to fit the miniature into.
	 */
	void renderInline(const math::vec2& iSize);

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
	/**
	 * @brief Draw the content matching the current state, in the current window.
	 */
	void renderContent();

	/**
	 * @brief Apply a font scale, taking the miniature factor into account.
	 * @param iScale The scale asked for by the layout.
	 */
	void setFontScale(float iScale) const;

	/**
	 * @brief Draw a centered title at the top of the region.
	 * @param iTitle The title text.
	 * @param iRegion The region to center it in.
	 * @param iExtraScale An extra factor on the configured title scale.
	 */
	void renderTitle(const std::string& iTitle, const math::vec2& iRegion, float iExtraScale = 1.0f) const;

	void renderRoundReady() const;
	void renderRoundRunning() const;
	void renderRoundEnd() const;
	void renderEventPause();
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
	/// Factor applied to every font scale, below one while drawing the miniature.
	float m_contentScale = 1.0f;
	core::clock::time_point m_diapoChanged;
	size_t m_currentDiapoIndex = 0;
	size_t m_totalDiapoImages = 0;
};

}// namespace evl::gui::views
