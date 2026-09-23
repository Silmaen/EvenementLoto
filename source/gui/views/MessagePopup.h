/**
 * @file MessagePopup.h
 * @author Silmaen
 * @date 23/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Popups.h"

namespace evl::gui::views {

/**
 * @brief Tells the organizer something went wrong, on screen.
 *
 * A failure written only to the log is a failure nobody sees during a game. This is for
 * the ones the organizer has to know about right away — a file that would not load, a
 * save that did not go through.
 */
class PopupMessage final : public Popup {
public:
	/// Default constructor.
	PopupMessage();
	/// Default destructor.
	~PopupMessage() override;

	PopupMessage(const PopupMessage&) = delete;
	PopupMessage(PopupMessage&&) = delete;
	auto operator=(const PopupMessage&) -> PopupMessage& = delete;
	auto operator=(PopupMessage&&) -> PopupMessage& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_message"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return m_title; }

	/**
	 * @brief Show the popup with the given content.
	 * @param[in] iTitle The window title.
	 * @param[in] iMessage What to tell the organizer.
	 * @param[in] iDetail A second line, often the path involved. May be empty.
	 */
	void show(const std::string& iTitle, const std::string& iMessage, const std::string& iDetail = {});

private:
	/// The window title.
	std::string m_title{"Information"};
	/// What to tell the organizer.
	std::string m_message;
	/// A second line, often the path involved.
	std::string m_detail;
};

}// namespace evl::gui::views
