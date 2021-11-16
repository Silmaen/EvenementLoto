/**
 * @file ConfigGeneral.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "gui/ConfigGeneral.h"
#include "gui/BaseDialog.h"
#include "gui/MainWindow.h"
#include "gui/baseDefinitions.h"
#include <fstream>
#include <iostream>

// Les trucs de QT
#include "gui/moc_ConfigGeneral.cpp"
#include "ui/ui_ConfigGeneral.h"

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
        mwd->getTheme().setParam("name", ui->editThemeName->text());
        mwd->getTheme().setParam("baseRatio", ui->spinMainScale->value());
        mwd->getTheme().setParam("titleRatio", ui->spinTitleScale->value());
        mwd->getTheme().setParam("gridTextRatio", ui->spinGridTextScale->value());
        mwd->getTheme().setParam("shortTextRatio", ui->spinShortTextScale->value());
        mwd->getTheme().setParam("longTextRatio", ui->spinLongTextScale->value());
        mwd->getTheme().writeInSettings();
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
    if(mwd)
        mwd->getTheme().setFromSettings();
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

void ConfigGeneral::actRestoreTheme() {
    mwd->getTheme().resetFactory();
    preExec();
}

void ConfigGeneral::actImportTheme() {
    auto path= dialog::openFile(dialog::FileTypes::ThemeFile, true);
    if(path.empty())
        return;
    mwd->getTheme().importJSON(path);
    preExec();
}

void ConfigGeneral::actExportTheme() {
    auto path= dialog::openFile(dialog::FileTypes::ThemeFile, false);
    if(path.empty())
        return;
    mwd->getTheme().exportJSON(path);
}

void buttonSetColor(QPushButton* btn, const QColor& col) {
    auto palette= btn->palette();
    palette.setColor(QPalette::Button, col);
    if(col.lightness() < 100)
        palette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    else
        palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    btn->setPalette(palette);
    btn->setText(col.name());
}

void ConfigGeneral::actBackgroundColorChange() {
    QColor base    = ui->buttonBackgroundColor->palette().color(QPalette::Button);
    QColor newColor= dialog::colorSelection(base);
    buttonSetColor(ui->buttonBackgroundColor, newColor);
}

void ConfigGeneral::preExec() {
    if(mwd != nullptr) {
        ui->DataLocation->setText(mwd->getSettings().value(settings::dataPathKey, settings::dataPathDefault).toString());
        ui->editThemeName->setText(mwd->getTheme().getParam("name").toString());
        ui->spinMainScale->setValue(mwd->getTheme().getParam("baseRatio").toDouble());
        ui->spinTitleScale->setValue(mwd->getTheme().getParam("titleRatio").toDouble());
        ui->spinShortTextScale->setValue(mwd->getTheme().getParam("shortTextRatio").toDouble());
        ui->spinLongTextScale->setValue(mwd->getTheme().getParam("longTextRatio").toDouble());
        ui->spinGridTextScale->setValue(mwd->getTheme().getParam("gridTextRatio").toDouble());
        //buttonSetColor(ui->buttonBackgroundColor,mwd->getTheme().getParam("gridTextRatio").to);
    }
}

}// namespace evl::gui
