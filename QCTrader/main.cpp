#include "QCTrader.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCTrader w;
    w.show();
    return a.exec();
}
