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
        return;
    case core::Event::Status::GameRunning:
        ui->PageManager->setCurrentIndex(2);
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

}// namespace evl::gui
