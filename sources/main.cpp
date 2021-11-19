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
#include <QCommandLineOption>
#include <QCommandLineParser>

using namespace evl::gui;
using namespace std::filesystem;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    baseExecPath= absolute(path(argv[0])).parent_path();
    QCoreApplication::setOrganizationName("Damien Lachouette");
    QCoreApplication::setApplicationName("Evenemlent Loto");
    QCoreApplication::setApplicationVersion("1.0");

    MainWindow window;
    window.syncSettings();
    window.show();
    return app.exec();
}
