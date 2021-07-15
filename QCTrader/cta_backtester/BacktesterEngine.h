#pragma once
#include "CTAAPI.h"
#include<string>
#include<map>
#include<vector>
#include<set>
#include"qcstructs.h"
#include<QDateTime>
#include "../cta_strategy/StrategyTemplate.h"

class StrategyTemplate;
class EventEngine;


class DailyTradingResult
{
public:
	DailyTradingResult(QDate date, double price);
	~DailyTradingResult();

	void add_trade(Event_Trade trade);
	void calculate_pnl(float pre_close, float start_pos, int size, float rate, float slippage);


	QDate date = date;
	double m_close_price;//今天收盘价
	double	m_pre_close = 0;//前一天的收盘价

	 std::vector<Event_Trade> m_trades;
	int m_trade_count = 0;

	int m_start_pos = 0;
	int m_end_pos = 0;

	double m_turnover = 0;
	double m_commission = 0;//手续费
	double m_slippage = 0;//滑点

	double	m_trading_pnl = 0;
	double	m_holding_pnl = 0;
	double	m_total_pnl = 0;
	double		m_net_pnl = 0;

	double	m_balance=0;//资金
	double m_highlevel=0;//资金最大值
		
	double  m_return=0;//每日回报
	double 	m_drawdown=0;//每日相对之前最高点的回撤
	double  m_ddpercent=0;//回撤百分比
};


class BacktesterEngine :
    public CTAAPI
{
public:
	BacktesterEngine(EventEngine* eventengine);
	~BacktesterEngine();


	StrategyTemplate* m_strategy=nullptr;
	std::map<std::string, float>  m_settingMap;

	std::string m_strategy_classname;
	std::string m_strategyName;
	std::string vt_symbol;
	std::string	m_symbol;

	std::string  exchange;
	QDate m_startDay;
	QDate	m_endDay;
	float	m_rate = 0;
	float	m_slippage = 0;
	float	m_size = 1;
	float m_pricetick = 0;
	float m_capital = 100000;
	float risk_free= 0.02;
	//mode = BacktestingMode.BAR;
	//self.inverse = False
	std::string m_barDate;



	TickData m_tick;
	BarData m_bar;
	QDateTime m_datetime;

	Interval m_iInterval=MINUTE;
	int m_days = 0;
	//self.callback = None
	// 
	//bar模式还是tick模式
	std::string m_backtestmode = BAR_MODE;								

	int m_stop_order_count = 0;
	//stop_orders = {}
	//active_stop_orders = {}

	int m_limit_order_count = 0;
	//self.limit_orders = {}
	//self.active_limit_orders = {}

	std::map<std::string, std::shared_ptr<Event_StopOrder>>m_stop_orders;				//所有的单，不会删除
	std::map<std::string, std::shared_ptr<Event_StopOrder>>m_active_stop_orders;		   //工作中的单   

	std::map<std::string, std::shared_ptr<Event_Order>>m_limit_orders;				//所有的单，不会删除
	std::map<std::string, std::shared_ptr<Event_Order>>m_active_limit_orders;		//工作中的单   

	//int m_trade_count = 0;
	int  m_tradeCount = 0;							//成交次数

	std::map<std::string, std::shared_ptr<Event_Trade>>m_tradeMap;//成交单					
	//std::map<std::string, Result>m_result;											

	std::vector<BarData> vector_history_data;



	std::map <QDate , std::shared_ptr<DailyTradingResult >> m_daily_resultMap;
	std::map < std::string, std::string> m_result_statistics;

	void calculate_result();
	void calculate_statistics(bool bOutput = false);

	//提供给Strategytemplate调用的接口
	std::vector<std::string> sendOrder(bool bStopOrder,std::string symbol, std::string strDirection, std::string strOffset, double price, double volume, StrategyTemplate* Strategy);
	void cancelOrder(std::string orderID, std::string gatewayname);
	void cancel_stop_order(std::string orderID, std::string gatewayname);
	void cancel_limit_order(std::string orderID, std::string gatewayname);
	void cancelAllOrder();

	void cross_limit_order(const BarData& data);
	void cross_stop_order(const BarData& data);
	std::vector<std::string> send_limit_order(std::string symbol, std::string strDirection, std::string strOffset, 
		double price, double volume, StrategyTemplate* Strategy);
	std::vector<std::string> send_stop_order(std::string symbol, std::string strDirection, std::string strOffset, 
		double price, double volume, StrategyTemplate* Strategy);

	void PutEvent(std::shared_ptr<Event>e);

	std::vector<TickData> loadTickbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<BarData> loadBarbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<TickData> loadTick(std::string symbol, int days);
	std::vector<BarData> loadBar(std::string symbol, int days);

	EventEngine* m_eventengine;
	//void InitUI();


	//回测函数
	std::vector<BarData> LoadHistoryData();				//读取回测用的历史数据
	void update_daily_close(float price);



	void StartBacktesting(
		std::string strStrategyName,
		std::string strStrategyClassName,
		std::string strSymbol,
		Interval iInterval,
		QDate startDay,
		QDate	endDay,
		float rate,
		float slippage,
		float contractsize,
		float pricetick,
		float capital,
		std::map<std::string, float>  ctaStrategyMap);
	void runBacktesting();
	void writeCtaLog(std::string msg);

	//回测的变量
	//int m_limitorderCount=1;						//人工ORDERID
};

