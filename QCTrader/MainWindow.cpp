#include "MainWindow.h"
#include"CTPConnectWidgets.h"
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);//���ô������
}

MainWindow::~MainWindow()
{
}

void MainWindow::menu_exit()
{
	this->close();
}

void MainWindow::menu_ctp_connect()
{
	CTPConnectWidgets* e= new CTPConnectWidgets(this);
	e->show();

}