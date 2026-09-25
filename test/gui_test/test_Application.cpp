/**
 * @file test_Application.cpp
 * @author Silmaen
 * @date 03/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"
#include "gui/Application.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

using namespace evl::gui;

namespace {

/// A view that throws on every frame, to exercise the net around the render loop.
class ThrowingView final : public views::View {
public:
	explicit ThrowingView(const uint32_t iThrowCount) : m_remaining{iThrowCount} {}

	void onUpdate() override {
		if (m_remaining == 0)
			return;
		--m_remaining;
		++m_thrown;
		throw std::runtime_error("exception de test");
	}

	[[nodiscard]] auto getName() const -> std::string override { return "throwing_view"; }
	[[nodiscard]] auto thrown() const -> uint32_t { return m_thrown; }

private:
	uint32_t m_remaining = 0;
	uint32_t m_thrown = 0;
};

}// namespace

/// The application needs a display. Under `xvfb-run` it gets a virtual one and Vulkan
/// falls back on lavapipe, which is all these tests need — they check the loop, not the
/// pixels.
TEST(gui_Application, InstantiateAndRun) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	EXPECT_EQ(app->getState(), Application::State::Running);
	app->setMaxFrame(2);
	EXPECT_EQ(app->getMaxFrame(), 2U);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Closed);
}

TEST(gui_Application, SurvivesAnExceptionFromAView) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// Two throws, then the view behaves: fewer than the five in a row that give up.
	const auto view = std::make_shared<ThrowingView>(2);
	app->addView(view);
	app->setMaxFrame(6);
	app->run();
	EXPECT_EQ(view->thrown(), 2U);
	// Closed and not Error: the frames that threw were survived.
	EXPECT_EQ(app->getState(), Application::State::Closed);
}

TEST(gui_Application, StopsAfterTooManyFailedFrames) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// Never recovers: the loop must give up rather than spin on the same error for the
	// whole afternoon.
	const auto view = std::make_shared<ThrowingView>(1000);
	app->addView(view);
	app->setMaxFrame(100);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Error);
	// It gave up early, long before the frame limit.
	EXPECT_LT(view->thrown(), 100U);
}

#ifndef EVL_PLATFORM_WINDOWS
TEST(gui_Application, LeavesCleanlyWhenThereIsNoDisplay) {
	// What the first real launch did: started from a session without the X authority,
	// GLFW could not open the display. Reporting the error was right. What followed was
	// not: the teardown walked over a Vulkan device that had never been created, and
	// the process died on a signal instead of returning EXIT_FAILURE.
	const auto* display = std::getenv("DISPLAY");
	const std::string saved = display == nullptr ? std::string{} : display;
	ASSERT_EQ(unsetenv("DISPLAY"), 0);
	{
		const auto app = createApplication(0, nullptr);
		ASSERT_NE(app, nullptr);
		// Reported, not hidden: main() turns this into EXIT_FAILURE.
		EXPECT_EQ(app->getState(), Application::State::Error);
		// The destructor runs at the end of this scope. That is where it used to crash.
	}
	// And the instance is forgotten, so nothing can reach the object that just went.
	EXPECT_FALSE(Application::instanced());
	if (!saved.empty()) {
		// Braced: the macro expands to an if/else of its own.
		ASSERT_EQ(setenv("DISPLAY", saved.c_str(), 1), 0);
	}
}
#endif
