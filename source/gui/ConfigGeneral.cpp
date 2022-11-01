/**
 * @file ConfigGeneral.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */

#include "ConfigGeneral.h"
#include "BaseDialog.h"
#include "MainWindow.h"
#include "baseDefinitions.h"
#include <fstream>
#include <iostream>

// Les trucs de QT
#include "moc_ConfigGeneral.cpp"
#include "ui/ui_ConfigGeneral.h"

namespace evl::gui {

// color button utility

QColor getButtonColor(QPushButton* btn) {
    return btn->palette().color(QPalette::Button);
}
QString getButtonColorName(QPushButton* btn) {
    return btn->palette().color(QPalette::Button).name();
}
void setButtonColor(QPushButton* btn, const QColor& col) {
    auto palette= btn->palette();
    palette.setColor(QPalette::Button, col);
    if(col.lightness() < 100)
        palette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    else
        palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    btn->setPalette(palette);
    btn->setText(col.name());
}
void setButtonColor(QPushButton* btn, const QString& name) {
    setButtonColor(btn, QColor(name));
}

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
        mwd->getTheme().setParam("backgroundColor", getButtonColorName(ui->buttonBackgroundColor));
        mwd->getTheme().setParam("gridBackgroundColor", getButtonColorName(ui->buttonGridBackgroundColor));
        mwd->getTheme().setParam("truncatePrice", ui->checkTruncatePrice->isChecked());
        mwd->getTheme().setParam("truncatePriceLines", ui->spinTruncatePriceLines->value());
        mwd->getTheme().setParam("textColor", getButtonColorName(ui->buttonTextColor));
        mwd->getTheme().setParam("selectedNumberColor", getButtonColorName(ui->buttonSelectedNumberColor));
        mwd->getTheme().setParam("fadeNumbers", ui->checkFadeNumbers->isChecked());
        mwd->getTheme().setParam("fadeNumbersAmount", ui->spinFadeAmount->value());
        mwd->getTheme().setParam("fadeNumbersStrength", ui->spinFadeStrength->value());
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

void ConfigGeneral::actBackgroundColorChange() {
    QColor base    = getButtonColor(ui->buttonBackgroundColor);
    QColor newColor= dialog::colorSelection(base);
    setButtonColor(ui->buttonBackgroundColor, newColor);
}

void ConfigGeneral::actGridBackgroundColorChange() {
    QColor base    = getButtonColor(ui->buttonGridBackgroundColor);
    QColor newColor= dialog::colorSelection(base);
    setButtonColor(ui->buttonGridBackgroundColor, newColor);
}

void ConfigGeneral::actSelectedNumbersColorChange() {
    QColor base    = getButtonColor(ui->buttonSelectedNumberColor);
    QColor newColor= dialog::colorSelection(base);
    setButtonColor(ui->buttonSelectedNumberColor, newColor);
}

void ConfigGeneral::actTextColorChange() {
    QColor base    = getButtonColor(ui->buttonTextColor);
    QColor newColor= dialog::colorSelection(base);
    setButtonColor(ui->buttonTextColor, newColor);
}

void ConfigGeneral::actFadeActivationChange() {
    ui->spinFadeAmount->setEnabled(ui->checkFadeNumbers->isChecked());
    ui->spinFadeStrength->setEnabled(ui->checkFadeNumbers->isChecked());
}

void ConfigGeneral::actTruncatePriceChange() {
    ui->spinTruncatePriceLines->setEnabled(ui->checkTruncatePrice->isChecked());
}

void ConfigGeneral::preExec() {
    if(mwd != nullptr) {
        ui->DataLocation->setText(mwd->getSettings().value(settings::dataPathKey, settings::dataPathDefault).toString());
        ui->editThemeName->setText(mwd->getTheme().getParam("name").toString());
        ui->spinMainScale->setValue(mwd->getTheme().getParam("baseRatio").toDouble());
        ui->spinTitleScale->setValue(mwd->getTheme().getParam("titleRatio").toDouble());
        ui->spinShortTextScale->setValue(mwd->getTheme().getParam("shortTextRatio").toDouble());
        ui->spinLongTextScale->setValue(mwd->getTheme().getParam("longTextRatio").toDouble());
        ui->checkTruncatePrice->setChecked(mwd->getTheme().getParam("truncatePrice").toBool());
        ui->spinTruncatePriceLines->setValue(mwd->getTheme().getParam("truncatePriceLines").toInt());
        actTruncatePriceChange();
        ui->spinGridTextScale->setValue(mwd->getTheme().getParam("gridTextRatio").toDouble());
        setButtonColor(ui->buttonBackgroundColor, mwd->getTheme().getParam("backgroundColor").toString());
        setButtonColor(ui->buttonGridBackgroundColor, mwd->getTheme().getParam("gridBackgroundColor").toString());
        setButtonColor(ui->buttonTextColor, mwd->getTheme().getParam("textColor").toString());
        setButtonColor(ui->buttonSelectedNumberColor, mwd->getTheme().getParam("selectedNumberColor").toString());
        ui->checkFadeNumbers->setChecked(mwd->getTheme().getParam("fadeNumbers").toBool());
        actFadeActivationChange();
        ui->spinFadeAmount->setValue(mwd->getTheme().getParam("fadeNumbersAmount").toInt());
        ui->spinFadeStrength->setValue(mwd->getTheme().getParam("fadeNumbersStrength").toInt());
    }
}

}// namespace evl::gui
