
#include "../TestMainHelper.h"
#include "gui/Application.h"
#include "gui/event/AppEvent.h"
#include "gui/event/KeyEvent.h"
#include "gui/event/MouseEvent.h"

TEST(gui_event, WindowsEvents) {
	const evl::gui::event::WindowResizeEvent resizeEvent({800, 600});
	EXPECT_EQ(resizeEvent.getWidth(), 800);
	EXPECT_EQ(resizeEvent.getHeight(), 600);
	EXPECT_EQ(resizeEvent.getSize(), (evl::math::vec2ui{800, 600}));
	EXPECT_EQ(resizeEvent.getType(), evl::gui::event::Type::WindowResize);
	EXPECT_STREQ(resizeEvent.toString().c_str(), "WindowResizeEvent: 800, 600");
	EXPECT_STREQ(resizeEvent.getName().c_str(), "WindowResizeEvent");
	EXPECT_EQ(resizeEvent.getStaticType(), evl::gui::event::Type::WindowResize);
	EXPECT_EQ(resizeEvent.getCategoryFlags(), evl::gui::event::Category::Application);
	EXPECT_TRUE(resizeEvent.isInCategory(evl::gui::event::Category::Application));
	const evl::gui::event::WindowCloseEvent closeEvent{};
	EXPECT_EQ(closeEvent.getType(), evl::gui::event::Type::WindowClose);
	EXPECT_EQ(closeEvent.getStaticType(), evl::gui::event::Type::WindowClose);
	EXPECT_STREQ(closeEvent.toString().c_str(), "WindowCloseEvent");
	EXPECT_STREQ(closeEvent.getName().c_str(), "WindowCloseEvent");
	EXPECT_EQ(closeEvent.getCategoryFlags(), evl::gui::event::Category::Application);
}

TEST(gui_event, KeyEvents) {
	const evl::gui::event::KeyPressedEvent keyPressEvent(evl::gui::KeyCode::A, 1);
	EXPECT_EQ(keyPressEvent.getKeyCode(), evl::gui::KeyCode::A);
	EXPECT_EQ(keyPressEvent.getRepeatCount(), 1);
	EXPECT_EQ(keyPressEvent.getType(), evl::gui::event::Type::KeyPressed);
	EXPECT_EQ(keyPressEvent.getStaticType(), evl::gui::event::Type::KeyPressed);
	EXPECT_STREQ(keyPressEvent.toString().c_str(), "KeyPressedEvent: A (repeat = 1)");
	EXPECT_STREQ(keyPressEvent.getName().c_str(), "KeyPressedEvent");
	EXPECT_EQ(keyPressEvent.getCategoryFlags(),
	          evl::gui::event::Category::Keyboard | evl::gui::event::Category::Input);

	const evl::gui::event::KeyReleasedEvent keyReleaseEvent(evl::gui::KeyCode::B);
	EXPECT_EQ(keyReleaseEvent.getKeyCode(), evl::gui::KeyCode::B);
	EXPECT_EQ(keyReleaseEvent.getType(), evl::gui::event::Type::KeyReleased);
	EXPECT_EQ(keyReleaseEvent.getStaticType(), evl::gui::event::Type::KeyReleased);
	EXPECT_STREQ(keyReleaseEvent.toString().c_str(), "KeyReleasedEvent: B");
	EXPECT_STREQ(keyReleaseEvent.getName().c_str(), "KeyReleasedEvent");
	EXPECT_EQ(keyReleaseEvent.getCategoryFlags(),
			  evl::gui::event::Category::Keyboard | evl::gui::event::Category::Input);

	const evl::gui::event::KeyTypedEvent keyTypedEvent(U'C');
	EXPECT_EQ(keyTypedEvent.getCodepoint(), U'C');
	EXPECT_EQ(keyTypedEvent.getType(), evl::gui::event::Type::KeyTyped);
	EXPECT_EQ(keyTypedEvent.getStaticType(), evl::gui::event::Type::KeyTyped);
	EXPECT_STREQ(keyTypedEvent.toString().c_str(), "KeyTypedEvent: C");
	EXPECT_STREQ(keyTypedEvent.getName().c_str(), "KeyTypedEvent");
	EXPECT_EQ(keyTypedEvent.getCategoryFlags(),
			  evl::gui::event::Category::Keyboard | evl::gui::event::Category::Input);
}

