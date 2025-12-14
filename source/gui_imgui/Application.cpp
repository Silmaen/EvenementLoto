/**
 * @file Application.cpp
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Application.h"

#include "actions/FileActions.h"
#include "actions/SettingsActions.h"
#include "baseDefine.h"
#include "core/Log.h"
#include "core/utilities.h"
#include "event/AppEvent.h"
#include "views/MenuBar.h"


namespace evl::gui_imgui {

Application* Application::m_instance = nullptr;

Application::Application() {
	log_info("Starting application.");
	m_instance = this;
	m_mainWindow.init({.title = std::format("Application Loto ({})", EVL_VERSION), .size = {800, 600}, .iconPath = ""});
	if (m_state == State::Error)
		return;
	// Create views
	m_views.push_back(std::make_shared<views::MenuBar>());

	// Create actions
	m_actions.push_back(std::make_shared<actions::NewFileAction>());
	m_actions.push_back(std::make_shared<actions::LoadFileAction>());
	m_actions.push_back(std::make_shared<actions::SaveFileAction>());
	m_actions.push_back(std::make_shared<actions::SaveAsFileAction>());
	m_actions.push_back(std::make_shared<actions::StartGameAction>());
	m_actions.push_back(std::make_shared<actions::StopGameAction>());
	m_actions.push_back(std::make_shared<actions::PreferencesAction>());
	m_actions.push_back(std::make_shared<actions::EventSettingsAction>());
	m_actions.push_back(std::make_shared<actions::GameSettingsAction>());
	m_actions.push_back(std::make_shared<actions::ThemeSettingsAction>());
	m_actions.push_back(std::make_shared<actions::QuitAction>());
	m_actions.back()->setShortcut({KeyCode::A, {.ctrl = true}});

	m_theme.loadFromSettings(core::getSettings()->extract("theme"));
	setTheme(m_theme);

	m_mainWindow.setEventCallback([this]<typename T>(T&& ioEvent) -> auto { onEvent(std::forward<T>(ioEvent)); });

	m_state = State::Running;
}

Application::~Application() {
	log_info("Shutting down application.");
	// Cleanup
	m_mainWindow.close();
}

void Application::run() {
	// Main loop
	while (m_state == State::Running || m_state == State::Waiting) {
		if (m_mainWindow.shouldClose()) {
			m_state = State::Closed;
			continue;
		}
		m_mainWindow.newFrame();
		if (m_state != State::Running)
			continue;
		for (const auto& view: m_views) { view->update(); }
		m_mainWindow.render(m_theme.windowBackground);
	}
}

void Application::reportError(const std::string& iMessage) {
	log_error("Application reported error: {}", iMessage);
	m_state = State::Error;
}

auto Application::getTheme() const -> const Theme& { return m_theme; }

void Application::setTheme(const Theme& iTheme) {
	m_theme = iTheme;
	m_mainWindow.setTheme(m_theme);
	core::getSettings()->include(m_theme.saveToSettings(), "theme");
}

auto createApplication([[maybe_unused]] int iArgc, [[maybe_unused]] char* iArgv[]) -> std::shared_ptr<Application> {
	return std::make_shared<Application>();
}

void Application::requestClose() { m_state = State::Closed; }

void Application::onEvent(event::Event& ioEvent) {
	event::EventDispatcher dispatcher(ioEvent);
	dispatcher.dispatch<event::WindowCloseEvent>([this]<typename T>(const T&) -> auto {
		requestClose();
		return true;
	});
	dispatcher.dispatch<event::WindowResizeEvent>([]<typename T>(const T&) -> auto {
		log_trace("Resize Event");
		return true;
	});
	// does any action handle the event?
	for (const auto& action: m_actions) {
		if (ioEvent.handled)
			return;
		action->onEvent(ioEvent);
	}
	// does any view handle the event?
	for (const auto& view: m_views) {
		if (ioEvent.handled)
			return;
		view->onEvent(ioEvent);
	}
	m_mainWindow.onEvent(ioEvent);
}

auto Application::isKeyPressed(const KeyCode& iKeycode) const -> bool { return m_mainWindow.isKeyPressed(iKeycode); }
auto Application::getModifiers() const -> Modifiers { return m_mainWindow.getModifiers(); }
}// namespace evl::gui_imgui
