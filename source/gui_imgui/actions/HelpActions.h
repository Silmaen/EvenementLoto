/**
 * @file HelpActions.h
 * @author Silmaen
 * @date 17/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Action.h"
#include "core/Log.h"

namespace evl::gui_imgui::actions {

/**
 * @brief Class HelpActions.
 */
class HelpAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	HelpAction();
	/**
	 * @brief Default destructor.
	 */
	~HelpAction() override;

	HelpAction(const HelpAction&) = delete;
	HelpAction(HelpAction&&) = delete;
	auto operator=(const HelpAction&) -> HelpAction& = delete;
	auto operator=(HelpAction&&) -> HelpAction& = delete;

	[[nodiscard]] auto getName() const -> std::string override { return "help"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class HelpActions.
 */
class AboutAction final : public Action {
public:
	/**
	 * @brief Default constructor.
	 */
	AboutAction();
	/**
	 * @brief Default destructor.
	 */
	~AboutAction() override;

	AboutAction(const AboutAction&) = delete;
	AboutAction(AboutAction&&) = delete;
	auto operator=(const AboutAction&) -> AboutAction& = delete;
	auto operator=(AboutAction&&) -> AboutAction& = delete;

	[[nodiscard]] auto getName() const -> std::string override { return "about"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};


}// namespace evl::gui_imgui::actions
