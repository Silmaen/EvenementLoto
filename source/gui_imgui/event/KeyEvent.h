/**
 * @file KeyEvent.h
 * @author Silmaen
 * @date 04/12/2022
 * Copyright © 2022 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once
#include "Event.h"
#include "KeyCodes.h"

namespace evl::gui_imgui::event {

/**
 * @brief Semi-abstracted class for all Keyboard events.
 */
class KeyEvent : public Event {
public:
	/**
	 * @brief Get the Key code.
	 * @return The Event Key code.
	 */
	[[nodiscard]] auto getKeyCode() const -> KeyCode { return m_keyCode; }

	/**
	 * @brief Get the event's category flags.
	 * @return Event's category flags.
	 */
	[[nodiscard]] auto getCategoryFlags() const -> uint8_t final;

protected:
	explicit KeyEvent(const KeyCode iKeyCode) : m_keyCode(iKeyCode) {}
	/// Key code.
	KeyCode m_keyCode;
};

/**
 * @brief Event of key presse
 */
class KeyPressedEvent final : public KeyEvent {
public:
	/**
	 * @brief Constructor.
	 * @param[in] iKeyCode Code of the key pressed.
	 * @param[in] iRepeatCount Number of repeat.
	 */
	explicit KeyPressedEvent(const KeyCode iKeyCode, const uint16_t iRepeatCount)
		: KeyEvent(iKeyCode), m_repeatCount(iRepeatCount) {}

	/**
	 * @brief Check if the key is repeated
	 * @return True if repeated
	 */
	[[nodiscard]] auto getRepeatCount() const -> uint16_t { return m_repeatCount; }

	/**
	 * @brief Get the event as string.
	 * @return String of the event.
	 */
	[[nodiscard]] auto toString() const -> std::string override {
		return std::format("KeyPressedEvent: {} (repeat = {})", formatKeyCode(m_keyCode), m_repeatCount);
	}

	/**
	 * @brief Get the event's name.
	 * @return Event's name.
	 */
	[[nodiscard]] auto getName() const -> std::string override;

	/**
	 * @brief Get the event's static type.
	 * @return Event's static type.
	 */
	[[nodiscard]] static auto getStaticType() -> Type { return Type::KeyPressed; }

	/**
	 * @brief Get the event's type.
	 * @return Event's type.
	 */
	[[nodiscard]] auto getType() const -> Type override { return getStaticType(); }

private:
	/// Counter of repeat.
	uint16_t m_repeatCount;
};

/**
 * @brief Event of key released
 */
class KeyReleasedEvent final : public KeyEvent {
public:
	/**
	 * @brief Constructor.
	 * @param[in] iKeyCode Code of the key released.
	 */
	explicit KeyReleasedEvent(const KeyCode iKeyCode) : KeyEvent(iKeyCode) {}

	/**
	 * @brief Get the event as string.
	 * @return String of the event.
	 */
	[[nodiscard]] auto toString() const -> std::string override {
		return std::format("KeyReleasedEvent: {}", m_keyCode);
	}

	/**
	 * @brief Get the event's name.
	 * @return Event's name.
	 */
	[[nodiscard]] auto getName() const -> std::string override;

	/**
	 * @brief Get the event's static type.
	 * @return Event's static type.
	 */
	[[nodiscard]] static auto getStaticType() -> Type { return Type::KeyReleased; }

	/**
	 * @brief Get the event's type.
	 * @return Event's type.
	 */
	[[nodiscard]] auto getType() const -> Type override { return getStaticType(); }
};

/**
 * @brief Event of key typed
 */
class KeyTypedEvent final : public Event {
public:
	/**
	 * @brief Constructor.
	 * @param[in] iKeyCode Code of the key typed.
	 */
	explicit KeyTypedEvent(const char32_t iKeyCode) : m_codepoint(iKeyCode) {}

	/**
	 * @brief Get the event as string.
	 * @return String of the event.
	 */
	[[nodiscard]] auto toString() const -> std::string override { return std::format("KeyTypedEvent: {}", getChar()); }

	/**
	 * @brief Get the event's name.
	 * @return Event's name.
	 */
	[[nodiscard]] auto getName() const -> std::string override;

	/**
	 * @brief Get the event's static type.
	 * @return Event's static type.
	 */
	[[nodiscard]] static auto getStaticType() -> Type { return Type::KeyTyped; }

	/**
	 * @brief Get the event's type.
	 * @return Event's type.
	 */
	[[nodiscard]] auto getType() const -> Type override { return getStaticType(); }

	/**
	 * @brief Get the event's category flags.
	 * @return Event's category flags.
	 */
	[[nodiscard]] auto getCategoryFlags() const -> uint8_t override;
	/**
	 * @brief Get the codepoint of the key typed.
	 * @return The codepoint.
	 */
	[[nodiscard]] auto getCodepoint() const -> char32_t { return m_codepoint; }

	/**
	 * @brief Get the character of the key typed.
	 * @return The character.
	 */
	[[nodiscard]] auto getChar() const -> std::string {
		// Conversion UTF-32 vers UTF-8
		std::string result;
		if (m_codepoint < 0x80) {
			result += static_cast<char>(m_codepoint);
		} else if (m_codepoint < 0x800) {
			result += static_cast<char>(0xC0 | (m_codepoint >> 6));
			result += static_cast<char>(0x80 | (m_codepoint & 0x3F));
		} else if (m_codepoint < 0x10000) {
			result += static_cast<char>(0xE0 | (m_codepoint >> 12));
			result += static_cast<char>(0x80 | ((m_codepoint >> 6) & 0x3F));
			result += static_cast<char>(0x80 | (m_codepoint & 0x3F));
		}
		return result;
	}

private:
	/// The codepoint of the key typed.
	char32_t m_codepoint;
};

}// namespace evl::gui_imgui::event
