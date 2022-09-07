/**
 * @file ConfigCardPack.cpp
 * @author Silmaen
 * @date 19/10/2021
 * Copyright © 2021 All rights reserved.
 * All modification must get authorization from the author.
 */
#include "ConfigCardPack.h"
#include "baseDefinitions.h"
#include <QMessageBox>

// Les trucs de QT
#include "moc_ConfigCardPack.cpp"
#include "ui/ui_ConfigCardPack.h"

namespace evl::gui {

ConfigCardPack::ConfigCardPack(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ConfigCardPack) {
    ui->setupUi(this);
    lines[0][0].spin = ui->UICell_L1_C0;
    lines[0][0].check= ui->UIChk_L1_C0;
    lines[0][1].spin = ui->UICell_L1_C1;
    lines[0][1].check= ui->UIChk_L1_C1;
    lines[0][2].spin = ui->UICell_L1_C2;
    lines[0][2].check= ui->UIChk_L1_C2;
    lines[0][3].spin = ui->UICell_L1_C3;
    lines[0][3].check= ui->UIChk_L1_C3;
    lines[0][4].spin = ui->UICell_L1_C4;
    lines[0][4].check= ui->UIChk_L1_C4;
    lines[0][5].spin = ui->UICell_L1_C5;
    lines[0][5].check= ui->UIChk_L1_C5;
    lines[0][6].spin = ui->UICell_L1_C6;
    lines[0][6].check= ui->UIChk_L1_C6;
    lines[0][7].spin = ui->UICell_L1_C7;
    lines[0][7].check= ui->UIChk_L1_C7;
    lines[0][8].spin = ui->UICell_L1_C8;
    lines[0][8].check= ui->UIChk_L1_C8;
    lines[1][0].spin = ui->UICell_L2_C0;
    lines[1][0].check= ui->UIChk_L2_C0;
    lines[1][1].spin = ui->UICell_L2_C1;
    lines[1][1].check= ui->UIChk_L2_C1;
    lines[1][2].spin = ui->UICell_L2_C2;
    lines[1][2].check= ui->UIChk_L2_C2;
    lines[1][3].spin = ui->UICell_L2_C3;
    lines[1][3].check= ui->UIChk_L2_C3;
    lines[1][4].spin = ui->UICell_L2_C4;
    lines[1][4].check= ui->UIChk_L2_C4;
    lines[1][5].spin = ui->UICell_L2_C5;
    lines[1][5].check= ui->UIChk_L2_C5;
    lines[1][6].spin = ui->UICell_L2_C6;
    lines[1][6].check= ui->UIChk_L2_C6;
    lines[1][7].spin = ui->UICell_L2_C7;
    lines[1][7].check= ui->UIChk_L2_C7;
    lines[1][8].spin = ui->UICell_L2_C8;
    lines[1][8].check= ui->UIChk_L2_C8;
    lines[2][0].spin = ui->UICell_L3_C0;
    lines[2][0].check= ui->UIChk_L3_C0;
    lines[2][1].spin = ui->UICell_L3_C1;
    lines[2][1].check= ui->UIChk_L3_C1;
    lines[2][2].spin = ui->UICell_L3_C2;
    lines[2][2].check= ui->UIChk_L3_C2;
    lines[2][3].spin = ui->UICell_L3_C3;
    lines[2][3].check= ui->UIChk_L3_C3;
    lines[2][4].spin = ui->UICell_L3_C4;
    lines[2][4].check= ui->UIChk_L3_C4;
    lines[2][5].spin = ui->UICell_L3_C5;
    lines[2][5].check= ui->UIChk_L3_C5;
    lines[2][6].spin = ui->UICell_L3_C6;
    lines[2][6].check= ui->UIChk_L3_C6;
    lines[2][7].spin = ui->UICell_L3_C7;
    lines[2][7].check= ui->UIChk_L3_C7;
    lines[2][8].spin = ui->UICell_L3_C8;
    lines[2][8].check= ui->UIChk_L3_C8;
}

ConfigCardPack::~ConfigCardPack() {
    delete ui;
}

void ConfigCardPack::actLoadFile() {
    showNotImplemented("actLoadFile");
}

void ConfigCardPack::actSaveFile() {
    showNotImplemented("actSaveFile");
}

void ConfigCardPack::actOk() {
    actSaveFile();
    accept();
}

void ConfigCardPack::actApply() {
    actSaveFile();
}

void ConfigCardPack::actCancel() {
    cardPack.fullReset();
    reject();
}
void ConfigCardPack::actSaveInList() {
    showNotImplemented("actSaveInList");
}

void ConfigCardPack::actLoadFromList() {
    showNotImplemented("actLoadFromList");
}

void ConfigCardPack::actLoadLine() {
    showNotImplemented("actLoadLine");
}

void ConfigCardPack::actGenerate() {
    uint32_t val= static_cast<uint32_t>(ui->UIGenerateNumber->value());
    if(val < 1 || val > 2000000000) {
        QMessageBox message;
        message.setIcon(QMessageBox::Critical);
        message.setWindowTitle("Générateur de carton");
        message.setText("Erreur dans le nombre de cartons à générer");
        message.setInformativeText("Veuillez entrer un nombre de cartons entre 1 et 4 000 000 000.");
        message.exec();
    }
    cardPack.generate(val);
    actUpdateDisplay();
}

void ConfigCardPack::actUpdateGrid() {
    for(auto& line: lines) {
        int nActive= 0;
        for(auto& ce: line) {
            ce.toggle();
            ce.check->setEnabled(true);
            nActive+= ce.check->isChecked();
        }
        if(nActive >= 5) {
            for(auto& ce: line) {
                ce.check->setEnabled(ce.check->isChecked());
            }
        }
    }
}

void ConfigCardPack::actUpdateDisplay() {
    ui->UIListName->setText(QString::fromStdString(cardPack.getName()));
    ui->UIFileName->setText(path);
    ui->UINumberCartons->display(static_cast<int>(cardPack.size()));
}

void ConfigCardPack::actUpdateName() {
    cardPack.getName()= ui->UIListName->text().toStdString();
}

void ConfigCardPack::actUpdateFileName() {
    path= ui->UIFileName->text();
}

void ConfigCardPack::preExec() {
    actUpdateDisplay();
    actUpdateGrid();
}

}// namespace evl::gui
