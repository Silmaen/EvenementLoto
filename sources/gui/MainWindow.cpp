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
    numberGrid(new WidgetNumberGrid()) {
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
    updateDisplay();
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
    cfg.preExec();
    cfg.exec();
}

void MainWindow::actShowCardPackParameters() {
    ConfigCardPack cfg(this);
    cfg.getCardPack().getName()= "default";
    cfg.preExec();
    cfg.exec();
}

void MainWindow::actShowGameRoundsParameters() {
    ConfigGameRounds cfg;
    cfg.setEvent(currentEvent);
    cfg.preExec();
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
        currentEvent.ActiveFirstRound();
    } else if(currentEvent.getStatus() == core::Event::Status::GameStart) {
        currentEvent.startCurrentRound();
    } else if(currentEvent.getStatus() == core::Event::Status::GameRunning) {
        currentEvent.addWinnerToCurrentRound(111);
    } else if(currentEvent.getStatus() == core::Event::Status::GameFinished) {
        currentEvent.closeCurrentRound();
        numberGrid->resetPushed();
        rng.resetPick();
    }
    updateDisplay();
}

void MainWindow::actPauseResumeRound() {
    /// TODO
    //showNotImplemented("actPauseResumeRound");
    if(currentEvent.getStatus() == core::Event::Status::GameFinished || currentEvent.getStatus() == core::Event::Status::GameRunning) {
        currentEvent.pauseEvent();
    } else if(currentEvent.getStatus() == core::Event::Status::Paused) {
        currentEvent.resumeEvent();
    }
    updateDisplay();
}

void MainWindow::actRandomPick() {
    auto number= rng.pick();
    numberGrid->setPushed(number);
    currentEvent.findFirstNotFinished()->addPickedNumber(number);
    updateDisplay();
}

void MainWindow::actCancelPick() {
    auto cur= currentEvent.findFirstNotFinished();
    if(cur->sizeDraws() == 0)
        return;
    numberGrid->resetPushed(*cur->beginReverseDraws());
    cur->removeLastPick();
    updateDisplay();
}

void MainWindow::actRadioPureRandom() {
    currentDrawMode= DrawMode::PickOnly;
    updateDisplay();
}

void MainWindow::actRadioPureManual() {
    currentDrawMode= DrawMode::ManualOnly;
    updateDisplay();
}

void MainWindow::actRadioBoth() {
    currentDrawMode= DrawMode::Both;
    updateDisplay();
}

void MainWindow::actDisplayRules() {
    if(currentEvent.getStatus() == core::Event::Status::DisplayRules)
        currentEvent.resumeEvent();
    else
        currentEvent.displayRules();
    updateDisplay();
}

void MainWindow::syncSettings() {
    settings.sync();
}

void MainWindow::updateClocks() {
    ui->CurrentTime->setText(QString::fromStdString(core::formatClock(core::clock::now())));
    if(currentEvent.getStarting() == core::epoch) {
        ui->CurrentDate->setText("");
    } else {
        ui->CurrentDate->setText(QString::fromStdString(core::formatCalendar(currentEvent.getStarting())));
    }

    if(currentEvent.getStarting() == core::epoch) {
        ui->Duration->setText("");
    } else {
        ui->StartingHour->setText(QString::fromStdString(core::formatClock(currentEvent.getStarting())));
        auto length= core::clock::now() - currentEvent.getStarting();
        if(currentEvent.getEnding() != core::epoch)
            length= currentEvent.getEnding() - currentEvent.getStarting();
        ui->Duration->setText(QString::fromStdString(core::formatDuration(length)));
    }
    if(currentEvent.getEnding() == core::epoch) {
        ui->EndingHour->setText("");
    } else {
        ui->EndingHour->setText(QString::fromStdString(core::formatClock(currentEvent.getEnding())));
    }
    auto cur= currentEvent.findFirstNotFinished();
    if(cur == currentEvent.endRounds())
        return;
    if(cur->getStarting() == core::epoch) {
        ui->RoundStartTime->setText("");
        ui->RoundDuration->setText("");
    } else {
        ui->RoundStartTime->setText(QString::fromStdString(core::formatClock(cur->getStarting())));
        auto length= core::clock::now() - cur->getStarting();
        if(cur->getEnding() != core::epoch)
            length= cur->getEnding() - cur->getStarting();
        ui->RoundDuration->setText(QString::fromStdString(core::formatDuration(length)));
    }
}

