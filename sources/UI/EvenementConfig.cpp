/**
 * @author Silmaen
 * @date 20/10/2021
 */
#include "UI/EvenementConfig.h"
#include "UI/MainWindow.h"
#include "UI/baseDefinitions.h"
#include <QFileDialog>
#include <QMessageBox>

// Les trucs de QT
#include "UI/moc_EvenementConfig.cpp"
#include "UI/ui_EvenementConfig.h"

namespace evl::gui {

EvenementConfig::EvenementConfig(MainWindow* parent):
    QDialog(parent),
    ui(new Ui::EvenementConfig), mwd(parent) {
    ui->setupUi(this);
}

EvenementConfig::~EvenementConfig() {
    delete ui;
}

void EvenementConfig::SaveFile() {
    evenement.setNom(ui->EventName->text().toStdString());
    evenement.setNomOrganisateur(ui->OrgaName->text().toStdString());
    evenement.setLieu(ui->EventLocation->text().toStdString());
    evenement.setLogo(ui->EventLogo->text().toStdString());
    evenement.setLogoOrganisateur(ui->OrgaLogo->text().toStdString());
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

void EvenementConfig::actSearchOrgaLogo() {
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

void EvenementConfig::actSearchLogo() {
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

void EvenementConfig::setEvenement(const core::Evenement& e) {
    evenement= e;
    updateDisplay();
}

void EvenementConfig::updateDisplay() {
    ui->EventName->setText(QString::fromStdString(evenement.getNom()));
    ui->EventLocation->setText(QString::fromStdString(evenement.getLieu()));
    ui->EventLogo->setText(QString::fromStdString(evenement.getLogo().string()));
    ui->OrgaName->setText(QString::fromStdString(evenement.getNomOrganisateur()));
    ui->OrgaLogo->setText(QString::fromStdString(evenement.getLogoOrganisateur().string()));
}

}// namespace evl::gui
