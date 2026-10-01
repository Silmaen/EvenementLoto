/**
 * @file Theme.cpp
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Theme.h"

#include "core/Log.h"

namespace evl::gui {

namespace {

/// Les cinq couleurs dont tout un habillage se déduit.
struct Palette {
	math::vec4 surface;///< Le fond des fenêtres.
	math::vec4 raised;///< Les panneaux, barres et en-têtes, posés sur le fond.
	math::vec4 sunken;///< Les champs de saisie et autres creux.
	math::vec4 accent;///< La seule couleur qui veuille dire « ici ».
	math::vec4 text;///< Le texte.
};

/// Mélange deux couleurs, l'alpha de la première étant conservé.
auto mix(const math::vec4& iFrom, const math::vec4& iTo, const float iRatio) -> math::vec4 {
	return {iFrom.x() + (iTo.x() - iFrom.x()) * iRatio, iFrom.y() + (iTo.y() - iFrom.y()) * iRatio,
			iFrom.z() + (iTo.z() - iFrom.z()) * iRatio, iFrom.w()};
}

/// La même couleur, avec une autre opacité.
auto fade(const math::vec4& iColor, const float iAlpha) -> math::vec4 {
	return {iColor.x(), iColor.y(), iColor.z(), iAlpha};
}

/**
 * @brief Déduit tout l'habillage d'une palette de cinq couleurs.
 *
 * C'est ce qui fait qu'un habillage se tient : les cinquante couleurs d'ImGui ne sont
 * pas choisies une par une, elles descendent toutes des mêmes cinq, par mélange. Un
 * survol est un pas vers l'accent, un état actif deux pas, un texte grisé un pas vers
 * le fond.
 *
 * @param[in,out] ioTheme Le thème à remplir.
 * @param[in] iPalette La palette de départ.
 */
void applyPalette(Theme& ioTheme, const Palette& iPalette) {
	const auto& surface = iPalette.surface;
	const auto& raised = iPalette.raised;
	const auto& sunken = iPalette.sunken;
	const auto& accent = iPalette.accent;

	ioTheme.text = iPalette.text;
	ioTheme.textDisabled = mix(iPalette.text, surface, 0.55f);
	ioTheme.windowBackground = surface;
	ioTheme.childBackground = raised;
	ioTheme.backgroundPopup = mix(surface, iPalette.text, 0.06f);
	ioTheme.border = mix(surface, iPalette.text, 0.18f);

	ioTheme.frameBackground = sunken;
	ioTheme.frameBackgroundHovered = mix(sunken, accent, 0.20f);
	ioTheme.frameBackgroundActive = mix(sunken, accent, 0.40f);

	ioTheme.titleBar = raised;
	ioTheme.titleBarActive = mix(raised, accent, 0.45f);
	ioTheme.titleBarCollapsed = raised;
	ioTheme.menubarBackground = raised;

	ioTheme.scrollbarBackground = fade(sunken, 0.6f);
	ioTheme.scrollbarGrab = mix(raised, accent, 0.35f);
	ioTheme.scrollbarGrabHovered = mix(raised, accent, 0.55f);
	ioTheme.scrollbarGrabActive = accent;

	ioTheme.checkMark = accent;
	ioTheme.sliderGrab = mix(raised, accent, 0.7f);
	ioTheme.sliderGrabActive = accent;

	ioTheme.button = mix(raised, iPalette.text, 0.08f);
	ioTheme.buttonHovered = mix(raised, accent, 0.45f);
	ioTheme.buttonActive = accent;

	ioTheme.groupHeader = raised;
	ioTheme.groupHeaderHovered = mix(raised, accent, 0.40f);
	ioTheme.groupHeaderActive = mix(raised, accent, 0.60f);

	ioTheme.separator = mix(surface, iPalette.text, 0.15f);
	ioTheme.separatorHovered = fade(accent, 0.8f);
	ioTheme.separatorActive = accent;

	ioTheme.resizeGrip = fade(accent, 0.3f);
	ioTheme.resizeGripHovered = fade(accent, 0.7f);
	ioTheme.resizeGripActive = accent;

	ioTheme.tab = sunken;
	ioTheme.tabHovered = mix(raised, accent, 0.40f);
	ioTheme.tabSelected = raised;
	ioTheme.tabSelectedOverline = accent;
	ioTheme.tabDimmed = mix(sunken, surface, 0.5f);
	ioTheme.tabDimmedSelected = raised;
	ioTheme.tabDimmedSelectedOverline = mix(raised, accent, 0.5f);

	ioTheme.dockingPreview = fade(accent, 0.7f);
	ioTheme.dockingEmptyBackground = raised;

	ioTheme.plotLines = accent;
	ioTheme.plotLinesHovered = mix(accent, iPalette.text, 0.3f);
	ioTheme.plotHistogram = mix(accent, surface, 0.2f);
	ioTheme.plotHistogramHovered = accent;

	ioTheme.tableHeaderBg = raised;
	ioTheme.tableBorderLight = mix(surface, iPalette.text, 0.12f);
	ioTheme.tableRowBg = surface;
	ioTheme.tableRowBgAlt = raised;

	ioTheme.textSelectedBg = fade(mix(surface, accent, 0.5f), 0.8f);
	ioTheme.dragDropTarget = fade(accent, 0.9f);

	ioTheme.navHighlight = accent;
	ioTheme.navWindowingHighlight = accent;
	ioTheme.navWindowingDimBg = fade(surface, 0.5f);
	ioTheme.modalWindowDimBg = fade(mix(surface, {0.0f, 0.0f, 0.0f, 1.0f}, 0.7f), 0.6f);

	ioTheme.highlight = accent;
	ioTheme.propertyField = sunken;
}

}// namespace

