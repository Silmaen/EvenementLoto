/**
 * @file FileActions.h
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
 * @brief Class NewFileAction.
 */
class NewFileAction final : public Action {
public:
	NewFileAction();
	~NewFileAction() override;
	NewFileAction(const NewFileAction&) = delete;
	NewFileAction(NewFileAction&&) = delete;
	auto operator=(const NewFileAction&) -> NewFileAction& = delete;
	auto operator=(NewFileAction&&) -> NewFileAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "new_file"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class LoadFileAction.
 */
class LoadFileAction final : public Action {
public:
	LoadFileAction();
	~LoadFileAction() override;
	LoadFileAction(const LoadFileAction&) = delete;
	LoadFileAction(LoadFileAction&&) = delete;
	auto operator=(const LoadFileAction&) -> LoadFileAction& = delete;
	auto operator=(LoadFileAction&&) -> LoadFileAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "load_file"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class SaveFileAction.
 */
class SaveFileAction final : public Action {
public:
	SaveFileAction();
	~SaveFileAction() override;
	SaveFileAction(const SaveFileAction&) = delete;
	SaveFileAction(SaveFileAction&&) = delete;
	auto operator=(const SaveFileAction&) -> SaveFileAction& = delete;
	auto operator=(SaveFileAction&&) -> SaveFileAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "save_file"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

/**
 * @brief Class SaveAsFileAction.
 */
class SaveAsFileAction final : public Action {
public:
	SaveAsFileAction();
	~SaveAsFileAction() override;
	SaveAsFileAction(const SaveAsFileAction&) = delete;
	SaveAsFileAction(SaveAsFileAction&&) = delete;
	auto operator=(const SaveAsFileAction&) -> SaveAsFileAction& = delete;
	auto operator=(SaveAsFileAction&&) -> SaveAsFileAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "save_file_as"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};


class QuitAction final : public Action {
public:
	QuitAction();
	~QuitAction() override;
	QuitAction(const QuitAction&) = delete;
	QuitAction(QuitAction&&) = delete;
	auto operator=(const QuitAction&) -> QuitAction& = delete;
	auto operator=(QuitAction&&) -> QuitAction& = delete;
	[[nodiscard]] auto getName() const -> std::string override { return "quit_application"; }

private:
	/**
	 * @brief Execute the action.
	 */
	void onExecute() override;
};

}// namespace evl::gui_imgui::actions
