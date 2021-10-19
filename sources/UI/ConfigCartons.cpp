/**
 * @author Silmaen
 * @date 19/10/2021
 */
#include "UI/ConfigCartons.h"

// Les trucs de QT
#include "UI/moc_ConfigCartons.cpp"
#include "UI/ui_ConfigCartons.h"

namespace evl::gui {

ConfigCartons::ConfigCartons(QWidget* parent):
    QDialog(parent),
    ui(new Ui::ConfigCartons) {
    ui->setupUi(this);
}

ConfigCartons::~ConfigCartons() {
    delete ui;
}

void ConfigCartons::LoadFile() {
}

void ConfigCartons::BottomButton(QAbstractButton*) {
}

void ConfigCartons::SaveInList() {
}

void ConfigCartons::LoadFromList() {
}

void ConfigCartons::LoadLine() {
}

void ConfigCartons::Generate() {
}

void ConfigCartons::UpdateGrid() {
}

}// namespace evl::gui
