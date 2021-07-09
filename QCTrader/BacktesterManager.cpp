#include "BacktesterManager.h"
#include"utility.h"
#include"cta_backtester/BackteserEngine.h"
#include"json11.hpp"
#include"utils.hpp"
#include"MongoCxx.h"
#include"../include/libmongoc-1.0/mongoc.h"
#include"../include/libbson-1.0/bson.h"
#include <qvalidator.h>
#include"MainWindow.h"

BacktesterManager::BacktesterManager(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::CustomizeWindowHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowMaximizeButtonHint);
	setWindowFlags(Qt::Window);

	ui.setupUi(this);
	m_mainwindow = (MainWindow*)parent;
	m_backtesterEngine = m_mainwindow->m_backtesterEngine;
	

}

BacktesterManager::~BacktesterManager()
{

}

void BacktesterManager::InitUI()
{
	//设置QCustomplot组件
	QVector<double>x(101), y(101);
	for (int i = 0; i < 101; ++i)
	{
		x[i] = i / 50.0 - 1;
		y[i] = x[i] * x[i];
	}
	ui.widget->setWindowTitle("acout");
	ui.widget->windowTitle();
	ui.widget->addGraph();
	ui.widget->graph(0)->setData(x, y);
	ui.widget->xAxis->setLabel("x");
	ui.widget->yAxis->setLabel("y");
	ui.widget->xAxis->setRange(-1, 1);
	ui.widget->yAxis->setRange(0, 1);
	ui.widget->replot();
	ui.widget->show();


	ui.widget->plotLayout()->insertRow(0);
	ui.widget->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget, str2qstr_new("账户净值")));

	ui.widget_2->plotLayout()->insertRow(0);
	ui.widget_2->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_2, str2qstr_new("净值回撤")));

	ui.widget_3->plotLayout()->insertRow(0);
	ui.widget_3->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_3, str2qstr_new("每日盈亏")));

	ui.widget_4->plotLayout()->insertRow(0);
	ui.widget_4->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_4, str2qstr_new("盈亏分布")));

	//读取策略名和配置
	std::string strName = "./Strategy/cta_strategy_setting.json_backtester";
	m_ctaStrategyMap=ReadStrategyConfFileJson(strName, m_backtesterEngine);

	std::map<std::string, std::map<std::string, float>>::iterator iter;
	for (iter = m_ctaStrategyMap.begin(); iter != m_ctaStrategyMap.end(); iter++)
	{
		QString strStrategyName = QString::fromStdString(iter->first).section("_", 0, 0);
		ui.comboBox->addItem(strStrategyName);

	}



	QValidator* validator = new QIntValidator(0, 9999, this);
	ui.lineEdit_9->setValidator(validator);
	ui.lineEdit_10->setValidator(validator);
	ui.lineEdit_12->setValidator(validator);
	ui.lineEdit_13->setValidator(validator);
	ui.lineEdit_14->setValidator(validator);
}


void BacktesterManager::startBacktest_clicked()
{
	std::string strStrategyName = ui.comboBox->currentText().toStdString();
	std::string strSymbol = ui.lineEdit_2->text().toStdString();
	QString strStategyClassName;

	std::map<std::string, std::map<std::string, float>>::iterator iter;
	for (iter = m_ctaStrategyMap.begin(); iter != m_ctaStrategyMap.end(); iter++)
	{
		if(iter->first.find(strStrategyName)!=iter->first.npos)
			 strStategyClassName = QString::fromStdString(iter->first).section("_", 1, 1);

	}


	Interval iInterval = Interval(ui.comboBox_2->currentIndex());
	std::string starDate, endDate;
	float rate = ui.lineEdit_10->text().toFloat();//费率
	float slippage = ui.lineEdit_10->text().toFloat();//交易滑点
	float contractsize = ui.lineEdit_10->text().toFloat();//合约乘数
	float pricetick = ui.lineEdit_10->text().toFloat();//价格跳动
	float capital = ui.lineEdit_10->text().toFloat();//资金

	int iResult=m_backtesterEngine->StartBacktesting(strStrategyName, strStategyClassName.toStdString(), strSymbol, iInterval,
		starDate, endDate, rate, slippage, contractsize, pricetick, capital);

}
