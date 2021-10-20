/**
 * @author Silmaen
 * @date 20/10/2021
 */
#include "UI/GeneralConfig.h"
#include <QMessageBox>

// Les trucs de QT
#include "UI/moc_GeneralConfig.cpp"
#include "UI/ui_GeneralConfig.h"

namespace evl::gui {

GeneralConfig::GeneralConfig(QWidget* parent):
    QDialog(parent),
    ui(new Ui::GeneralConfig) {
    ui->setupUi(this);
}

GeneralConfig::~GeneralConfig() {
    delete ui;
}

void GeneralConfig::LoadFile() {
    showNotImplemented("LoadFile");
}

void GeneralConfig::SaveFile() {
    showNotImplemented("SaveFile");
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
    showNotImplemented("searchFolder");
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
    return QDialog::exec();
}

}// namespace evl::gui
