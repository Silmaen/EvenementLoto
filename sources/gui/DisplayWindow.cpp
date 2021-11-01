/**
* @file DisplayWindow.cpp
* @author Silmaen
* @date 18/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "gui/DisplayWindow.h"
#include "core/timeFunctions.h"

// Les trucs de QT
#include <gui/moc_DisplayWindow.cpp>
#include <ui/ui_DisplayWindow.h>

namespace evl::gui {

DisplayWindow::DisplayWindow(core::Event* evt, QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::DisplayWindow),
    timer(new QTimer(this)),
    event(evt) {
    ui->setupUi(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&DisplayWindow::updateDisplay));
    if(event != nullptr) {
        timer->setInterval(200);
        timer->start();
        initializeNumberGrid();
        resize();
    }
}

DisplayWindow::~DisplayWindow() {
    delete ui;
    delete timer;
}

void DisplayWindow::initializeDisplay() {
    //  les images
    if(event->getOrganizerLogo().string().empty()) {
        ui->ET_OrganizerLogo->setVisible(false);
        ui->RR_Logo->setVisible(false);
    } else {
        if(!exists(event->getOrganizerLogo())) {
            ui->ET_OrganizerLogo->setVisible(false);
            ui->RR_Logo->setVisible(false);
        } else {
            QString imgName(QString::fromUtf8(event->getOrganizerLogo().string()));
            if(ui->ET_OrganizerLogo->text() != imgName) {// charge l’image seulement une fois
                ui->ET_OrganizerLogo->setText(imgName);
                QImage img;
                img.load(imgName);
                ui->ET_OrganizerLogo->setPixmap(QPixmap::fromImage(img.scaled(ui->ET_OrganizerLogo->size(), Qt::KeepAspectRatio)));
            }
            if(ui->RR_Logo->text() != imgName) {// charge l’image seulement une fois
                ui->RR_Logo->setText(imgName);
                QImage img;
                img.load(imgName);
                ui->RR_Logo->setPixmap(QPixmap::fromImage(img.scaled(ui->RR_Logo->size(), Qt::KeepAspectRatio)));
            }
        }
    }
    if(event->getLogo().empty()) {
        ui->ET_EventLogo->setVisible(false);
    } else {
        if(!exists(event->getLogo())) {
            ui->ET_EventLogo->setVisible(false);
        } else {
            QString imgName(QString::fromUtf8(event->getLogo().string()));
            if(ui->ET_EventLogo->text() != imgName) {// charge l’image seulement une fois
                ui->ET_EventLogo->setText(imgName);
                QImage img;
                img.load(imgName);
                ui->ET_EventLogo->setPixmap(QPixmap::fromImage(img.scaled(ui->ET_EventLogo->size())));
            }
        }
    }
}

void DisplayWindow::updateDisplay() {
    if(event == nullptr)
        return;
    switch(event->getStatus()) {
    case core::Event::Status::Invalid:
    case core::Event::Status::MissingParties:
    case core::Event::Status::Ready:
    case core::Event::Status::EventStarted:
        ui->PageManager->setCurrentIndex(0);
        updateEventTitlePage();
        return;
    case core::Event::Status::Paused:
        ui->PageManager->setCurrentIndex(5);
        return;
    case core::Event::Status::GamePaused:
        ui->PageManager->setCurrentIndex(3);
        return;
    case core::Event::Status::GameStart:
        ui->PageManager->setCurrentIndex(1);
        updateRoundTitlePage();
        return;
    case core::Event::Status::GameRunning:
        ui->PageManager->setCurrentIndex(2);
        updateRoundRunning();
        return;
    case core::Event::Status::GameFinished:
        ui->PageManager->setCurrentIndex(4);
        return;
    case core::Event::Status::Finished:
        ui->PageManager->setCurrentIndex(6);
        return;
    }
}

void DisplayWindow::updateEventTitlePage() {
    ui->ET_EventDate->setText(QString::fromUtf8(core::formatCalendar(event->getStarting())));
    ui->ET_EventTitle->setText(QString::fromUtf8(event->getName()));
    ui->ET_OrganizerName->setText(QString::fromUtf8(event->getOrganizerName()));
    if(event->getLocation().empty()) {
        ui->ET_EventLocation->setVisible(false);
    } else {
        ui->ET_EventLocation->setVisible(true);
        ui->ET_EventLocation->setText(QString::fromUtf8(event->getLocation()));
    }
    resize();
}

void DisplayWindow::updateRoundTitlePage() {
    auto round= event->getGameRound(event->getCurrentIndex());
    if(round->getType() == core::GameRound::Type::Normal) {
        ui->RT_RoundTitle->setText("Partie " + QString::number(event->getCurrentIndex() + 1));
        ui->RT_SubRound_2->setVisible(true);
        ui->RT_SubRound_3->setVisible(true);
        ui->RT_SubRound_1->setText("Gain une ligne: \n" + QString::fromUtf8(round->getSubRound(0)->getPrices()));
        ui->RT_SubRound_2->setText("Gain deux Lignes: \n" + QString::fromUtf8(round->getSubRound(1)->getPrices()));
        ui->RT_SubRound_3->setText("Gain carton plein: \n" + QString::fromUtf8(round->getSubRound(2)->getPrices()));
    } else {
        ui->RT_SubRound_2->setVisible(false);
        ui->RT_SubRound_3->setVisible(false);
        if(round->getType() == core::GameRound::Type::Enfant) {
            ui->RT_RoundTitle->setText("Partie " + QString::number(event->getCurrentIndex() + 1) + " Enfants");
            ui->RT_SubRound_1->setText("Gains: \n" + QString::fromUtf8(round->getSubRound(0)->getPrices()));
        }
        if(round->getType() == core::GameRound::Type::Inverse) {
            ui->RT_RoundTitle->setText("Partie " + QString::number(event->getCurrentIndex() + 1) + " Inversée");
            ui->RT_SubRound_1->setText("Gain dernier sans numéro: \n" + QString::fromUtf8(round->getSubRound(0)->getPrices()));
        }
    }
}

void DisplayWindow::updateRoundRunning() {

    // action de redimensionnement
    if(currentSize != size())
        resize();
    ui->RR_Title->setText("Partie " + QString::number(event->getCurrentIndex() + 1));
    auto round= event->getGameRound(event->getCurrentIndex());
    // mise à jour de l’heure et durée
    auto now= core::clock::now();
    ui->RR_Clock->setText(QString::fromStdString(core::formatClockNoSecond(now)));
    auto duration= now - round->getStarting();
    ui->RR_Duration->setText(QString::fromStdString(core::formatDuration(duration)));
    // mise à jour de l’affichage de la grille
    resetGrid();
    QBrush br;
    br.setColor(QColor(255, 112, 0));
    br.setStyle(Qt::BrushStyle::SolidPattern);
    for(auto iDraw= round->beginDraws(); iDraw != round->endDraws(); ++iDraw) {
        int row= (*iDraw - 1) / 10;
        int col= (*iDraw - 1) % 10;
        ui->RR_NumberGrid->item(row, col)->setBackground(br);
    }
    // mise à jour de l’affichage des derniers numéros
    ui->RR_CurrentDraw->display(0);
    ui->RR_LastNumber1->display(0);
    ui->RR_LastNumber2->display(0);
    ui->RR_LastNumber3->display(0);
    auto it= round->beginReverseDraws();
    if(it != round->endReverseDraws()) {
        ui->RR_CurrentDraw->display(*it);
        ++it;
        if(it != round->endReverseDraws()) {
            ui->RR_LastNumber1->display(*it);
            ++it;
            if(it != round->endReverseDraws()) {
                ui->RR_LastNumber2->display(*it);
                ++it;
                if(it != round->endReverseDraws()) {
                    ui->RR_LastNumber3->display(*it);
                }
            }
        }
    }
    // mise à jour affichage des lots et type de partie
    ui->RR_SubRoundVictoryCondition->setText("Condition de gain: " + QString::fromUtf8(round->getCurrentCSubRound()->getTypeStr()));
    ui->RR_SubRoundPrice->setText("Gain:\n" + QString::fromUtf8(round->getCurrentCSubRound()->getPrices()));
}

void DisplayWindow::initializeNumberGrid() {
    ui->RR_NumberGrid->setColumnCount(10);
    ui->RR_NumberGrid->setRowCount(9);
    for(int row= 0; row < 9; ++row) {
        for(int col= 0; col < 10; ++col) {
            QTableWidgetItem* pCell= ui->RR_NumberGrid->item(row, col);
            if(!pCell) {
                pCell= new QTableWidgetItem;
                ui->RR_NumberGrid->setItem(row, col, pCell);
            }
            pCell->setFlags(Qt::ItemFlag::ItemIsEnabled);
            pCell->setText(QString::number(10 * row + col + 1));
            pCell->setTextAlignment(Qt::AlignCenter);
            int px= std::min(ui->RR_NumberGrid->width() / 10, ui->RR_NumberGrid->height() / 9) / 3;
            pCell->setFont(QFont("Segoe UI", px));
        }
    }
}

void DisplayWindow::resetGrid() {
    QBrush br;
    br.setColor(QColor(0, 0, 0, 0));
    br.setStyle(Qt::BrushStyle::SolidPattern);
    for(int row= 0; row < 9; ++row) {
        for(int col= 0; col < 10; ++col) {
            ui->RR_NumberGrid->item(row, col)->setBackground(br);
        }
    }
}

void DisplayWindow::resize() {
    // taille de police dans la grille
    auto font= ui->RR_NumberGrid->font();
    int px   = std::min(ui->RR_NumberGrid->width() / 10, ui->RR_NumberGrid->height() / 9) / 3;
    font.setPointSize(px);
    ui->RR_NumberGrid->setFont(font);
    ui->RR_SubRoundVictoryCondition->setFont(font);
    ui->RR_SubRoundPrice->setFont(font);
    for(int row= 0; row < 9; ++row) {
        for(int col= 0; col < 10; ++col) {
            QTableWidgetItem* pCell= ui->RR_NumberGrid->item(row, col);
            if(pCell) {
                pCell->setFont(font);
            }
        }
    }
    for(int col= 0; col < 10; ++col)
        ui->RR_NumberGrid->setColumnWidth(col, ui->RR_NumberGrid->width() / 10);
    for(int row= 0; row < 9; ++row)
        ui->RR_NumberGrid->setRowHeight(row, ui->RR_NumberGrid->height() / 9);
    currentSize= size();
    initializeDisplay();
}

}// namespace evl::gui
