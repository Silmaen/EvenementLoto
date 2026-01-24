
#include "../TestMainHelper.h"
#include "gui/Theme.h"

TEST(gui_Theme, instantiate) {
	constexpr evl::gui::Theme theme;
	EXPECT_EQ(theme.backgroundPopup, evl::math::vec4(0.25f, 0.25f, 0.27f, 1.0f));
}

TEST(gui_Theme, saveLoadSettings) {
	evl::gui::Theme theme;
	theme.text = evl::math::vec4(0.1f, 0.2f, 0.3f, 0.4f);
	const auto settings = theme.saveToSettings();
	evl::gui::Theme loadedTheme;
	loadedTheme.loadFromSettings(settings);
	EXPECT_EQ(loadedTheme.text, evl::math::vec4(0.1f, 0.2f, 0.3f, 0.4f));
}