auto Theme::presetName(const Preset iPreset) -> std::string_view {
	switch (iPreset) {
		case Preset::Nuit:
			return "Nuit";
		case Preset::Ardoise:
			return "Ardoise";
		case Preset::Salle:
			return "Salle";
	}
	return "Nuit";
}

auto Theme::fromPreset(const Preset iPreset) -> Theme {
	Theme theme;
	theme.preset = iPreset;
	switch (iPreset) {
		case Preset::Nuit:
			// L'habillage d'origine : ce sont les valeurs par défaut de la structure, il
			// n'y a donc rien à faire. Il reste tel quel pour pouvoir y revenir.
			break;
		case Preset::Ardoise:
			// Gris neutres et un seul accent ambre — celui qui marque déjà le dernier
			// numéro tiré sur le panneau d'affichage, pour que l'outil et l'écran des
			// joueurs parlent la même langue. Angles plus francs et rythme vertical plus
			// large : un instrument de travail, lisible à un mètre.
			applyPalette(theme, {.surface = {0.13f, 0.13f, 0.14f, 1.0f},
								 .raised = {0.18f, 0.18f, 0.19f, 1.0f},
								 .sunken = {0.09f, 0.09f, 0.10f, 1.0f},
								 .accent = {1.0f, 0.62f, 0.13f, 1.0f},
								 .text = {0.92f, 0.92f, 0.91f, 1.0f}});
			theme.windowRounding = 4.0f;
			theme.frameRounding = 4.0f;
			theme.tabRounding = 4.0f;
			theme.controlsRounding = 4.0f;
			theme.frameBorderSize = 1.0f;
			theme.tabOverline = 2.5f;
			theme.itemSpacing = {8.0f, 6.0f};
			theme.itemInnerSpacing = {6.0f, 4.0f};
			theme.framePadding = {8.0f, 5.0f};
			theme.cellPadding = {6.0f, 3.0f};
			theme.indentSpacing = 14.0f;
			break;
		case Preset::Salle:
			// Une salle des fêtes est mal éclairée et l'écran de contrôle est souvent vu
			// de biais : fond clair, texte presque noir, cibles larges.
			applyPalette(theme, {.surface = {0.95f, 0.95f, 0.94f, 1.0f},
								 .raised = {0.88f, 0.88f, 0.87f, 1.0f},
								 .sunken = {1.0f, 1.0f, 1.0f, 1.0f},
								 .accent = {0.13f, 0.40f, 0.72f, 1.0f},
								 .text = {0.10f, 0.10f, 0.11f, 1.0f}});
			theme.windowRounding = 3.0f;
			theme.frameRounding = 3.0f;
			theme.tabRounding = 3.0f;
			theme.controlsRounding = 3.0f;
			theme.frameBorderSize = 1.0f;
			theme.tabOverline = 3.0f;
			theme.itemSpacing = {10.0f, 8.0f};
			theme.itemInnerSpacing = {6.0f, 6.0f};
			theme.framePadding = {10.0f, 7.0f};
			theme.cellPadding = {8.0f, 4.0f};
			theme.indentSpacing = 16.0f;
			break;
	}
	return theme;
}

