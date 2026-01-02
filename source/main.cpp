/**
 * @file main.cpp
 * @author Silmaen
 * @date 17/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#ifdef USE_QT
#include <QApplication>
#include <QCommandLineParser>
#endif
#include <core/Log.h>
#include <core/Settings.h>
#include <core/utilities.h>
#include <gui_imgui/Application.h>
#ifdef USE_QT
#include <gui_qt/MainWindow.h>
#include <gui_qt/baseDefinitions.h>
#endif

#include <magic_enum/magic_enum.hpp>

#ifdef USE_QT
using namespace evl::gui;
#endif
using namespace std::filesystem;

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
	int ret = 0;

	if (settings->getValue<bool>("general/use_imgui", false)) {
		log_info("Utilisation de l'interface ImGui");
		// Startup
		auto app = evl::gui_imgui::createApplication(iArgc, iArgv);
		// Runtime
		app->run();
		// Shutdown
		app.reset();
	} else {

#ifdef USE_QT
		log_info("Utilisation de l'interface Qt");
		const QApplication app(iArgc, iArgv);
		QCommandLineParser parser;
		parser.setApplicationDescription(QCoreApplication::applicationName());
		parser.addHelpOption();
		parser.addVersionOption();
		parser.process(app);
		QCoreApplication::setOrganizationName(QString::fromStdString(evl::EVL_AUTHOR_STR));
		QCoreApplication::setApplicationName(QString::fromStdString(evl::EVL_APP));
		QCoreApplication::setApplicationVersion(QString::fromStdString(evl::EVL_VERSION));
		MainWindow window;
		window.syncSettings();
		window.show();
		//NOLINTNEXTLINE
		ret = app.exec();
#else
		log_error("L'application n'a pas été compilée avec le support de Qt, impossible de démarrer l'interface "
				  "graphique.");
		ret = EXIT_FAILURE;
#endif
	}
	log_info("Sortie de l'application {} Avec le code {}", evl::EVL_APP, ret);
	log_info("---------------------------------------------------------------------------------------");
	evl::core::leaveSettings();
	// Destroy the logger
	evl::Log::invalidate();
	return ret;
}
