/**
 * @file SettingsActions.h
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#pragma once

#include "Action.h"
#include "core/Log.h"

namespace evl::gui_imgui::actions {

/**
 * @brief Class PreferencesAction.
 */
class PreferencesAction final : public Action {
public:
	PreferencesAction();
	~PreferencesAction() override;
	PreferencesAction(const PreferencesAction&) = delete;
	PreferencesAction(PreferencesAction&&) = delete;
	auto operator=(const PreferencesAction&) -> PreferencesAction& = delete;
	auto operator=(PreferencesAction&&) -> PreferencesAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "preferences"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};
/**
 * @brief Class EventSettingsAction.
 */
class EventSettingsAction final : public Action {
public:
	EventSettingsAction();
	~EventSettingsAction() override;
	EventSettingsAction(const EventSettingsAction&) = delete;
	EventSettingsAction(EventSettingsAction&&) = delete;
	auto operator=(const EventSettingsAction&) -> EventSettingsAction& = delete;
	auto operator=(EventSettingsAction&&) -> EventSettingsAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "event_settings"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class GameSettingsAction.
 */
class GameSettingsAction final : public Action {
public:
	GameSettingsAction();
	~GameSettingsAction() override;
	GameSettingsAction(const GameSettingsAction&) = delete;
	GameSettingsAction(GameSettingsAction&&) = delete;
	auto operator=(const GameSettingsAction&) -> GameSettingsAction& = delete;
	auto operator=(GameSettingsAction&&) -> GameSettingsAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "game_settings"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class ThemeSettingsAction.
 */
class ThemeSettingsAction final : public Action {
public:
	ThemeSettingsAction();
	~ThemeSettingsAction() override;
	ThemeSettingsAction(const ThemeSettingsAction&) = delete;
	ThemeSettingsAction(ThemeSettingsAction&&) = delete;
	auto operator=(const ThemeSettingsAction&) -> ThemeSettingsAction& = delete;
	auto operator=(ThemeSettingsAction&&) -> ThemeSettingsAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "theme_settings"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

}// namespace evl::gui_imgui::actions
