/**
 * @author Silmaen
 * @date 20/10/2021
 */
#include "UI/EvenementConfig.h"
#include <QMessageBox>

// Les trucs de QT
#include "UI/moc_EvenementConfig.cpp"
#include "UI/ui_EvenementConfig.h"

namespace evl::gui {

EvenementConfig::EvenementConfig(QWidget* parent):
    QDialog(parent),
    ui(new Ui::EvenementConfig) {
    ui->setupUi(this);
}

EvenementConfig::~EvenementConfig() {
    delete ui;
}

void EvenementConfig::LoadFile() {
    showNotImplemented("LoadFile");
}

void EvenementConfig::SaveFile() {
    showNotImplemented("SaveFile");
}

void EvenementConfig::actOk() {
    SaveFile();
    accept();
}

void EvenementConfig::actApply() {
    SaveFile();
}

void EvenementConfig::actCancel() {
    reject();
}

void EvenementConfig::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

int EvenementConfig::exec() {
    return QDialog::exec();
}

}// namespace evl::gui
