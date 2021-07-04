#include "CTAStrategyManager.h"

#include <string>  
#include <json/json.h>
#include <iostream>  
#include <fstream>  
#include"utility.h"
#include<qnamespace.h>

CTAStrategyManager::CTAStrategyManager(QWidget *parent)
	: QWidget(parent, Qt::WindowMinMaxButtonsHint)
{
	//setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	//setWindowModality(Qt::NonModal);
	setWindowFlags(Qt::Window);
	ui.setupUi(this);
	m_mainwindow = (MainWindow*)parent;
	setUI();

}

CTAStrategyManager::~CTAStrategyManager()
{
}

void CTAStrategyManager::ReadStrategyConfFileJson()
{
	Json::Reader reader;
	Json::Value root;

	//从文件中读取，保证当前文件有demo.json文件  
	std::ifstream in("./Strategy/cta_strategy_setting.json",std::ios::binary);

	if (!in.is_open())
	{
		this->UpdateLogTable("打开策略配置文件失败");
		return;
	}

	if (reader.parse(in, root))
	{
		this->UpdateLogTable("打开策略配置文件成功");
		for (int i = 0; i < root.size(); i++)
		{
			//读取策略名称和合约名称
			
			std::string name = root[i]["strategy_name"].asString();
			std::string vt_symbol = root[i]["vt_symbol"].asString();

			//读取策略配置信息 
			std::map<std::string, float> settingMap;
			Json::Value::Members members;
			members = root[i]["setting"].getMemberNames();
			//std::vector<std::string> settingKeys= root["setting"].getMemberNames();
			for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // 遍历每个key
			{
				std::string strKey = *iterMember;
				float fValue= root[i]["setting"][strKey.c_str()].asFloat();
				/*
				if (root[i]["setting"][strKey.c_str()].isString())
				{
					fValue = root[i]["setting"][strKey.c_str()].asString();
				}
				else
					fValue = root[i]["setting"][strKey.c_str()].asFloat();
				*/
				//if(fValue.ist)
				settingMap.insert({ strKey,  fValue});

			}
			//插入到策略配置map中
			m_strategyConfigInfo_map.insert({ name +"__"+vt_symbol,settingMap});
		}

	}
	else
	{
		this->UpdateLogTable("解析策略配置文件失败");
	}

	in.close();
}



void CTAStrategyManager::setUI()
{
	//ui.comboBox->addItem();
	ReadStrategyConfFileJson();
	std::map<std::string, std::map<std::string, float>>::iterator it;
	for (it = m_strategyConfigInfo_map.begin(); it != m_strategyConfigInfo_map.end(); it++)
	{
		ui.comboBox->addItem(QString::fromStdString(it->first));
	}

	//设置策略配置表
	m_StrategyConf = new QStandardItemModel;
	QStringList StrategyConfHeader;
	StrategyConfHeader << str2qstr_new("策略名") << str2qstr_new("合约") << str2qstr_new("状态");
	m_StrategyConf->setHorizontalHeaderLabels(StrategyConfHeader);
	ui.tableView->setModel(m_StrategyConf);
	ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	ui.tableView->setAlternatingRowColors(true);


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

void CTAStrategyManager::initStrategy_clicked()
{
	int row = ui.tableView->currentIndex().row();
	QModelIndex index = m_StrategyConf->index(row, 0);
	std::string strStrategyName = m_StrategyConf->data(index).toString().toStdString();
	index = m_StrategyConf->index(row, 1);
	std::string strSymbolName = m_StrategyConf->data(index).toString().toStdString();

}

void CTAStrategyManager::StrategyTable_clicked()
{
	//int m_StrategyTableRowSelected = ui.tableView->currentIndex().row();

}
void CTAStrategyManager::startStrategy_cliced()
{


}
void CTAStrategyManager::stopStragegy_clicked()
{

}
void CTAStrategyManager::startAllStrategy_clicked()
{

}
void CTAStrategyManager::stopAllStrategy_clicked() {

}
void CTAStrategyManager::clearLog() {

}

//更新日志窗口
void CTAStrategyManager::UpdateLogTable(std::string msg)
{
	LogData data;
	data.gatewayname = "CTP";
	data.msg = msg;
	data.logTime= Utils::getCurrentSystemTime();
	int rowCount = ui.tableWidget_3->rowCount();
	ui.tableWidget_3->insertRow(rowCount);
	ui.tableWidget_3->setItem(rowCount, 0, new QTableWidgetItem(str2qstr_new(data.logTime)));
	ui.tableWidget_3->setItem(rowCount, 1, new QTableWidgetItem(str2qstr_new(data.msg)));
	ui.tableWidget_3->setItem(rowCount, 2, new QTableWidgetItem(str2qstr_new(data.gatewayname)));
}