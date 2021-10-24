/**
* @file DisplayWindow.cpp
* @author Silmaen
* @date 18/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "gui/DisplayWindow.h"

// Les trucs de QT
#include <gui/moc_DisplayWindow.cpp>
#include <ui/ui_DisplayWindow.h>

namespace evl::gui {

DisplayWindow::DisplayWindow(QWidget* parent):
    QDialog(parent),
    ui(new Ui::DisplayWindow) {
    ui->setupUi(this);
}

DisplayWindow::~DisplayWindow() {
    delete ui;
}

}// namespace evl::gui