void Theme::loadFromSettings(const core::Settings& iSettings) {
	if (!iSettings.contains("Preset")) {
		// Des réglages d'avant les habillages : les couleurs qu'ils portent ne sont pas
		// un choix, ce sont les anciennes valeurs par défaut, qu'aucune interface ne
		// permettait de modifier. L'habillage par défaut prend donc leur place, et se
		// change en un clic.
		*this = fromPreset(g_defaultPreset);
		log_info("Habillage par défaut appliqué : {}", presetName(preset));
		return;
	}
	if (const auto name = iSettings.getValue<std::string>("Preset", std::string{presetName(preset)}); !name.empty()) {
		for (const auto& candidate: magic_enum::enum_values<Preset>()) {
			if (presetName(candidate) == name) {
				preset = candidate;
				break;
			}
		}
	}
	text = iSettings.getValue("Text", text);
	textDisabled = iSettings.getValue("TextDisabled", textDisabled);
	windowBackground = iSettings.getValue("WindowBackground", windowBackground);
	childBackground = iSettings.getValue("ChildBackground", childBackground);
	backgroundPopup = iSettings.getValue("BackgroundPopup", backgroundPopup);
	border = iSettings.getValue("Border", border);

	frameBackground = iSettings.getValue("FrameBackground", frameBackground);
	frameBackgroundHovered = iSettings.getValue("FrameBackgroundHovered", frameBackgroundHovered);
	frameBackgroundActive = iSettings.getValue("FrameBackgroundActive", frameBackgroundActive);

	titleBar = iSettings.getValue("TitleBar", titleBar);
	titleBarActive = iSettings.getValue("TitleBarActive", titleBarActive);
	titleBarCollapsed = iSettings.getValue("TitleBarCollapsed", titleBarCollapsed);
	menubarBackground = iSettings.getValue("MenubarBackground", menubarBackground);

	scrollbarBackground = iSettings.getValue("ScrollbarBackground", scrollbarBackground);
	scrollbarGrab = iSettings.getValue("ScrollbarGrab", scrollbarGrab);
	scrollbarGrabHovered = iSettings.getValue("ScrollbarGrabHovered", scrollbarGrabHovered);
	scrollbarGrabActive = iSettings.getValue("ScrollbarGrabActive", scrollbarGrabActive);

	checkMark = iSettings.getValue("CheckMark", checkMark);

	sliderGrab = iSettings.getValue("SliderGrab", sliderGrab);
	sliderGrabActive = iSettings.getValue("SliderGrabActive", sliderGrabActive);

	button = iSettings.getValue("Button", button);
	buttonHovered = iSettings.getValue("ButtonHovered", buttonHovered);
	buttonActive = iSettings.getValue("ButtonActive", buttonActive);

	groupHeader = iSettings.getValue("GroupHeader", groupHeader);
	groupHeaderHovered = iSettings.getValue("GroupHeaderHovered", groupHeaderHovered);
	groupHeaderActive = iSettings.getValue("GroupHeaderActive", groupHeaderActive);

	separator = iSettings.getValue("Separator", separator);
	separatorActive = iSettings.getValue("SeparatorActive", separatorActive);
	separatorHovered = iSettings.getValue("SeparatorHovered", separatorHovered);

	resizeGrip = iSettings.getValue("ResizeGrip", resizeGrip);
	resizeGripHovered = iSettings.getValue("ResizeGripHovered", resizeGripHovered);
	resizeGripActive = iSettings.getValue("ResizeGripActive", resizeGripActive);

	tabHovered = iSettings.getValue("TabHovered", tabHovered);
	tab = iSettings.getValue("Tab", tab);
	tabSelected = iSettings.getValue("TabSelected", tabSelected);
	tabSelectedOverline = iSettings.getValue("TabSelectedOverline", tabSelectedOverline);
	tabDimmed = iSettings.getValue("TabDimmed", tabDimmed);
	tabDimmedSelected = iSettings.getValue("TabDimmedSelected", tabDimmedSelected);
	tabDimmedSelectedOverline = iSettings.getValue("TabDimmedSelectedOverline", tabDimmedSelectedOverline);

	dockingPreview = iSettings.getValue("DockingPreview", dockingPreview);
	dockingEmptyBackground = iSettings.getValue("DockingEmptyBackground", dockingEmptyBackground);

	plotLines = iSettings.getValue("PlotLines", plotLines);
	plotLinesHovered = iSettings.getValue("PlotLinesHovered", plotLinesHovered);
	plotHistogram = iSettings.getValue("PlotHistogram", plotHistogram);
	plotHistogramHovered = iSettings.getValue("PlotHistogramHovered", plotHistogramHovered);

	tableHeaderBg = iSettings.getValue("TableHeaderBg", tableHeaderBg);
	tableBorderLight = iSettings.getValue("TableBorderLight", tableBorderLight);
	tableRowBg = iSettings.getValue("TableRowBg", tableRowBg);
	tableRowBgAlt = iSettings.getValue("TableRowBgAlt", tableRowBgAlt);

	textSelectedBg = iSettings.getValue("TextSelectedBg", textSelectedBg);
	dragDropTarget = iSettings.getValue("DragDropTarget", dragDropTarget);

	navHighlight = iSettings.getValue("NavHighlight", navHighlight);
	navWindowingHighlight = iSettings.getValue("NavWindowingHighlight", navWindowingHighlight);
	navWindowingDimBg = iSettings.getValue("NavWindowingDimBg", navWindowingDimBg);
	modalWindowDimBg = iSettings.getValue("ModalWindowDimBg", modalWindowDimBg);

	highlight = iSettings.getValue("Highlight", highlight);
	propertyField = iSettings.getValue("PropertyField", propertyField);

	windowRounding = iSettings.getValue("WindowRounding", windowRounding);
	frameRounding = iSettings.getValue("FrameRounding", frameRounding);
	frameBorderSize = iSettings.getValue("FrameBorderSize", frameBorderSize);
	indentSpacing = iSettings.getValue("IndentSpacing", indentSpacing);

	tabRounding = iSettings.getValue("TabRounding", tabRounding);
	tabOverline = iSettings.getValue("TabOverline", tabOverline);
	tabBorder = iSettings.getValue("TabBorder", tabBorder);

	controlsRounding = iSettings.getValue("ControlsRounding", controlsRounding);

	itemSpacing = iSettings.getValue("ItemSpacing", itemSpacing);
	itemInnerSpacing = iSettings.getValue("ItemInnerSpacing", itemInnerSpacing);
	cellPadding = iSettings.getValue("CellPadding", cellPadding);
	framePadding = iSettings.getValue("FramePadding", framePadding);
	buttonTextAlign = iSettings.getValue("ButtonTextAlign", buttonTextAlign);
	selectableTextAlign = iSettings.getValue("SelectableTextAlign", selectableTextAlign);
	displayWindowPadding = iSettings.getValue("DisplayWindowPadding", displayWindowPadding);
	displaySafeAreaPadding = iSettings.getValue("DisplaySafeAreaPadding", displaySafeAreaPadding);

	mouseCursorScale = iSettings.getValue("MouseCursorScale", mouseCursorScale);
}

