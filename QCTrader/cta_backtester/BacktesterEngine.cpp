#include "BacktesterEngine.h"
#include"utility.h"
#include"qcstructs.h"
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

DailyTradingResult::DailyTradingResult(QDate date, double price)
{
}
DailyTradingResult::~DailyTradingResult()
{
}

void DailyTradingResult::add_trade(Event_Trade trade)
{

	m_trades.push_back(trade);
}

void DailyTradingResult::calculate_pnl (float pre_close,float start_pos,int size,float rate,float slippage)
{
	if (pre_close != -1)//输入为-1表示不知道之前的收盘价
		m_pre_close = pre_close;
	else
		m_pre_close = 1;

	m_start_pos = start_pos;
	m_end_pos = start_pos;

	m_holding_pnl = m_start_pos * (m_close_price - m_pre_close) * size;

	//Trading pnl is the pnl from new trade during the day
	m_trade_count = m_trades.size();
	for (int i = 0; i < m_trade_count; i++)
	{
		int pos_change;
		if (m_trades[i].direction == DIRECTION_LONG)
			pos_change = m_trades[i].volume;
		else
			pos_change = -m_trades[i].volume;

		m_end_pos += pos_change;

		m_turnover = m_trades[i].volume * size * m_trades[i].price;
		m_trading_pnl += pos_change * (m_close_price - m_trades[i].price) * size;
		m_slippage += m_trades[i].volume * size * slippage;

		m_turnover += m_turnover;
		m_commission += m_turnover * rate;
	}

	// Net pnl takes account of commissionand slippage cost
	m_total_pnl = m_trading_pnl + m_holding_pnl;
	m_net_pnl = m_total_pnl - m_commission - m_slippage;


}

BacktesterEngine::BacktesterEngine(EventEngine* eventengine)
{

	m_eventengine = eventengine;
}

BacktesterEngine::~BacktesterEngine()
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
	QDate startDay,
	QDate	endDay,
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
	m_startDay = startDay;
	m_endDay = endDay;
	m_iInterval = iInterval;
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
	 
	 //std::vector<BarData>datalist = loadBar(m_symbol, initDays);
	 bool bTrading = false;
	 for (int i=0;i< vector_history_data.size();i++)
	 {
		 if (bTrading)
		 {
			 m_barDate = vector_history_data[i].date;//赋值当前推送bar的时间给m_barDate方便传递参数
			 CrossLimitOrder(vector_history_data[i]);//检查价格是否触发之前的order
			 m_strategy->onBar(vector_history_data[i]);//调用策略的onBar函数推送bar数据
			 update_daily_close(vector_history_data[i].close);//更新m_daily_results map的收盘价，为后面计算做准备
		 }
		 else
		 {
			 if (m_strategy->inited)
			 {
				 m_strategy->trading = true;//初始化完成后开始执行策略
				 writeCtaLog("策略初始化完成，开始回测策略");
				 bTrading = true;
			 }
		 }
	 }
	 if (m_strategy->inited==false)
		 writeCtaLog("策略初始化无法完成，不能回测");
	 

//计算统计结果
	 calculate_result();
	 calculate_statistics(true);

	// Clear thread object handler.
	//self.thread = None
 //推送回测完成信号
	 std::shared_ptr<Event_TesterFinished>e = std::make_shared<Event_TesterFinished>();
	 m_eventengine->Put(e);

	writeCtaLog("策略回测完成");
}
void BacktesterEngine::update_daily_close(float price)
{
	QDate date;
	date.fromString(QString::fromStdString(m_barDate), "yyyy/mm/dd");//把字符串时间转换成QDate类型
	std::map<QDate, std::shared_ptr<DailyTradingResult> >::iterator iter = m_daily_resultMap.find(date);
	if (iter!= m_daily_resultMap.end())//已经有这天的数据
	{
		//更新closeprice
		iter->second->m_close_price = price;
	}
	else
	{
		m_daily_resultMap[date] = std::make_shared<DailyTradingResult>(date, price);
	}

}

