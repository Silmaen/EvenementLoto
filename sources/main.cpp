/**
 * @author Silmaen
 * @date 17/10/2021
 *
 */
#include "UI/MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    evl::MainWindow window;
    window.show();
    return QApplication::exec();
}
