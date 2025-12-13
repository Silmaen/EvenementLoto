/**
 * @file Action.h
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include <utility>

#include "gui_imgui/event/Event.h"
#include "gui_imgui/event/KeyCodes.h"

namespace evl::gui_imgui::actions {

/**
 * @brief Class Action.
 */
class Action {
public:
	/**
	 * @brief Default constructor.
	 */
	Action();
	/**
	 * @brief Default destructor.
	 */
	virtual ~Action();

	Action(const Action&) = delete;
	Action(Action&&) = delete;
	auto operator=(const Action&) -> Action& = delete;
	auto operator=(Action&&) -> Action& = delete;

	/**
	 * @brief Constructor with shortcut.
	 * @param[in] iShortcut The action shortcut.
	 */
	explicit Action(KeyCombination&& iShortcut) : m_shortcut(std::move(iShortcut)) {}

	/**
	 * @brief Check if this action is enabled.
	 * @return True if this action is enabled.
	 */
	[[nodiscard]] auto isEnabled() const -> bool { return m_enabled; }

	/**
	 * @brief Get the name of tha action.
	 * @return The name of the action.
	 */
	[[nodiscard]] virtual auto getName() const -> std::string = 0;

	/**
	 * @brief activate the action.
	 */
	void enable() {
		if (onEnable())
			m_enabled = true;
	}

	/**
	 * @brief deactivate
	 */
	void disable() {
		if (onDisable())
			m_enabled = false;
	}

	/**
	 * @brief Execute the action.
	 */
	void execute() {
		if (!m_enabled)
			return;
		onExecute();
	}

	/**
	 * @brief Get the action shortcut.
	 * @return Action shortcut.
	 */
	[[nodiscard]] virtual auto getShortcut() const -> std::string;

	/**
	 * @brief Event handler.
	 * @param[in,out] ioEvent The Event to react.
	 */
	virtual void onEvent([[maybe_unused]] event::Event& ioEvent);

	/**
	 * @brief Set the action shortcut.
	 * @param iShortcut The new shortcut.
	 */
	void setShortcut(const KeyCombination& iShortcut) { m_shortcut = iShortcut; }

private:
	/// if the action is enabled
	bool m_enabled = true;
	/// The action shortcut
	KeyCombination m_shortcut;

	/**
	 * @brief Callback function to check if the activation is possible.
	 * @return true if activation is possible
	 */
	virtual auto onEnable() -> bool { return true; }
	/**
	 * @brief Callback function to check if the deactivation is possible.
	 * @return true if deactivation is possible.
	 */
	virtual auto onDisable() -> bool { return true; }
	/**
	 * @brief Task to execute by the action.
	 */
	virtual void onExecute() = 0;
};

}// namespace evl::gui_imgui::actions
