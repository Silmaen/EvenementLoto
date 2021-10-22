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

using namespace evl::gui;
using namespace std::filesystem;

int main(int argc, char* argv[]) {
    baseExecPath= absolute(path(argv[0])).parent_path();
    QApplication a(argc, argv);
    MainWindow window;
    window.syncSettings();
    window.show();
    return QApplication::exec();
}
