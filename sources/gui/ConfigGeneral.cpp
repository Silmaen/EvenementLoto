/**
 * @file ConfigGeneral.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

// Les trucs de QT
#include "gui/moc_ConfigGeneral.cpp"
#include "ui/ui_ConfigGeneral.h"

#include "gui/BaseDialog.h"
#include "gui/ConfigGeneral.h"
#include "gui/MainWindow.h"
#include "gui/baseDefinitions.h"
#include <fstream>
#include <iostream>

namespace evl::gui {

using json= nlohmann::json;

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
        mwd->getSettings().setValue(settings::dataPathKey, ui->DataLocation->text());
        mwd->getSettings().setValue(settings::themeNameKey, ui->editThemeName->text());
        mwd->getSettings().setValue(settings::globalScaleKey, ui->spinMainScale->value());
        mwd->getSettings().setValue(settings::titleScaleKey, ui->spinTitleScale->value());
        mwd->getSettings().setValue(settings::shortTextScaleKey, ui->spinShortTextScale->value());
        mwd->getSettings().setValue(settings::gridTextScaleKey, ui->spinGridTextScale->value());
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
    auto path= dialog::openFile(dialog::FileTypes::Folder, true);
    if(path.empty())
        return;
    ui->DataLocation->setText(QString::fromUtf8(path.string()));
}
void ConfigGeneral::actResetTheme() {
    preExec();
}

void ConfigGeneral::actImportTheme() {
    auto path= dialog::openFile(dialog::FileTypes::ThemeFile, true);
    if(path.empty())
        return;
}

void ConfigGeneral::actExportTheme() {
    auto path= dialog::openFile(dialog::FileTypes::ThemeFile, false);
    if(path.empty())
        return;
    std::ofstream file(path, std::ios::out);
    file << std::setw(4)
         << json{
                    {"name", mwd->getSettings().value(settings::themeNameKey, settings::themeNameDefault).toString().toStdString()},
                    {"globalScale", mwd->getSettings().value(settings::globalScaleKey, settings::globalScaleDefault).toFloat()},
                    {"titleScale", mwd->getSettings().value(settings::titleScaleKey, settings::titleScaleDefault).toFloat()},
                    {"shortTextScale", mwd->getSettings().value(settings::shortTextScaleKey, settings::shortTextScaleDefault).toFloat()},
                    {"longTextScale", mwd->getSettings().value(settings::longTextScaleKey, settings::longTextScaleDefault).toFloat()},
                    {"gridTextScale", mwd->getSettings().value(settings::gridTextScaleKey, settings::gridTextScaleDefault).toFloat()},
            };
    file.close();
}

void ConfigGeneral::preExec() {
    if(mwd != nullptr) {
        ui->DataLocation->setText(mwd->getSettings().value(settings::dataPathKey, settings::dataPathDefault).toString());
        ui->editThemeName->setText(mwd->getSettings().value(settings::themeNameKey, settings::themeNameDefault).toString());
        ui->spinMainScale->setValue(mwd->getSettings().value(settings::globalScaleKey, settings::globalScaleDefault).toFloat());
        ui->spinTitleScale->setValue(mwd->getSettings().value(settings::titleScaleKey, settings::titleScaleDefault).toFloat());
        ui->spinShortTextScale->setValue(mwd->getSettings().value(settings::shortTextScaleKey, settings::shortTextScaleDefault).toFloat());
        ui->spinShortTextScale->setValue(mwd->getSettings().value(settings::longTextScaleKey, settings::longTextScaleDefault).toFloat());
        ui->spinGridTextScale->setValue(mwd->getSettings().value(settings::gridTextScaleKey, settings::gridTextScaleDefault).toFloat());
    }
}

}// namespace evl::gui
