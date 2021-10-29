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
    timer->setInterval(500);
    timer->start();
    initializeNumberGrid();
}

DisplayWindow::~DisplayWindow() {
    delete ui;
    delete timer;
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
    if(event->getOrganizerLogo().empty()) {
        ui->ET_OrganizerLogo->setVisible(false);
    } else {
        if(!exists(event->getOrganizerLogo())) {
            ui->ET_OrganizerLogo->setVisible(false);
        } else {
            QString imgName(QString::fromUtf8(event->getOrganizerLogo().string()));
            if(ui->ET_OrganizerLogo->text() != imgName) {// charge l’image seulement une fois
                ui->ET_OrganizerLogo->setText(imgName);
                QImage img;
                img.load(imgName);
                ui->ET_OrganizerLogo->setPixmap(QPixmap::fromImage(img.scaled(ui->ET_OrganizerLogo->size())));
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
    ui->RR_Title->setText("Partie " + QString::number(event->getCurrentIndex() + 1));
    auto round= event->getGameRound(event->getCurrentIndex());
    // mise à jour de l'affichage de la grille
    resizeGrid();
    QBrush br;
    br.setColor(QColor(255, 112, 0));
    br.setStyle(Qt::BrushStyle::Dense1Pattern);
    for(auto iDraw= round->beginDraws(); iDraw != round->endDraws(); ++iDraw) {
        int row= *iDraw / 10;
        int col= *iDraw % 10 - 1;
        ui->RR_NumberGrid->item(row, col)->setBackground(br);
    }
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
            pCell->setText(QString::number(10 * row + col + 1));
            pCell->setTextAlignment(Qt::AlignCenter);
            pCell->setFont(QFont("Segoe UI", 25));
        }
    }
}
void DisplayWindow::resizeGrid() {
    for(int col= 0; col < 10; ++col)
        ui->RR_NumberGrid->setColumnWidth(col, ui->RR_NumberGrid->width() / 10);
    for(int row= 0; row < 9; ++row)
        ui->RR_NumberGrid->setRowHeight(row, ui->RR_NumberGrid->height() / 9);
}

}// namespace evl::gui
