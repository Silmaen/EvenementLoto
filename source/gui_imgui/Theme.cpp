/**
 * @file Theme.cpp
 * @author Silmaen
 * @date 07/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "pch.h"

#include "Theme.h"

namespace evl::gui_imgui {

void Theme::loadFromSettings(const core::Settings& iSettings) {

	text = iSettings.getValue("Text", text);
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
	menubarBackground = iSettings.getValue("TitleBarCollapsed", titleBarCollapsed);

	scrollbarBackground = iSettings.getValue("ScrollbarBackground", scrollbarBackground);
	scrollbarGrab = iSettings.getValue("ScrollbarGrab", scrollbarGrab);
	scrollbarGrabHovered = iSettings.getValue("ScrollbarGrabHovered ", scrollbarGrabHovered);
	scrollbarGrabActive = iSettings.getValue("ScrollbarGrabActive", scrollbarGrabActive);

	checkMark = iSettings.getValue("CheckMark", checkMark);

	sliderGrab = iSettings.getValue("SliderGrab", sliderGrab);
	sliderGrabActive = iSettings.getValue("SliderGrabAct", sliderGrabActive);

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
}


auto Theme::saveToSettings() -> core::Settings {
	core::Settings settings;
	settings.setValue("Text", text);
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
	settings.setValue("SliderGrabAct", sliderGrabActive);

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
	return settings;
}

}// namespace evl::gui_imgui
