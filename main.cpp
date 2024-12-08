#include "mainwindow.h"
#include "mapwidget.h"
#include <QApplication>
#include "mapwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
