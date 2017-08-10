#ifndef BACKTESTENGINE_H
#define BACKTESTENGINE_H
#include <qobject.h>
#include <qstring.h>
#include <qstandarditemmodel.h>
#include <atomic>
#include <fstream>
#include "json11.h"
#include "CTAAPI.h"
#include "StrategyTemplate.h"
#include "eventengine.h"
struct VarData
{
	std::map<std::string, std::vector<BarData>>m_strategy_bardata;													//key是策略 Value是K线列
	std::map<std::string, std::map<std::string, std::vector<double>>>m_strategy_varplotrecord_mainchart;				//key是策略Value是一个map，key是变量名，value是值的序列
	std::map<std::string, std::map<std::string, std::vector<bool>>>m_strategy_varplotrecord_bool;					//key是策略Value是一个map，key是变量名，value是值的序列
	std::map<std::string, std::map<std::string, std::vector<std::string>>>m_strategy_varplotrecord_string;			//key是策略Value是一个map，key是变量名，value是值的序列
	std::map<std::string, std::map<std::string, std::vector<double>>>m_strategy_varplotrecord_indicator;				//key是策略Value是一个map，key是变量名，value是值的序列
	std::map<std::string, std::vector<double>>m_strategy_varplotrecord_pnl;											//在backtestengine中记录Bar级别盈亏
};

struct PLOTDATA
{
	std::map<std::string, std::vector<int>>m_capital_datetime;
	std::map<std::string, std::vector<double>>m_holding_and_totalwinning;						//动态加静态盈利
	std::map<std::string, std::vector<double>>m_totalwinning;									//静态净盈利
	std::map<std::string, std::vector<double>>m_Winning;										//平仓总盈利
	std::map<std::string, std::vector<double>>m_Losing;										//平仓总亏损
	std::map<std::string, std::vector<double>>m_drawdown;										//回撤
};

struct UnitResult
{
	double  totalwinning=0;
	double	maxCapital=0;
	double	drawdown=0;
	double	Winning=0;
	double	Losing=0;
	int totalResult=0;

	double holdingwinning=0;//持仓盈亏
	double holdingposition=0;
	double holdingprice=0;
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

class BacktestEngine :public QObject, public CTAAPI
{
	Q_OBJECT
signals :
	void WriteLog(QString msg);
	void addItem(QString strategyname);
	void PlotCurve(PLOTDATA plotdata);
	void PlotVars(VarData vardata);
	void setrangesignal(int max);
	void setvaluesignal(int value);
public:
	BacktestEngine();
	~BacktestEngine();
	//提供给Strategytemplate调用的接口
	std::vector<std::string> sendOrder(std::string symbol, std::string orderType, double price, double volume, StrategyTemplate* Strategy);
	void cancelOrder(std::string orderID, std::string gatewayname);
	void writeCtaLog(std::string msg, std::string gatewayname);
	void PutEvent(std::shared_ptr<Event>e);
	std::vector<TickData> loadTick(std::string tickDbName, std::string symbol, int days);
	std::vector<BarData> loadBar(std::string BarDbName, std::string symbol, int days);
	//提供给界面的接口
	void loadstrategy();											//加载策略
	void setStartDate(time_t datetime);
	void setStopDate(time_t datetime);
	void setMode(std::string mode);
	void GetTableViewData(QStandardItemModel &m_Model);
	void savetraderecord(std::string strategyname, std::shared_ptr<Event_Trade>etrade);
	std::string getparam(std::string strategyname, std::string param);
private:

	//progressbar value
	int m_progressbarvalue;

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
	PLOTDATA m_plot_data;													std::mutex m_plotmtx;//绘图数据
	//回测时间
	time_t startDatetime;
	time_t endDatetime;
	//回测函数
	void LoadHistoryData(std::string collectionstring);				//读取回测用的历史数据
	void processTickEvent(std::shared_ptr<Event>e);					//处理tick事件
	void processBarEvent(std::shared_ptr<Event>e);					//处理bar事件
	void CrossLimitOrder(const TickData&data);
	void CrossLimitOrder(const BarData&data);
	void Settlement(std::shared_ptr<Event_Trade>etrade, std::map<std::string, StrategyTemplate*>orderStrategymap);
	void RecordCapital(const TickData &data);
	void RecordCapital(const BarData &data);
	void RecordPNL(const TickData &data);
	void RecordPNL(const BarData &data);


	//回测神器_(:з」∠)_
	std::mutex m_varplotrecordmtx;
	void VarPlotRecord(StrategyTemplate* strategy_ptr,const BarData& bardata);
	VarData m_Vardata;

	//事件驱动
	EventEngine *m_eventengine;
	void RegEvent();

	//MongoDB
	mongoc_uri_t         *m_uri;

	//字符串转换
	QString str2qstr(const std::string str);
	//时间转换
	std::string time_t2str(time_t datetime);
	private slots:
	//
	void Runbacktest();
	void Stopbacktest();
};
#endif