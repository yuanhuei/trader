#include "BackteserEngine.h"
#include"utility.h"

#include"json11.hpp"
#include"utils.hpp"
#include"MongoCxx.h"
#include"../include/libmongoc-1.0/mongoc.h"
#include"../include/libbson-1.0/bson.h"
#include"../event_engine/eventengine.h"
#include<thread>
#include<functional>
#include <QtCore/qobject.h>
#include<qstring.h>
#include<QDateTime>
#include"../cta_strategy/strategies/BollChannelStrategy.h"

extern mongoc_uri_t* g_uri;
extern mongoc_client_pool_t* g_pool;

BacktesterEngine::BacktesterEngine(EventEngine* eventengine)
{





	m_eventengine = eventengine;

}

BacktesterEngine::~BacktesterEngine()
{
	mongoc_client_pool_destroy(m_pool);
	mongoc_uri_destroy(m_uri);
	mongoc_cleanup();
}

std::vector<BarData> BacktesterEngine::LoadHistoryData()
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

void BacktesterEngine::StartBacktesting(
	std::string strStrategyName,
	std::string strStrategyClassName,
	std::string strSymbol,
	Interval iInterval,
	QDateTime starDate,
	QDateTime	endDate,
	float rate,
	float slippage,
	float contractsize,
	float pricetick,
	float capital,
	 std::map<std::string, float>  ctaStrategyMap)
{
	m_strategyName = strStrategyName;
	m_strategy_classname = strStrategyClassName;
	//vt_symbol;
	m_symbol = strSymbol;
	//exchange;
	m_startDay = starDate;
	m_endDay = endDate;
	m_rate = rate;
	m_slippage = slippage;
	m_size = contractsize;
	m_pricetick = pricetick;
	m_capital = capital;
	m_settingMap = ctaStrategyMap;

	//std::thread tRunBacktest(runBacktesting);
	std::thread  m_thread;
	m_thread =std::thread(std::bind(&BacktesterEngine::runBacktesting, this));


}
void BacktesterEngine::runBacktesting()
{
//生成策略实例

	if (m_strategy_classname == "BollChannelStrategy")
		m_strategy = new BollChannelStrategy(this, m_strategyName, m_symbol);

	//赋值参数给策略中的strategeData,策略中必须生成strategeData,里面包含了配置参数和变量参数，下面是根据配置文件更新。
	for (std::map<std::string, float>::iterator it =m_settingMap.begin(); it != m_settingMap.end(); it++)
	{
		//遍历parameter
		std::string value = std::to_string(it->second);
		m_strategy->updateParam(it->first.c_str(), value.c_str());
	}
	m_strategy->updateSetting();//更新策略实例里面配置参数的值

	//加载数据

	LoadHistoryData();

	//推送数据


	//计算统计结果


	//推送回测完成信号
	this->writeCtaLog("回测完成", "回测模块");
}

void BacktesterEngine::writeCtaLog(std::string msg, std::string gatewayname)
{
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->msg = msg;
	e->gatewayname = gatewayname;
	m_eventengine->Put(e);
}

std::vector<BarData> BacktesterEngine::LoadHistoryData()
{
	vector_history_data = loadBar(m_symbol, m_startDay, m_endDay);
	return vector_history_data;
}	

std::vector<TickData> loadTick(std::string symbol, int days)
{
	//QDateTime startDay
	QDateTime endDay = QDateTime::currentDateTime();
	QDateTime startDay = endDay.addDays(0 - days);
	return loadTick(symbol, startDay, endDay);


}
std::vector<BarData> loadBar(std::string symbol, int days)
{
	//QDateTime startDay
	QDateTime endDay = QDateTime::currentDateTime();
	QDateTime startDay = endDay.addDays(0 - days);
	return loadBar(symbol, startDay, endDay);
}

