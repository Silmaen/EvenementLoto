/**
 * @file Event.cpp
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Event.h"

#include "AppEvent.h"
#include "KeyEvent.h"
#include "MouseEvent.h"


namespace evl::gui_imgui::event {

Event::Event() = default;

Event::~Event() = default;

auto WindowResizeEvent::getCategoryFlags() const -> uint8_t { return Application; }
auto WindowCloseEvent::getCategoryFlags() const -> uint8_t { return Application; }
auto AppTickEvent::getCategoryFlags() const -> uint8_t { return Application; }
auto AppUpdateEvent::getCategoryFlags() const -> uint8_t { return Application; }
auto AppRenderEvent::getCategoryFlags() const -> uint8_t { return Application; }

auto KeyEvent::getCategoryFlags() const -> uint8_t { return Input | Keyboard; }
auto KeyPressedEvent::getName() const -> std::string { return std::format("KeyPressedEvent"); }
auto KeyReleasedEvent::getName() const -> std::string { return std::format("KeyReleasedEvent"); }
auto KeyTypedEvent::getName() const -> std::string { return std::format("KeyTypedEvent"); }
auto KeyTypedEvent::getCategoryFlags() const -> uint8_t { return Input | Keyboard; }

auto MouseMovedEvent::getCategoryFlags() const -> uint8_t { return Input | Mouse; }
auto MouseScrolledEvent::getCategoryFlags() const -> uint8_t { return Input | Mouse; }
auto MouseButtonEvent::getCategoryFlags() const -> uint8_t {
	return static_cast<uint8_t>(Input | Mouse) | static_cast<uint8_t>(MouseButton);
}
auto MouseButtonPressedEvent::getName() const -> std::string { return std::format("MouseButtonPressedEvent"); }
auto MouseButtonReleasedEvent::getName() const -> std::string { return std::format("MouseButtonReleasedEvent"); }

}// namespace evl::gui_imgui::event
