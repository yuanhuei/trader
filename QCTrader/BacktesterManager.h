#pragma once

#include <QWidget>
#include "ui_BacktesterManager.h"
#include<map>
#include<string>
#include"MainWindow.h"

class BacktesterEngine;
class MainWindow;


class BacktesterManager : public QWidget
{
	Q_OBJECT

public:
	BacktesterManager(QWidget *parent = Q_NULLPTR);
	~BacktesterManager();

	void InitUI();

	BacktesterEngine* m_backtesterEngine;
	std::map<std::string, std::map<std::string, float>>  m_ctaStrategyMap;
	QCPBars* fossil=nullptr;

	MainWindow* m_mainwindow;

	void UpdateLogTable(LogData data);
	void RegisterEvent();
	void ProcecssTesterFisnishedEvent(std::shared_ptr<Event>e);
signals:
	void UpdateTesterResultSignal();


private:
	Ui::BacktesterManager ui;

private slots:
	void startBacktest_clicked();
	void UpdateTesterResult();
};
