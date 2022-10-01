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
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

using namespace evl::gui;
using namespace std::filesystem;

void startSpdlog() {
    spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>((baseExecPath / "exec.log").generic_string()));
#ifdef EVL_DEBUG
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_every(std::chrono::seconds(1U));
#else
    spdlog::set_level(spdlog::level::info);
#endif
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    baseExecPath= absolute(path(argv[0])).parent_path();
    QCoreApplication::setOrganizationName("Damien Lachouette");
    QCoreApplication::setApplicationName("Événement Loto");
    QCoreApplication::setApplicationVersion("1.0");
    startSpdlog();
    spdlog::info("Démarrage de l'application Événement Loto version 1.0 créée par Damien Lachouette");
    MainWindow window;
    window.syncSettings();
    window.show();
    auto ret= app.exec();
    spdlog::info("Sortie de l'application Événement Loto Avec le code {}", ret);
    return ret;
}