std::vector<BarData> loadBar(std::string symbol, QDateTime startDay, QDateTime endDay)
{
	std::vector<BarData>datavector;
	if (symbol == " " || symbol == "")
	{
		return datavector;
	}
	const char* databasename = DATABASE_NAME;
	const char* collectionsname = BARCOLLECTION_NAME;
	//auto targetday = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (days * 24 * 3600);//获取当前的系统时间
	auto start = startDay.toTime_t();
	auto end = endDay.toTime_t();

	mongoc_cursor_t* cursor;
	bson_error_t error;
	const bson_t* doc;


	bson_t parent;
	bson_t child;
	mongoc_collection_t* collection;
	bson_init(&parent);
	//查询bson
	BSON_APPEND_UTF8(&parent, "symbol", symbol.c_str());
	BSON_APPEND_DOCUMENT_BEGIN(&parent, "datetime", &child);
	BSON_APPEND_TIME_T(&child, "$gt", start);//$gt大于某个时间，"$lt"小于某个时间
	BSON_APPEND_TIME_T(&child, "$lt", end);//$gt大于某个时间，"$lt"小于某个时间
	bson_append_document_end(&parent, &child);


	char* str = bson_as_json(&parent, NULL);
	//	printf("\n%s\n", str);

	// 从客户端池中获取一个客户端
	mongoc_client_t* client = mongoc_client_pool_pop(g_pool);																		//取一个mongo连接

	collection = mongoc_client_get_collection(client, DATABASE_NAME, symbol.c_str());

	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, &parent, NULL, NULL);

	while (mongoc_cursor_next(cursor, &doc))
	{
		str = bson_as_json(doc, NULL);
		std::string s = str;
		std::string err;


		auto json = json11::Json::parse(s, err);
		if (!err.empty())
		{
			mongoc_cursor_destroy(cursor);
			mongoc_client_pool_push(g_pool, client);																						//放回一个mongo连接
			return datavector;
		}
		BarData bardata;
		bardata.symbol = json["symbol"].string_value();
		bardata.exchange = json["exchange"].string_value();
		bardata.open = json["open"].number_value();
		bardata.high = json["high"].number_value();
		bardata.low = json["low"].number_value();
		bardata.close = json["close"].number_value();
		bardata.volume = json["volume"].number_value();

		json11::Json::object datetime = json["datetime"].object_items();
		bardata.unixdatetime = datetime["$date"].number_value() / 1000;

		bardata.date = json["date"].string_value();
		bardata.time = json["time"].string_value();

		bardata.openPrice = json["openPrice"].number_value();//今日开
		bardata.highPrice = json["highPrice"].number_value();//今日高
		bardata.lowPrice = json["lowPrice"].number_value();//今日低
		bardata.preClosePrice = json["preClosePrice"].number_value();//昨收

		bardata.upperLimit = json["upperLimit"].number_value();//涨停
		bardata.lowerLimit = json["lowerLimit"].number_value();//跌停

		bardata.openInterest = json["openInterest"].number_value();//持仓

		datavector.push_back(bardata);

		//		printf("%s\n", str);
		bson_free(str);
	}

	if (mongoc_cursor_error(cursor, &error)) {
		fprintf(stderr, "An error occurred: %s\n", error.message);
	}

	mongoc_cursor_destroy(cursor);
	mongoc_client_pool_push(g_pool, client);																						//放回一个mongo连接
	return datavector;
}