TEST(gui_event, MouseEvents) {
	const evl::gui::event::MouseMovedEvent mouseMoveEvent(100.0f, 150.0f);
	EXPECT_FLOAT_EQ(mouseMoveEvent.getX(), 100.0f);
	EXPECT_FLOAT_EQ(mouseMoveEvent.getY(), 150.0f);
	EXPECT_EQ(mouseMoveEvent.getType(), evl::gui::event::Type::MouseMoved);
	EXPECT_EQ(mouseMoveEvent.getStaticType(), evl::gui::event::Type::MouseMoved);
	EXPECT_STREQ(mouseMoveEvent.toString().c_str(), "MouseMovedEvent: 100.0, 150.0");
	EXPECT_STREQ(mouseMoveEvent.getName().c_str(), "MouseMovedEvent");
	EXPECT_EQ(mouseMoveEvent.getCategoryFlags(),
			  evl::gui::event::Category::Mouse | evl::gui::event::Category::Input);
	const evl::gui::event::MouseScrolledEvent mouseScrollEvent(0.0f, -1.0f);
	EXPECT_FLOAT_EQ(mouseScrollEvent.getXOff(), 0.0f);
	EXPECT_FLOAT_EQ(mouseScrollEvent.getYOff(), -1.0f);
	EXPECT_EQ(mouseScrollEvent.getType(), evl::gui::event::Type::MouseScrolled);
	EXPECT_EQ(mouseScrollEvent.getStaticType(), evl::gui::event::Type::MouseScrolled);
	EXPECT_STREQ(mouseScrollEvent.toString().c_str(), "MouseScrolledEvent: 0, -1");
	EXPECT_STREQ(mouseScrollEvent.getName().c_str(), "MouseScrolledEvent");
	EXPECT_EQ(mouseScrollEvent.getCategoryFlags(),
			  evl::gui::event::Category::Mouse | evl::gui::event::Category::Input);
	const evl::gui::event::MouseButtonPressedEvent mouseButtonPressEvent(1);
	EXPECT_EQ(mouseButtonPressEvent.getMouseButton(), 1);
	EXPECT_EQ(mouseButtonPressEvent.getType(), evl::gui::event::Type::MouseButtonPressed);
	EXPECT_EQ(mouseButtonPressEvent.getStaticType(), evl::gui::event::Type::MouseButtonPressed);
	EXPECT_STREQ(mouseButtonPressEvent.toString().c_str(), "MouseButtonPressedEvent: 1");
	EXPECT_STREQ(mouseButtonPressEvent.getName().c_str(), "MouseButtonPressedEvent");
	EXPECT_EQ(mouseButtonPressEvent.getCategoryFlags(),
			  evl::gui::event::Category::Mouse | evl::gui::event::Category::Input | evl::gui::event::Category::MouseButton);
	const evl::gui::event::MouseButtonReleasedEvent mouseButtonReleaseEvent(2);
	EXPECT_EQ(mouseButtonReleaseEvent.getMouseButton(), 2);
	EXPECT_EQ(mouseButtonReleaseEvent.getType(), evl::gui::event::Type::MouseButtonReleased);
	EXPECT_EQ(mouseButtonReleaseEvent.getStaticType(), evl::gui::event::Type::MouseButtonReleased);
	EXPECT_STREQ(mouseButtonReleaseEvent.toString().c_str(), "MouseButtonReleasedEvent: 2");
	EXPECT_STREQ(mouseButtonReleaseEvent.getName().c_str(), "MouseButtonReleasedEvent");
	EXPECT_EQ(mouseButtonReleaseEvent.getCategoryFlags(),
			  evl::gui::event::Category::Mouse | evl::gui::event::Category::Input | evl::gui::event::Category::MouseButton);
}