void MainWindow::updateDisplay() {
    // Mise à jour des menus
    updateMenus();
    // TODO: Mise à jour de la barre de statut
    // Mise à jour Zone du bas
    updateBottomFrame();
    // Mise à jour info événement
    updateInfoEvent();
    // Mise à jour info partie
    updateInfoRound();
    // Mise à jour des numéros tirés
    updateDraws();
    // Mise à jour des commandes
    updateCommands();
    // timer
    if(!timer->isActive())
        timer->start();
}

void MainWindow::updateMenus() {
    // ui->menuFile             <- toujours actif, change jamais de texte
    //   ui->actionNewEvent     <- toujours actif, change jamais de texte
    //   ui->actionLoadEvent    <- toujours actif, change jamais de texte
    //   ui->actionSaveEvent
    if(currentEvent.getStatus() == core::Event::Status::Invalid ||
       currentEvent.getStatus() == core::Event::Status::MissingParties)
        ui->actionStartEvent->setEnabled(false);
    else
        ui->actionStartEvent->setEnabled(true);
    //   ui->actionSaveEventAs  <- jamais actif, change jamais de texte
    //   ui->actionStartEvent
    if(currentEvent.getStatus() == core::Event::Status::Ready)
        ui->actionStartEvent->setEnabled(true);
    else
        ui->actionStartEvent->setEnabled(false);
    //   ui->actionEndEvent
    if(currentEvent.getStatus() == core::Event::Status::Finished)
        ui->actionEndEvent->setEnabled(true);
    else
        ui->actionEndEvent->setEnabled(false);
    //   ui->actionQuit         <- toujours actif, change jamais de texte
    // ui->menuParameters       <- toujours actif, change jamais de texte
    //   ui->actionGlobalParam  <- toujours actif, change jamais de texte
    //   ui->actionConfigEvent  <- toujours actif, change jamais de texte
    //   ui->actionConfigCard   <- jamais actif, caché
    ui->actionConfigCard->setVisible(false);
    //   ui->actionConfigRounds
    if(currentEvent.getStatus() == core::Event::Status::Invalid)
        ui->actionConfigRounds->setEnabled(false);
    else
        ui->actionConfigRounds->setEnabled(true);
    // ui->menuHelp             <- toujours actif, change jamais de texte
    //   ui->actionAbout        <- toujours actif, change jamais de texte
    //   ui->actionHelp         <- toujours actif, change jamais de texte
}

void MainWindow::updateBottomFrame() {
    ui->tabStdResult->setVisible(false);
    ui->tabStdResult->setEnabled(false);
    ui->tabInverseResult->setVisible(false);
    ui->tabInverseResult->setEnabled(false);
    updateRadioButtons();
}

void MainWindow::updateInfoEvent() {
    if(currentEvent.isEditable()) {
        ui->EventInfos->setEnabled(false);
        return;
    }
    ui->EventInfos->setEnabled(true);
    core::timePoint start= currentEvent.getStarting();
    core::timePoint end  = currentEvent.getEnding();
    ui->CurrentDate->setText("");
    ui->StartingHour->setText("");
    ui->EndingHour->setText("");
    ui->Duration->setText("");
    ui->Progression->setRange(0, currentEvent.sizeRounds());
    ui->Progression->setValue(currentEvent.getCurrentIndex() + 1);
    if(start == core::epoch)
        return;
    ui->CurrentDate->setText(QString::fromUtf8(core::formatCalendar(start)));
    ui->StartingHour->setText(QString::fromUtf8(core::formatClockNoSecond(start)));
    if(end == core::epoch)
        return;
    ui->EndingHour->setText(QString::fromUtf8(core::formatClockNoSecond(end)));
    ui->Duration->setText(QString::fromUtf8(core::formatDuration(end - start)));
}

