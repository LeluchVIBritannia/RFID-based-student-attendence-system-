#include <QApplication>
#include <QDebug>
#include "MainWindow.h"
#include "SerialManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "Application started!";

    MainWindow w;
    w.show();

    qDebug() << "Window should be visible now!";

    SerialManager manager;
    return app.exec();
}