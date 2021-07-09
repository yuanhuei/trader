#include "CTAStrategyManager.h"

#include <string>  
#include <json/json.h>
#include <iostream>  
#include <fstream>  
#include"utility.h"
#include<qnamespace.h>
#include"./cta_strategy/StrategyTemplate.h"
#include"./cta_strategy/CtaEngine.h"

CTAStrategyManager::CTAStrategyManager(QWidget* parent)
	: QWidget(parent, Qt::WindowMinMaxButtonsHint)
{
	//setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	//setWindowModality(Qt::NonModal);
	setWindowFlags(Qt::Window);

	//setWindowFlags(Qt::CustomizeWindowHint |
	//	Qt::WindowMinimizeButtonHint |
	//	Qt::WindowMaximizeButtonHint);

	ui.setupUi(this);
	m_mainwindow = (MainWindow*)parent;
	m_ctaEngine = m_mainwindow->m_ctaEngine;
	InitUI();

}

CTAStrategyManager::~CTAStrategyManager()
{
}


void CTAStrategyManager::InitUI()
{
	//读取策略配置文件
	//ReadStrategyConfFileJson();


	
	//设置策略配置表
	m_StrategyConf = new QStandardItemModel;
	QStringList StrategyConfHeader;
	StrategyConfHeader << str2qstr_new("策略名") << str2qstr_new("合约") << str2qstr_new("策略类名")<< str2qstr_new("状态");
	m_StrategyConf->setHorizontalHeaderLabels(StrategyConfHeader);
	ui.tableView->setModel(m_StrategyConf);
	ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	//ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	ui.tableView->setAlternatingRowColors(true);

	//根据读取的策略文件内容，填入策略表
	std::map<std::string, std::map<std::string, float>>::iterator it;
	int i = 0;
	for (it = m_ctaEngine->m_strategyConfigInfo_map.begin(); it != m_ctaEngine->m_strategyConfigInfo_map.end(); it++)
	{
		QString strStrategy = QString::fromStdString(it->first).section("_",0,0);
		QString strSymbol = QString::fromStdString(it->first).section("_", 1, 1);
		QString strClassname = QString::fromStdString(it->first).section("_", 2, 2);
		
		m_StrategyConf->setItem(i, 0, new QStandardItem(strStrategy));
		m_StrategyConf->setItem(i, 1, new QStandardItem(strSymbol));
		m_StrategyConf->setItem(i, 2, new QStandardItem(strClassname));
		m_StrategyConf->setItem(i, 3, new QStandardItem(str2qstr_new("未初始化")));
		i++;


		ui.comboBox->addItem(strStrategy+" "+ strSymbol);

	}
	ui.pushButton->setDisabled(true);//暂时不用

}
void CTAStrategyManager::addStrategy_clicked()
{
	QString strCurrentStrategy = ui.comboBox->currentText().section("__", 0, 0);
	QString strCurrentSymbol = ui.comboBox->currentText().section("__", 1, 1);

	for (int i = 0; i < m_StrategyConf->rowCount(); i++)
	{
		QString str = m_StrategyConf->item(i, 0)->text() + "__" + m_StrategyConf->item(i, 1)->text();
		if (str == ui.comboBox->currentText())//判断策略已经加入，就返回
			return;
		
	}
	int i = m_StrategyConf->rowCount();
	m_StrategyConf->setItem(i, 0, new QStandardItem(strCurrentStrategy));
	m_StrategyConf->setItem(i, 1, new QStandardItem(strCurrentSymbol));
	m_StrategyConf->setItem(i, 2, new QStandardItem(str2qstr_new("未初始化")));


}

//策略初始化
void CTAStrategyManager::initStrategy_clicked()
{
	//获得在策略表中 当前选取的策略和合约
	int row = ui.tableView->currentIndex().row();
	QModelIndex index = m_StrategyConf->index(row, 0);
	std::string strStrategyName = m_StrategyConf->data(index).toString().toStdString();
	index = m_StrategyConf->index(row, 1);
	std::string strSymbolName = m_StrategyConf->data(index).toString().toStdString();
	index = m_StrategyConf->index(row, 2);
	std::string strClassName = m_StrategyConf->data(index).toString().toStdString();

	m_ctaEngine->initStrategy(strStrategyName + "__" + strSymbolName+"__"+ strClassName);//策略名加合约名加策略类名确定一个策略实例对象

}

void CTAStrategyManager::StrategyTable_clicked()
{
	//int m_StrategyTableRowSelected = ui.tableView->currentIndex().row();
	//获得在策略表中 当前选取的策略和合约

}
//策略启动
void CTAStrategyManager::startStrategy_cliced()
{
	//获得在策略表中 当前选取的策略和合约
	int row = ui.tableView->currentIndex().row();
	QModelIndex index = m_StrategyConf->index(row, 0);
	std::string strStrategyName = m_StrategyConf->data(index).toString().toStdString();
	index = m_StrategyConf->index(row, 1);
	std::string strSymbolName = m_StrategyConf->data(index).toString().toStdString();
	index = m_StrategyConf->index(row, 2);
	std::string strClassName = m_StrategyConf->data(index).toString().toStdString();

	m_ctaEngine->startStrategy(strStrategyName + "__" + strSymbolName + "__" + strClassName);

}
//策略停止
void CTAStrategyManager::stopStragegy_clicked()
{
	//获得在策略表中 当前选取的策略和合约
	int row = ui.tableView->currentIndex().row();
	QModelIndex index = m_StrategyConf->index(row, 0);
	std::string strStrategyName = m_StrategyConf->data(index).toString().toStdString();
	index = m_StrategyConf->index(row, 1);
	std::string strSymbolName = m_StrategyConf->data(index).toString().toStdString();
	index = m_StrategyConf->index(row, 2);
	std::string strClassName = m_StrategyConf->data(index).toString().toStdString();

	m_ctaEngine->stopStrategy(strStrategyName + "__" + strSymbolName + "__" + strClassName);
}
//启动所有策略
void CTAStrategyManager::startAllStrategy_clicked()
{
	m_ctaEngine->startallStrategy();
}
//停止所有策略
void CTAStrategyManager::stopAllStrategy_clicked() 
{
	m_ctaEngine->stopallStrategy();
}
//清理日志窗口
void CTAStrategyManager::clearLog()
{
	ui.tableWidget_3->clear();
}

//更新日志窗口
void CTAStrategyManager::UpdateLogTable(LogData data)
{
	//LogData data;
	//data.gatewayname = "CTP";
	//data.msg = msg;
	//data.logTime= Utils::getCurrentSystemTime();
	
	int rowCount = ui.tableWidget_3->rowCount();
	ui.tableWidget_3->insertRow(rowCount);
	ui.tableWidget_3->setItem(rowCount, 0, new QTableWidgetItem(str2qstr_new(data.logTime)));
	ui.tableWidget_3->setItem(rowCount, 1, new QTableWidgetItem(str2qstr_new(data.msg)));
	ui.tableWidget_3->setItem(rowCount, 2, new QTableWidgetItem(str2qstr_new(data.gatewayname)));
}

void CTAStrategyManager::pushLogData(std::string msg)
{
	
	LogData data;
	data.gatewayname = "CTP";
	data.msg = msg;
	data.logTime= Utils::getCurrentSystemTime();
	this->UpdateLogTable(data);

}