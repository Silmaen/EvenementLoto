/**
 * @file main.cpp
 * @author Silmaen
 * @date 17/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/MainWindow.h"
#include "gui/baseDefinitions.h"
#include <QApplication>
#include <QCommandLineParser>
#include <core/Logger.h>

using namespace evl::gui;
using namespace std::filesystem;

auto main(int argc, char* argv[]) -> int {
    const QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    evl::baseExecPath= absolute(path(argv[0])).parent_path();
    QCoreApplication::setOrganizationName(QString::fromStdString(evl::EVL_AUTHOR_STR));
    QCoreApplication::setApplicationName(QString::fromStdString(evl::EVL_APP));
    QCoreApplication::setApplicationVersion(QString::fromStdString(evl::EVL_VERSION));
    evl::resetLogFile();
    evl::startSpdlog();
    spdlog::info("---------------------------------------------------------------------------------------");
    spdlog::info("Démarrage de l'application {} version {} créée par {}", evl::EVL_APP, evl::EVL_VERSION, evl::EVL_AUTHOR_STR);
    spdlog::info("Chemin d'exécution : {}", evl::baseExecPath.string());
    MainWindow window;
    window.syncSettings();
    window.show();
    auto ret= app.exec();
    spdlog::info("Sortie de l'application {} Avec le code {}", evl::EVL_APP, ret);
    spdlog::info("---------------------------------------------------------------------------------------");
    spdlog::shutdown();
    return ret;
}
