#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    a.setApplicationName("qt_communications test");
    w.setWindowState(Qt::WindowMaximized);
    w.show();
    return a.exec();
}