void MainWindow::updateRadioButtons() {
    switch(currentDrawMode) {
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

void MainWindow::updateInfoRound() {
    // reset all infos
    ui->RoundStartTime->setText("");
    ui->RoundDuration->setText("");
    ui->RoundDraws->setText("");
    ui->RoundName->setText("");
    ui->RoundPhase->setText("");
    if(currentEvent.getStatus() == core::Event::Status::GameStart ||
       currentEvent.getStatus() == core::Event::Status::GameRunning ||
       currentEvent.getStatus() == core::Event::Status::GameFinished ||
       currentEvent.getStatus() == core::Event::Status::Paused) {
        auto cur= currentEvent.findFirstNotFinished();
        ui->RoundPhase->setText(QString::fromStdString(cur->getStatusStr()));
        ui->RoundName->setText(QString::number(currentEvent.getCurrentIndex() + 1) + " - " + QString::fromStdString(cur->getTypeStr()));
        if(cur->getStarting() == core::epoch) {
            ui->RoundDraws->setText("0");
        } else {
            ui->RoundDraws->setText(QString::number(cur->sizeDraws()));
        }
        if(cur->sizeDraws() > 0) {
            ui->CancelLastPick->setEnabled(true);
        }
    }
}

void MainWindow::updateDraws() {
    ui->CurrentDraw->display(0);
    ui->LastNumber1->display(0);
    ui->LastNumber2->display(0);
    ui->LastNumber3->display(0);
    auto cur= currentEvent.findFirstNotFinished();
    if(cur == currentEvent.endRounds())
        return;
    if(cur->sizeDraws() > 0) {
        auto it= cur->beginReverseDraws();
        ui->CurrentDraw->display(*it);
        ++it;
        if(it != cur->endReverseDraws()) {
            ui->LastNumber1->display(*it);
            ++it;
            if(it != cur->endReverseDraws()) {
                ui->LastNumber2->display(*it);
                ++it;
                if(it != cur->endReverseDraws()) {
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
    }
}

void MainWindow::updateCommands() {
    updateStartStopButton();
    updatePauseButtons();
    ui->DisplayRules->setEnabled(false);
    ui->CancelLastPick->setEnabled(false);
    if(currentEvent.isEditable())
        return;
    ui->DisplayRules->setEnabled(true);
    if(currentEvent.getStatus() == core::Event::Status::DisplayRules) {
        ui->DisplayRules->setText("Retour");
        ui->CancelLastPick->setEnabled(false);
        ui->CancelLastPick->setEnabled(false);
        ui->CancelLastPick->setEnabled(false);
    } else {
        ui->DisplayRules->setText("Affichage règlement");
    }
    // update cancel last pick
    auto cur= currentEvent.findFirstNotFinished();
    if(cur == currentEvent.endRounds())
        return;
    if(cur->sizeDraws() > 0) {
        ui->CancelLastPick->setEnabled(true);
    }
}

void MainWindow::updatePauseButtons() {
    ui->PauseResumeGame->setEnabled(false);
    if(currentEvent.getStatus() == core::Event::Status::DisplayRules)
        return;
    if(currentEvent.getStatus() == core::Event::Status::GameRunning || currentEvent.getStatus() == core::Event::Status::GameFinished) {
        ui->PauseResumeGame->setEnabled(true);
        ui->PauseResumeGame->setText("Pause partie");
        if(currentEvent.getStatus() == core::Event::Status::GameFinished)
            ui->PauseResumeGame->setText("Pause événement");
    }
    if(currentEvent.getStatus() == core::Event::Status::Paused) {
        ui->PauseResumeGame->setEnabled(true);
        ui->PauseResumeGame->setText("Reprise partie");
        if(currentEvent.getStatus() == core::Event::Status::Paused)
            ui->PauseResumeGame->setText("Reprise événement");
    }
}

void MainWindow::updateStartStopButton() {
    ui->StartEndGameRound->setEnabled(false);
    if(currentEvent.getStatus() == core::Event::Status::DisplayRules)
        return;
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
        auto round= currentEvent.getGameRound(currentEvent.getCurrentIndex());
        if(round->isCurrentSubRoundLast())
            ui->StartEndGameRound->setText("Terminer la partie");
        else
            ui->StartEndGameRound->setText("Terminer la sous-partie");
    }
    if(currentEvent.getStatus() == core::Event::Status::GameFinished) {
        ui->StartEndGameRound->setEnabled(true);
        ui->StartEndGameRound->setText("Passer à la partie suivante");
    }
}

void MainWindow::actGridPushed(int value) {
    currentEvent.findFirstNotFinished()->addPickedNumber(value);
    rng.addPick(value);
    updateDisplay();
}

}// namespace evl::gui
