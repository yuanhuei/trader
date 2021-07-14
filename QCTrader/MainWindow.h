#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
//#include"event_engine/eventengine.h"
//#include"gateway/gatewaymanager.h"
#include<qstandarditemmodel.h>
#include"qcstructs.h"

class EventEngine;
class Gatewaymanager;
class riskmanager;
class CtaEngine;
class CTAStrategyManager;
class BacktesterManager;
class BacktesterEngine;
class ContractQueryManager;

#pragma pack(1)
struct UpdatePriceTableData
{
	std::string symbol;
	std::string exchange;
	std::string gatewayname;
	std::string date;//日期
	std::string time;//时间

	//成交数据
	double lastprice;//最新成交价
	double openInterest;//持仓量

	double upperLimit;//涨停
	double lowerLimit;//跌停
	double bidprice1;
	double askprice1;
};
#pragma pack()
struct UpdateOrderTableData
{

	//编号相关
	std::string symbol;
	std::string exchange;
	std::string orderID;//订单编号
	std::string gatewayname;
	//报单相关
	std::string direction;//方向
	std::string offset;//开平方向
	double price; //报单价格
	double totalVolume;//报单总量
	double tradedVolume;//成交数量
	std::string status;//报单状态

	std::string orderTime;//发单时间
	std::string cancelTime;//撤单时间

	int frontID;//前置机编号
};

struct UpdateTradeTableData
{
	//代码编号
	std::string symbol;
	std::string exchange;
	std::string tradeID;   //交易编号
	std::string orderID;  //订单编号
	std::string gatewayname;
	//成交相关
	std::string direction;//方向
	std::string offset; //成交开平仓
	double price;//成交价格
	double volume;//成交量
	std::string tradeTime;//成交时间
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

struct LogData
{
	std::string msg;//log信息
	std::string gatewayname; //接口名
	std::string logTime;//时间

};


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

	void setUI();
	void LoadEngine();
	void RegEvent();

	//事件回调函数
	void OnLogUpdate(std::shared_ptr<Event>e);
	void onAccountUpdate(std::shared_ptr<Event>e);
	void onPositionUpdate(std::shared_ptr<Event>e);
	void onPriceTableUpdate(std::shared_ptr<Event>e);
	void onOrderTableUpdate(std::shared_ptr<Event>e);
	void onTradeTableUpdate(std::shared_ptr<Event>e);

	//连接信号和槽
	void ConnectSignalAndSlot();
	//日志记录
	void write_log(std::string msg, std::string gateway_name);

	void UpdateSymbolBox(UpdatePriceTableData data);

signals:
	void UpdateLogSignal(LogData data);
	//void LoadStrategySignal(LoadStrategyData data);
	//void UpdateStrategySignal(UpdateStrategyData data);
	void UpdateAccountSignal(AccountData data);
	void UpdatePositionSignal(PositionData  data);
	//void UpdatePortfolioSignal(PortfolioData data);
	void UpdatePriceTableSignal(UpdatePriceTableData data);
	void UpdateOrderTableSignal(UpdateOrderTableData data);
	void UpdateTradeTableSignal(UpdateTradeTableData data);
	//void UpdatePortfolioWinning(QString winning);

private:
	Ui::MainWindow ui;

private slots:
	void menu_ctp_connect();
	void menu_exit();
	void menu_CTAStrategy();
	void menu_CTABacktest();
	void menu_contractQueryclicked();

	void symbol_ReturnPressed();
	void SendOrder_clicked();


	void UpdateAccountBox(AccountData data);
	void UpdatePositionBox(PositionData data);
	void UpdateLogTable(LogData data);
	void UpdateTickTable(UpdatePriceTableData data);
	void UpdateOrderTable(UpdateOrderTableData data);
	void UpdateTradeTable(UpdateTradeTableData data);

public:
	//各种引擎管理器指针
	EventEngine* m_eventengine;//事件驱动引擎
	Gatewaymanager* m_gatewaymanager;//接口管理器
	riskmanager* m_riskmanager;//风险管理器

	CtaEngine* m_ctaEngine;//cta管理器
	CTAStrategyManager* m_ctaStrategyDailog=NULL;
	BacktesterEngine* m_backtesterEngine = NULL;
	BacktesterManager* m_ctaBacktesterManager=NULL;
	ContractQueryManager* m_ContractQueryManager= NULL;



    //model
	QStandardItemModel* m_AccountModel;
	QStandardItemModel* m_PositionModel;
	QStandardItemModel* m_SymbolSubscribedTableModel;
	QStandardItemModel* m_OrderSubmitTableModel;
	QStandardItemModel* m_TradeSubmitTableModel;
};
