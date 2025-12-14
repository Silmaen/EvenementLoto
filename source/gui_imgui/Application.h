/**
 * @file Application.h
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

#include "MainWindow.h"
#include "actions/Action.h"
#include "event/KeyCodes.h"
#include "views/View.h"

#include <list>
#include <memory>

namespace evl::gui_imgui {

/**
 * @brief Class Application.
 */
class Application final {
public:
	/**
	 * @brief Default constructor.
	 */
	Application();
	/**
	 * @brief Default destructor.
	 */
	~Application();

	Application(const Application&) = delete;
	Application(Application&&) = delete;
	auto operator=(const Application&) -> Application& = delete;
	auto operator=(Application&&) -> Application& = delete;

	/**
	 * @brief Access to Application instance.
	 * @return Single instance of application.
	 */
	static auto get() -> Application& { return *m_instance; }

	/**
	 * @brief Only check for app existence.
	 * @return True if application is instanced.
	 */
	static auto instanced() -> bool { return m_instance != nullptr; }

	/**
	 * @brief Application states.
	 */
	enum class State : uint8_t {
		Created,///< The application is created.
		Running,///< The application is running.
		Waiting,///< The application is waiting.
		Closed,///< The application is closed.
		Error,///< The application is in error state.
	};

	/**
	 * @brief Get the application state.
	 * @return The current state.
	 */
	[[nodiscard]]
	auto getState() const -> const State& {
		return m_state;
	}

	void run();

	/**
	 * @brief Request Error report.
	 * @param[in] iMessage The error message.
	 */
	void reportError(const std::string& iMessage);

	void setWaiting() {
		if (m_state == State::Running)
			m_state = State::Waiting;
	}
	void setRunning() {
		if (m_state == State::Waiting)
			m_state = State::Running;
	}

	/**
	 * @brief Get the application theme.
	 * @return The current theme.
	 */
	[[nodiscard]] auto getTheme() const -> const Theme&;
	/**
	 * @brief Set the application theme.
	 * @param[in] iTheme The new theme.
	 */
	void setTheme(const Theme& iTheme);

	/**
	 * @brief Request application close.
	 */
	void requestClose();

	/**
	 * @brief Get a view by name.
	 * @param iName The view name.
	 * @return The view pointer or nullptr if not found.
	 */
	[[nodiscard]] auto getView(const std::string& iName) const -> std::shared_ptr<views::View> {
		for (const auto& view: m_views) {
			if (view->getName() == iName)
				return view;
		}
		return nullptr;
	}

	/**
	 * @brief Get an action by name.
	 * @param iName The action name.
	 * @return The action pointer or nullptr if not found.
	 */
	[[nodiscard]] auto getAction(const std::string& iName) const -> std::shared_ptr<actions::Action> {
		for (const auto& action: m_actions) {
			if (action->getName() == iName)
				return action;
		}
		return nullptr;
	}

	/**
	 * @brief Event handler.
	 * @param[in,out] ioEvent The Event to react.
	 */
	void onEvent(event::Event& ioEvent);

	/**
	 * @brief Keyboard pressed check.
	 * @param[in] iKeycode The Key to check.
	 * @return True if pressed.
	 */
	[[nodiscard]] auto isKeyPressed(const KeyCode& iKeycode) const -> bool;

	/**
	 * @brief Get the current modifiers.
	 * @return The current modifiers.
	 */
	[[nodiscard]] auto getModifiers() const -> Modifiers;

private:
	/// The application Instance.
	static Application* m_instance;
	/// The application state.
	State m_state = State::Created;
	/// The main window.
	MainWindow m_mainWindow;
	//// The views list.
	std::list<std::shared_ptr<views::View>> m_views;
	/// The actions list.
	std::list<std::shared_ptr<actions::Action>> m_actions;
	/// The clear color.
	const math::vec4 m_clearColor = {0.45f, 0.55f, 0.60f, 1.00f};

	/// The application theme.
	Theme m_theme;
};

auto createApplication(int iArgc, char* iArgv[]) -> std::shared_ptr<Application>;

}// namespace evl::gui_imgui
