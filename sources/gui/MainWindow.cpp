/**
 * @file MainWindow.cpp
 * @author Silmaen
 * @date 18/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/MainWindow.h"
#include "gui/About.h"
#include "gui/BaseDialog.h"
#include "gui/ConfigCardPack.h"
#include "gui/ConfigEvent.h"
#include "gui/ConfigGameRounds.h"
#include "gui/ConfigGeneral.h"
#include <QFileDialog>
#include <QScreen>
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
    numberGrid(new WidgetNumberGrid()),
    theme(&settings) {
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
    timer->setInterval(100);
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
    currentFile = path{};
    updateDisplay();
}

void MainWindow::actLoadFile() {
    path file= dialog::openFile(dialog::FileTypes::EventSave, true);
    if(!file.empty()) {
        std::ifstream f;
        f.open(file, std::ios::in | std::ios::binary);
        currentEvent.setBasePath(file);
        currentEvent.read(f, 0);
        f.close();
        currentFile= file;
    }
    updateDisplay();
}

void MainWindow::actSaveFile() {
    if(currentEvent.getStatus() == core::Event::Status::Invalid)
        return;
    if(currentFile.empty()) {
        actSaveAsFile();
        return;
    }
    saveFile(currentFile);
}

void MainWindow::saveFile(const path& where) {
    std::ofstream f;
    f.open(where, std::ios::out | std::ios::binary);
    currentEvent.setBasePath(currentFile.parent_path());
    // copy image file if not in child directory
    if(!currentEvent.getLogo().empty()) {
        path ori= currentEvent.getLogoFull();
        if(ori.parent_path() != currentEvent.getBasePath()) {
            path dest= currentEvent.getBasePath() / ori.filename();
            if(exists(dest))
                remove(dest);
            copy_file(ori, dest);
            currentEvent.setLogo(dest);
        }
    }
    if(!currentEvent.getOrganizerLogo().empty()) {
        path ori= currentEvent.getOrganizerLogoFull();
        if(ori.parent_path() != currentEvent.getBasePath()) {
            path dest= currentEvent.getBasePath() / ori.filename();
            if(exists(dest))
                remove(dest);
            copy_file(ori, dest);
            currentEvent.setOrganizerLogo(dest);
        }
    }
    currentEvent.write(f);
    f.close();
}

void MainWindow::actSaveAsFile() {
    if(currentEvent.getStatus() == core::Event::Status::Invalid)
        return;
    path file= dialog::openFile(dialog::FileTypes::EventSave, false);
    if(file.empty())
        return;
    currentFile= file;
    actSaveFile();
}

void MainWindow::actStartEvent() {
    if(!dialog::question("Démarrage de l’événement.",
                         "Êtes-vous sûr de vouloir démarrer l’événement ?",
                         "Une fois démarré, la configuration de l’événement et de ses parties ne pourra plus être modifié."))
        return;
    currentEvent.startEvent();
    updateDisplay();
    timer->start();
    displayWindow= new DisplayWindow(&currentEvent, this);
    actChangeFullScreen();
}

void MainWindow::actChangeFullScreen() {
    if(displayWindow == nullptr)
        return;
    QList<QScreen*> screens= QApplication::screens();
    if(screens.size() < 2 || !ui->actionFullScreen->isChecked()) {
        displayWindow->showNormal();
    } else {
        for(QScreen* s: screens) {
            if(s == screen())
                continue;
            QRect sizes= s->geometry();
            displayWindow->move(sizes.x(), sizes.y());
            break;
        }
        displayWindow->showFullScreen();
    }
}

void MainWindow::actEndEvent() {
    if(!dialog::question("Terminer de l’événement.",
                         "Êtes-vous sûr de vouloir terminer l’événement ?",
                         "Une fois terminé, l’événement ne pourra plus être démarré à nouveau."))
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

void MainWindow::actDisplaySanity() {
    if(currentEvent.getStatus() == core::Event::Status::DisplaySanity)
        currentEvent.resumeEvent();
    else
        currentEvent.displaySanity();
    updateDisplay();
}

void MainWindow::syncSettings() {
    settings.sync();
}

void MainWindow::updateClocks() {
    //
    // Cette fonction est exécutée très régulièrement et doit être super rapide.
    //
    // Horloge principale
    ui->CurrentTime->setText(QString::fromStdString(core::formatClock(core::clock::now())));
    // horloge partie en cours
    auto cur= currentEvent.findFirstNotFinished();
    // check si la fenêtre d'affichage est ouverte ou fermée
    checkDisplayWindow();
    if(cur == currentEvent.endRounds())// pas de round en cours
        return;
    if(cur->getEnding() != core::epoch)// round déjà fini, pas de mise à jour
        return;
    if(cur->getStarting() == core::epoch)// round pas commencé, pas de mise à jour
        return;
    ui->RoundDuration->setText(QString::fromStdString(core::formatDuration(core::clock::now() - cur->getStarting())));
}

void MainWindow::checkDisplayWindow() {
    // pas d'action à prendre
    if(currentEvent.getStatus() == core::Event::Status::Finished ||
       currentEvent.getStatus() == core::Event::Status::Invalid)
        return;
    if(displayWindow == nullptr || !displayWindow->isVisible()) {
        ui->actionStartEvent->setEnabled(true);
    }
    ++autoSaveCounter;
    if(autoSaveCounter < 15)
        return;
    autoSaveCounter= 0;
    QVariant p     = settings.value("path/data_path", "");
    if(p.toString() == "")
        return;
    path pt(p.toString().toStdString());
    if(!exists(pt))
        return;
    pt= pt / "rescue.lev";
    saveFile(pt);
}

void MainWindow::updateDisplay() {
    onUpdate= true;
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
    onUpdate= false;
}

void MainWindow::updateMenus() {
    // ui->menuFile             <- toujours actif, change jamais de texte
    //   ui->actionNewEvent     <- toujours actif, change jamais de texte
    //   ui->actionLoadEvent    <- toujours actif, change jamais de texte
    //   ui->actionSaveEvent
    if(currentEvent.getStatus() == core::Event::Status::Invalid ||
       currentEvent.getStatus() == core::Event::Status::MissingParties) {
        ui->actionSaveEvent->setEnabled(false);
        ui->actionSaveEventAs->setEnabled(false);
    } else {
        ui->actionSaveEvent->setEnabled(true);
        ui->actionSaveEventAs->setEnabled(true);
    }
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
        ui->actionRandomPick->setEnabled(true);
        ui->GroupPickManual->setEnabled(true);
        break;
    case DrawMode::PickOnly:
        ui->radioBtnPureManual->setChecked(false);
        ui->radioBtnPureRandom->setChecked(true);
        ui->radioBtnBoth->setChecked(false);
        ui->RandomPick->setEnabled(true);
        ui->actionRandomPick->setEnabled(true);
        ui->GroupPickManual->setEnabled(false);
        break;
    case DrawMode::ManualOnly:
        ui->radioBtnPureManual->setChecked(true);
        ui->radioBtnPureRandom->setChecked(false);
        ui->radioBtnBoth->setChecked(false);
        ui->RandomPick->setEnabled(false);
        ui->actionRandomPick->setEnabled(false);
        ui->GroupPickManual->setEnabled(true);
        break;
    }
    ui->CancelLastPick->setEnabled(true);
    ui->actionCancelLastPick->setEnabled(true);
    if(currentEvent.getStatus() != core::Event::Status::GameRunning) {
        ui->RandomPick->setEnabled(false);
        ui->actionRandomPick->setEnabled(false);
        ui->GroupPickManual->setEnabled(false);
        ui->CancelLastPick->setEnabled(false);
        ui->actionCancelLastPick->setEnabled(false);
    }
}

void MainWindow::updateInfoRound() {
    // reset all infos
    ui->RoundStartTime->setText("");
    //ui->RoundDuration->setText(""); // fait côé horloge
    ui->RoundDraws->setText("");
    ui->RoundName->setText("");
    ui->RoundPhase->setText("");
    auto cur= currentEvent.findFirstNotFinished();
    if(cur == currentEvent.endRounds())
        return;
    if(cur->getStarting() == core::epoch)
        return;
    ui->RoundStartTime->setText(QString::fromUtf8(core::formatClock(cur->getStarting())));
    if(cur->getStarting() == core::epoch) {
        ui->RoundDraws->setText("0");
    } else {
        ui->RoundDraws->setText(QString::number(cur->sizeDraws()));
    }
    ui->RoundName->setText(QString::number(currentEvent.getCurrentIndex() + 1) + " - " + QString::fromStdString(cur->getTypeStr()));
    QString phase= QString::fromStdString(cur->getStatusStr());

    if(cur->getStatus() == core::GameRound::Status::Started) {
        auto sub= cur->getCurrentCSubRound();
        if(sub != cur->endSubRound()) {
            phase+= " - " + QString::fromUtf8(sub->getTypeStr());
        }
    }

    ui->RoundPhase->setText(phase);
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
    QString draws;
    numberGrid->resetPushed();
    for(auto draw= cur->beginDraws(); draw != cur->endDraws(); ++draw) {
        draws+= QString::number(*draw) + " ";
        numberGrid->setPushed(*draw);
    }
    ui->textLastDraw->setText(draws);
}

void MainWindow::updateCommands() {
    updateStartStopButton();
    updatePauseButtons();
    ui->DisplayRules->setEnabled(false);
    ui->CancelLastPick->setEnabled(false);
    ui->DisplaySanity->setEnabled(false);
    ui->actionDisplayRules->setEnabled(false);
    ui->actionCancelLastPick->setEnabled(false);
    ui->actionDisplaySanity->setEnabled(false);
    if(currentEvent.isEditable())
        return;
    ui->DisplayRules->setEnabled(true);
    ui->actionDisplayRules->setEnabled(true);
    ui->DisplaySanity->setEnabled(true);
    ui->actionDisplaySanity->setEnabled(true);
    if(currentEvent.getStatus() == core::Event::Status::DisplayRules) {
        ui->DisplayRules->setText("Retour");
        ui->actionDisplayRules->setText("Retour");
        ui->DisplaySanity->setEnabled(false);
        ui->StartEndGameRound->setEnabled(false);
        ui->PauseResumeGame->setEnabled(false);
        ui->CancelLastPick->setEnabled(false);
        ui->actionDisplaySanity->setEnabled(false);
        ui->actionStartEndGameRound->setEnabled(false);
        ui->actionPauseResumeGame->setEnabled(false);
        ui->actionCancelLastPick->setEnabled(false);
    } else if(currentEvent.getStatus() == core::Event::Status::DisplaySanity) {
        ui->DisplaySanity->setText("Retour");
        ui->actionDisplaySanity->setText("Retour");
        ui->DisplayRules->setEnabled(false);
        ui->StartEndGameRound->setEnabled(false);
        ui->PauseResumeGame->setEnabled(false);
        ui->CancelLastPick->setEnabled(false);
        ui->actionDisplayRules->setEnabled(false);
        ui->actionStartEndGameRound->setEnabled(false);
        ui->actionPauseResumeGame->setEnabled(false);
        ui->actionCancelLastPick->setEnabled(false);
    } else {
        ui->DisplayRules->setText("Affichage règlement");
        ui->actionDisplayRules->setText("Affichage règlement");
        ui->DisplaySanity->setText("Affichage sanitaire");
        ui->actionDisplaySanity->setText("Affichage sanitaire");
    }
    // update cancel last pick
    auto cur= currentEvent.findFirstNotFinished();
    if(cur == currentEvent.endRounds())
        return;
    if(cur->sizeDraws() > 0) {
        ui->CancelLastPick->setEnabled(true);
        ui->actionCancelLastPick->setEnabled(true);
    }
}

void MainWindow::updatePauseButtons() {
    ui->PauseResumeGame->setEnabled(false);
    if(currentEvent.getStatus() == core::Event::Status::DisplayRules)
        return;
    if(currentEvent.getStatus() == core::Event::Status::DisplaySanity)
        return;
    if(currentEvent.getStatus() == core::Event::Status::GameRunning || currentEvent.getStatus() == core::Event::Status::GameFinished) {
        ui->PauseResumeGame->setEnabled(true);
        ui->PauseResumeGame->setText("Pause partie");
        ui->actionPauseResumeGame->setEnabled(true);
        ui->actionPauseResumeGame->setText("Pause partie");
        if(currentEvent.getStatus() == core::Event::Status::GameFinished) {
            ui->PauseResumeGame->setText("Pause événement");
            ui->actionPauseResumeGame->setText("Pause événement");
        }
    }
    if(currentEvent.getStatus() == core::Event::Status::Paused) {
        ui->PauseResumeGame->setEnabled(true);
        ui->PauseResumeGame->setText("Reprise partie");
        ui->actionPauseResumeGame->setEnabled(true);
        ui->actionPauseResumeGame->setText("Reprise partie");
        if(currentEvent.getStatus() == core::Event::Status::Paused) {
            ui->PauseResumeGame->setText("Reprise événement");
            ui->actionPauseResumeGame->setText("Reprise événement");
        }
    }
}

void MainWindow::updateStartStopButton() {
    ui->StartEndGameRound->setEnabled(false);
    if(currentEvent.getStatus() == core::Event::Status::DisplayRules)
        return;
    if(currentEvent.getStatus() == core::Event::Status::DisplaySanity)
        return;
    if(currentEvent.getStatus() == core::Event::Status::EventStarted) {
        ui->StartEndGameRound->setEnabled(true);
        ui->StartEndGameRound->setText("Afficher première partie");
        ui->actionStartEndGameRound->setEnabled(true);
        ui->actionStartEndGameRound->setText("Afficher première partie");
    }
    if(currentEvent.getStatus() == core::Event::Status::GameStart) {
        ui->StartEndGameRound->setEnabled(true);
        ui->StartEndGameRound->setText("Démarrer la partie");
        ui->actionStartEndGameRound->setEnabled(true);
        ui->actionStartEndGameRound->setText("Démarrer la partie");
    }
    if(currentEvent.getStatus() == core::Event::Status::GameRunning) {
        ui->StartEndGameRound->setEnabled(true);
        ui->actionStartEndGameRound->setEnabled(true);
        auto round= currentEvent.getGameRound(currentEvent.getCurrentIndex());
        if(round->isCurrentSubRoundLast()) {
            ui->StartEndGameRound->setText("Terminer la partie");
            ui->actionStartEndGameRound->setText("Terminer la partie");
        } else {
            ui->StartEndGameRound->setText("Terminer la sous-partie");
            ui->actionStartEndGameRound->setText("Terminer la sous-partie");
        }
    }
    if(currentEvent.getStatus() == core::Event::Status::GameFinished) {
        ui->StartEndGameRound->setEnabled(true);
        ui->StartEndGameRound->setText("Passer à la partie suivante");
        ui->actionStartEndGameRound->setEnabled(true);
        ui->actionStartEndGameRound->setText("Passer à la partie suivante");
    }
}

void MainWindow::actGridPushed(int value) {
    if(onUpdate)
        return;
    currentEvent.findFirstNotFinished()->addPickedNumber(value);
    rng.addPick(value);
    updateDisplay();
}

}// namespace evl::gui
