#pragma once

#include <QWidget>
#include "ui_CTAStrategyManager.h"
#include"mainwindow.h"

class CTAStrategyManager : public QWidget
{
	Q_OBJECT

public:
	CTAStrategyManager(QWidget *parent = Q_NULLPTR);
	~CTAStrategyManager();
private:
	void setUI();
	void ReadStrategyConfFileJson();
	//void pushLogToCTAStrategyWindow(std::string msg);
	void UpdateLogTable(std::string msg);

private:
	Ui::CTAStrategyManager ui;
	MainWindow* m_mainwindow;
	std::map<std::string, std::map<std::string, float>> m_strategyConfigInfo_map;

	QStandardItemModel* m_StrategyConf;
private slots:
	void addStrategy_clicked();
	void initStrategy_clicked();
	void startStrategy_cliced();
	void stopStragegy_clicked();
	void startAllStrategy_clicked();
	void stopAllStrategy_clicked();
	void clearLog();


};
