/**
 * @author Silmaen
 * @date 18/10/2021
 */
#include "UI/About.h"

// Les trucs de QT
#include "UI/moc_About.cpp"
#include "UI/ui_About.h"

namespace evl {

About::About(QWidget* parent):
    QDialog(parent),
    ui(new Ui::About) {
    ui->setupUi(this);
}
About::~About() {
    delete ui;
}

}// namespace evl
