#include "QCTrader.h"
//#include "jstradergui.h"
#include <QtWidgets/QApplication>
#include"MainWindow.h"





int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //JSTraderGUI w;
    //w.show();


    MainWindow  t;
    t.show();
    return a.exec();
}
