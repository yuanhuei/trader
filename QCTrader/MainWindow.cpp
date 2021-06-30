#include "MainWindow.h"
#include"CTPConnectWidgets.h"
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);//设置窗口最大化
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