#include "jstradergui.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QApplication>
#include <QFile>
#include <QApplication>



class CommonHelper
{
public:
	static void setStyle(const QString &style) {
		QFile qss(style);
		qss.open(QFile::ReadOnly);
		qApp->setStyleSheet(qss.readAll());
		qss.close();
	}
};

int main(int argc, char *argv[])
{

	QApplication a(argc, argv);
	CommonHelper::setStyle(":qdarkstyle/style.qss");
	JSTraderGUI w;
	w.show();
	return a.exec();
}
