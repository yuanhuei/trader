#pragma once

#include <QWidget>
#include "ui_BacktesterManager.h"

class BacktesterManager : public QWidget
{
	Q_OBJECT

public:
	BacktesterManager(QWidget *parent = Q_NULLPTR);
	~BacktesterManager();

private:
	Ui::BacktesterManager ui;
};