void BacktesterEngine::calculate_result()
{
	writeCtaLog("开始计算逐日盯市盈亏");

	if (m_tradeMap.size() == 0)
	{
		writeCtaLog("成交记录为空，无法计算");
		return;	
	}
	//遍历trapmap，将成交记录根据日期加入到m_daily_resultMap中
	for (std::map<std::string, std::shared_ptr<Event_Trade>>::iterator iter = m_tradeMap.begin(); iter!=m_tradeMap.end(); iter++)
	{
		std::string tradetime = iter->second->tradeTime;
		QDate date;
		date=QDateTime::fromString(QString::fromStdString(tradetime), "yyyy/mm/dd hh:mm::ss").date();
		
		m_daily_resultMap[date]->add_trade(*iter->second);
	}

	// Calculate daily result by iteration.初始化价格，仓位
	int	pre_close = 0;
	int start_pos = 0;
	//计算每日的利润和损失
	for(std::map<QDate, std::shared_ptr<DailyTradingResult>>::iterator iter= m_daily_resultMap.begin();iter!= m_daily_resultMap.end();iter++)
	{
		iter->second->calculate_pnl(pre_close, start_pos, m_size,  m_rate, m_slippage);
		pre_close=iter->second->m_close_price;
		start_pos=iter->second->m_end_pos;
	}
}
std::map<std::string, double> BacktesterEngine::calculate_statistics(bool bOutput = false)
{
	m_result_statistics["start_date"] = m_startDay.toString().toStdString();
	m_result_statistics["end_date"] = m_endDay.toString().toStdString();
	m_result_statistics["total_days"] = int(m_daily_resultMap.size());

	int iProfit_days = 0;
	int iLoss_days = 0;
	double preBalance = m_capital;
	for (auto &iter: m_daily_resultMap)
	{
		if (iter.second->m_net_pnl > 0)
			iProfit_days++;
		else if (iter.second->m_net_pnl < 0)
			iLoss_days++;
		iter.second->m_balance += iter.second->m_net_pnl+ preBalance;
		iter.second->m_highlevel = std::max(preBalance, iter.second->m_balance);
		iter.second->m_return = iter.second->m_net_pnl / preBalance;
		iter.second->m_drawdown = iter.second->m_balance - iter.second->m_highlevel;
		iter.second->m_ddpercent = iter.second->m_drawdown / iter.second->m_highlevel;
		preBalance = iter.second->m_balance;
		
	}
	m_result_statistics["profit_days"] = iProfit_days;

	m_result_statistics["loss_days"] = iLoss_days;
		m_result_statistics["end_balance"] =


		m_result_statistics["max_drawdown"] =
		m_result_statistics["max_ddpercent"] =
		m_result_statistics["max_drawdown_end"] =

		m_result_statistics["total_net_pnl"] =
		m_result_statistics["daily_net_pnl"] =
		m_result_statistics["total_commission"] =

		m_result_statistics["daily_commission"] =
		m_result_statistics["total_slippage"] =
		m_result_statistics["total_turnover"] =


		m_result_statistics["total_trade_count"] =
		m_result_statistics["daily_trade_count"] =
		m_result_statistics["total_return"] =

		m_result_statistics["annual_return"] =
		m_result_statistics["daily_return"] =
		m_result_statistics["return_std"] =

		m_result_statistics["sharpe_ratio"] =
		m_result_statistics["return_drawdown_ratio"] =
		m_result_statistics["return_std"] =
}

void BacktesterEngine::CrossLimitOrder(const BarData& data)
{
	double	buyCrossPrice = data.low;
	double	sellCrossPrice = data.high;
	double	buyBestCrossPrice = data.open;
	double	sellBestCrossPrice = data.open;
	std::unique_lock<std::mutex>lck(m_ordermapmtx);
	for (std::map<std::string, std::shared_ptr<Event_Order>>::iterator iter = m_WorkingOrdermap.begin(); iter != m_WorkingOrdermap.end();)
	{
		if (data.symbol == iter->second->symbol)
		{
			bool buyCross = (iter->second->direction == DIRECTION_LONG && iter->second->price >= buyCrossPrice);
			bool sellCross = (iter->second->direction == DIRECTION_SHORT && iter->second->price <= sellCrossPrice);

			//如果发生了成交
			if (buyCross || sellCross)
			{
				m_tradeCount += 1;
				std::string tradeID = iter->first;
				std::shared_ptr<Event_Trade>trade = std::make_shared<Event_Trade>();
				trade->symbol = iter->second->symbol;
				trade->tradeID = tradeID;
				trade->orderID = iter->second->orderID;
				trade->direction = iter->second->direction;
				trade->offset = iter->second->offset;
				trade->volume = iter->second->totalVolume;
				trade->tradeTime = time_t2str(m_datetime);
				std::unique_lock<std::mutex>lck2(m_orderStrategymtx);
				if (buyCross)
				{
					trade->price = std::min(iter->second->price, buyBestCrossPrice);
					m_orderStrategymap[iter->first]->changeposmap(iter->second->symbol, m_orderStrategymap[iter->first]->getpos(iter->second->symbol) + trade->volume);
				}
				else if (sellCross)
				{
					trade->price = std::max(iter->second->price, sellBestCrossPrice);
					m_orderStrategymap[iter->first]->changeposmap(iter->second->symbol, m_orderStrategymap[iter->first]->getpos(iter->second->symbol) - trade->volume);
				}
				m_orderStrategymap[iter->first]->onTrade(trade);

				savetraderecord(m_orderStrategymap[trade->orderID]->getparam("name"), trade);

				Settlement(trade, m_orderStrategymap);

				iter->second->tradedVolume = iter->second->totalVolume;
				iter->second->status = STATUS_ALLTRADED;
				m_Ordermap[iter->first]->status = STATUS_ALLTRADED;

				m_orderStrategymap[iter->first]->onOrder(iter->second);
				m_WorkingOrdermap.erase(iter++); //#1 
			}
			else
			{
				iter++;
			}
		}
		else
		{
			iter++;
		}
	}
}

