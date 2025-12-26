/**
 * @file GameActions.h
 * @author Silmaen
 * @date 26/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Action.h"

namespace evl::gui_imgui::actions {


/**
 * @brief Class StartGameAction.
 */
class StartGameAction final : public Action {
public:
	StartGameAction();
	~StartGameAction() override;
	StartGameAction(const StartGameAction&) = delete;
	StartGameAction(StartGameAction&&) = delete;
	auto operator=(const StartGameAction&) -> StartGameAction& = delete;
	auto operator=(StartGameAction&&) -> StartGameAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "start_game"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};


/**
 * @brief Class StopGameAction.
 */
class StopGameAction final : public Action {
public:
	StopGameAction();
	~StopGameAction() override;
	StopGameAction(const StopGameAction&) = delete;
	StopGameAction(StopGameAction&&) = delete;
	auto operator=(const StopGameAction&) -> StopGameAction& = delete;
	auto operator=(StopGameAction&&) -> StopGameAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "stop_game"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class GameNextActions.
 */
class GameNextActions final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	GameNextActions();
	/**
	 * @brief Default destructor.
	 */
	~GameNextActions() override;

	GameNextActions(const GameNextActions&) = delete;
	GameNextActions(GameNextActions&&) = delete;
	auto operator=(const GameNextActions&) -> GameNextActions& = delete;
	auto operator=(GameNextActions&&) -> GameNextActions& = delete;

	[[nodiscard]] auto getName() const -> std::string override { return "game_next_step"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

class RandomPickAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	RandomPickAction();
	/**
	 * @brief Default destructor.
	 */
	~RandomPickAction() override;
	RandomPickAction(const RandomPickAction&) = delete;
	RandomPickAction(RandomPickAction&&) = delete;
	auto operator=(const RandomPickAction&) -> RandomPickAction& = delete;
	auto operator=(RandomPickAction&&) -> RandomPickAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "random_pick"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

class CancelPickAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	CancelPickAction();
	/**
	 * @brief Default destructor.
	 */
	~CancelPickAction() override;
	CancelPickAction(const CancelPickAction&) = delete;
	CancelPickAction(CancelPickAction&&) = delete;
	auto operator=(const CancelPickAction&) -> CancelPickAction& = delete;
	auto operator=(CancelPickAction&&) -> CancelPickAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "cancel_pick"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

class DisplayRulesAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	DisplayRulesAction();
	/**
	 * @brief Default destructor.
	 */
	~DisplayRulesAction() override;
	DisplayRulesAction(const DisplayRulesAction&) = delete;
	DisplayRulesAction(DisplayRulesAction&&) = delete;
	auto operator=(const DisplayRulesAction&) -> DisplayRulesAction& = delete;
	auto operator=(DisplayRulesAction&&) -> DisplayRulesAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "display_rules"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

}// namespace evl::gui_imgui::actions
