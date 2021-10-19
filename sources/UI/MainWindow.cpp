/**
* @author Silmaen
* @date 18/10/2021
*/
#include "UI/MainWindow.h"
#include "UI/About.h"

// Les trucs de QT
#include <UI/moc_MainWindow.cpp>
#include <UI/ui_MainWindow.h>

namespace evl::gui {

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
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

}// namespace elv
