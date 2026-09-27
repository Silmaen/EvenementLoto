/**
 * @file test_Application.cpp
 * @author Silmaen
 * @date 03/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "../TestMainHelper.h"
#include "gui/Application.h"
#include "gui/views/DisplayView.h"

#include <cstdlib>
#include <filesystem>
#include <imgui.h>
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

/// A view drawing the display miniature, the path the presenter tab takes.
class MiniatureView final : public views::View {
public:
	void onUpdate() override {
		const auto display = std::static_pointer_cast<views::DisplayView>(Application::get().getView("display_window"));
		if (display == nullptr)
			return;
		// An explicit size, like the tab that hosts the miniature for real: an
		// auto-fitting window would have ImGui complain about the cursor moves the
		// display layout makes.
		ImGui::SetNextWindowSize({360.0f, 240.0f});
		if (ImGui::Begin("MiniatureHost")) {
			// Deliberately cramped: at this size the display panels end up with regions
			// too small to write in, which is the case that used to trip ImGui.
			display->renderInline({320.0f, 180.0f});
			++m_drawn;
		}
		ImGui::End();
	}

	[[nodiscard]] auto getName() const -> std::string override { return "miniature_view"; }
	[[nodiscard]] auto drawn() const -> uint32_t { return m_drawn; }

private:
	uint32_t m_drawn = 0;
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

TEST(gui_Application, DrawsTheDisplayMiniature) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// A running game, so the miniature draws the grid and not the empty-state message.
	auto& event = app->getCurrentEvent();
	event.setName("mini");
	event.setOrganizerName("organisateur");
	event.pushGameRound(evl::core::GameRound{});
	ASSERT_EQ(event.getStatus(), evl::core::Event::Status::Ready);
	event.nextState();
	event.nextState();
	ASSERT_EQ(event.getStatus(), evl::core::Event::Status::GameRunning);

	const auto view = std::make_shared<MiniatureView>();
	app->addView(view);
	app->setMaxFrame(3);
	app->run();
	// Drawn every frame, and the loop never had to catch anything.
	EXPECT_EQ(view->drawn(), 3U);
	EXPECT_EQ(app->getState(), Application::State::Closed);
}

TEST(gui_Application, SurvivesAnUnreadableFont) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// A font that is not there, asked for from the settings window: the atlas has to be
	// rebuilt anyway, with the embedded face, and the event must go on.
	app->setFont("/n/existe/pas.ttf", 18.0f);
	app->setMaxFrame(2);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Closed);
	EXPECT_GT(ImGui::GetIO().Fonts->Fonts.Size, 0);
}

TEST(gui_Application, RejectsAFileThatIsNotAFont) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// A real file, readable, and not a font at all: the signature check is what keeps
	// ImGui from asserting on it.
	app->setFont(EVL_TEST_DOC_FILE, 20.0f);
	app->setMaxFrame(2);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Closed);
	EXPECT_GT(ImGui::GetIO().Fonts->Fonts.Size, 0);
}

TEST(gui_Application, LoadsAFontFromAFile) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	// A system font, when the image ships one: the happy path is worth checking where
	// it can be, and skipping where it cannot rather than pinning a path.
	const std::filesystem::path candidate{"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"};
	if (!is_regular_file(candidate))
		GTEST_SKIP() << "aucune police système à essayer";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	app->setFont(candidate, 22.0f);
	app->setMaxFrame(3);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Closed);
	// The chosen face first, then the three embedded ones behind it.
	EXPECT_EQ(ImGui::GetIO().Fonts->Fonts.Size, 4);
}

TEST(gui_Application, ChangesTheFontSizeAtRuntime) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// Empty path: the embedded face, at another size. Rebuilding the atlas mid-run is
	// what the Vulkan backend's dynamic textures are for.
	app->setFont({}, 28.0f);
	app->setMaxFrame(3);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Closed);
	EXPECT_GT(ImGui::GetIO().Fonts->Fonts.Size, 0);
}

TEST(gui_Application, RebuildsTheRendererWithoutLosingTheEvent) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// A game under way: what a lost graphics card must not take with it.
	auto& event = app->getCurrentEvent();
	event.setName("reprise");
	event.setOrganizerName("organisateur");
	event.pushGameRound(evl::core::GameRound{});
	event.nextState();
	event.nextState();
	ASSERT_EQ(event.getStatus(), evl::core::Event::Status::GameRunning);

	// A few frames, so textures and glyphs are really on the device before it goes.
	app->setMaxFrame(2);
	app->run();
	ASSERT_EQ(app->getState(), Application::State::Closed);

	// The rebuild, on a device in good health: that is the teardown and setup sequence
	// being checked, without waiting for a card to fail.
	app->setRunning();
	EXPECT_TRUE(app->recoverRenderer());

	// And it keeps rendering, with the event untouched.
	app->setMaxFrame(3);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Closed);
	EXPECT_EQ(app->getCurrentEvent().getStatus(), evl::core::Event::Status::GameRunning);
	EXPECT_EQ(app->getCurrentEvent().getName(), "reprise");
	// The icons were uploaded again, so the toolbar is not a row of blanks.
	EXPECT_NE(app->getTextureLibrary().getTextureId("dice"), 0U);
}

TEST(gui_Application, DrawsTheSettingsWindow) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// The settings window is where the layout bites: a page that outgrows its box, a
	// child left open. Drawing it for a few frames is what catches that.
	const auto popup = app->getPopup("popup_main_config");
	ASSERT_NE(popup, nullptr);
	popup->open();
	app->setMaxFrame(4);
	app->run();
	EXPECT_EQ(app->getState(), Application::State::Closed);
}

TEST(gui_Application, StartsOnTheDefaultPreset) {
	if (g_needsDisplay)
		GTEST_SKIP() << "pas de session graphique sur cet agent";
	const auto app = createApplication(0, nullptr);
	ASSERT_NE(app, nullptr);
	// Settings that know nothing of presets get the default one, which is what makes the
	// restyle visible without anybody going looking for it.
	EXPECT_EQ(app->getTheme().preset, Theme::g_defaultPreset);
}
