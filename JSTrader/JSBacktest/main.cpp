#include "jsbacktest.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	JSBacktest w;
	w.show();
	return a.exec();
}
