/**
* @file DisplayWindow.cpp
* @author Silmaen
* @date 18/10/2021
* Copyright © 2021 All rights reserved.
* All modification must get authorization from the author.
*/
#include "gui/DisplayWindow.h"

// Les trucs de QT
#include <gui/moc_DisplayWindow.cpp>
#include <ui/ui_DisplayWindow.h>

namespace evl::gui {

DisplayWindow::DisplayWindow(core::Event* evt, QWidget* parent):
    QDialog(parent),
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

}// namespace evl::gui
