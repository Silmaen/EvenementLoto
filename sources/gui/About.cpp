/**
 * @file About.cpp
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/About.h"

// Les trucs de QT
#include "gui/moc_About.cpp"
#include "ui/ui_About.h"

namespace evl::gui {

About::About(QWidget* parent):
    QDialog(parent),
    ui(new Ui::About) {
    ui->setupUi(this);
}
About::~About() {
    delete ui;
}

}// namespace evl
