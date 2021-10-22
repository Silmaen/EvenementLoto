/**
 * @file ConfigGameRounds.cpp
 * @author Silmaen
 * @date 20/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "gui/ConfigGameRounds.h"
#include <QMessageBox>
#include <ctime>

// Les trucs de QT
#include "gui/moc_ConfigGameRounds.cpp"
#include "ui/ui_ConfigGameRounds.h"

namespace evl::gui {

ConfigGameRounds::ConfigGameRounds(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ConfigGameRounds) {
    ui->setupUi(this);
    ui->PartieType->addItems(getGameRoundTypes());
}

ConfigGameRounds::~ConfigGameRounds() {
    delete ui;
}

int ConfigGameRounds::SaveFile() {
    bool good= true;
    for(const auto& p: gameEvent.getGameRounds()) {
        if(p.getStatus() == core::GameRound::Status::Invalid) {
            good= false;
            break;
        }
    }
    if(!good) {
        QMessageBox message;
        message.setIcon(QMessageBox::Critical);
        message.setWindowTitle("Sauvegarde impossible");
        message.setText("Sauvegarde impossible");
        message.setInformativeText("Certaines parties on un statut invalide, veuillez corriger avant de continuer.");
        message.exec();
        return 0;
    }
    return 1;
}

void ConfigGameRounds::actOk() {
    if(!gameEvent.isEditable())
        reject();
    if(SaveFile() > 0)
        accept();
}

void ConfigGameRounds::actApply() {
    SaveFile();
}

void ConfigGameRounds::actCancel() {
    reject();
}

void ConfigGameRounds::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

int ConfigGameRounds::exec() {
    updateDisplay(true);
    return QDialog::exec();
}

void ConfigGameRounds::actCreateGameRound() {
    gameEvent.getGameRounds().emplace_back();
    updateDisplay(true);
}

void ConfigGameRounds::actDeleteGameRound() {
    int idx= ui->SelectedPartie->currentIndex();
    if(idx < 0) return;
    gameEvent.getGameRounds().erase(std::next(gameEvent.getGameRounds().begin(), idx));
    ui->SelectedPartie->setCurrentIndex(idx - 1);
    updateDisplay();
}

void ConfigGameRounds::actLoadGameRound() {
    updateDisplay();
}

void ConfigGameRounds::actMoveGameRoundAfter() {
    int idx= getCurrentGameRoundIndex();
    if(idx >= (int)gameEvent.getGameRounds().size() - 1)
        return;
    std::swap(gameEvent.getGameRounds()[idx], gameEvent.getGameRounds()[idx + 1]);
    ui->SelectedPartie->setCurrentIndex(idx + 1);
    updateDisplay();
}

void ConfigGameRounds::actMoveGameRoundBefore() {
    int idx= getCurrentGameRoundIndex();
    if(idx <= 0)
        return;
    std::swap(gameEvent.getGameRounds()[idx], gameEvent.getGameRounds()[idx - 1]);
    ui->SelectedPartie->setCurrentIndex(idx - 1);
    updateDisplay();
}

void ConfigGameRounds::actSaveGameRound() {
    getCurrentGameRound().setType(getCurrentGameRoundType());
    updateDisplay();
}

void ConfigGameRounds::setEvent(const core::Event& e) {
    gameEvent= e;
    updateDisplay();
}

void ConfigGameRounds::updateDisplay(bool loadLast) {
    ui->PartiesNumber->display((int)gameEvent.getGameRounds().size());

    int a= ui->SelectedPartie->currentIndex();
    if(loadLast) {
        a= gameEvent.getGameRounds().size() - 1;
    }
    ui->SelectedPartie->clear();
    uint16_t c= 1;
    for(const auto& p: gameEvent.getGameRounds()) {
        ui->SelectedPartie->addItem("Partie " + QVariant(c).toString() + " - " + QString::fromStdString(p.getTypeStr()));
        c++;
    }
    ui->SelectedPartie->setCurrentIndex(a);

    if(a < 0) {
        ui->DetailPartie->setEnabled(false);
        ui->BtnLoadPartie->setEnabled(false);
        return;
    }
    ui->DetailPartie->setEnabled(true);
    ui->BtnLoadPartie->setEnabled(true);

    ui->PartieOrder->setText(QVariant(a + 1).toString());
    core::GameRound& par= gameEvent.getGameRounds()[a];
    int b               = getIdByGameRoundType(par.getType());
    ui->PartieType->setCurrentIndex(b);
    if(b < 0) {
        ui->PartieType->setEditText("<Sélectionnez un type>");
    }
    ui->PartieStatus->setText(QString::fromStdString(par.getStatusStr()));

    // nettoyage affichage
    ui->PartieStartingDate->clear();
    ui->PartieEndingDate->clear();
    ui->PartieDuration->clear();
    ui->IdWinnerGrid->clear();
    ui->ListTirage->clear();
    if(par.getStatus() == core::GameRound::Status::Started || par.getStatus() == core::GameRound::Status::Finished) {
        std::time_t t= std::chrono::system_clock::to_time_t(par.getStarting());
        QString s(std::ctime(&t));
        ui->PartieStartingDate->setText(s);
        QString text;
        for(auto tt: par.getDraws()) {
            text+= QVariant(tt).toString() + " ";
        }
        ui->ListTirage->setText(text);
    }
    if(par.getStatus() == core::GameRound::Status::Finished) {
        std::time_t t= std::chrono::system_clock::to_time_t(par.getEnding());
        QString s(std::ctime(&t));
        ui->PartieEndingDate->setText(s);
    }
    // désactive les boutons modifiants les parties si événement démarré
    if(!gameEvent.isEditable()) {
        ui->BtnSupprimePartie->setEnabled(false);
        ui->BtnCreatePArtie->setEnabled(false);
        ui->PartieMoveAfter->setEnabled(false);
        ui->PartieType->setEnabled(false);
        ui->PartieMoveBefore->setEnabled(false);
        ui->BtnSavePartie->setEnabled(false);
        ui->ButtonApply->setEnabled(false);
    }
}

QStringList ConfigGameRounds::getGameRoundTypes() {
    return {"Un quine",
            "Deux quines",
            "Carton Plein",
            "Inverse"};
}

int ConfigGameRounds::getIdByGameRoundType(const core::GameRound::Type& p) {
    switch(p) {
    case core::GameRound::Type::None: return -1;
    case core::GameRound::Type::OneQuine: return 0;
    case core::GameRound::Type::TwoQuines: return 1;
    case core::GameRound::Type::FullCard: return 2;
    case core::GameRound::Type::Inverse: return 3;
    }
    return -1;
}

core::GameRound::Type ConfigGameRounds::getCurrentGameRoundType() {
    switch(ui->PartieType->currentIndex()) {
    case 0: return core::GameRound::Type::OneQuine;
    case 1: return core::GameRound::Type::TwoQuines;
    case 2: return core::GameRound::Type::FullCard;
    case 3: return core::GameRound::Type::Inverse;
    }
    return core::GameRound::Type::None;
}

core::GameRound& ConfigGameRounds::getCurrentGameRound() {
    return gameEvent.getGameRounds()[getCurrentGameRoundIndex()];
}

int ConfigGameRounds::getCurrentGameRoundIndex() {
    return ui->PartieOrder->text().toInt() - 1;
}

}// namespace evl::gui
