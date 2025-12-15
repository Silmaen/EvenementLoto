/**
 * @file Action.cpp
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Action.h"

#include "core/Log.h"
#include "gui_imgui/Application.h"
#include "gui_imgui/event/KeyEvent.h"

namespace evl::gui_imgui::actions {

Action::Action() = default;

Action::~Action() = default;

auto Action::getShortcut() const -> std::string { return formatKeyCombination(m_shortcut); }

void Action::onEvent(event::Event& ioEvent) {
	if (!m_enabled || m_shortcut.key == KeyCode::Void)
		return;
	if (ioEvent.getType() == event::Type::KeyPressed) {
		const auto& app = Application::get();
		if (!app.isKeyPressed(m_shortcut.key))
			return;
		auto [ctrl, shift, alt, altGr] = app.getModifiers();
		if (m_shortcut.modifiers.ctrl && !ctrl)
			return;
		if (m_shortcut.modifiers.shift && !shift)
			return;
		if (m_shortcut.modifiers.alt && !alt)
			return;
		if (m_shortcut.modifiers.altGr && !altGr)
			return;
		log_trace("Action '{}' triggered by shortcut '{}'", getName(), getShortcut());
		onExecute();
		ioEvent.handled = true;
	}
}

}// namespace evl::gui_imgui::actions
