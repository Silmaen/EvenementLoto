
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

TEST(gui_Theme, presetsAreDistinctAndNamed) {
	using Preset = evl::gui::Theme::Preset;
	EXPECT_EQ(evl::gui::Theme::presetName(Preset::Nuit), "Nuit");
	EXPECT_EQ(evl::gui::Theme::presetName(Preset::Ardoise), "Ardoise");
	EXPECT_EQ(evl::gui::Theme::presetName(Preset::Salle), "Salle");

	// Nuit is the original look, so it must stay byte for byte the struct's defaults:
	// it is the one to come back to.
	const evl::gui::Theme defaults;
	const auto nuit = evl::gui::Theme::fromPreset(Preset::Nuit);
	EXPECT_EQ(nuit.windowBackground, defaults.windowBackground);
	EXPECT_EQ(nuit.button, defaults.button);
	EXPECT_EQ(nuit.preset, Preset::Nuit);

	// The other two really are other looks.
	const auto ardoise = evl::gui::Theme::fromPreset(Preset::Ardoise);
	EXPECT_NE(ardoise.windowBackground, defaults.windowBackground);
	const auto salle = evl::gui::Theme::fromPreset(Preset::Salle);
	// A light look: its background is brighter than its text, the dark ones the reverse.
	EXPECT_GT(salle.windowBackground.x(), salle.text.x());
	EXPECT_LT(ardoise.windowBackground.x(), ardoise.text.x());
}

TEST(gui_Theme, presetSurvivesSettings) {
	auto theme = evl::gui::Theme::fromPreset(evl::gui::Theme::Preset::Ardoise);
	const auto settings = theme.saveToSettings();
	evl::gui::Theme loaded;
	loaded.loadFromSettings(settings);
	EXPECT_EQ(loaded.preset, evl::gui::Theme::Preset::Ardoise);
	EXPECT_EQ(loaded.windowBackground, theme.windowBackground);
	EXPECT_EQ(loaded.textDisabled, theme.textDisabled);
}
