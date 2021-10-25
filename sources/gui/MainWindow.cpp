/**
 * @file MainWindow.cpp
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/MainWindow.h"
#include "core/timeFunctions.h"
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
    timer(new QTimer(this)),
    numberGrid(new WidgetNumberGrid(ui->GroupPickManual)) {
    // initialise l'ui depuis le fichier ui.
    ui->setupUi(this);
    // initialise la numberGrid
    auto layout= new QGridLayout(ui->GroupPickManual);
    numberGrid->setParent(ui->GroupPickManual);
    numberGrid->setObjectName(QString::fromUtf8("numberGrid"));
    layout->addWidget(numberGrid, 0, 0, 1, 1);
    connect(numberGrid, &WidgetNumberGrid::buttonPushed, this, &MainWindow::actGridPushed);
    // connecte le timer à la fonction de mise à jour de l’affichage en jeu.
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::updateClocks));
    timer->setInterval(500);
    updateInGameDisplay();
}

MainWindow::~MainWindow() {
    delete ui;
    delete timer;
    delete numberGrid;
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
    updateInGameDisplay();
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
    updateInGameDisplay();
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
        numberGrid->resetPushed();
        rng.resetPick();
    }
    updateInGameDisplay();
}

void MainWindow::actPauseResumeRound() {
    /// TODO
    showNotImplemented("actPauseResumeRound");
}

void MainWindow::actRandomPick() {
    auto number= rng.pick();
    numberGrid->setPushed(number);
    currentEvent.findFirstNotFinished()->addPickedNumber(number);
    updateDisplay();
}

void MainWindow::actCancelPick() {
    core::Event::itGameround cur= currentEvent.findFirstNotFinished();
    if(cur->getDraws().size() == 0)
        return;
    numberGrid->resetPushed(cur->getDraws().back());
    cur->removeLastPick();
    updateInGameDisplay();
}

void MainWindow::actRadioPureRandom() {
    currentMode= DrawMode::PickOnly;
    updateInGameDisplay();
}

void MainWindow::actRadioPureManual() {
    currentMode= DrawMode::ManualOnly;
    updateInGameDisplay();
}

void MainWindow::actRadioBoth() {
    currentMode= DrawMode::Both;
    updateInGameDisplay();
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
    ui->actionSauver_Evenement->setEnabled(false);
    ui->actionSauver_Evenement_sous->setEnabled(false);
    ui->actionCommencer_Evenement->setEnabled(false);
    ui->actionTerminer_Evenement->setEnabled(false);
    ui->actionConfiguration_des_parties->setEnabled(false);
    ui->EvenementControl->setEnabled(false);
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
    updateInGameDisplay();
}

void MainWindow::updateClocks() {
    std::chrono::system_clock::time_point epoch{};
    ui->CurrentTime->setText(QString::fromStdString(core::formatClock(std::chrono::system_clock::now())));
    if(currentEvent.getStarting() == epoch) {
        ui->CurrentDate->setText("");
    } else {
        ui->CurrentDate->setText(QString::fromStdString(core::formatCalendar(currentEvent.getStarting())));
    }

    if(currentEvent.getStarting() == epoch) {
        ui->Duration->setText("");
    } else {
        ui->StartingHour->setText(QString::fromStdString(core::formatClock(currentEvent.getStarting())));
        auto length= std::chrono::system_clock::now() - currentEvent.getStarting();
        if(currentEvent.getEnding() != epoch)
            length= currentEvent.getEnding() - currentEvent.getStarting();
        ui->Duration->setText(QString::fromStdString(core::formatDuration(length)));
    }
    if(currentEvent.getEnding() == epoch) {
        ui->EndingHour->setText("");
    } else {
        ui->EndingHour->setText(QString::fromStdString(core::formatClock(currentEvent.getEnding())));
    }
    core::Event::itGameround cur= currentEvent.findFirstNotFinished();
    if(cur == currentEvent.getGREnd())
        return;
    if(cur->getStarting() == epoch) {
        ui->RoundStartTime->setText("");
        ui->RoundDuration->setText("");
    } else {
        ui->RoundStartTime->setText(QString::fromStdString(core::formatClock(cur->getStarting())));
        auto length= std::chrono::system_clock::now() - cur->getStarting();
        if(cur->getEnding() != epoch)
            length= cur->getEnding() - cur->getStarting();
        ui->RoundDuration->setText(QString::fromStdString(core::formatDuration(length)));
    }
}

void MainWindow::updateInGameDisplay() {

    // les radios boutons
    {
        switch(currentMode) {
        case DrawMode::Both:
            ui->radioBtnPureManual->setChecked(false);
            ui->radioBtnPureRandom->setChecked(false);
            ui->radioBtnBoth->setChecked(true);
            ui->RandomPick->setEnabled(true);
            ui->GroupPickManual->setEnabled(true);
            break;
        case DrawMode::PickOnly:
            ui->radioBtnPureManual->setChecked(false);
            ui->radioBtnPureRandom->setChecked(true);
            ui->radioBtnBoth->setChecked(false);
            ui->RandomPick->setEnabled(true);
            ui->GroupPickManual->setEnabled(false);
            break;
        case DrawMode::ManualOnly:
            ui->radioBtnPureManual->setChecked(true);
            ui->radioBtnPureRandom->setChecked(false);
            ui->radioBtnBoth->setChecked(false);
            ui->RandomPick->setEnabled(false);
            ui->GroupPickManual->setEnabled(true);
            break;
        }
        ui->CancelLastPick->setEnabled(true);
        if(currentEvent.getStatus() != core::Event::Status::GameRunning) {
            ui->RandomPick->setEnabled(false);
            ui->GroupPickManual->setEnabled(false);
            ui->CancelLastPick->setEnabled(false);
        }
    }
    // le bouton de pause
    ui->PauseResumeGame->setEnabled(false);
    if(currentEvent.getStatus() == core::Event::Status::GameRunning) {
        ui->PauseResumeGame->setEnabled(true);
        ui->PauseResumeGame->setText("Pause partie");
    }
    if(currentEvent.getStatus() == core::Event::Status::Paused || currentEvent.getStatus() == core::Event::Status::GamePaused) {
        ui->PauseResumeGame->setEnabled(true);
        ui->PauseResumeGame->setText("Reprise partie");
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
    // informations parties
    ui->Progression->setRange(0, currentEvent.getGameRounds().size());
    ui->Progression->setValue(currentEvent.getCurrentIndex());
    if(currentEvent.getStatus() == core::Event::Status::GameStart ||
       currentEvent.getStatus() == core::Event::Status::GameRunning ||
       currentEvent.getStatus() == core::Event::Status::GameFinished ||
       currentEvent.getStatus() == core::Event::Status::GamePaused) {
        std::chrono::system_clock::time_point epoch{};
        core::Event::itGameround cur= currentEvent.findFirstNotFinished();
        ui->RoundPhase->setText(QString::fromStdString(cur->getStatusStr()));
        ui->RoundName->setText(QString::number(currentEvent.getCurrentIndex() + 1) + " - " + QString::fromStdString(cur->getTypeStr()));
        if(cur->getStarting() == epoch) {
            ui->RoundDraws->setText("0");
        } else {
            ui->RoundDraws->setText(QString::number(cur->getDraws().size()));
        }
        if(cur->getDraws().size() > 0) {
            ui->CancelLastPick->setEnabled(true);
            auto it= cur->getDraws().rbegin();
            ui->CurrentDraw->display(*it);
            ++it;
            if(it != cur->getDraws().rend()) {
                ui->LastNumber1->display(*it);
                ++it;
                if(it != cur->getDraws().rend()) {
                    ui->LastNumber2->display(*it);
                    ++it;
                    if(it != cur->getDraws().rend()) {
                        ui->LastNumber3->display(*it);
                    } else {
                        ui->LastNumber3->display(0);
                    }
                } else {
                    ui->LastNumber2->display(0);
                }
            } else {
                ui->LastNumber1->display(0);
            }
        } else {
            ui->CancelLastPick->setEnabled(false);
            ui->RoundStartTime->setText("");
            ui->RoundDuration->setText("");
            ui->RoundDraws->setText("");
            ui->RoundName->setText("");
            ui->RoundPhase->setText("");
            ui->CurrentDraw->display(0);
            ui->LastNumber1->display(0);
            ui->LastNumber2->display(0);
            ui->LastNumber3->display(0);
        }
    } else {
        ui->CancelLastPick->setEnabled(false);
        ui->RoundStartTime->setText("");
        ui->RoundDuration->setText("");
        ui->RoundDraws->setText("");
        ui->RoundName->setText("");
        ui->RoundPhase->setText("");
        ui->CurrentDraw->display(0);
        ui->LastNumber1->display(0);
        ui->LastNumber2->display(0);
        ui->LastNumber3->display(0);
    }
    if(!timer->isActive())
        timer->start();
}

void MainWindow::actGridPushed(int value) {
    currentEvent.findFirstNotFinished()->addPickedNumber(value);
    rng.addPick(value);
    updateDisplay();
}

}// namespace evl::gui
