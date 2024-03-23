#include "diplomovka_TH.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataHandler w;
    w.show();
    return a.exec();
}
