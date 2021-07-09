#pragma once
#include "CTAAPI.h"
#include<string>
#include<map>
#include<set>
#include"qcstructs.h"
#include<QDateTime>

class StrategyTemplate;
class EventEngine;
struct UnitResult
{
	double  totalwinning = 0;
	double  lastdayTotalwinning;
	double  delta;
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
	TradingResult(double entryPrice, std::string entryDt, double exitPrice, 
		std::string exitDt, double volume, double rate, double slippage, double size);
	TradingResult(double entryPrice, std::string entryDt, double exitPrice,
		std::string exitDt, double volume,double size);
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
	BacktesterEngine(EventEngine* eventengine);
	~BacktesterEngine();


	StrategyTemplate* m_strategy=nullptr;
	std::map<std::string, float>  m_settingMap;

	std::string m_strategy_classname;
	std::string m_strategyName;
	std::string vt_symbol;
	std::string	m_symbol;

	std::string  exchange;
	QDateTime m_startDay;
	QDateTime	m_endDay;
	float	m_rate = 0;
	float	m_fSlippage = 0;
	float	m_size = 1;
	float m_pricetick = 0;
	float m_capital = 1000000;
	//floatself.risk_free: float = 0.02;
	//mode = BacktestingMode.BAR;
	//self.inverse = False


	TickData m_tick;
	BarData m_bar;
	//self.datetime = None

	Interval m_iInterval=MINUTE;
	int m_days = 0;
	//self.callback = None
	std::vector<BarData> vector_history_data;

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
	//void writeCtaLog(std::string msg, std::string gatewayname);
	void PutEvent(std::shared_ptr<Event>e);
	std::vector<TickData> loadTickbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<BarData> loadBarbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<TickData> loadTick(std::string symbol, int days);
	std::vector<BarData> loadBar(std::string symbol, int days);

	EventEngine* m_eventengine;
	void InitUI();

	//MONGOC 线程池
	//mongoc_uri_t* m_uri;
	//mongoc_client_pool_t* m_pool;

	//回测函数
	std::vector<BarData> LoadHistoryData();				//读取回测用的历史数据
	void processTickEvent(std::shared_ptr<Event>e);					//处理tick事件
	void processBarEvent(std::shared_ptr<Event>e);					//处理bar事件
	void CrossLimitOrder(const TickData& data);
	void CrossLimitOrder(const BarData& data);
	void Settlement(std::shared_ptr<Event_Trade>etrade, std::map<std::string, StrategyTemplate*>orderStrategymap);
	void RecordCapital(const TickData& data);
	void RecordCapital(const BarData& data);
	void RecordPNL(const TickData& data);
	void RecordPNL(const BarData& data);

	void StartBacktesting(
		std::string strStrategyName ,
		std::string strStrategyClassName,
		std::string strSymbol ,
		Interval iInterval ,
		QDateTime starDate,
		QDateTime	endDate,
		float rate ,
		float slippage ,
		float contractsize ,
		float pricetick ,
		float capital,
		std::map<std::string, float>  ctaStrategyMap);

	void runBacktesting();


	void writeCtaLog(std::string msg);
	void writeCtaLog(std::string msg, std::string gatewayname);

	//回测的变量
	std::atomic_int m_limitorderCount;						//人工ORDERID
	std::atomic_int  m_tradeCount;							//成交次数
	std::string m_backtestmode;								//bar模式还是tick模式
	std::atomic<time_t> m_datetime;							//对比时间
	std::atomic_int working_worker; 	std::mutex m_workermutex;	//同步锁
	std::condition_variable m_cv;
	std::map<std::string, double>symbol_size;						//合约乘数，本地硬盘读取	只读不写，不用加锁
	std::map<std::string, double>symbol_rate;						//合约乘数，本地硬盘读取	只读不写，不用加锁
	std::map<std::string, double>m_slippage;
	std::map<std::string, HINSTANCE>dllmap;																//存放策略dll容器		只读不写，不用加锁
	std::map<std::string, std::shared_ptr<Event_Order>>m_Ordermap;			std::mutex m_ordermapmtx;	//所有的单，不会删除
	std::map<std::string, std::shared_ptr<Event_Order>>m_WorkingOrdermap;								//工作中的单   和workingordermap用一个锁
	//key 是OrderID  value 是策略对象 用途是为了保证这个单是这个策略发出去的  成交回报计算持仓正确加载对应的策略上，以防多个策略交易同一个合约出现BUG
	std::map<std::string, StrategyTemplate*>m_orderStrategymap;	            std::mutex m_orderStrategymtx;
	std::map<std::string, StrategyTemplate*>m_strategymap;					std::mutex m_strategymtx;	//存策略名和策略对象指针
	//key 合约名，value 策略指针(实例)vector  用来把不同的合约tick推送到每一个策略对象
	std::map<std::string, std::vector<StrategyTemplate*>>m_tickstrategymap;	std::mutex m_tickstrategymtx;

	std::set<std::string>m_backtestsymbols;									std::mutex	m_backtestsymbolmtx;//要回测的合约列表 用以提取数据
	std::vector<TickData>m_Tickdatavector;									std::mutex m_HistoryDatamtx;//历史数据加锁
	std::vector<BarData>m_Bardatavector;

	std::map<std::string, std::map<std::string, std::vector<Event_Trade>>>m_tradelist_memory;					std::mutex m_tradelistmtx;//清算缓存
	std::map<std::string, Result>m_result;											std::mutex m_resultmtx;			//缓存结果
	//回测时间
	time_t startDatetime;
	time_t endDatetime;

};

