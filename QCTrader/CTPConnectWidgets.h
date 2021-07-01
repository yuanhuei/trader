#pragma once

#include <QDialog>
#include "ui_CTPConnectWidgets.h"
#include"MainWindow.h"

class CTPGateway;

class CTPConnectWidgets : public QDialog
{
	Q_OBJECT

public:
	CTPConnectWidgets(QWidget *parent = Q_NULLPTR);
	~CTPConnectWidgets();
private slots:
	void buttonOk_clicked();
	void buttonCancel_clicked();
	

private:
	Ui::CTPConnectWidgets ui;
	MainWindow* m_mainwindow;
	CTPGateway* m_ctpgateway;
};
