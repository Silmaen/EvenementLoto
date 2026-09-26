/**
 * @file ReportPopup.h
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Popups.h"

namespace evl::gui::views {

/**
 * @brief Shows the end-of-event report, and lets it be kept.
 *
 * What the organizer needs the morning after: what was played, who won what, what it
 * was worth, and who to thank. The text comes from `core::buildReport`, so what is read
 * here and what is written to disk are the same thing.
 */
class PopupReport final : public Popup {
public:
	/// Default constructor.
	PopupReport();
	/// Default destructor.
	~PopupReport() override;

	PopupReport(const PopupReport&) = delete;
	PopupReport(PopupReport&&) = delete;
	auto operator=(const PopupReport&) -> PopupReport& = delete;
	auto operator=(PopupReport&&) -> PopupReport& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_report"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Rapport de fin d'événement"; }

protected:
	/**
	 * @brief Function called when the popup is opened.
	 */
	void onOpen() override;

private:
	/// The report, built when the popup opens so it does not change under the reader.
	std::string m_report;
};

}// namespace evl::gui::views
