/**
 * @file KeyCode.cpp
 * @author Silmaen
 * @date 12/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "KeyCodes.h"
#include "core/defines.h"

#include <GLFW/glfw3.h>

namespace evl::gui_imgui {

auto formatKeyCode(const KeyCode& iKeyCode) -> std::string {
	EVL_DIAG_PUSH
	EVL_DIAG_DISABLE_CLANG("-Wswitch-enum")
	switch (iKeyCode) {
		case KeyCode::LeftControl:
		case KeyCode::RightControl:
			return "Ctrl";
		case KeyCode::LeftShift:
		case KeyCode::RightShift:
			return "Shift";
		case KeyCode::LeftAlt:
			return "Alt";
		case KeyCode::RightAlt:
			return "Alt gr";
		case KeyCode::Escape:
			return "Esc";
		case KeyCode::Enter:
			return "Enter";
		case KeyCode::Backspace:
			return "Backspace";
		case KeyCode::Tab:
			return "Tab";
		case KeyCode::Space:
			return "Space";
		case KeyCode::CapsLock:
			return "Caps Lock";
		case KeyCode::PageUp:
			return "Page Up";
		case KeyCode::PageDown:
			return "Page Down";
		case KeyCode::PrintScreen:
			return "Print Screen";
		case KeyCode::ScrollLock:
			return "Scroll Lock";
		case KeyCode::NumLock:
			return "Num Lock";
		case KeyCode::Pause:
			return "Pause";
		case KeyCode::Insert:
			return "Insert";
		case KeyCode::Delete:
			return "Delete";
		case KeyCode::Home:
			return "Home";
		case KeyCode::End:
			return "End";
		case KeyCode::Left:
			return "Left Arrow";
		case KeyCode::Right:
			return "Right Arrow";
		case KeyCode::Up:
			return "Up Arrow";
		case KeyCode::Down:
			return "Down Arrow";
		case KeyCode::F1:
			return "F1";
		case KeyCode::F2:
			return "F2";
		case KeyCode::F3:
			return "F3";
		case KeyCode::F4:
			return "F4";
		case KeyCode::F5:
			return "F5";
		case KeyCode::F6:
			return "F6";
		case KeyCode::F7:
			return "F7";
		case KeyCode::F8:
			return "F8";
		case KeyCode::F9:
			return "F9";
		case KeyCode::F10:
			return "F10";
		case KeyCode::F11:
			return "F11";
		case KeyCode::F12:
			return "F12";
		case KeyCode::Void:
			return "";
		case KeyCode::D0:
			return "0";
		case KeyCode::D1:
			return "1";
		case KeyCode::D2:
			return "2";
		case KeyCode::D3:
			return "3";
		case KeyCode::D4:
			return "4";
		case KeyCode::D5:
			return "5";
		case KeyCode::D6:
			return "6";
		case KeyCode::D7:
			return "7";
		case KeyCode::D8:
			return "8";
		case KeyCode::D9:
			return "9";
		default:
			const char* keyName = glfwGetKeyName(static_cast<int>(iKeyCode), 0);
			if (keyName != nullptr) {
				std::string name = keyName;
				if (!name.empty())
					name[0] = static_cast<char>(std::toupper(name[0]));
				return name;
			}
			return std::string{static_cast<char>(iKeyCode)};
	}
	EVL_DIAG_POP
}

auto formatKeyCombination(const KeyCombination& iCombination) -> std::string {
	if (iCombination.key == KeyCode::Void)
		return "";
	return std::format("{}{}{}{}{}", iCombination.modifiers.ctrl ? formatKeyCode(KeyCode::LeftControl) + " + " : "",
					   iCombination.modifiers.shift ? formatKeyCode(KeyCode::LeftShift) + " + " : "",
					   iCombination.modifiers.alt ? formatKeyCode(KeyCode::LeftAlt) + " + " : "",
					   iCombination.modifiers.altGr ? formatKeyCode(KeyCode::RightAlt) + " + " : "",
					   formatKeyCode(iCombination.key));
}

}// namespace evl::gui_imgui
