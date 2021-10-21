/**
 * @author Silmaen
 * @date 20/10/2021
 */
#include "UI/ConfigurationParties.h"
#include <QMessageBox>
#include <ctime>

// Les trucs de QT
#include "UI/moc_ConfigurationParties.cpp"
#include "UI/ui_ConfigurationParties.h"

namespace evl::gui {

ConfigurationParties::ConfigurationParties(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ConfigurationParties) {
    ui->setupUi(this);
    ui->PartieType->addItems(getPartieTypes());
}

ConfigurationParties::~ConfigurationParties() {
    delete ui;
}

int ConfigurationParties::SaveFile() {
    bool good= true;
    for(auto p: evenement.getParties()) {
        if(p.getStatus() == core::Partie::Status::Invalid) {
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

void ConfigurationParties::actOk() {
    if(SaveFile() > 0)
        accept();
}

void ConfigurationParties::actApply() {
    SaveFile();
}

void ConfigurationParties::actCancel() {
    reject();
}

void ConfigurationParties::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

int ConfigurationParties::exec() {
    updateDisplay();
    return QDialog::exec();
}

void ConfigurationParties::partieCreate() {
    evenement.getParties().emplace_back();
    updateDisplay(true);
}

void ConfigurationParties::partieLoad() {
    updateDisplay();
}

void ConfigurationParties::partieOrderAfter() {
    if(ui->SelectedPartie->currentIndex() >= (int)evenement.getParties().size() - 1)
        return;
    showNotImplemented("partieOrderAfter");
}

void ConfigurationParties::partieOrderBefore() {
    if(ui->SelectedPartie->currentIndex() <= 0)
        return;
    showNotImplemented("partieOrderBefore");
}

void ConfigurationParties::partieSave() {
    getCurrentPartie().setType(getCurrentPartyType());
    updateDisplay();
}

void ConfigurationParties::setEvenement(const core::Evenement& e) {
    evenement= e;
    updateDisplay();
}

void ConfigurationParties::updateDisplay(bool loadLast) {
    ui->PartiesNumber->display((int)evenement.getParties().size());

    int a= ui->SelectedPartie->currentIndex();
    if(loadLast) {
        a= evenement.getParties().size() - 1;
    }
    ui->SelectedPartie->clear();
    uint16_t c= 1;
    for(auto p: evenement.getParties()) {
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
    core::Partie& par= evenement.getParties()[a];
    int b            = getIdByPartyType(par.getType());
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
    if(par.getStatus() == core::Partie::Status::Started || par.getStatus() == core::Partie::Status::Finished) {
        std::time_t t= std::chrono::system_clock::to_time_t(par.getStarting());
        QString s(std::ctime(&t));
        ui->PartieStartingDate->setText(s);
        QString text;
        for(auto t: par.getTirage()) {
            text+= QVariant(t).toString() + " ";
        }
        ui->ListTirage->setText(text);
    }
    if(par.getStatus() == core::Partie::Status::Finished) {
        std::time_t t= std::chrono::system_clock::to_time_t(par.getEnding());
        QString s(std::ctime(&t));
        ui->PartieEndingDate->setText(s);
    }
}

QStringList ConfigurationParties::getPartieTypes() {
    return {"Un quine",
            "Deux quines",
            "Carton Plein",
            "Inverse"};
}

int ConfigurationParties::getIdByPartyType(const core::Partie::Type& p) {
    switch(p) {
    case core::Partie::Type::Aucun: return -1;
    case core::Partie::Type::UneQuine: return 0;
    case core::Partie::Type::DeuxQuines: return 1;
    case core::Partie::Type::CartonPlein: return 2;
    case core::Partie::Type::Inverse: return 3;
    }
    return -1;
}

core::Partie::Type ConfigurationParties::getCurrentPartyType() {
    switch(ui->PartieType->currentIndex()) {
    case 0: return core::Partie::Type::UneQuine;
    case 1: return core::Partie::Type::DeuxQuines;
    case 2: return core::Partie::Type::CartonPlein;
    case 3: return core::Partie::Type::Inverse;
    }
    return core::Partie::Type::Aucun;
}

core::Partie& ConfigurationParties::getCurrentPartie() {
    int i= ui->PartieOrder->text().toInt() - 1;
    return evenement.getParties()[i];
}

}// namespace evl::gui
