#pragma once
#include "CTAAPI.h"
#include<string>
#include<map>
#include"qcstructs.h"

class StrategyTemplate;
class EventEngine;
struct UnitResult
{
	double  totalwinning = 0;
	double	maxCapital = 0;
	double	drawdown = 0;
	double	Winning = 0;
	double	Losing = 0;
	int totalResult = 0;

	double holdingwinning = 0;//持仓盈亏
	double holdingposition = 0;
	double holdingprice = 0;
};

class TradingResult
{
public:
	TradingResult(double entryPrice, std::string entryDt, double exitPrice, std::string exitDt, double volume, double rate, double slippage, double size);
	double m_entryPrice;  //开仓价格
	double m_exitPrice;   // 平仓价格
	std::string m_entryDt;   // 开仓时间datetime
	std::string  m_exitDt;  // 平仓时间
	double	m_volume;//交易数量（ + / -代表方向）
	double	m_pnl;  //净盈亏
	double m_turnover;
	double m_commission;
	double m_slippage;
};

typedef std::map<std::string, UnitResult> Result;//key是合约 value是一个结果单位

class BacktesterEngine :
    public CTAAPI
{
public:
	BacktesterEngine(EventEngine* eventengine)j;
	~BacktesterEngine();


	std::string vt_symbol;
	std::string		m_symbol;
	std::string  exchange;
	std::string m_start;
	std::string	m_end;
	float	m_rate = 0;
	float	m_slippage = 0;
	float	m_size = 1;
	float m_pricetick = 0;
	float m_capital = 1_000_000;
	//floatself.risk_free: float = 0.02;
	//mode = BacktestingMode.BAR;
	//self.inverse = False

	std::string m_strategy_class;
	std::string m_strategyName;
	TickData m_tick;
	BarData m_bar;
	//self.datetime = None

	Interval m_iInterval;
	int m_days = 0;
	//self.callback = None
	//self.history_data = []

	int m_stop_order_count = 0;
	//stop_orders = {}
	//active_stop_orders = {}

	int m_limit_order_count = 0;
	//self.limit_orders = {}
	//self.active_limit_orders = {}

	int m_trade_count = 0;
	//trades = {}

	//logs = []

	//daily_results = {}
	//daily_df = None


	//提供给Strategytemplate调用的接口
	std::vector<std::string> sendOrder(std::string symbol, std::string orderType, double price, double volume, StrategyTemplate* Strategy);
	void cancelOrder(std::string orderID, std::string gatewayname);
	void writeCtaLog(std::string msg, std::string gatewayname);
	void PutEvent(std::shared_ptr<Event>e);
	std::vector<TickData> loadTick(std::string tickDbName, std::string symbol, int days);
	std::vector<BarData> loadBar(std::string BarDbName, std::string symbol, int days);

	EventEngine* m_eventengine;
	void InitUI();

	//MONGOC 线程池
	mongoc_uri_t* m_uri;
	mongoc_client_pool_t* m_pool;

	//回测函数
	void LoadHistoryData(std::string collectionstring);				//读取回测用的历史数据
	void processTickEvent(std::shared_ptr<Event>e);					//处理tick事件
	void processBarEvent(std::shared_ptr<Event>e);					//处理bar事件
	void CrossLimitOrder(const TickData& data);
	void CrossLimitOrder(const BarData& data);
	void Settlement(std::shared_ptr<Event_Trade>etrade, std::map<std::string, StrategyTemplate*>orderStrategymap);
	void RecordCapital(const TickData& data);
	void RecordCapital(const BarData& data);
	void RecordPNL(const TickData& data);
	void RecordPNL(const BarData& data);

	int StartBacktesting(
		std::string strStrategyName ,
		std::string strStrategyClassName,
		std::string strSymbol ,
		Interval iInterval ,
		std::string starDate,
		std::string	endDate,
		float rate ,
		float slippage ,
		float contractsize ,
		float pricetick ,
		float capital);
	void runBacktesting();


	void writeCtaLog(std::string msg, std::string gatewayname);

};

