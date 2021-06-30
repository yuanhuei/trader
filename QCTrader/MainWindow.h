#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
//#include"event_engine/eventengine.h"
//#include"gateway/gatewaymanager.h"
#include<qstandarditemmodel.h>

class EventEngine;
class Gatewaymanager;

struct UpdatePriceTableData
{
	std::string symbol;
	//成交数据
	double lastprice;//最新成交价
	double openInterest;//持仓量
	std::string date;//日期
	std::string time;//时间
	double upperLimit;//涨停
	double lowerLimit;//跌停
	double bidprice1;
	double askprice1;
};

struct UpdateStrategyData
{
	std::string strategyname;
	std::map<std::string, std::string>parammap;
	std::map<std::string, std::string>varmap;
};

struct LoadStrategyData
{
	std::string strategyname;
	std::map<std::string, std::string>parammap;
	std::map<std::string, std::string>varmap;
};

struct PositionData
{
	std::string symbol;
	std::string direction;
	std::string gatewayname;
	double position;
	double todayPosition;
	double ydPosition;
	double todayPositionCost;
	double ydPositionCost;
	double price;
	double frozen;
};

struct AccountData
{
	std::string gatewayname;
	std::string accountid;
	double preBalance;//昨日账户结算净值
	double balance;//账户净值
	double available;//可用资金
	double commission;//今日手续费
	double margin;//保证金占用
	double closeProfit;//平仓盈亏
	double positionProfit;//持仓盈亏
};

struct PortfolioData
{
	std::string dllname;
	std::string strategyname;
	std::string symbol;
	Portfolio_Result_Data Portfoliodata;
	std::vector<int>strategyrows;
};


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

	void LoadEngine();
	void RegEvent();
	void OnLogUpdate(std::shared_ptr<Event>e);
	void onAccountUpdate(std::shared_ptr<Event>e);
	void onPositionUpdate(std::shared_ptr<Event>e);
	void onPriceTableUpdate(std::shared_ptr<Event>e);
	void ConnectSignalAndSlot();



signals:
	void WriteLog(QString msg);
	//void LoadStrategySignal(LoadStrategyData data);
	//void UpdateStrategySignal(UpdateStrategyData data);
	void UpdateAccountSignal(AccountData data);
	void UpdatePositionSignal(PositionData  data);
	//void UpdatePortfolioSignal(PortfolioData data);
	void UpdatePriceTableSignal(UpdatePriceTableData data);
	//void UpdatePortfolioWinning(QString winning);

private:
	Ui::MainWindow ui;

private slots:
	void menu_ctp_connect();
	void menu_exit();
	void UpdateAccountBox(AccountData data);
	void UpdatePositionBox(PositionData data);
	void UpdateLogTable(QString str);

public:
	//各种引擎管理器指针
	EventEngine* m_eventengine;//事件驱动引擎
	Gatewaymanager* m_gatewaymanager;//接口管理器
	//riskmanager* m_riskmanager;//风险管理器
	//CTAmanager* m_ctamanager;//cta管理器


    //model
	QStandardItemModel* m_AccountModel;
	QStandardItemModel* m_PositionModel;
};
