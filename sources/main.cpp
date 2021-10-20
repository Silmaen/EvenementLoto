/**
 * @author Silmaen
 * @date 17/10/2021
 *
 */
#include "UI/MainWindow.h"
#include <QApplication>

using namespace evl::gui;
using namespace std::filesystem;

int main(int argc, char* argv[]) {
    baseExecPath= absolute(path(argv[0])).parent_path();
    QApplication a(argc, argv);
    MainWindow window;
    window.syncSettings();
    window.show();
    return QApplication::exec();
}
