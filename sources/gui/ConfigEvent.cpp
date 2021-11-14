/**
 * @file ConfigEvent.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/ConfigEvent.h"
#include "gui/BaseDialog.h"
#include "gui/MainWindow.h"
#include "gui/baseDefinitions.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>

// Les trucs de QT
#include "gui/moc_ConfigEvent.cpp"
#include "ui/ui_ConfigEvent.h"

namespace evl::gui {

ConfigEvent::ConfigEvent(MainWindow* parent):
    QDialog(parent),
    ui(new Ui::ConfigEvent), mwd(parent) {
    ui->setupUi(this);
}

ConfigEvent::~ConfigEvent() {
    delete ui;
}

void ConfigEvent::SaveFile() {
    gameEvent.setName(ui->EventName->text().toStdString());
    gameEvent.setOrganizerName(ui->OrgaName->text().toStdString());
    gameEvent.setLocation(ui->EventLocation->text().toStdString());
    gameEvent.setLogo(ui->EventLogo->text().toStdString());
    gameEvent.setOrganizerLogo(ui->OrgaLogo->text().toStdString());
    gameEvent.setRules(ui->textRules->toPlainText().toStdString());
}

void ConfigEvent::actOk() {
    if(!gameEvent.isEditable())
        reject();
    SaveFile();
    accept();
}

void ConfigEvent::actApply() {
    SaveFile();
}

void ConfigEvent::actCancel() {
    reject();
}

void ConfigEvent::actSearchOrgaLogo() {
    QFileDialog dia;
    dia.setAcceptMode(QFileDialog::AcceptOpen);
    dia.setFileMode(QFileDialog::FileMode::ExistingFile);
    dia.setNameFilters(imageFilter);
    if(mwd != nullptr)
        dia.setDirectory(mwd->getSettings().value(dataPathKey).toString());
    if(dia.exec()) {
        ui->OrgaLogo->setText(dia.selectedFiles()[0]);
    }
}

void ConfigEvent::actSearchLogo() {
    QFileDialog dia;
    dia.setAcceptMode(QFileDialog::AcceptOpen);
    dia.setFileMode(QFileDialog::FileMode::ExistingFile);
    dia.setNameFilters(imageFilter);
    if(mwd != nullptr)
        dia.setDirectory(mwd->getSettings().value(dataPathKey).toString());
    if(dia.exec()) {
        ui->EventLogo->setText(dia.selectedFiles()[0]);
    }
}

void ConfigEvent::setEvent(const core::Event& e) {
    gameEvent= e;
    updateDisplay();
}

void ConfigEvent::updateDisplay() {
    ui->EventName->setText(QString::fromUtf8(gameEvent.getName()));
    ui->EventLocation->setText(QString::fromUtf8(gameEvent.getLocation()));
    ui->EventLogo->setText(QString::fromUtf8(gameEvent.getLogo().string()));
    ui->OrgaName->setText(QString::fromUtf8(gameEvent.getOrganizerName()));
    ui->OrgaLogo->setText(QString::fromUtf8(gameEvent.getOrganizerLogo().string()));
    ui->textRules->setText(QString::fromUtf8(gameEvent.getRules()));
    if(!gameEvent.isEditable()) {
        ui->EventName->setEnabled(false);
        ui->EventLocation->setEnabled(false);
        ui->EventLogo->setEnabled(false);
        ui->SearchLogo->setEnabled(false);
        ui->OrgaName->setEnabled(false);
        ui->OrgaLogo->setEnabled(false);
        ui->SearchOrgaLogo->setEnabled(false);
        ui->ButtonApply->setEnabled(false);
        ui->textRules->setEnabled(false);
        ui->buttonImportRules->setEnabled(false);
    } else {
        ui->EventName->setEnabled(true);
        ui->EventLocation->setEnabled(true);
        ui->EventLogo->setEnabled(true);
        ui->SearchLogo->setEnabled(true);
        ui->OrgaName->setEnabled(true);
        ui->OrgaLogo->setEnabled(true);
        ui->SearchOrgaLogo->setEnabled(true);
        ui->ButtonApply->setEnabled(true);
        ui->textRules->setEnabled(true);
        ui->buttonImportRules->setEnabled(true);
    }
}

void ConfigEvent::actImportRules() {
    auto path= dialog::openFile(dialog::FileTypes::Text, true);
    if(path.empty())
        return;
    std::ifstream file;
    file.open(path, std::ios::in);
    gameEvent.setRules(string{(std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())});
    file.close();
    updateDisplay();
}

void ConfigEvent::actExportRules() {
    auto path= dialog::openFile(dialog::FileTypes::Text, false);
    if(path.empty())
        return;
    std::ofstream file;
    file.open(path, std::ios::out);
    file << gameEvent.getRules();
    file.close();
}

}// namespace evl::gui
