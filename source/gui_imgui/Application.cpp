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
#include "actions/HelpActions.h"
#include "actions/SettingsActions.h"
#include "baseDefine.h"
#include "core/Log.h"
#include "core/utilities.h"
#include "event/AppEvent.h"
#include "views/ConfigPopups.h"
#include "views/HelpPopups.h"
#include "views/MenuBar.h"
#include "views/StatusBar.h"
#include "views/ToolBar.h"


namespace evl::gui_imgui {

Application* Application::m_instance = nullptr;

Application::Application() {
	log_info("Starting application.");
	m_instance = this;

	m_mainWindow.init({.title = std::format("Application Loto ({})", EVL_VERSION), .size = {800, 600}, .iconPath = ""});
	if (m_state == State::Error)
		return;

	const auto resourcePath = core::getExecPath() / "resources";
	if (const auto texturePath = resourcePath / "darkicons"; exists(texturePath) && is_directory(texturePath))
		m_textureLibrary.loadFolder(texturePath);
	else
		log_warn("Texture path '{}' does not exist or is not a directory.", texturePath.string());

	m_mainWindow.setIcon("mainIcon");

	// Create views
	m_views.push_back(std::make_shared<views::MenuBar>());
	m_views.push_back(std::make_shared<views::ToolBar>());
	m_views.push_back(std::make_shared<views::StatusBar>());

	// Create popups
	m_popups.push_back(std::make_shared<views::PopupAide>());
	m_popups.push_back(std::make_shared<views::PopupAbout>());
	m_popups.push_back(std::make_shared<views::MainConfigPopups>());
	m_popups.push_back(std::make_shared<views::EventConfigPopups>());
	m_popups.push_back(std::make_shared<views::GameRoundConfigPopups>());

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
	m_actions.back()->setShortcut({.key = KeyCode::A, .modifiers = {.ctrl = true}});
	m_actions.push_back(std::make_shared<actions::HelpAction>());
	m_actions.push_back(std::make_shared<actions::AboutAction>());

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
	uint32_t frameCount = 0;
	while (m_state == State::Running || m_state == State::Waiting) {
		if (m_mainWindow.shouldClose()) {
			m_state = State::Closed;
			continue;
		}
		checkActionEnable();
		m_mainWindow.newFrame();
		if (m_state != State::Running)
			continue;
		for (const auto& popup: m_popups) { popup->update(); }
		for (const auto& view: m_views) { view->update(); }
		m_mainWindow.render(m_theme.windowBackground);
		frameCount++;
		if (m_maxFrame != 0 && frameCount >= m_maxFrame) {
			log_info("Maximum frame count {} reached, closing application.", m_maxFrame);
			m_state = State::Closed;
		}
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
	// does any popup handle the event?
	for (const auto& popup: m_popups) {
		if (ioEvent.handled)
			return;
		popup->onEvent(ioEvent);
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

void Application::checkActionEnable() const {
	const auto status = m_currentEvent.getStatus();
	if (status == core::Event::Status::Invalid || status == core::Event::Status::MissingParties) {
		getAction("save_file_as")->disable();
		getAction("save_file")->disable();
	} else {
		getAction("save_file")->enable();
		getAction("save_file_as")->enable();
	}
	if (status == core::Event::Status::Ready) {
		getAction("start_game")->enable();
	} else {
		getAction("start_game")->disable();
	}
	if (status == core::Event::Status::Finished) {
		getAction("stop_game")->enable();
	} else {
		getAction("stop_game")->disable();
	}
	if (status == core::Event::Status::Invalid) {
		getAction("game_settings")->disable();
	} else {
		getAction("game_settings")->enable();
	}
}

}// namespace evl::gui_imgui
