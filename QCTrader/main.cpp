#include "QCTrader.h"
#include "jstradergui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    JSTraderGUI w;
    w.show();
    return a.exec();
}
