/**
 * @file WinnerPopups.h
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Popups.h"

#include <random>
#include <string>
#include <vector>

namespace evl::gui::views {

/**
 * @brief Asks who won the running sub-round, and settles a tie.
 *
 * The winner used to be a placeholder written straight into the game. Here the
 * organizer types the claimants as the room calls them out: one name is the ordinary
 * case, several means a tie, settled either by picking one or by drawing lots. Skipping
 * is allowed too — a quine nobody claims must not block the afternoon.
 */
class PopupWinner final : public Popup {
public:
	/// Default constructor.
	PopupWinner();
	/// Default destructor.
	~PopupWinner() override;

	PopupWinner(const PopupWinner&) = delete;
	PopupWinner(PopupWinner&&) = delete;
	auto operator=(const PopupWinner&) -> PopupWinner& = delete;
	auto operator=(PopupWinner&&) -> PopupWinner& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_winner"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Gagnant de la manche"; }

protected:
	/**
	 * @brief Function called when the popup is opened.
	 */
	void onOpen() override;

private:
	/**
	 * @brief Record the chosen name in the game and move on.
	 * @param iWinner The name to record, empty to skip the step.
	 */
	static void settle(const std::string& iWinner);

	/// What the room called out, one entry per claimant.
	std::vector<std::string> m_claimants;
	/// Which claimant carries the prize, an index in the list above.
	size_t m_selected = 0;
	/// Drawing lots between tied claimants.
	std::mt19937 m_rng{std::random_device{}()};
};

/**
 * @brief Lists every winner of the event, and lets them be corrected.
 *
 * A name typed while the room is waiting is a name typed too fast. This is where it gets
 * fixed, after the round and up to the end of the event, without replaying anything.
 */
class PopupWinners final : public Popup {
public:
	/// Default constructor.
	PopupWinners();
	/// Default destructor.
	~PopupWinners() override;

	PopupWinners(const PopupWinners&) = delete;
	PopupWinners(PopupWinners&&) = delete;
	auto operator=(const PopupWinners&) -> PopupWinners& = delete;
	auto operator=(PopupWinners&&) -> PopupWinners& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_winners"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Gagnants"; }
};

}// namespace evl::gui::views