std::vector<TickData> loadTick(std::string symbol, QDateTime startDay, QDateTime endDay)
{
	std::vector<TickData>datavector;
	if (symbol == " " || symbol == "")
	{
		return datavector;
	}
	const char* databasename = DATABASE_NAME;
	const char* collectionsname = TICKCOLLECTION_NAME;
	//auto targetday = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (days * 24 * 3600);//获取当前的系统时间
	auto start = startDay.toTime_t();
	auto end = endDay.toTime_t();

	mongoc_cursor_t* cursor;
	bson_error_t error;
	const bson_t* doc;


	// 从客户端池中获取一个客户端
	mongoc_client_t* client = mongoc_client_pool_pop(g_pool);																//取一个mongo连接

	bson_t parent;
	bson_t child;
	mongoc_collection_t* collection;
	bson_init(&parent);
	//查询bson
	BSON_APPEND_UTF8(&parent, "symbol", symbol.c_str());
	BSON_APPEND_DOCUMENT_BEGIN(&parent, "datetime", &child);
	BSON_APPEND_TIME_T(&child, "$gt", start);
	BSON_APPEND_TIME_T(&child, "$lt", end);

	bson_append_document_end(&parent, &child);


	char* str = bson_as_json(&parent, NULL);
	//	printf("\n%s\n", str);

	collection = mongoc_client_get_collection(client, DATABASE_NAME, symbol.c_str());

	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, &parent, NULL, NULL);

	while (mongoc_cursor_next(cursor, &doc)) {
		str = bson_as_json(doc, NULL);
		std::string s = str;
		std::string err;


		auto json = json11::Json::parse(s, err);
		if (!err.empty())
		{
			mongoc_cursor_destroy(cursor);
			mongoc_client_pool_push(g_pool, client);																						//放回一个mongo连接
			return datavector;
		}
		TickData tickdata;

		tickdata.symbol = json["symbol"].string_value();
		tickdata.exchange = json["exchange"].string_value();
		tickdata.gatewayname = json["gatewayname"].string_value();

		tickdata.lastprice = json["lastprice"].number_value();
		tickdata.volume = json["volume"].number_value();
		tickdata.openInterest = json["openInterest"].number_value();

		json11::Json::object datetime = json["datetime"].object_items();
		tickdata.unixdatetime = datetime["$date"].number_value() / 1000;

		tickdata.date = json["date"].string_value();
		tickdata.time = json["time"].string_value();

		tickdata.openPrice = json["openPrice"].number_value();//今日开
		tickdata.highPrice = json["highPrice"].number_value();//今日高
		tickdata.lowPrice = json["lowPrice"].number_value();//今日低
		tickdata.preClosePrice = json["preClosePrice"].number_value();//昨收

		tickdata.upperLimit = json["upperLimit"].number_value();//涨停
		tickdata.lowerLimit = json["lowerLimit"].number_value();//跌停

		tickdata.bidprice1 = json["bidprice1"].number_value();
		tickdata.bidprice2 = json["bidprice2"].number_value();
		tickdata.bidprice3 = json["bidprice3"].number_value();
		tickdata.bidprice4 = json["bidprice4"].number_value();
		tickdata.bidprice5 = json["bidprice5"].number_value();

		tickdata.askprice1 = json["askprice1"].number_value();
		tickdata.askprice2 = json["askprice2"].number_value();
		tickdata.askprice3 = json["askprice3"].number_value();
		tickdata.askprice4 = json["askprice4"].number_value();
		tickdata.askprice5 = json["askprice5"].number_value();

		tickdata.bidvolume1 = json["bidvolume1"].number_value();
		tickdata.bidvolume2 = json["bidvolume2"].number_value();
		tickdata.bidvolume3 = json["bidvolume3"].number_value();
		tickdata.bidvolume4 = json["bidvolume4"].number_value();
		tickdata.bidvolume5 = json["bidvolume5"].number_value();

		tickdata.askvolume1 = json["askvolume1"].number_value();
		tickdata.askvolume2 = json["askvolume2"].number_value();
		tickdata.askvolume3 = json["askvolume3"].number_value();
		tickdata.askvolume4 = json["askvolume4"].number_value();
		tickdata.askvolume5 = json["askvolume5"].number_value();

		datavector.push_back(tickdata);

		//		printf("%s\n", str);
		bson_free(str);
	}

	if (mongoc_cursor_error(cursor, &error)) {
		fprintf(stderr, "An error occurred: %s\n", error.message);
	}
	mongoc_cursor_destroy(cursor);
	mongoc_client_pool_push(g_pool, client);																						//放回一个mongo连接
	return datavector;


}