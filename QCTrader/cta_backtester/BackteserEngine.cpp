#include "BackteserEngine.h"
#include"utility.h"

#include"json11.hpp"
#include"utils.hpp"
#include"MongoCxx.h"
#include"../include/libmongoc-1.0/mongoc.h"
#include"../include/libbson-1.0/bson.h"
#include"../event_engine/eventengine.h"

BacktesterEngine::BacktesterEngine(EventEngine* eventengine)
{


	//初始化数据库
	mongoc_init();													//1
	m_uri = mongoc_uri_new("mongodb://localhost:27017/");			//2
	// 创建客户端池
	m_pool = mongoc_client_pool_new(m_uri);							//3


	m_eventengine = eventengine;

}

BacktesterEngine::~BacktesterEngine()
{
	mongoc_client_pool_destroy(m_pool);
	mongoc_uri_destroy(m_uri);
	mongoc_cleanup();
}

void BacktesterEngine::LoadHistoryData(std::string collectionstring)
{

}
void BacktesterEngine::processTickEvent(std::shared_ptr<Event>e)
{

}
void BacktesterEngine::processBarEvent(std::shared_ptr<Event>e)
{

}
void BacktesterEngine::CrossLimitOrder(const TickData& data)
{

}
void BacktesterEngine::CrossLimitOrder(const BarData& data)
{

}
void BacktesterEngine::Settlement(std::shared_ptr<Event_Trade>etrade, std::map<std::string, StrategyTemplate*>orderStrategymap)
{

}
void BacktesterEngine::RecordCapital(const TickData& data)
{

}
void BacktesterEngine::RecordCapital(const BarData& data)
{

}
void BacktesterEngine::RecordPNL(const TickData& data)
{

}
void BacktesterEngine::RecordPNL(const BarData& data)
{

}

int BacktesterEngine::StartBacktesting(
	std::string strStrategyName,
	std::string strStrategyClassName,
	std::string strSymbol,
	Interval iInterval,
	std::string starDate,
	std::string	endDate,
	float rate,
	float slippage,
	float contractsize,
	float pricetick,
	float capital)
{
	m_strategyName = strStrategyName;
	m_strategy_class = strStrategyClassName;
	//vt_symbol;
	m_symbol = strSymbol;
	//exchange;
	m_start = starDate;
	m_end = endDate;
	m_rate = rate;
	m_slippage = slippage;
	m_size = contractsize;
	m_pricetick = pricetick;
	m_capital = capital;



}
void BacktesterEngine::runBacktesting()
{

}

void BacktesterEngine::writeCtaLog(std::string msg, std::string gatewayname)
{
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->msg = msg;
	e->gatewayname = gatewayname;
	m_eventengine->Put(e);
}