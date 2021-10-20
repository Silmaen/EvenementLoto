/**
* @author Silmaen
* @date 18/10/2021
*/
#include "UI/MainWindow.h"
#include "UI/About.h"
#include "UI/ConfigCartons.h"
#include "UI/ConfigurationParties.h"
#include "UI/EvenementConfig.h"
#include "UI/GeneralConfig.h"
#include <QMessageBox>
#include <iostream>

// Les trucs de QT
#include <UI/moc_MainWindow.cpp>
#include <UI/ui_MainWindow.h>

namespace evl::gui {

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings{QString(getIniFile().generic_u8string().c_str()), QSettings::IniFormat} {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}
void MainWindow::showAbout() {
    evl::gui::About AboutWindow(this);
    AboutWindow.setModal(true);
    AboutWindow.exec();
}

void MainWindow::showHelp() {
    showNotImplemented("aide");
}

void MainWindow::showParametresGeneraux() {
    //showNotImplemented("Paramètres généraux");
    GeneralConfig cfg(this);
    settings.value("path/datapath", "").toString();
    cfg.exec();
}

void MainWindow::showParametresCartons() {
    //showNotImplemented("Configuration des cartons");
    ConfigCartons cfg(this);
    cfg.getCartons().getNom()= "default";
    cfg.exec();
}

void MainWindow::showParametresParties() {
    //showNotImplemented("Configuration des parties");
    ConfigurationParties cfg;
    cfg.exec();
}

void MainWindow::showParametresEvenement() {
    //showNotImplemented("Configuration de l’événement");
    EvenementConfig cfg;
    cfg.exec();
}

void MainWindow::fichierNew() {
    showNotImplemented("Nouvel événement");
}

void MainWindow::fichierLoad() {
    showNotImplemented("Charger événement");
}

void MainWindow::fichierSave() {
    showNotImplemented("Sauver événement");
}

void MainWindow::fichierSaveAs() {
    showNotImplemented("Sauver événement sous...");
}

void MainWindow::fichierCommencer() {
    showNotImplemented("Commencer événement");
}

void MainWindow::fichierTerminer() {
    showNotImplemented("Terminer événement");
}

void MainWindow::fichierQuitter() {
    close();
}

void MainWindow::showNotImplemented(const QString& from) {
    QMessageBox message;
    message.setIcon(QMessageBox::Warning);
    message.setWindowTitle(from);
    message.setText("Ce programme est encore en construction");
    message.setInformativeText("La fonction '" + from + "' N’a pas encore été implémentée.");
    message.exec();
}

void MainWindow::syncSettings() {
    settings.sync();
}

}// namespace evl::gui
