#pragma once

#include <QDialog>
#include "ui_CTPConnectWidgets.h"

class CTPConnectWidgets : public QDialog
{
	Q_OBJECT

public:
	CTPConnectWidgets(QWidget *parent = Q_NULLPTR);
	~CTPConnectWidgets();
public slots:
	void buttonOk_clicked();
	void buttonCancel_clicked();
	

private:
	Ui::CTPConnectWidgets ui;
};
