/**
 * @file QuickGamePopup.h
 * @author Silmaen
 * @date 26/09/2026
 * Copyright © 2026 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Popups.h"
#include "core/GameRound.h"

namespace evl::gui::views {

/**
 * @brief Adds a round to an event already under way.
 *
 * The afternoon rarely goes as printed: the room asks for one more game, a sponsor
 * turns up with a prize, a round has to be squeezed in before the break. This is that
 * round — type, position and, if there is time, its prizes. Nothing more is required:
 * the lots can stay empty, and a round without lots plays just the same. It is a round
 * like any other, so it is drawn, saved and counted in the end-of-event report exactly
 * like the ones prepared in advance.
 */
class PopupQuickGame final : public Popup {
public:
	/// Default constructor.
	PopupQuickGame();
	/// Default destructor.
	~PopupQuickGame() override;

	PopupQuickGame(const PopupQuickGame&) = delete;
	PopupQuickGame(PopupQuickGame&&) = delete;
	auto operator=(const PopupQuickGame&) -> PopupQuickGame& = delete;
	auto operator=(PopupQuickGame&&) -> PopupQuickGame& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_quick_game"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Partie improvisée"; }

protected:
	/**
	 * @brief Function called when the popup is opened.
	 */
	void onOpen() override;

private:
	/**
	 * @brief Build the round from what the popup holds and insert it.
	 * @return True when the round made it into the event.
	 */
	[[nodiscard]] auto insertRound() const -> bool;

	/// The round being improvised, edited in place so its sub-rounds follow its type.
	core::GameRound m_round{core::GameRound::Type::OneQuine};
	/// Where it goes in the programme.
	uint32_t m_position = 0;
	/// Which sub-round of the round the prize list is being filled for.
	size_t m_selectedSubRound = 0;
};

}// namespace evl::gui::views
