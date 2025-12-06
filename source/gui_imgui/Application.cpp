/**
 * @file Application.cpp
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "Application.h"

#include "MainWindow.h"

#include <GLFW/glfw3.h>
#include <core/Log.h>
#include <imgui.h>


namespace evl::gui_imgui {
namespace {
	/**
	 * @brief GLFW error callback.
	 * @param iError The error code.
	 * @param iDescription The error description.
	 */
	void glfwErrorCallback(int iError, const char* iDescription) { log_error("GLFW Error ({}): {}", iError, iDescription); }
}// namespace

Application* Application::m_instance = nullptr;

Application::Application() {
	if (m_instance != nullptr) {
		log_error("Application instance already exists!");
		return;
	}
	log_debug("Creating Application instance");
	m_instance = this;
	// initialize glfw
	log_debug("Initializing GLFW for ImGui window");
	if (const auto init = glfwInit(); init == GLFW_FALSE) {
		log_error("Could not initialize GLFW ({}): {}", init, glfwGetError(nullptr));
		reportError("");
	}
	glfwSetErrorCallback(glfwErrorCallback);

	// create the main window
	log_debug("Creating main window");
	m_windows.push_back(std::make_shared<MainWindow>(MainWindow::Parameters{.title = "Evenement Loto (ImGui Version)",
																			.width = 1044,
																			.height = 1068,
																			.iconPath = "",
																			.showOnCreate = true}));

	if (m_state != State::Error)
		m_state = State::Running;
}

Application::~Application() {
	log_debug("Destroying Application instance");
	for (const auto& window: m_windows) { window->close(); }
	m_windows.clear();
	log_debug("Terminating GLFW for ImGui window");
	glfwTerminate();
	m_instance = nullptr;
	log_debug("Application instance destroyed");
}

void Application::close() { m_state = State::Stopped; }

void Application::invalidate() { m_instance = nullptr; }

void Application::run() {
	log_info("Starting Application main loop");
	while (m_state == State::Running) {
		// main loop
		glfwPollEvents();
		// check if all windows are closed
		std::erase_if(m_windows, [](const std::shared_ptr<Window>& iWindow) -> bool { return !iWindow->isAlive(); });
		if (m_windows.empty()) {
			log_info("All windows are closed, exiting main loop");
			m_state = State::Stopped;
			break;
		}
		// render and update all windows
		for (const auto& window: m_windows)
			window->onRender();
		for (const auto& window: m_windows)
			window->onUpdate();
	}
	log_info("Exiting Application main loop");
}

void Application::reportClose() { m_state = State::Stopped; }

void Application::reportError(const std::string& iMessage) {
	log_error("Application reported error: {}", iMessage);
	m_state = State::Error;
}

auto createApplication([[maybe_unused]] int iArgc, [[maybe_unused]] char* iArgv[]) -> std::shared_ptr<Application> {
	return std::make_shared<Application>();
}

}// namespace evl::gui_imgui
