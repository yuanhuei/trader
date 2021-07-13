#pragma once

#include <QWidget>
#include "ui_ContractQueryManager.h"

class ContractQueryManager : public QWidget
{
	Q_OBJECT

public:
	ContractQueryManager(QWidget *parent = Q_NULLPTR);
	~ContractQueryManager();

private:
	Ui::ContractQueryManager ui;
};
