/**
 * @file ConfigGameRounds.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "ConfigGameRounds.h"
#include "BaseDialog.h"

// Les trucs de QT
#include "moc_ConfigGameRounds.cpp"
#include "ui/ui_ConfigGameRounds.h"

namespace evl::gui {

static bool in_update_loop= false;

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

void ConfigGameRounds::actImportRounds() {
    path file= dialog::openFile(dialog::FileTypes::JSON, true);
    if(file.empty())
        return;
    gameEvent.importJSON(file);
}

void ConfigGameRounds::actExportRounds() {
    path file= dialog::openFile(dialog::FileTypes::JSON, false);
    if(file.empty())
        return;
    gameEvent.exportJSON(file);
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
    gameEvent.deleteRoundByIndex(static_cast<uint16_t>(cur));
    updateDisplay();
}

void ConfigGameRounds::actMoveGameRoundAfter() {
    int cur= ui->listGameRound->currentRow();
    if(cur < 0)// rien de sélectionné
        return;
    if(cur >= static_cast<int>(gameEvent.sizeRounds() - 1))// déjà en dernière position
        return;
    gameEvent.swapRoundByIndex(static_cast<uint16_t>(cur), static_cast<uint16_t>(cur + 1));
    updateDisplay();
}

void ConfigGameRounds::actMoveGameRoundBefore() {
    int cur= ui->listGameRound->currentRow();
    if(cur < 1)// rien de sélectionné ou déjà en première position
        return;
    gameEvent.swapRoundByIndex(static_cast<uint16_t>(cur), static_cast<uint16_t>(cur - 1));
    updateDisplay();
}

void ConfigGameRounds::actChangeGameRoundType(int newIndex) {
    if(in_update_loop)
        return;
    int cur= ui->listGameRound->currentRow();
    if(cur < 0)
        return;
    auto round= gameEvent.getGameRound(static_cast<uint16_t>(cur));
    round->setType(static_cast<core::GameRound::Type>(newIndex));
    updateDisplay();
}

void ConfigGameRounds::actChangeRoundNumber(int) {
    if(in_update_loop)
        return;
    int cur= ui->listGameRound->currentRow();
    if(cur < 0)
        return;
    gameEvent.getGameRound(static_cast<uint16_t>(cur))->setID(ui->spinRoundNumber->value());
    updateDisplay();
}

void ConfigGameRounds::actEndEditingPrice() {
    if(in_update_loop)
        return;
    int cur= ui->listGameRound->currentRow();
    if(cur < 0)
        return;
    auto round= gameEvent.getGameRound(static_cast<uint16_t>(cur));
    int scur  = ui->listSubRound->currentRow();
    if(scur < 0)
        return;
    auto subround= round->getSubRound(static_cast<uint32_t>(scur));
    subround->define(subround->getType(), ui->TextPrices->toPlainText().toStdString(), ui->PricesValue->value());
    updateDisplay();
}

void ConfigGameRounds::actChangePriceValue() {
    actEndEditingPrice();
}

// -----------------
void ConfigGameRounds::actChangeSubGameRoundType([[maybe_unused]] int newIndex) {
}
void ConfigGameRounds::actCreateSubGameRound() {
}
void ConfigGameRounds::actDeleteSubGameRound() {
}
void ConfigGameRounds::actMoveSubGameRoundAfter() {
}
void ConfigGameRounds::actMoveSubGameRoundBefore() {
}
// -----------------

void ConfigGameRounds::actChangeSelectedGameRound() {
    updateDisplay();
}

void ConfigGameRounds::actChangeSelectedSubGameRound() {
    updateDisplay();
}

void ConfigGameRounds::setEvent(const core::Event& e) {
    gameEvent= e;
    updateDisplay();
}

void ConfigGameRounds::updateDisplay() {
    if(onUpdate)
        return;
    onUpdate= true;
    // pas d’édition des sous-parties pour le moment : on cache
    ui->btnAddSubRound->setVisible(false);
    ui->btnDelSubRound->setVisible(false);
    ui->btnUpSubround->setVisible(false);
    ui->btnDownSubRound->setVisible(false);
    ui->GroupEdit->setEnabled(true);
    //
    if(gameEvent.isEditable()) {
        ui->groupResult->setEnabled(false);
        ui->groupSubRound->setEnabled(true);
        ui->groupPhase->setEnabled(true);
        ui->btnAddRound->setEnabled(true);
        ui->btnAddRound->setEnabled(true);
        ui->btnDelRound->setEnabled(true);
        ui->btnDownRound->setEnabled(true);
        ui->btnUpRound->setEnabled(true);
        updateDisplayEdits();
    } else {
        ui->groupResult->setEnabled(true);
        ui->groupSubRound->setEnabled(false);
        ui->groupPhase->setEnabled(false);
        ui->btnAddRound->setEnabled(false);
        ui->btnDelRound->setEnabled(false);
        ui->btnDownRound->setEnabled(false);
        ui->btnUpRound->setEnabled(false);
        updateDisplayResults();
    }
    onUpdate= false;
}

void ConfigGameRounds::updateDisplayRoundList() {

    // remplissage de la liste des parties
    int cur= ui->listGameRound->currentRow();
    ui->listGameRound->clear();
    int idx= 1;
    for(auto it= gameEvent.beginRounds(); it != gameEvent.endRounds(); ++it, ++idx) {
        ui->listGameRound->addItem("Partie " + QString::number(idx) + " : " +
                                   QString::fromUtf8(it->getTypeStr()) +
                                   " - " + QString::fromUtf8(it->getName()));
    }
    if(cur >= ui->listGameRound->count()) {
        ui->listGameRound->setCurrentRow(ui->listGameRound->count());
    } else {
        ui->listGameRound->setCurrentRow(cur);
    }
}

void ConfigGameRounds::updateDisplayEdits() {
    // tuer les récursions infinies
    if(in_update_loop) return;
    in_update_loop= true;
    updateDisplayRoundList();
    int cur= ui->listGameRound->currentRow();
    // remplissage des sous-rounds
    if(cur >= 0) {
        ui->groupSubRound->setEnabled(true);
        ui->GameRoundTypes->clear();
        ui->GameRoundTypes->addItems(getRoundTypes());
        auto round= gameEvent.getGameRound(static_cast<uint16_t>(cur));
        ui->GameRoundTypes->setCurrentIndex(static_cast<int>(round->getType()));
        ui->spinRoundNumber->setValue(round->getID());
        int scur= ui->listSubRound->currentRow();
        ui->listSubRound->clear();
        int idx= 1;
        for(auto it= round->beginSubRound(); it != round->endSubRound(); ++it, ++idx) {
            ui->listSubRound->addItem("Phase " + QString::number(idx) + " : " + QString::fromUtf8(it->getTypeStr()));
        }
        if(scur >= ui->listSubRound->count()) {
            ui->listSubRound->setCurrentRow(ui->listGameRound->count());
        } else {
            ui->listSubRound->setCurrentRow(scur);
        }
        scur= ui->listSubRound->currentRow();// re-update l'indice
        // remplissage de la configuration de la phase
        if(scur >= 0) {
            ui->groupPhase->setEnabled(true);
            auto subRound= round->getSubRound(static_cast<uint32_t>(scur));
            ui->SubRoundTypes->clear();
            ui->SubRoundTypes->addItems(getVictoryType(round->getType()));
            ui->SubRoundTypes->setCurrentIndex(getVictoryIndex(round->getType(), subRound->getType()));
            auto p= ui->TextPrices->textCursor();
            ui->TextPrices->clear();
            ui->TextPrices->setText(QString::fromUtf8(subRound->getPrices()));
            ui->TextPrices->setTextCursor(p);
            ui->PricesValue->setValue(subRound->getValue());
        } else {
            ui->groupPhase->setEnabled(false);
            ui->SubRoundTypes->clear();
            ui->TextPrices->clear();
            ui->PricesValue->setValue(0);
        }
    } else {
        ui->listSubRound->clear();
        ui->groupSubRound->setEnabled(false);
        ui->GameRoundTypes->clear();
        ui->groupPhase->setEnabled(false);
        ui->SubRoundTypes->clear();
        ui->TextPrices->clear();
    }
    // on libère le mutex
    in_update_loop= false;
}

void ConfigGameRounds::updateDisplayResults() {
    updateDisplayRoundList();
    int cur= ui->listGameRound->currentRow();
    // nettoyage de tous les champs
    ui->StartingDate->setText("");
    ui->EndingDate->setText("");
    ui->Duration->setText("");
    ui->NumberDraws->setText("");
    ui->TextDraws->setText("");
    ui->ListWinners->setText("");
    if(cur < 0) {
        ui->StartingDate->setEnabled(false);
        ui->EndingDate->setEnabled(false);
        ui->Duration->setEnabled(false);
        ui->NumberDraws->setEnabled(false);
        ui->TextDraws->setEnabled(false);
        ui->ListWinners->setEnabled(false);
        return;
    }
    auto round= gameEvent.getGameRound(static_cast<uint16_t>(cur));
    if(round->getStatus() == core::GameRound::Status::Ready)
        return;
    ui->StartingDate->setText(QString::fromUtf8(core::formatClock(round->getStarting())));
    ui->NumberDraws->setText(QString::number(round->drawsCount()));
    ui->TextDraws->setText(QString::fromStdString(round->getDrawStr()));
    if(round->getStatus() == core::GameRound::Status::Running) {
        ui->EndingDate->setEnabled(false);
        ui->Duration->setEnabled(false);
        ui->ListWinners->setEnabled(false);
        ui->ListWinners->setText("En cours...");
        return;
    }
    ui->EndingDate->setText(QString::fromUtf8(core::formatClock(round->getEnding())));
    ui->Duration->setText(QString::fromUtf8(core::formatDuration(round->getEnding() - round->getStarting())));
    QString resultats;
    int i= 1;
    for(auto ii= round->beginSubRound(); ii != round->endSubRound(); ++ii) {
        resultats+= "Sous-partie " + QString::number(i) + ": " + QString::fromUtf8(ii->getTypeStr()) + "\n";
        resultats+= "  grille " + QString::fromStdString(ii->getWinner()) + " gagne: " + QString::fromUtf8(ii->getPrices()) + "\n";
        ++i;
    }
    ui->ListWinners->setText(resultats);
}

QStringList ConfigGameRounds::getRoundTypes() {
    return {"Un quine", "Deux quines", "Carton plein", "Un quine et carton plein", "Normale", "Enfants", "Inverse", "Pause"};
}

QStringList ConfigGameRounds::getVictoryType(const core::GameRound::Type& t) {
    switch(t) {
    case core::GameRound::Type::OneQuine:
        return {"un quine"};
    case core::GameRound::Type::TwoQuines:
        return {"deux quines"};
    case core::GameRound::Type::FullCard:
        return {"carton plein"};
    case core::GameRound::Type::OneQuineFullCard:
        return {"un quine", "carton plein"};
    case core::GameRound::Type::OneTwoQuineFullCard:
        return {"un quine", "deux quines", "carton plein"};
    case core::GameRound::Type::Enfant:
        return {"une ligne"};
    case core::GameRound::Type::Inverse:
        return {"inverse"};
    case core::GameRound::Type::Pause:
        return {"none"};
    }
    return {};
}

int ConfigGameRounds::getVictoryIndex(const core::GameRound::Type& t, const core::SubGameRound::Type& st) {
    switch(t) {
    case core::GameRound::Type::OneQuineFullCard:
        switch(st) {
        case core::SubGameRound::Type::OneQuine:
            return 0;
        case core::SubGameRound::Type::TwoQuines:
            return -1;
        case core::SubGameRound::Type::FullCard:
            return 1;
        case core::SubGameRound::Type::Inverse:
            return -1;
        }
        return -1;
    case core::GameRound::Type::OneTwoQuineFullCard:
        switch(st) {
        case core::SubGameRound::Type::OneQuine:
            return 0;
        case core::SubGameRound::Type::TwoQuines:
            return 1;
        case core::SubGameRound::Type::FullCard:
            return 2;
        case core::SubGameRound::Type::Inverse:
            return -1;
        }
        return -1;
    case core::GameRound::Type::OneQuine:
    case core::GameRound::Type::TwoQuines:
    case core::GameRound::Type::FullCard:
    case core::GameRound::Type::Enfant:
    case core::GameRound::Type::Inverse:
    case core::GameRound::Type::Pause:
        return 0;
    }
    return -1;
}

}// namespace evl::gui