auto Theme::saveToSettings() -> core::Settings {
	core::Settings settings;

	settings.setValue("Preset", std::string{presetName(preset)});
	settings.setValue("Text", text);
	settings.setValue("TextDisabled", textDisabled);
	settings.setValue("WindowBackground", windowBackground);
	settings.setValue("ChildBackground", childBackground);
	settings.setValue("BackgroundPopup", backgroundPopup);
	settings.setValue("Border", border);

	settings.setValue("FrameBackground", frameBackground);
	settings.setValue("FrameBackgroundHovered", frameBackgroundHovered);
	settings.setValue("FrameBackgroundActive", frameBackgroundActive);

	settings.setValue("TitleBar", titleBar);
	settings.setValue("TitleBarActive", titleBarActive);
	settings.setValue("TitleBarCollapsed", titleBarCollapsed);
	settings.setValue("MenubarBackground", menubarBackground);

	settings.setValue("ScrollbarBackground", scrollbarBackground);
	settings.setValue("ScrollbarGrab", scrollbarGrab);
	settings.setValue("ScrollbarGrabHovered", scrollbarGrabHovered);
	settings.setValue("ScrollbarGrabActive", scrollbarGrabActive);

	settings.setValue("CheckMark", checkMark);

	settings.setValue("SliderGrab", sliderGrab);
	settings.setValue("SliderGrabActive", sliderGrabActive);

	settings.setValue("Button", button);
	settings.setValue("ButtonHovered", buttonHovered);
	settings.setValue("ButtonActive", buttonActive);

	settings.setValue("GroupHeader", groupHeader);
	settings.setValue("GroupHeaderHovered", groupHeaderHovered);
	settings.setValue("GroupHeaderActive", groupHeaderActive);

	settings.setValue("Separator", separator);
	settings.setValue("SeparatorActive", separatorActive);
	settings.setValue("SeparatorHovered", separatorHovered);

	settings.setValue("ResizeGrip", resizeGrip);
	settings.setValue("ResizeGripHovered", resizeGripHovered);
	settings.setValue("ResizeGripActive", resizeGripActive);

	settings.setValue("TabHovered", tabHovered);
	settings.setValue("Tab", tab);
	settings.setValue("TabSelected", tabSelected);
	settings.setValue("TabSelectedOverline", tabSelectedOverline);
	settings.setValue("TabDimmed", tabDimmed);
	settings.setValue("TabDimmedSelected", tabDimmedSelected);
	settings.setValue("TabDimmedSelectedOverline", tabDimmedSelectedOverline);

	settings.setValue("DockingPreview", dockingPreview);
	settings.setValue("DockingEmptyBackground", dockingEmptyBackground);

	settings.setValue("PlotLines", plotLines);
	settings.setValue("PlotLinesHovered", plotLinesHovered);
	settings.setValue("PlotHistogram", plotHistogram);
	settings.setValue("PlotHistogramHovered", plotHistogramHovered);

	settings.setValue("TableHeaderBg", tableHeaderBg);
	settings.setValue("TableBorderLight", tableBorderLight);
	settings.setValue("TableRowBg", tableRowBg);
	settings.setValue("TableRowBgAlt", tableRowBgAlt);

	settings.setValue("TextSelectedBg", textSelectedBg);
	settings.setValue("DragDropTarget", dragDropTarget);

	settings.setValue("NavHighlight", navHighlight);
	settings.setValue("NavWindowingHighlight", navWindowingHighlight);
	settings.setValue("NavWindowingDimBg", navWindowingDimBg);
	settings.setValue("ModalWindowDimBg", modalWindowDimBg);

	settings.setValue("Highlight", highlight);
	settings.setValue("PropertyField", propertyField);

	settings.setValue("WindowRounding", windowRounding);
	settings.setValue("FrameRounding", frameRounding);
	settings.setValue("FrameBorderSize", frameBorderSize);
	settings.setValue("IndentSpacing", indentSpacing);

	settings.setValue("TabRounding", tabRounding);
	settings.setValue("TabOverline", tabOverline);
	settings.setValue("TabBorder", tabBorder);

	settings.setValue("ControlsRounding", controlsRounding);

	settings.setValue("ItemSpacing", itemSpacing);
	settings.setValue("ItemInnerSpacing", itemInnerSpacing);
	settings.setValue("CellPadding", cellPadding);
	settings.setValue("FramePadding", framePadding);
	settings.setValue("ButtonTextAlign", buttonTextAlign);
	settings.setValue("SelectableTextAlign", selectableTextAlign);
	settings.setValue("DisplayWindowPadding", displayWindowPadding);
	settings.setValue("DisplaySafeAreaPadding", displaySafeAreaPadding);

	settings.setValue("MouseCursorScale", mouseCursorScale);

	return settings;
}

}// namespace evl::gui
