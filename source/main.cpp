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

auto main(int iArgc, char* iArgv[]) -> int {
#ifdef EVL_DEBUG
	evl::Log::init(evl::Log::Level::Trace);
#else
	evl::Log::init(evl::Log::Level::Info);
#endif
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
	app->run();
	const int ret = app->getState() == evl::gui::Application::State::Error ? EXIT_FAILURE : EXIT_SUCCESS;
	app.reset();

	log_info("Sortie de l'application {} Avec le code {}", evl::EVL_APP, ret);
	log_info("---------------------------------------------------------------------------------------");
	evl::core::leaveSettings();
	evl::Log::invalidate();
	return ret;
}
