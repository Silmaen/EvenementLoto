/**
 * @file main.cpp
 * @author Silmaen
 * @date 17/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui_imgui/Application.h"
#include "gui_qt/MainWindow.h"
#include "gui_qt/baseDefinitions.h"
#include <QApplication>
#include <QCommandLineParser>
#include <core/Log.h>

using namespace evl::gui;
using namespace std::filesystem;

auto main(int iArgc, char* iArgv[]) -> int {
#ifdef EVL_DEBUG
	evl::Log::init(evl::Log::Level::Trace);
#else
	evl::Log::init(evl::Log::Level::Info);
#endif
	evl::g_baseExecPath = absolute(path(iArgv[0])).parent_path();
	log_info("---------------------------------------------------------------------------------------")
			log_info("Démarrage de l'application {} version {} créée par {}", evl::EVL_APP, evl::EVL_VERSION,
					 evl::EVL_AUTHOR_STR) log_info("Chemin d'exécution : {}", evl::g_baseExecPath.string()) int ret = 0;
	if (exists(evl::g_baseExecPath / "config.yaml")) {
		// TODO: load config from config file
		evl::g_useImGui = true;
	}
	if (evl::g_useImGui) {
		log_info("Utilisation de l'interface ImGui")
				// Startup
				auto app = evl::gui_imgui::createApplication(iArgc, iArgv);
		// Runtime
		app->run();
		// Shutdown
		app.reset();
	} else {
		log_info("Utilisation de l'interface Qt") const QApplication app(iArgc, iArgv);
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
		ret = app.exec();
	}
	log_info("Sortie de l'application {} Avec le code {}", evl::EVL_APP, ret)
			log_info("---------------------------------------------------------------------------------------")
			// Destroy the logger
			evl::Log::invalidate();
	return ret;
}
