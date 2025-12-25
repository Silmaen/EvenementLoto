/**
 * @file ConfigPopups.h
 * @author Silmaen
 * @date 20/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Popups.h"
#include "core/Event.h"
#include "core/maths/vectors.h"
#include "core/utilities.h"


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

	/**
	 * @brief Function called when the popup is opened.
	 */
	void onOpen() override;

private:
	/**
	 * @brief Structure holding configuration data.
	 */
	struct Data {
		std::filesystem::path dataLocation{core::getExecPath() / "data"};
		float mainScale{0.020f};
		float titleScale{2.0f};
		float shortTextScale{1.4f};
		float longTextScale{0.6f};
		float gridTextScale{0.85f};
		math::vec4 backgroundColor{0.94f, 0.94f, 0.94f, 1.0f};
		math::vec4 gridBackgroundColor{0.94f, 0.94f, 0.94f, 1.0f};
		math::vec4 textColor{0.0f, 0.0f, 0.0f, 1.0f};
		math::vec4 selectedNumberColor{1.0f, 0.44f, 0.0f, 1.0f};
		bool truncatePrice{false};
		int truncatePriceLines{3};
		bool fadeNumbers{true};
		int fadeAmount{3};
		int fadeStrength{0};
	} m_data;
	/**
	 * @brief Convert data to settings.
	 */
	void dataToSettings();
	/**
	 * @brief Convert settings to data.
	 */
	void settingsToData();
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

	/**
	 * @brief Function called when the popup is opened.
	 */
	void onOpen() override;

private:
	/// The event copy.
	core::Event m_event;

	/**
	 * @brief Load data from current event.
	 */
	void fromCurrentEvent();
	/**
	 * @brief Save data to current event.
	 */
	void toCurrentEvent() const;
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

	/**
	 * @brief Function called when the popup is opened.
	 */
	void onOpen() override;

private:
	/// The event copy.
	core::Event m_event;

	void renderFirstColumn();
	void renderSecondColumn();
	void renderThirdColumn();
	void renderResult();
	void addGameRound();
	void deleteGameRound();
	void moveGameRoundUp();
	void moveGameRoundDown();

	size_t m_selectedSubRound = 0;
	size_t m_selectedGameRound = 0;
	/**
	 * @brief Load data from current event.
	 */
	void fromCurrentEvent();
	/**
	 * @brief Save data to current event.
	 */
	void toCurrentEvent() const;
};

}// namespace evl::gui_imgui::views
