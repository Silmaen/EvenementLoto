/**
 * @author Silmaen
 * @date 19/10/2021
 */
#include "UI/ConfigCartons.h"
#include <QMessageBox>

// Les trucs de QT
#include "UI/moc_ConfigCartons.cpp"
#include "UI/ui_ConfigCartons.h"

namespace evl::gui {

ConfigCartons::ConfigCartons(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ConfigCartons) {
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

ConfigCartons::~ConfigCartons() {
    delete ui;
}

void ConfigCartons::LoadFile() {
    showNotImplemented("LoadFile");
}

void ConfigCartons::SaveFile() {
    showNotImplemented("SaveFile");
}

void ConfigCartons::ClicOk() {
    SaveFile();
    accept();
}

void ConfigCartons::ClicApply() {
    SaveFile();
}

void ConfigCartons::ClicCancel() {
    cartons.fullReset();
    reject();
}
void ConfigCartons::SaveInList() {
    showNotImplemented("SaveInList");
}

void ConfigCartons::LoadFromList() {
    showNotImplemented("LoadFromList");
}

void ConfigCartons::LoadLine() {
    showNotImplemented("LoadLine");
}

void ConfigCartons::Generate() {
    int val= ui->UIGenerateNumber->value();
    if(val < 1 || val > 2000000000) {
        QMessageBox message;
        message.setIcon(QMessageBox::Critical);
        message.setWindowTitle("Générateur de carton");
        message.setText("Erreur dans le nombre de cartons à générer");
        message.setInformativeText("Veuillez entrer un nombre de cartons entre 1 et 4 000 000 000.");
        message.exec();
    }
    cartons.generate(val);
    UpdateDisplay();
}

void ConfigCartons::UpdateGrid() {
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

void ConfigCartons::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

void ConfigCartons::UpdateDisplay() {
    ui->UIListName->setText(QString::fromStdString(cartons.getNom()));
    ui->UIFileName->setText(path);
    ui->UINumberCartons->display((int)cartons.getNumber());
}

void ConfigCartons::updateName() {
    cartons.getNom()= ui->UIListName->text().toStdString();
}

void ConfigCartons::updateFileName() {
    path= ui->UIFileName->text();
}

int ConfigCartons::exec() {
    UpdateDisplay();
    UpdateGrid();
    return QDialog::exec();
}

}// namespace evl::gui
