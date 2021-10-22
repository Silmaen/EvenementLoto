/**
 * @file MainWindow.cpp
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/MainWindow.h"
#include "gui/About.h"
#include "gui/ConfigCardPack.h"
#include "gui/ConfigEvent.h"
#include "gui/ConfigGameRounds.h"
#include "gui/ConfigGeneral.h"
#include "gui/baseDefinitions.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <iostream>

// Les trucs de QT
#include <gui/moc_MainWindow.cpp>
#include <ui/ui_MainWindow.h>

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
void MainWindow::actShowAbout() {
    evl::gui::About AboutWindow(this);
    AboutWindow.setModal(true);
    AboutWindow.exec();
}

void MainWindow::actShowHelp() {
    showNotImplemented("aide");
}

void MainWindow::actShowGlobalParameters() {
    ConfigGeneral cfg(this);
    cfg.exec();
}

void MainWindow::actShowCardPackParameters() {
    ConfigCardPack cfg(this);
    cfg.getCardPack().getName()= "default";
    cfg.exec();
}

void MainWindow::actShowGameRoundsParameters() {
    ConfigGameRounds cfg;
    cfg.setEvent(currentEvent);
    if(cfg.exec() == QDialog::Accepted) {
        currentEvent= cfg.getEvent();
        updateDisplay();
    }
}

void MainWindow::actShowEventParameters() {
    ConfigEvent cfg;
    cfg.setEvent(currentEvent);
    if(cfg.exec() == QDialog::Accepted) {
        currentEvent= cfg.getEvent();
        updateDisplay();
    }
}

void MainWindow::actNewFile() {
    currentEvent= core::Event();
    updateDisplay();
}

void MainWindow::actLoadFile() {

    QFileDialog dia;
    dia.setAcceptMode(QFileDialog::AcceptOpen);
    dia.setFileMode(QFileDialog::FileMode::ExistingFile);
    dia.setDirectory(settings.value(dataPathKey, QString::fromStdString(baseExecPath.string())).toString());
    dia.setNameFilter("fichier événement (*.lev)");
    if(dia.exec()) {
        std::ifstream f;
        f.open(dia.selectedFiles()[0].toStdString(), std::ios::in | std::ios::binary);
        currentEvent.read(f);
        f.close();
    }

    updateDisplay();
}

void MainWindow::actSaveFile() {
    if(currentEvent.getStatus() == core::Event::Status::Invalid)
        return;
    std::filesystem::path base= settings.value(dataPathKey, QString::fromStdString(baseExecPath.string())).toString().toStdString();
    base/= currentEvent.getName() + ".lev";
    std::ofstream f;
    f.open(base, std::ios::out | std::ios::binary);
    currentEvent.write(f);
    f.close();
}

void MainWindow::actSaveAsFile() {
    showNotImplemented("Sauver événement sous...");
}

void MainWindow::actStartEvent() {
}

void MainWindow::actEndEvent() {
    showNotImplemented("Terminer événement");
}

void MainWindow::actQuit() {
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
    switch(currentEvent.getStatus()) {
    case core::Event::Status::Invalid:
        ui->actionSauver_Evenement->setEnabled(false);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(false);
        ui->actionTerminer_Evenement->setEnabled(false);
        ui->actionConfiguration_des_parties->setEnabled(false);
        ui->EvenementControl->setEnabled(false);
        break;
    case core::Event::Status::MissingParties:
        ui->actionSauver_Evenement->setEnabled(true);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(false);
        ui->actionTerminer_Evenement->setEnabled(false);
        ui->actionConfiguration_des_parties->setEnabled(true);
        ui->EvenementControl->setEnabled(false);
        break;
    case core::Event::Status::Ready:
        ui->actionSauver_Evenement->setEnabled(true);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(true);
        ui->actionTerminer_Evenement->setEnabled(false);
        ui->actionConfiguration_des_parties->setEnabled(true);
        ui->EvenementControl->setEnabled(false);
        break;
    case core::Event::Status::OnGoing:
        ui->actionSauver_Evenement->setEnabled(true);
        ui->actionSauver_Evenement_sous->setEnabled(false);
        ui->actionCommencer_Evenement->setEnabled(false);
        ui->actionTerminer_Evenement->setEnabled(true);
        ui->actionConfiguration_des_parties->setEnabled(true);
        ui->EvenementControl->setEnabled(true);
        break;
    case core::Event::Status::Finished:
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
