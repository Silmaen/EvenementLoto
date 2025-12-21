/**
 * @file ConfigPopups.h
 * @author Silmaen
 * @date 20/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Popups.h"


namespace evl::gui_imgui::views {

/**
 * @brief Class ConfigPopups.
 */
class MainConfigPopups final : public Popup {
public:
	/**
	 * @brief Default constructor.
	 */
	MainConfigPopups();
	/**
	 * @brief Default destructor.
	 */
	~MainConfigPopups() override;

	MainConfigPopups(const MainConfigPopups&) = delete;
	MainConfigPopups(MainConfigPopups&&) = delete;
	auto operator=(const MainConfigPopups&) -> MainConfigPopups& = delete;
	auto operator=(MainConfigPopups&&) -> MainConfigPopups& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_main_config"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Configuration générale"; }

private:
};

/**
 * @brief Class ConfigPopups.
 */
class EventConfigPopups final : public Popup {
public:
	/**
	 * @brief Default constructor.
	 */
	EventConfigPopups();
	/**
	 * @brief Default destructor.
	 */
	~EventConfigPopups() override;

	EventConfigPopups(const EventConfigPopups&) = delete;
	EventConfigPopups(EventConfigPopups&&) = delete;
	auto operator=(const EventConfigPopups&) -> EventConfigPopups& = delete;
	auto operator=(EventConfigPopups&&) -> EventConfigPopups& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_event_config"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Configuration événement"; }

private:
};

/**
 * @brief Class ConfigPopups.
 */
class GameRoundConfigPopups final : public Popup {
public:
	/**
	 * @brief Default constructor.
	 */
	GameRoundConfigPopups();
	/**
	 * @brief Default destructor.
	 */
	~GameRoundConfigPopups() override;

	GameRoundConfigPopups(const GameRoundConfigPopups&) = delete;
	GameRoundConfigPopups(GameRoundConfigPopups&&) = delete;
	auto operator=(const GameRoundConfigPopups&) -> GameRoundConfigPopups& = delete;
	auto operator=(GameRoundConfigPopups&&) -> GameRoundConfigPopups& = delete;

	/**
	 * @brief Function called at Update Time.
	 */
	void onPopupUpdate() override;

	/**
	 * @brief Get the name of the view.
	 * @return The name of the view.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "popup_game_round_config"; }

	/**
	 * @brief Get the popup title.
	 * @return The popup title.
	 */
	[[nodiscard]] auto getPopupTitle() const -> std::string override { return "Configuration parties"; }

private:
};

}// namespace evl::gui_imgui::views
