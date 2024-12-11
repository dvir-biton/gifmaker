#include "mainwindow.h"

#include <QApplication>

// very cool project by bidbid (let's goo)
// main function to build the MainWindow object to display the ui window
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
