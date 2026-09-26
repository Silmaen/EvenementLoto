/**
 * @file GameActions.h
 * @author Silmaen
 * @date 26/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "Action.h"

namespace evl::gui::actions {

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
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
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
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
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

	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "game_next_step"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class RandomPickAction.
 */
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
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "random_pick"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class CancelPickAction.
 */
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
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "cancel_pick"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class DisplayRulesAction.
 */
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
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "display_rules"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class WinnersAction: opens the list of winners, to correct a name.
 */
class WinnersAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	WinnersAction();
	/**
	 * @brief Default destructor.
	 */
	~WinnersAction() override;
	WinnersAction(const WinnersAction&) = delete;
	WinnersAction(WinnersAction&&) = delete;
	auto operator=(const WinnersAction&) -> WinnersAction& = delete;
	auto operator=(WinnersAction&&) -> WinnersAction& = delete;
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "winners"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class QuickGameAction: opens the improvised round dialog.
 */
class QuickGameAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	QuickGameAction();
	/**
	 * @brief Default destructor.
	 */
	~QuickGameAction() override;
	QuickGameAction(const QuickGameAction&) = delete;
	QuickGameAction(QuickGameAction&&) = delete;
	auto operator=(const QuickGameAction&) -> QuickGameAction& = delete;
	auto operator=(QuickGameAction&&) -> QuickGameAction& = delete;
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "quick_game"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class ReportAction: opens the end-of-event report.
 */
class ReportAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	ReportAction();
	/**
	 * @brief Default destructor.
	 */
	~ReportAction() override;
	ReportAction(const ReportAction&) = delete;
	ReportAction(ReportAction&&) = delete;
	auto operator=(const ReportAction&) -> ReportAction& = delete;
	auto operator=(ReportAction&&) -> ReportAction& = delete;
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "report"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class CatalogueAction: opens the prize catalogue and its distribution tool.
 */
class CatalogueAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	CatalogueAction();
	/**
	 * @brief Default destructor.
	 */
	~CatalogueAction() override;
	CatalogueAction(const CatalogueAction&) = delete;
	CatalogueAction(CatalogueAction&&) = delete;
	auto operator=(const CatalogueAction&) -> CatalogueAction& = delete;
	auto operator=(CatalogueAction&&) -> CatalogueAction& = delete;
	/**
	 * @brief Get the Name object.
	 * @return The name.
	 */
	[[nodiscard]] auto getName() const -> std::string override { return "catalogue"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

}// namespace evl::gui::actions