void BacktesterEngine::writeCtaLog(std::string msg)
{
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->msg = msg;
	e->gatewayname = "BacktesterEngine";
	m_eventengine->Put(e);
}

std::vector<BarData> BacktesterEngine::LoadHistoryData()
{
	vector_history_data = loadBarbyDateTime(m_symbol, m_startDay, m_endDay);
	return vector_history_data;
}	



std::vector<BarData> BacktesterEngine::loadBarbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay)
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

std::vector<TickData> BacktesterEngine::loadTickbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay)
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

std::vector<TickData> BacktesterEngine::loadTick(std::string symbol, int days)
{
	//QDateTime startDay
	QDateTime endDay = QDateTime::currentDateTime();
	QDateTime startDay = endDay.addDays(qint64(0 - days));

	std::vector<TickData> tickData = loadTickbyDateTime(symbol, startDay, endDay);
	return tickData;

}
std::vector<BarData> BacktesterEngine::loadBar(std::string symbol, int days)
{
	//QDateTime startDay
	QDateTime endDay = QDateTime::currentDateTime();
	QDateTime startDay = endDay.addDays(qint64(0 - days));
	return loadBarbyDateTime(symbol, startDay, endDay);
}


//提供给Strategytemplate
std::vector<std::string> BacktesterEngine::sendOrder(std::string symbol, std::string orderType, double price, double volume, StrategyTemplate* Strategy)
{
	std::unique_lock<std::mutex>lck(m_ordermapmtx);
	std::unique_lock<std::mutex>lck2(m_orderStrategymtx);
	std::shared_ptr<Event_Order> req = std::make_shared<Event_Order>();
	req->symbol = symbol;
	req->price = price;
	req->totalVolume = volume;
	req->status = STATUS_WAITING;
	if (orderType == CTAORDER_BUY)
	{
		req->direction = DIRECTION_LONG;//做多
		req->offset = OFFSET_OPEN;//开仓
		//
		//发单
		std::string orderID = Utils::doubletostring(m_limitorderCount++); //
		req->orderID = orderID;
		m_WorkingOrdermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));
		m_Ordermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));

		m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
		std::vector<std::string>result;
		result.push_back(orderID);
		return result;
	}
	else if (orderType == CTAORDER_SELL)
	{
		req->direction = DIRECTION_SHORT;//平多
		req->offset = OFFSET_CLOSE;
		std::string orderID = Utils::doubletostring(m_limitorderCount++); //
		req->orderID = orderID;
		m_WorkingOrdermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));
		m_Ordermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));

		m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
		std::vector<std::string>result;
		result.push_back(orderID);
		return result;
	}
	else if (orderType == CTAORDER_SHORT)
	{
		//做空
		req->direction = DIRECTION_SHORT;
		req->offset = OFFSET_OPEN;
		//发单

		std::string orderID = Utils::doubletostring(m_limitorderCount++); //
		req->orderID = orderID;
		m_WorkingOrdermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));
		m_Ordermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));

		m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
		std::vector<std::string>result;
		result.push_back(orderID);
		return result;
	}
	else if (orderType == CTAORDER_COVER)
	{
		//平空
		req->direction = DIRECTION_LONG;
		req->offset = OFFSET_CLOSE;
		std::string orderID = Utils::doubletostring(m_limitorderCount++); //
		req->orderID = orderID;
		m_WorkingOrdermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));
		m_Ordermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));
		m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
		std::vector<std::string>result;
		result.push_back(orderID);
		return result;
	}
}

void BacktesterEngine::cancelOrder(std::string orderID, std::string gatewayname)
{
	std::unique_lock<std::mutex>lck(m_ordermapmtx);
	if (m_WorkingOrdermap.find(orderID) != m_WorkingOrdermap.end())
	{
		std::shared_ptr<Event_Order>order = m_WorkingOrdermap[orderID];
		if (order != nullptr)
		{
			//报单有效
			if (!(order->status == STATUS_ALLTRADED || order->status == STATUS_CANCELLED))
			{
				//可撤单状态
				m_Ordermap[orderID]->status = STATUS_CANCELLED;
				m_WorkingOrdermap.erase(orderID);
			}
		}
	}
}

void BacktesterEngine::PutEvent(std::shared_ptr<Event>e)
{
	m_eventengine->Put(e);
}