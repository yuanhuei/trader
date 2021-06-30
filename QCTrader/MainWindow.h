#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

private:
	Ui::MainWindow ui;

public slots:
	void menu_ctp_connect();
	void menu_exit();
};
