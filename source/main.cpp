/**
 * @file main.cpp
 * @author Silmaen
 * @date 17/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include <core/Log.h>
#include <core/Settings.h>
#include <core/utilities.h>
#include <gui/Application.h>

#include <magic_enum/magic_enum.hpp>

namespace {

auto run(int iArgc, char* iArgv[]) -> int {
	evl::core::initializeUtilities(iArgc, iArgv);
	evl::core::loadSettings();
	evl::core::mergeDefaultSettings();
	const auto settings = evl::core::getSettings();
	if (!settings->getValue<std::string>("general/log_level", "").empty()) {
		const auto loglevel = settings->getValue<std::string>(
				"general/log_level", std::string(magic_enum::enum_name(evl::Log::getVerbosityLevel())));
		if (const auto val = magic_enum::enum_cast<evl::Log::Level>(loglevel); val.has_value()) {
			evl::Log::setVerbosityLevel(val.value());
		}
	}
	settings->setValue("general/log_level", std::string(magic_enum::enum_name(evl::Log::getVerbosityLevel())));
	log_info("---------------------------------------------------------------------------------------");
	log_info("Démarrage de l'application {} version {} créée par {}", evl::EVL_APP, evl::EVL_VERSION,
			 evl::EVL_AUTHOR_STR);
	log_info("Chemin d'exécution : {}", evl::core::getExecPath().string());

	auto app = evl::gui::createApplication(iArgc, iArgv);
	try {
		app->run();
	} catch (...) {
		// Save while the application is still alive: its destructor runs during the
		// unwinding that follows.
		app->saveProgress();
		throw;
	}
	const int ret = app->getState() == evl::gui::Application::State::Error ? EXIT_FAILURE : EXIT_SUCCESS;
	app.reset();

	log_info("Sortie de l'application {} Avec le code {}", evl::EVL_APP, ret);
	log_info("---------------------------------------------------------------------------------------");
	evl::core::leaveSettings();
	return ret;
}

void reportFatal(const std::string_view& iWhat) noexcept {
	try {
		log_critical("Exception non rattrapée : {}", iWhat);
		// NOLINTNEXTLINE(bugprone-empty-catch): last resort, nothing left to report with
	} catch (...) {}
}

}// namespace

auto main(int iArgc, char* iArgv[]) -> int {
	int ret = EXIT_FAILURE;
	// Nothing must escape: an uncaught exception would terminate the process without a
	// trace, in the middle of a game.
	try {
#ifdef EVL_DEBUG
		evl::Log::init(evl::Log::Level::Trace);
#else
		evl::Log::init(evl::Log::Level::Info);
#endif
		ret = run(iArgc, iArgv);
		evl::Log::invalidate();
	} catch (const std::exception& e) { reportFatal(e.what()); } catch (...) {
		reportFatal("type inconnu");
	}
	return ret;
}
