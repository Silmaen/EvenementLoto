/**
 * @author Silmaen
 * @date 20/10/2021
 */
#include "UI/GeneralConfig.h"
#include "UI/MainWindow.h"
#include "UI/baseDefinitions.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>

// Les trucs de QT
#include "UI/moc_GeneralConfig.cpp"
#include "UI/ui_GeneralConfig.h"

namespace evl::gui {

GeneralConfig::GeneralConfig(MainWindow* parent):
    QDialog(parent),
    ui(new Ui::GeneralConfig), mwd(parent) {
    ui->setupUi(this);
}

GeneralConfig::~GeneralConfig() {
    delete ui;
}

void GeneralConfig::SaveFile() {
    if(mwd != nullptr) {
        mwd->getSettings().setValue(dataPathKey, ui->DataLocation->text());
        mwd->syncSettings();
    }
}

void GeneralConfig::actOk() {
    SaveFile();
    accept();
}

void GeneralConfig::actApply() {
    SaveFile();
}

void GeneralConfig::actCancel() {
    reject();
}

void GeneralConfig::searchFolder() {
    QFileDialog dia;
    dia.setAcceptMode(QFileDialog::AcceptOpen);
    dia.setFileMode(QFileDialog::FileMode::Directory);
    if(dia.exec()) {
        ui->DataLocation->setText(dia.selectedFiles()[0]);
    }
}

void GeneralConfig::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

int GeneralConfig::exec() {
    if(mwd != nullptr) {
        ui->DataLocation->setText(mwd->getSettings().value(dataPathKey, "").toString());
    }
    return QDialog::exec();
}

}// namespace evl::gui
