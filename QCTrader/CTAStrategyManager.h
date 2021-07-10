#pragma once

#include <QWidget>
#include "ui_CTAStrategyManager.h"
#include"mainwindow.h"
class CtaEngine;

class CTAStrategyManager : public QWidget
{
	Q_OBJECT

public:
	CTAStrategyManager(QWidget *parent = Q_NULLPTR);
	~CTAStrategyManager();
public:
	void InitUI();
	//void pushLogToCTAStrategyWindow(std::string msg);
	void UpdateLogTable(LogData data);
	//void pushLogData(std::string msg);

private:
	Ui::CTAStrategyManager ui;
	MainWindow* m_mainwindow;
	//std::map<std::string, std::map<std::string, float>>* m_strategyConfigInfo_map;

	QStandardItemModel* m_StrategyConf;
	CtaEngine* m_ctaEngine;
private slots:
	void addStrategy_clicked();
	void initStrategy_clicked();
	void startStrategy_cliced();
	void stopStragegy_clicked();
	void startAllStrategy_clicked();
	void stopAllStrategy_clicked();
	void clearLog();
	void StrategyTable_clicked();


};
