/**
 * @file utilities.cpp
 * @author Silmaen
 * @date 03/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "utilities.h"
#include "Log.h"
#include "pch.h"

namespace evl::core {

// 7: the file opens with a magic and ends with a checksum, lengths and dates are
// written on an explicit fixed width. Files from 6 and below are still read.
constexpr uint16_t g_currentSaveVersion = 7;

namespace {

std::shared_ptr<Settings> g_settings;

std::filesystem::path g_baseExecPath;

}// namespace

void initializeUtilities([[maybe_unused]] int iArgc, char* iArgv[]) {
	g_baseExecPath = std::filesystem::absolute(std::filesystem::path(iArgv[0])).parent_path();
	g_settings = std::make_shared<Settings>();
}

auto getExecPath() -> std::filesystem::path { return g_baseExecPath; }

auto getDataLocation() -> std::filesystem::path {
	const auto configured = getSettings()->getValue<std::string>("general/data_location");
	if (configured.empty())
		return g_baseExecPath / "data";
	return std::filesystem::path{configured};
}

auto getConfigFile() -> std::filesystem::path { return g_baseExecPath / "config.yml"; }

auto getSettings() -> std::shared_ptr<Settings> {
	if (g_settings == nullptr)
		g_settings = std::make_shared<Settings>();
	return g_settings;
}

void loadSettings() {
	const auto settings = getSettings();
	const auto path = getConfigFile();
	std::error_code error;
	const bool present = exists(path, error) && !error;
	settings->fromFile(path);
	if (present)
		return;
	// First start: the defaults are written out right away, so the organizer has a file
	// to read and edit instead of having to guess which keys exist.
	mergeDefaultSettings();
	saveSettings();
	log_info("Réglages par défaut enregistrés dans '{}'.", path.string());
}

void mergeDefaultSettings() {
	if (g_settings != nullptr) {
		// General settings
		if (!g_settings->contains("general/log_level")) {
			g_settings->setValue("general/log_level", std::string("info"));
		}
		if (!g_settings->contains("gui/vulkan_device")) {
			// Empty: a dedicated card is preferred on its own. A fragment of a device
			// name imposes one, which is what a machine with two graphics devices
			// needs — the better card is the one carrying the video output.
			g_settings->setValue("gui/vulkan_device", std::string(""));
		}
		if (!g_settings->contains("gui/display_server")) {
			// x11 by default, XWayland included: it is the only way a detached window
			// can be placed on a chosen screen. `wayland` or `auto` are the other
			// values.
			g_settings->setValue("gui/display_server", std::string("x11"));
		}
		if (!g_settings->contains("gui/recent_paths")) {
			// The recently used files and folders, newline separated: the YAML backend
			// writes strings, and five paths do not deserve a structure.
			g_settings->setValue("gui/recent_paths", std::string(""));
		}
		if (!g_settings->contains("general/data_location")) {
			// A string, like everything the YAML backend can write back.
			g_settings->setValue("general/data_location", (g_baseExecPath / "data").string());
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
