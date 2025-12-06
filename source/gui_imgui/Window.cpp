/**
 * @file Window.cpp
 * @author Silmaen
 * @date 04/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "Window.h"

#include "Application.h"

#include <GLFW/glfw3.h>
#include <core/Log.h>
#include <utility>

namespace evl::gui_imgui {


Window::Window(Parameters iParams) : m_parameters(std::move(iParams)) {
	// initiate GLFW if first window

	log_info("Creating window {} ({}, {})", m_parameters.title, m_parameters.width, m_parameters.height);
	{
		m_window = glfwCreateWindow(static_cast<int>(m_parameters.width), static_cast<int>(m_parameters.height),
									m_parameters.title.c_str(), nullptr, nullptr);
		if (m_window == nullptr) {
			log_error("Could not create GLFW window: {}", glfwGetError(nullptr));
			Application::get().reportError("");
		}
	}
	auto* glfwWin = static_cast<GLFWwindow*>(m_window);
	glfwSetWindowUserPointer(glfwWin, this);
	/// set Icon if any
	if (!m_parameters.iconPath.empty()) {
		// TODO set icon
	}
	/// set callbacks
	{
		glfwSetWindowSizeCallback(glfwWin,
								  []([[maybe_unused]] GLFWwindow* iWindow, [[maybe_unused]] const int iWidth,
									 [[maybe_unused]] const int iHeight) -> void {
									  auto* window = static_cast<Window*>(glfwGetWindowUserPointer(iWindow));
									  window->onResize(iWidth, iHeight);
								  });

		glfwSetWindowCloseCallback(glfwWin, [](GLFWwindow* iWindow) -> void {
			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(iWindow));
			window->onclose();
		});

		glfwSetKeyCallback(glfwWin,
						   []([[maybe_unused]] GLFWwindow* iWindow, const int iKey, [[maybe_unused]] int iScancode,
							  const int iAction, [[maybe_unused]] int iMods) -> void {
							   auto* window = static_cast<Window*>(glfwGetWindowUserPointer(iWindow));
							   switch (iAction) {
								   case GLFW_REPEAT:
								   case GLFW_PRESS:
									   {
										   window->onKeyPressed(iKey);
										   break;
									   }
								   case GLFW_RELEASE:
									   {
										   window->onKeyReleased(iKey);
										   break;
									   }
								   default:
									   break;
							   }
						   });

		glfwSetMouseButtonCallback(glfwWin, [](GLFWwindow* iWindow, const int iButton, int iAction, int) -> void {
			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(iWindow));
			switch (iAction) {
				case GLFW_PRESS:
					{
						window->onMouseButtonPressed(iButton);
						break;
					}
				case GLFW_RELEASE:
					{
						window->onMouseButtonReleased(iButton);
						break;
					}
				default:
					break;
			}
		});

		glfwSetScrollCallback(glfwWin, [](GLFWwindow* iWindow, const double iXOffset, const double iYOffset) -> void {
			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(iWindow));
			window->onMouseScrolled(iXOffset, iYOffset);
		});

		glfwSetCursorPosCallback(glfwWin, [](GLFWwindow* iWindow, const double iX, const double iY) -> void {
			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(iWindow));
			window->onMouseMoved(iX, iY);
		});
	}
	/// show window
	m_isVisible = !m_parameters.showOnCreate;// to force show/hide
	if (m_parameters.showOnCreate)
		show();
	else
		hide();
}

Window::~Window() { close(); }

void Window::show() {
	if (m_window == nullptr || m_isVisible)
		return;
	auto* glfwWin = static_cast<GLFWwindow*>(m_window);
	glfwShowWindow(glfwWin);
	m_isVisible = true;
}

void Window::hide() {
	if (m_window == nullptr || !m_isVisible)
		return;
	auto* glfwWin = static_cast<GLFWwindow*>(m_window);
	glfwHideWindow(glfwWin);
	m_isVisible = false;
}

void Window::close() {
	if (m_window == nullptr)
		return;
	log_info("Closing window {}", m_parameters.title);

	glfwDestroyWindow(static_cast<GLFWwindow*>(m_window));
	m_window = nullptr;
}

void Window::onUpdate() { }

}// namespace evl::gui_imgui