TEST(gui_event, EventDispatcher) {
	evl::gui::event::KeyPressedEvent keyPressEvent(evl::gui::KeyCode::A, 1);
	evl::gui::event::EventDispatcher dispatcher(keyPressEvent);
	bool dispatched = dispatcher.dispatch<evl::gui::event::KeyPressedEvent>(
		[](const evl::gui::event::KeyPressedEvent& iE) {
			EXPECT_EQ(iE.getKeyCode(), evl::gui::KeyCode::A);
			EXPECT_EQ(iE.getRepeatCount(), 1);
			return true;
		});
	EXPECT_TRUE(dispatched);
	EXPECT_TRUE(keyPressEvent.handled);

	evl::gui::event::MouseMovedEvent mouseMoveEvent(200.0f, 250.0f);
	evl::gui::event::EventDispatcher dispatcher2(mouseMoveEvent);
	dispatched = dispatcher2.dispatch<evl::gui::event::MouseMovedEvent>(
		[](const evl::gui::event::MouseMovedEvent& iE) {
			EXPECT_FLOAT_EQ(iE.getX(), 200.0f);
			EXPECT_FLOAT_EQ(iE.getY(), 250.0f);
			return false;
		});
	EXPECT_TRUE(dispatched);
	EXPECT_FALSE(mouseMoveEvent.handled);

	evl::gui::event::WindowCloseEvent closeEvent;
	evl::gui::event::EventDispatcher dispatcher3(closeEvent);
	dispatched = dispatcher3.dispatch<evl::gui::event::KeyPressedEvent>(
		[](evl::gui::event::KeyPressedEvent&) {
			// This should not be called
			EXPECT_TRUE(false);
			return true;
		});
	EXPECT_FALSE(dispatched);
	EXPECT_FALSE(closeEvent.handled);
}

TEST(KeyCode, FormatModifiers) {
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::LeftControl).c_str(), "Ctrl");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::RightControl).c_str(), "Ctrl");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::LeftShift).c_str(), "Shift");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::RightShift).c_str(), "Shift");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::LeftAlt).c_str(), "Alt");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::RightAlt).c_str(), "Alt gr");
}

TEST(KeyCode, FormatSpecialKeys) {
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Enter).c_str(), "Enter");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Backspace).c_str(), "Backspace");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Tab).c_str(), "Tab");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::CapsLock).c_str(), "Caps Lock");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::PageUp).c_str(), "Page Up");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::PageDown).c_str(), "Page Down");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::PrintScreen).c_str(), "Print Screen");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::ScrollLock).c_str(), "Scroll Lock");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::NumLock).c_str(), "Num Lock");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Pause).c_str(), "Pause");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Insert).c_str(), "Insert");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Delete).c_str(), "Delete");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Home).c_str(), "Home");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::End).c_str(), "End");
}

TEST(KeyCode, FormatArrowKeys) {
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Left).c_str(), "Left Arrow");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Right).c_str(), "Right Arrow");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Up).c_str(), "Up Arrow");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Down).c_str(), "Down Arrow");
}

TEST(KeyCode, FormatFunctionKeys) {
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F1).c_str(), "F1");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F2).c_str(), "F2");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F3).c_str(), "F3");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F4).c_str(), "F4");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F5).c_str(), "F5");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F6).c_str(), "F6");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F7).c_str(), "F7");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F8).c_str(), "F8");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F9).c_str(), "F9");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F10).c_str(), "F10");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F11).c_str(), "F11");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::F12).c_str(), "F12");
}

TEST(KeyCode, FormatDigits) {
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D0).c_str(), "0");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D1).c_str(), "1");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D2).c_str(), "2");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D3).c_str(), "3");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D4).c_str(), "4");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D5).c_str(), "5");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D6).c_str(), "6");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D7).c_str(), "7");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D8).c_str(), "8");
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::D9).c_str(), "9");
}

TEST(KeyCode, FormatVoidKey) {
	EXPECT_STREQ(std::format("{}",evl::gui::KeyCode::Void).c_str(), "");
}

TEST(KeyCombination, Format) {
	evl::gui::KeyCombination combo;
	combo.key = evl::gui::KeyCode::A;
	EXPECT_STREQ(std::format("{}",combo).c_str(), "A");

	combo.modifiers.ctrl = true;
	EXPECT_STREQ(std::format("{}",combo).c_str(), "Ctrl + A");

	combo.modifiers.shift = true;
	EXPECT_STREQ(std::format("{}",combo).c_str(), "Ctrl + Shift + A");

	combo.modifiers.alt = true;
	EXPECT_STREQ(std::format("{}",combo).c_str(), "Ctrl + Shift + Alt + A");

	combo.modifiers.altGr = true;
	EXPECT_STREQ(std::format("{}",combo).c_str(), "Ctrl + Shift + Alt + Alt gr + A");

	combo.key = evl::gui::KeyCode::Void;
	EXPECT_STREQ(std::format("{}",combo).c_str(), "");
}
