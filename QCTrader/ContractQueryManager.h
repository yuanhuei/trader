#pragma once

#include <QWidget>
#include "ui_ContractQueryManager.h"
class MainWindow;
class ContractQueryManager : public QWidget
{
	Q_OBJECT

public:
	ContractQueryManager(QWidget *parent = Q_NULLPTR);
	~ContractQueryManager();
private slots:
	void ContractQueryManager::Query_clicked();


private:
	Ui::ContractQueryManager ui;
	MainWindow* m_mainwindow;
};
