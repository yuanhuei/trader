#ifndef PORTFOLIO_H
#define PORTFOLIO_H
#include<map>
#include<mutex>

#include"cta_strategy/StrategyTemplate.h"
#include"gateway/gatewaymanager.h"
#include"event_engine/eventengine.h"
#include"./cta_backtester/BackteserEngine.h"
class StrategyTemplate;
/*
class TradingResult
{
public:
	TradingResult(double entryPrice, std::string entryDt, double exitPrice, std::string exitDt, double volume, double size);
	double m_entryPrice;
	double m_exitPrice;
	std::string m_entryDt;
	std::string m_exitDt;
	double m_volume;
	double m_pnl;
};

struct UnitResult
{
	double  totalwinning;
	double  lastdayTotalwinning;
	double  delta;
	double	maxCapital;
	double	drawdown;
	double	Winning;
	double	Losing;
	int totalResult;

	double holdingwinning;//持仓盈亏
	double holdingposition;
	double holdingprice;
};

*/

typedef std::map<std::string, UnitResult> Result;//key是合约 value是一个结果单位
class Portfolio
{
public:
	Portfolio(EventEngine *eventengine, Gatewaymanager *gatewaymanager);
	void calculate();																											//计算硬盘平仓盈亏等信息
	void calculate_memory(std::shared_ptr<Event_Trade>etrade, std::map<std::string, StrategyTemplate*>orderStrategymap);		//计算实时平仓盈亏
	void updateportfolio(std::shared_ptr<Event>e);																				//更新组合到界面，同时计算持仓盈亏
	void recordNetValue(std::shared_ptr<Event>e);	
	void writelog(std::string log);																								//记录净值
private:
	bool netValueRecorded;
	std::mutex m_portfoliomtx;
	bool is_read;
	std::map<std::string, std::map<std::string, std::vector<Event_Trade>>>m_tradelist;							//硬盘盈亏
	std::map<std::string, std::map<std::string, std::vector<Event_Trade>>>m_tradelist_memory;					std::mutex m_tradelistmtx;//实时
	std::map<std::string, Result>m_result;//计算结果，key是策略，value是map ， key是合约 value是结果
	//key是策略名，value是map
	//第二层map key 是 合约名 value是vector
	//vector存TradeEvent

	EventEngine *m_eventengine;
	Gatewaymanager *m_gatewaymanager;
};
#endif