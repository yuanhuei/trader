#include "QCTrader.h"
#include "jstradergui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //JSTraderGUI w;
    //w.show();

    QCTrader t;
    t.show();
    return a.exec();
}
