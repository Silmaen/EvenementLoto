/**
* @author Silmaen
* @date 18/10/2021
*/
#include "UI/MainWindow.h"
#include "UI/About.h"
#include "UI/ConfigCartons.h"
#include "UI/ConfigurationParties.h"
#include "UI/EvenementConfig.h"
#include "UI/GeneralConfig.h"
#include "UI/baseDefinitions.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <iostream>

// Les trucs de QT
#include <UI/moc_MainWindow.cpp>
#include <UI/ui_MainWindow.h>

namespace evl::gui {

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings{QString(getIniFile().generic_u8string().c_str()), QSettings::IniFormat} {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}
void MainWindow::showAbout() {
    evl::gui::About AboutWindow(this);
    AboutWindow.setModal(true);
    AboutWindow.exec();
}

void MainWindow::showHelp() {
    showNotImplemented("aide");
}

void MainWindow::showParametresGeneraux() {
    GeneralConfig cfg(this);
    cfg.exec();
}

void MainWindow::showParametresCartons() {
    ConfigCartons cfg(this);
    cfg.getCartons().getNom()= "default";
    cfg.exec();
}

void MainWindow::showParametresParties() {
    ConfigurationParties cfg;
    cfg.setEvenement(currentEvenement);
    if(cfg.exec() == QDialog::Accepted) {
        currentEvenement= cfg.getEvenement();
        updateDisplay();
    }
}

void MainWindow::showParametresEvenement() {
    EvenementConfig cfg;
    cfg.setEvenement(currentEvenement);
    if(cfg.exec() == QDialog::Accepted) {
        currentEvenement= cfg.getEvenement();
        updateDisplay();
    }
}

void MainWindow::fichierNew() {
    currentEvenement= core::Evenement();
    updateDisplay();
}

void MainWindow::fichierLoad() {

    QFileDialog dia;
    dia.setAcceptMode(QFileDialog::AcceptOpen);
    dia.setFileMode(QFileDialog::FileMode::ExistingFile);
    dia.setDirectory(settings.value(dataPathKey, QString::fromStdString(baseExecPath.string())).toString());
    dia.setNameFilter("fichier événement (*.lev)");
    if(dia.exec()) {
        std::ifstream f;
        f.open(dia.selectedFiles()[0].toStdString(), std::ios::in | std::ios::binary);
        currentEvenement.read(f);
        f.close();
    }

    updateDisplay();
}

void MainWindow::fichierSave() {
    if(currentEvenement.getStatus() == core::Evenement::Status::Invalid)
        return;
    std::filesystem::path base= settings.value(dataPathKey, QString::fromStdString(baseExecPath.string())).toString().toStdString();
    base/= currentEvenement.getNom() + ".lev";
    std::ofstream f;
    f.open(base, std::ios::out | std::ios::binary);
    currentEvenement.write(f);
    f.close();
}

void MainWindow::fichierSaveAs() {
    showNotImplemented("Sauver événement sous...");
}

void MainWindow::fichierCommencer() {
    showNotImplemented("Commencer événement");
}

void MainWindow::fichierTerminer() {
    showNotImplemented("Terminer événement");
}

void MainWindow::fichierQuitter() {
    close();
}

void MainWindow::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

void MainWindow::syncSettings() {
    settings.sync();
}

void MainWindow::updateDisplay() {
    switch(currentEvenement.getStatus()) {
    case core::Evenement::Status::Invalid:
        ui->actionSauver_Evenement->setEnabled(false);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(false);
        ui->actionTerminer_Evenement->setEnabled(false);
        ui->actionConfiguration_des_parties->setEnabled(false);
        ui->EvenementControl->setEnabled(false);
        break;
    case core::Evenement::Status::MissingParties:
        ui->actionSauver_Evenement->setEnabled(true);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(false);
        ui->actionTerminer_Evenement->setEnabled(false);
        ui->actionConfiguration_des_parties->setEnabled(true);
        ui->EvenementControl->setEnabled(false);
        break;
    case core::Evenement::Status::Ready:
        ui->actionSauver_Evenement->setEnabled(true);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(true);
        ui->actionTerminer_Evenement->setEnabled(false);
        ui->actionConfiguration_des_parties->setEnabled(true);
        ui->EvenementControl->setEnabled(false);
        break;
    case core::Evenement::Status::OnGoing:
        ui->actionSauver_Evenement->setEnabled(true);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(false);
        ui->actionTerminer_Evenement->setEnabled(true);
        ui->actionConfiguration_des_parties->setEnabled(true);
        ui->EvenementControl->setEnabled(true);
        break;
    case core::Evenement::Status::Finished:
        ui->actionSauver_Evenement->setEnabled(true);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(false);
        ui->actionTerminer_Evenement->setEnabled(false);
        ui->actionConfiguration_des_parties->setEnabled(true);
        ui->EvenementControl->setEnabled(false);
        break;
    }
}

}// namespace evl::gui
