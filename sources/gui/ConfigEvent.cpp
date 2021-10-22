/**
 * @file ConfigEvent.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/ConfigEvent.h"
#include "gui/MainWindow.h"
#include "gui/baseDefinitions.h"
#include <QFileDialog>
#include <QMessageBox>

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

void ConfigEvent::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

int ConfigEvent::exec() {
    return QDialog::exec();
}

void ConfigEvent::setEvent(const core::Event& e) {
    gameEvent= e;
    updateDisplay();
}

void ConfigEvent::updateDisplay() {
    ui->EventName->setText(QString::fromStdString(gameEvent.getName()));
    ui->EventLocation->setText(QString::fromStdString(gameEvent.getLocation()));
    ui->EventLogo->setText(QString::fromStdString(gameEvent.getLogo().string()));
    ui->OrgaName->setText(QString::fromStdString(gameEvent.getOrganizerName()));
    ui->OrgaLogo->setText(QString::fromStdString(gameEvent.getOrganizerLogo().string()));

    if(!gameEvent.isEditable()) {
        ui->EventName->setEnabled(false);
        ui->EventLocation->setEnabled(false);
        ui->EventLogo->setEnabled(false);
        ui->SearchLogo->setEnabled(false);
        ui->OrgaName->setEnabled(false);
        ui->OrgaLogo->setEnabled(false);
        ui->SearchOrgaLogo->setEnabled(false);
        ui->ButtonApply->setEnabled(false);
    }
}

}// namespace evl::gui
