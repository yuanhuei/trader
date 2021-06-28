#pragma once

#include <QDialog>
#include "ui_CTPConnectWidgets.h"

class CTPConnectWidgets : public QDialog
{
	Q_OBJECT

public:
	CTPConnectWidgets(QWidget *parent = Q_NULLPTR);
	~CTPConnectWidgets();
	void SendOrderbuttonclicked();
	void CancelOrderbutttonclicded();
	

private:
	Ui::CTPConnectWidgets ui;
};
