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
    settings{QString(getIniFile().generic_u8string().c_str()), QSettings::IniFormat},
    timer(new QTimer(this)) {
    // initialise l'ui depuis le fichier ui.
    ui->setupUi(this);
    // connecte le timer à la fonction de mise à jour de l’affichage en jeu.
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::updateInGameDisplay));
    timer->setInterval(500);
    updateInGameDisplay();
}

MainWindow::~MainWindow() {
    delete ui;
    delete timer;
    if(displayWindow != nullptr)
        delete displayWindow;
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
    QMessageBox message;
    message.setIcon(QMessageBox::Question);
    message.setWindowTitle("Démarrage de l’événement.");
    message.setText("Êtes-vous sûr de vouloir démarrer l’événement ?");
    message.setInformativeText("Une fois démarré, la configuration de l’événement et de ses parties ne pourra plus être modifié.");
    message.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
    if(message.exec() != QMessageBox::StandardButton::Yes)
        return;
    currentEvent.startEvent();
    updateDisplay();
    timer->start();
    displayWindow= new DisplayWindow(&currentEvent, this);
    displayWindow->show();
}

void MainWindow::actEndEvent() {
    QMessageBox message;
    message.setIcon(QMessageBox::Question);
    message.setWindowTitle("Terminer de l’événement.");
    message.setText("Êtes-vous sûr de vouloir terminer l’événement ?");
    message.setInformativeText("Une fois terminé, l’événement ne pourra plus être démarré à nouveau.");
    message.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
    if(message.exec() != QMessageBox::StandardButton::Yes)
        return;
    currentEvent.stopEvent();
    timer->stop();
    displayWindow->hide();
    delete displayWindow;
    displayWindow= nullptr;
    updateDisplay();
}

void MainWindow::actQuit() {
    close();
}

void MainWindow::actStartStopRound() {
    timer->stop();
    if(currentEvent.getStatus() == core::Event::Status::EventStarted) {
        currentEvent.resumeEvent();
    } else if(currentEvent.getStatus() == core::Event::Status::GameStart) {
        currentEvent.startCurrentRound();
    } else if(currentEvent.getStatus() == core::Event::Status::GameRunning) {
        currentEvent.endCurrentRound();
    } else if(currentEvent.getStatus() == core::Event::Status::GameFinished) {
        currentEvent.closeCurrentRound();
    }
    updateInGameDisplay();
}

void MainWindow::actPauseResumeRound() {
    /// TODO
    showNotImplemented("actPauseResumeRound");
}

void MainWindow::actRandomPick() {
    /// TODO
    showNotImplemented("actRandomPick");
}

void MainWindow::actCancelPick() {
    /// TODO
    showNotImplemented("actCancelPick");
}

void MainWindow::actRadioPureRandom() {
    /// TODO
    showNotImplemented("actRadioPureRandom");
}

void MainWindow::actRadioPureManual() {
    /// TODO
    showNotImplemented("actRadioPureManual");
}

void MainWindow::actRadioBoth() {
    /// TODO
    showNotImplemented("actRadioBoth");
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
        updateInGameDisplay();
        break;
    case core::Event::Status::EventStarted:
    case core::Event::Status::Paused:
    case core::Event::Status::GamePaused:
    case core::Event::Status::GameStart:
    case core::Event::Status::GameRunning:
    case core::Event::Status::GameFinished:
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

void MainWindow::updateInGameDisplay() {
    // les dates et heures
    std::chrono::system_clock::time_point epoch{};
    {
        std::stringstream oss;
        auto tt= std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        oss << std::put_time(std::localtime(&tt), "%H:%M:%S");
        ui->CurrentTime->setText(QString::fromStdString(oss.str()));
    }
    {
        if(currentEvent.getStarting() == epoch) {
            ui->CurrentDate->setText("");
        } else {
            std::stringstream oss;
            auto tt= std::chrono::system_clock::to_time_t(currentEvent.getStarting());
            oss << std::put_time(std::localtime(&tt), "%d %B %Y");
            ui->CurrentDate->setText(QString::fromStdString(oss.str()));
        }
    }
    {
        if(currentEvent.getStarting() == epoch) {
            ui->Duration->setText("");
        } else {
            std::stringstream oss;
            auto length= std::chrono::system_clock::now() - currentEvent.getStarting();
            oss << std::chrono::duration_cast<std::chrono::hours>(length).count() << ":"
                << std::chrono::duration_cast<std::chrono::minutes>(length).count() << ":"
                << std::chrono::duration_cast<std::chrono::seconds>(length).count();
            ui->Duration->setText(QString::fromStdString(oss.str()));
        }
    }
    // le bouton de debut/fin de round
    ui->StartEndGameRound->setEnabled(false);
    if(currentEvent.getStatus() == core::Event::Status::EventStarted) {
        ui->StartEndGameRound->setEnabled(true);
        ui->StartEndGameRound->setText("Afficher première partie");
    }
    if(currentEvent.getStatus() == core::Event::Status::GameStart) {
        ui->StartEndGameRound->setEnabled(true);
        ui->StartEndGameRound->setText("Démarrer la partie");
    }
    if(currentEvent.getStatus() == core::Event::Status::GameRunning) {
        ui->StartEndGameRound->setEnabled(true);
        ui->StartEndGameRound->setText("Terminer la partie");
    }
    if(currentEvent.getStatus() == core::Event::Status::GameFinished) {
        ui->StartEndGameRound->setEnabled(true);
        ui->StartEndGameRound->setText("Passer à la partie suivante");
    }
    if(!timer->isActive())
        timer->start();
}

}// namespace evl::gui
