/**
 * @file RescuePopup.h
 * @author Silmaen
 * @date 22/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Popups.h"

#include "core/Rescue.h"

namespace evl::gui::views {

/**
 * @brief Offers to resume a game interrupted by a crash or a power loss.
 */
class PopupRescue final : public Popup {
public:
	/// Default constructor.
	PopupRescue();
	/// Default destructor.
	~PopupRescue() override;

	PopupRescue(const PopupRescue&) = delete;
	PopupRescue(PopupRescue&&) = delete;
	auto operator=(const PopupRescue&) -> PopupRescue& = delete;
	auto operator=(PopupRescue&&) -> PopupRescue& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_rescue"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Partie interrompue"; }

	/**
	 * @brief Show the popup for the given interrupted game.
	 * @param[in] iInfo What was found on disk.
	 */
	void propose(const core::RescueInfo& iInfo);

private:
	/// The interrupted game being proposed.
	core::RescueInfo m_info;
};

}// namespace evl::gui::views
