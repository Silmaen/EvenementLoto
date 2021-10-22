/**
 * @file ConfigGeneral.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/ConfigGeneral.h"
#include "gui/MainWindow.h"
#include "gui/baseDefinitions.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>

// Les trucs de QT
#include "gui/moc_ConfigGeneral.cpp"
#include "ui/ui_ConfigGeneral.h"

namespace evl::gui {

ConfigGeneral::ConfigGeneral(MainWindow* parent):
    QDialog(parent),
    ui(new Ui::ConfigGeneral), mwd(parent) {
    ui->setupUi(this);
}

ConfigGeneral::~ConfigGeneral() {
    delete ui;
}

void ConfigGeneral::SaveFile() {
    if(mwd != nullptr) {
        mwd->getSettings().setValue(dataPathKey, ui->DataLocation->text());
        mwd->syncSettings();
    }
}

void ConfigGeneral::actOk() {
    SaveFile();
    accept();
}

void ConfigGeneral::actApply() {
    SaveFile();
}

void ConfigGeneral::actCancel() {
    reject();
}

void ConfigGeneral::actSearchFolder() {
    QFileDialog dia;
    dia.setAcceptMode(QFileDialog::AcceptOpen);
    dia.setFileMode(QFileDialog::FileMode::Directory);
    if(dia.exec()) {
        ui->DataLocation->setText(dia.selectedFiles()[0]);
    }
}

void ConfigGeneral::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

int ConfigGeneral::exec() {
    if(mwd != nullptr) {
        ui->DataLocation->setText(mwd->getSettings().value(dataPathKey, "").toString());
    }
    return QDialog::exec();
}

}// namespace evl::gui
