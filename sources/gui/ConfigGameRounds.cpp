/**
 * @file ConfigGameRounds.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/ConfigGameRounds.h"
#include "core/timeFunctions.h"

// Les trucs de QT
#include "gui/moc_ConfigGameRounds.cpp"
#include "ui/ui_ConfigGameRounds.h"

namespace evl::gui {

ConfigGameRounds::ConfigGameRounds(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ConfigGameRounds) {
    ui->setupUi(this);
}

ConfigGameRounds::~ConfigGameRounds() {
    delete ui;
}

void ConfigGameRounds::actOk() {
    if(!gameEvent.isEditable())
        reject();
    accept();
}

void ConfigGameRounds::actApply() {
}

void ConfigGameRounds::actCancel() {
    reject();
}

void ConfigGameRounds::preExec() {
    updateDisplay();
}

void ConfigGameRounds::actCreateGameRound() {
    gameEvent.pushGameRound(core::GameRound());
    updateDisplay();
}

void ConfigGameRounds::actDeleteGameRound() {
    int cur= ui->listGameRound->currentRow();
    if(cur < 0)// rien de sélectionné
        return;
    gameEvent.deleteRoundByIndex(cur);
    updateDisplay();
}

void ConfigGameRounds::actMoveGameRoundAfter() {
    int cur= ui->listGameRound->currentRow();
    if(cur < 0)// rien de sélectionné
        return;
    if(cur >= (int)gameEvent.sizeRounds() - 1)// déjà en dernière position
        return;
    gameEvent.swapRoundByIndex(cur, cur + 1);
    updateDisplay();
}

void ConfigGameRounds::actMoveGameRoundBefore() {
    int cur= ui->listGameRound->currentRow();
    if(cur < 1)// rien de sélectionné ou déjà en première position
        return;
    gameEvent.swapRoundByIndex(cur, cur - 1);
    updateDisplay();
}

void ConfigGameRounds::actChangeGameRoundType([[maybe_unused]] int newIndex) {
}

void ConfigGameRounds::actChangeSubGameRoundType([[maybe_unused]] int newIndex) {
}

void ConfigGameRounds::actEndEditingPrice() {
}

void ConfigGameRounds::actCreateSubGameRound() {
}

void ConfigGameRounds::actDeleteSubGameRound() {
}

void ConfigGameRounds::actMoveSubGameRoundAfter() {
}

void ConfigGameRounds::actMoveSubGameRoundBefore() {
}

void ConfigGameRounds::actChangeSelectedGameRound([[maybe_unused]] int newIndex) {
    updateDisplay();
}

void ConfigGameRounds::actChangeSelectedSubGameRound([[maybe_unused]] int newIndex) {
    updateDisplay();
}

void ConfigGameRounds::setEvent(const core::Event& e) {
    gameEvent= e;
    updateDisplay();
}

void ConfigGameRounds::updateDisplay() {
    if(gameEvent.isEditable()) {
        ui->groupResult->setEnabled(false);
        ui->GroupEdit->setEnabled(true);
        updateDisplayEdits();
    } else {
        ui->groupResult->setEnabled(true);
        ui->GroupEdit->setEnabled(false);
        updateDisplayResults();
    }
}

void ConfigGameRounds::updateDisplayEdits() {
    // remplissage de la liste des parties
    int cur= ui->listGameRound->currentRow();
    ui->listGameRound->clear();
    int idx= 1;
    for(auto it= gameEvent.beginRounds(); it != gameEvent.endRounds(); ++it) {
        ui->listGameRound->addItem("Partie " + QString::number(idx) + " : " + QString::fromUtf8(it->getTypeStr()));
    }
    if(cur >= ui->listGameRound->count()) {
        ui->listGameRound->setCurrentRow(ui->listGameRound->count());
    } else {
        ui->listGameRound->setCurrentRow(cur);
    }
}

void ConfigGameRounds::updateDisplayResults() {
}

}// namespace evl::gui
