/**
 * @file utilities.cpp
 * @author Silmaen
 * @date 03/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "utilities.h"
#include "pch.h"

namespace evl::core {

constexpr uint16_t g_currentSaveVersion = 6;

namespace {

std::shared_ptr<Settings> g_settings;

std::filesystem::path g_baseExecPath;

}// namespace

void initializeUtilities([[maybe_unused]] int iArgc, char* iArgv[]) {
	g_baseExecPath = std::filesystem::absolute(std::filesystem::path(iArgv[0])).parent_path();
	g_settings = std::make_shared<Settings>();
}

auto getExecPath() -> std::filesystem::path { return g_baseExecPath; }

auto getConfigFile() -> std::filesystem::path { return g_baseExecPath / "config.yml"; }

auto getSettings() -> std::shared_ptr<Settings> {
	if (g_settings == nullptr)
		g_settings = std::make_shared<Settings>();
	return g_settings;
}

void loadSettings() {
	const auto settings = getSettings();
	settings->fromFile(getConfigFile());
}

void mergeDefaultSettings() {
	if (g_settings != nullptr) {
		// General settings
		if (!g_settings->contains("general/use_imgui")) {
			g_settings->setValue("general/use_imgui", false);
		}
		if (!g_settings->contains("general/log_level")) {
			g_settings->setValue("general/log_level", std::string("info"));
		}
	}
}

void saveSettings() {
	if (g_settings != nullptr) {
		g_settings->toFile(getConfigFile());
	}
}

void leaveSettings() {
	saveSettings();
	if (g_settings != nullptr) {
		g_settings.reset();
	}
}

auto getSaveVersion() -> uint16_t { return g_currentSaveVersion; }

}// namespace evl::core
