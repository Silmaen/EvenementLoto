/**
 * @author Silmaen
 * @date 20/10/2021
 */
#include "UI/ConfigurationParties.h"
#include <QMessageBox>

// Les trucs de QT
#include "UI/moc_ConfigurationParties.cpp"
#include "UI/ui_ConfigurationParties.h"

namespace evl::gui {

ConfigurationParties::ConfigurationParties(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ConfigurationParties) {
    ui->setupUi(this);
    ui->PartieType->addItems({"1 quine",
                              "2 quines",
                              "Carton Plein",
                              "Inverse"});
}

ConfigurationParties::~ConfigurationParties() {
    delete ui;
}


void ConfigurationParties::SaveFile() {
    showNotImplemented("SaveFile");
}

void ConfigurationParties::actOk() {
    SaveFile();
    accept();
}

void ConfigurationParties::actApply() {
    SaveFile();
}

void ConfigurationParties::actCancel() {
    reject();
}

void ConfigurationParties::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

int ConfigurationParties::exec() {
    return QDialog::exec();
}

void ConfigurationParties::partieCreate() {
    showNotImplemented("partieCreate");
}

void ConfigurationParties::partieLoad() {
    showNotImplemented("partieLoad");
}

void ConfigurationParties::partieOrderAfter() {
    showNotImplemented("partieOrderAfter");
}

void ConfigurationParties::partieOrderBefore() {
    showNotImplemented("partieOrderBefore");
}

void ConfigurationParties::partieSave() {
    showNotImplemented("partieSave");
}

void ConfigurationParties::partieTypeChanged(QString) {
    showNotImplemented("partieTypeChanged");
}

void ConfigurationParties::setEvenement(const core::Evenement& e) {
    evenement= e;
    updateDisplay();
}

void ConfigurationParties::updateDisplay() {
    ui->PartiesNumber->display((int)evenement.getParties().size());

    ui->SelectedPartie->clear();
}

}// namespace evl::gui
