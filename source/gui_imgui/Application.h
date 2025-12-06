/**
 * @file Application.h
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Window.h"
#include <core/Event.h>

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
	 * @brief Request the application to terminate.
	 */
	void close();

	/**
	 * @brief Request the application to terminate.
	 */
	static void invalidate();

	/**
	 * @brief Run the application.
	 */
	void run();

	/**
	 * @brief State of the application.
	 */
	enum struct State : uint8_t {
		Created,/// Application just created.
		Running,/// Application is running.
		Stopped,/// Application Stopped.
		Error/// Application in error.
	};

	/**
	 * @brief Get the application's state.
	 * @return The current application's state.
	 */
	[[nodiscard]] auto getState() const -> const State& { return m_state; }

	/**
	 * @brief Report an error and stop the application.
	 * @param[in] iErrorMessage The error message.
	 */
	void reportError(const std::string& iErrorMessage);

	/**
	 * @brief Report that all windows have been closed.
	 */
	void reportClose();

private:
	/// The event being managed.
	core::Event m_event;
	/// The application Instance.
	static Application* m_instance;
	/// The current state of the application.
	State m_state = State::Created;
	/// The list of windows.
	std::vector<std::shared_ptr<Window>> m_windows;
};

auto createApplication(int iArgc, char* iArgv[]) -> std::shared_ptr<Application>;

}// namespace evl::gui_imgui
