#include "mainwindow.h"

#include <QApplication>
#include <vector>
#include <string>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    qRegisterMetaType<std::vector<std::string> >("std::vector<std::string>");
    qRegisterMetaType<size_t>("size_t");
    w.show();

    return a.exec();
}
