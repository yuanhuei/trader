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
	//防止主线程退出了,子线程还没退出
	if(m_thread.joinable())
		m_thread.detach();
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
			 m_datetime = QDateTime::fromString(QString::fromStdString(vector_history_data[i].date + " " + vector_history_data[i].time),"yy/nn/dd hh:mm:ss");
			 cross_limit_order(vector_history_data[i]);//检查价格是否触发之前的order
			 cross_stop_order(vector_history_data[i]);//检查价格是否触发之前的order

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
	double preBalance =m_capital;
	double max_drawdown=0;
	double max_ddpercent = 0;
	double total_commission = 0;
	double	total_slippage = 0;
	double	total_turnover = 0;
	double total_trade_count = 0;
	double daily_return = 0;
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

		max_drawdown = std::min(iter.second->m_drawdown, max_drawdown);
		max_ddpercent = std::min(iter.second->m_ddpercent, max_ddpercent);

		total_commission = iter.second->m_commission + total_commission;
		total_slippage = iter.second->m_slippage + total_slippage;
		total_turnover = iter.second->m_turnover + total_turnover;
		total_trade_count = iter.second->m_trades.size() + total_trade_count;

		daily_return = iter.second->m_return + daily_return;
	}
	double lastBalance = preBalance;
	m_result_statistics["profit_days"] = iProfit_days;

	m_result_statistics["loss_days"] = iLoss_days;
	m_result_statistics["end_balance"] = lastBalance;


	m_result_statistics["max_drawdown"] = max_drawdown;
	m_result_statistics["max_ddpercent"] = max_ddpercent;
		//m_result_statistics["max_drawdown_end"] =

	m_result_statistics["total_net_pnl"] = preBalance - m_capital;
	m_result_statistics["daily_net_pnl"] = (preBalance - m_capital)/m_daily_resultMap.size();

	m_result_statistics["total_commission"] = total_commission;
	m_result_statistics["daily_commission"] = total_commission/ m_daily_resultMap.size();

	m_result_statistics["total_slippage"] = total_slippage;
	m_result_statistics["total_turnover"] = total_turnover;


	m_result_statistics["total_trade_count"] = total_trade_count;
	m_result_statistics["daily_trade_count"] = total_trade_count/ m_daily_resultMap.size();
	m_result_statistics["total_return"] = (lastBalance - m_capital) / m_capital;

	m_result_statistics["annual_return"] = (lastBalance - m_capital)*240 / m_capital;
	m_result_statistics["daily_return"] =daily_return/ m_daily_resultMap.size();
		//m_result_statistics["return_std"] =

		//m_result_statistics["sharpe_ratio"] =
		//m_result_statistics["return_drawdown_ratio"] =
		//m_result_statistics["return_std"] =
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
	vector_history_data = loadBarbyDateTime(m_symbol, QDateTime(m_startDay), QDateTime(m_endDay));
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
std::vector<std::string> BacktesterEngine::sendOrder(bool bStopOrder,std::string symbol, std::string strDirection, std::string strOffset, double price, double volume, StrategyTemplate* Strategy)
{
	
	if (bStopOrder)
		return send_stop_order(symbol, strDirection,strOffset,  price,  volume,  Strategy);
	else
		return send_limit_order(symbol, strDirection, strOffset, price, volume, Strategy);

/*
std::shared_ptr<Event_Order> req = std::make_shared<Event_Order>();
req->symbol = symbol;
req->price = price;
req->totalVolume = volume;
req->status = STATUS_WAITING;

	//发单
std::string orderID = Utils::doubletostring(m_limit_order_count++); //
req->orderID = orderID;
m_WorkingOrdermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));
m_Ordermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, req));

std::vector<std::string>result;
result.push_back(orderID);
return result;*/

}
std::vector<std::string> BacktesterEngine::send_limit_order(std::string symbol, std::string strDirection, std::string strOffset, double price, double volume, StrategyTemplate* Strategy)
{
	m_limit_order_count++;

	std::shared_ptr<Event_Order> ptr_order = std::make_shared<Event_Order>();

	ptr_order->symbol = symbol;
	ptr_order->direction = strDirection;
	ptr_order->offset = strOffset;
	ptr_order->price = price;
	ptr_order->totalVolume = volume;
	ptr_order->status = STATUS_SUBMITTING;
	//ptr_stop_order-> = pStrategy->m_strategyName;

	//m_limit_order_count++;
	std::string orderID = std::to_string(m_limit_order_count);
	ptr_order->orderID = orderID;
	//stop_order.orderID
	m_active_limit_orders.insert(std::pair <std::string, std::shared_ptr<Event_Order>>(orderID, ptr_order));

	m_limit_orders.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(orderID, ptr_order));


	std::vector<std::string> orderidVector;
	orderidVector.push_back(orderID);

	return orderidVector;


}
std::vector<std::string> BacktesterEngine::send_stop_order(std::string symbol, std::string strDirection, std::string strOffset, double price, double volume, StrategyTemplate* Strategy)
{
	m_stop_order_count++;

	std::shared_ptr<Event_StopOrder> ptr_stop_order = std::make_shared<Event_StopOrder>();

	ptr_stop_order->symbol = symbol;
	ptr_stop_order->direction = strDirection;
	ptr_stop_order->offset = strOffset;
	ptr_stop_order->price = price;
	ptr_stop_order->totalVolume = volume;

	std::string orderID = std::to_string(m_limit_order_count);
	ptr_stop_order->orderID = orderID;

	m_active_stop_orders.insert(std::pair <std::string, std::shared_ptr<Event_StopOrder>>(orderID, ptr_stop_order));

	m_stop_orders.insert(std::pair<std::string, std::shared_ptr<Event_StopOrder>>(orderID, ptr_stop_order));

	//m_stragegyOrderMap[pStrategy->m_strategyName].push_back(orderID);不在m_stragegyOrderMap中保存
	//pStrategy->onStopOrder(ptr_stop_order);

	std::vector<std::string> orderidVector;
	orderidVector.push_back(orderID);

	//PutEvent(ptr_stop_order);
	return orderidVector;

}
void BacktesterEngine::cross_limit_order(const BarData& data)
{
	double long_cross_price, short_cross_price, long_best_price, short_best_price,trade_price,pos_change;
	bool bLong_cross, bShort_cross;

	if (m_backtestmode == BAR_MODE)
	{
		long_cross_price = data.low;
		short_cross_price = data.high;
		long_best_price = data.open;
		short_best_price = data.open;
	}
	std::map<std::string, std::shared_ptr<Event_Order>>::iterator iter;
	for (iter= m_active_limit_orders.begin();iter!= m_active_limit_orders.end();)
	{
		std::shared_ptr<Event_Order> limitOrder = iter->second;

		if (limitOrder->status == STATUS_SUBMITTING)//和上面的send_limit_order部分赋值对应
		{
			limitOrder->status = STATUS_NOTRADED;
			m_strategy->onOrder(limitOrder);

		}
		if ((limitOrder->direction == DIRECTION_LONG && limitOrder->price > long_cross_price) && long_cross_price > 0)
			bLong_cross = true;
		if ((limitOrder->direction == DIRECTION_SHORT && limitOrder->price <= short_cross_price) && short_cross_price > 0)
			bShort_cross = true;
		if (bLong_cross == false && bShort_cross == false)
		{
			//不满足条件的跳过
			iter++;
			continue;
		}
		limitOrder->tradedVolume = limitOrder->totalVolume;
		limitOrder->status = STATUS_ALLTRADED;
		m_strategy->onOrder(limitOrder);

		m_tradeCount++;
		if (bLong_cross)
		{
			trade_price = std::min(limitOrder->price, long_best_price);
			pos_change = limitOrder->tradedVolume;
		}
		else
		{
			trade_price = std::max(limitOrder->price, short_best_price);
			pos_change = -limitOrder->tradedVolume;
		}
		std::shared_ptr<Event_Trade> ptr_trade = std::make_shared<Event_Trade>();
		ptr_trade->symbol = limitOrder->symbol;
		ptr_trade->exchange = limitOrder->exchange;
		ptr_trade->orderID = limitOrder->orderID;
		ptr_trade->tradeID = std::to_string(m_tradeCount);
		ptr_trade->direction = limitOrder->direction;
		ptr_trade->offset = limitOrder->offset;
		ptr_trade->volume = limitOrder->tradedVolume;
		ptr_trade->gatewayname = limitOrder->gatewayname;
		ptr_trade->tradeTime = m_datetime.toString().toStdString();

		m_strategy->setPos(m_strategy->getpos() + pos_change);
		m_strategy->onTrade(ptr_trade);
		//满足条件的处理完了删除这个节点
		m_active_limit_orders.erase(iter++);
	}

}
void BacktesterEngine::cross_stop_order(const BarData& data)
{
	double long_cross_price, short_cross_price, long_best_price, short_best_price, trade_price, pos_change;
	bool bLong_cross, bShort_cross;

	if (m_backtestmode == BAR_MODE)
	{
		long_cross_price = data.high;
		short_cross_price = data.low;
		long_best_price = data.open;
		short_best_price = data.open;
	}
	std::map<std::string, std::shared_ptr<Event_StopOrder>>::iterator iter;
	for (iter= m_active_stop_orders.begin();iter!= m_active_stop_orders.end();)
	{
		std::shared_ptr<Event_StopOrder> stopOrder = iter->second;


		if (stopOrder->direction == DIRECTION_LONG && stopOrder->price <= long_cross_price)
			bLong_cross = true;
		if (stopOrder->direction == DIRECTION_SHORT && stopOrder->price >= short_cross_price)
			bShort_cross = true;
		if (bLong_cross == false && bShort_cross == false)
		{
			//不满足条件的跳过
			iter++;
			continue;
		}
		std::shared_ptr<Event_Order> ptr_order = std::make_shared<Event_Order>();
		ptr_order->symbol = stopOrder->symbol;
		ptr_order->exchange = stopOrder->exchange;
		ptr_order->direction = stopOrder->direction;
		ptr_order->offset = stopOrder->offset;
		ptr_order->orderID = std::to_string(m_limit_order_count++);
		ptr_order->price = stopOrder->price;
		ptr_order->totalVolume = stopOrder->totalVolume;

		ptr_order->status = STATUS_ALLTRADED;
		ptr_order->tradedVolume = stopOrder->totalVolume;
		ptr_order->totalVolume = stopOrder->totalVolume;
		ptr_order->gatewayname = stopOrder->gatewayname;
		ptr_order->orderTime = m_datetime.toString().toStdString();

		m_limit_orders[ptr_order->orderID] = ptr_order;



		//stopOrder->tradedVolume = stopOrder->totalVolume;
		//stopOrder->status = STATUS_ALLTRADED;
		//m_strategy->onOrder(stopOrder);

		m_tradeCount++;
		if (bLong_cross)
		{
			trade_price = std::max(stopOrder->price, long_best_price);
			pos_change = stopOrder->tradedVolume;
		}
		else
		{
			trade_price = std::min(stopOrder->price, short_best_price);
			pos_change = -stopOrder->tradedVolume;
		}
		std::shared_ptr<Event_Trade> ptr_trade = std::make_shared<Event_Trade>();
		ptr_trade->symbol = stopOrder->symbol;
		ptr_trade->exchange = stopOrder->exchange;
		ptr_trade->orderID = stopOrder->orderID;
		ptr_trade->tradeID = std::to_string(m_tradeCount);
		ptr_trade->direction = stopOrder->direction;
		ptr_trade->offset = stopOrder->offset;
		ptr_trade->volume = stopOrder->tradedVolume;
		ptr_trade->gatewayname = stopOrder->gatewayname;
		ptr_trade->tradeTime = m_datetime.toString().toStdString();

		m_tradeMap[ptr_trade->tradeID] = ptr_trade;

		stopOrder->status = STATUS_TRIGGED;

		m_strategy->onStopOrder(stopOrder);
		m_strategy->onOrder(ptr_order);

		m_strategy->setPos(m_strategy->getpos() + pos_change);
		m_strategy->onTrade(ptr_trade);

		//满足条件的处理完了删除这个节点
		m_active_stop_orders.erase(iter++);
	}

}

void BacktesterEngine::cancel_stop_order(std::string orderID, std::string gatewayname)
{
	if (m_active_stop_orders.find(orderID) != m_active_stop_orders.end())
	{
		std::shared_ptr<Event_StopOrder>ptr_order = m_active_stop_orders[orderID];
		if (ptr_order != nullptr)
		{
			//报单有效
			if (!(ptr_order->status == STATUS_ALLTRADED || ptr_order->status == STATUS_CANCELLED))
			{
				//可撤单状态
				m_stop_orders[orderID]->status = STATUS_CANCELLED;
				m_active_stop_orders.erase(orderID);
			}
		}
	}

}

void BacktesterEngine::cancel_limit_order(std::string orderID, std::string gatewayname)
{
	if (m_active_limit_orders.find(orderID) != m_active_limit_orders.end())
	{
		std::shared_ptr<Event_Order>ptr_order = m_active_limit_orders[orderID];
		if (ptr_order != nullptr)
		{
			//报单有效
			if (!(ptr_order->status == STATUS_ALLTRADED || ptr_order->status == STATUS_CANCELLED))
			{
				//可撤单状态
				m_limit_orders[orderID]->status = STATUS_CANCELLED;
				m_active_limit_orders.erase(orderID);
			}
		}
	}
}

void BacktesterEngine::cancelOrder(std::string orderID, std::string gatewayname)
{
	cancel_stop_order(orderID, gatewayname);
	cancel_limit_order(orderID, gatewayname);
}
void BacktesterEngine::cancelAllOrder()
{
	std::map<std::string, std::shared_ptr<Event_Order>>::iterator iter;
	for(iter= m_active_limit_orders.begin();iter!= m_active_limit_orders.end();)
	{
		std::shared_ptr<Event_Order>ptr_order =iter->second;
		if (ptr_order != nullptr)
		{
			//报单有效
			if (!(ptr_order->status == STATUS_ALLTRADED || ptr_order->status == STATUS_CANCELLED))
			{
				//可撤单状态
				m_limit_orders[ptr_order->orderID]->status = STATUS_CANCELLED;
				m_active_limit_orders.erase(iter++);
			}
		}
		else
			iter++;
	}
	std::map<std::string, std::shared_ptr<Event_StopOrder>>::iterator iteror;
	for (iteror = m_active_stop_orders.begin(); iteror != m_active_stop_orders.end();)
	{
		std::shared_ptr<Event_StopOrder>ptr_order = iteror->second;
		if (ptr_order != nullptr)
		{
			//报单有效
			if (!(ptr_order->status == STATUS_ALLTRADED || ptr_order->status == STATUS_CANCELLED))
			{
				//可撤单状态
				m_stop_orders[ptr_order->orderID]->status = STATUS_CANCELLED;
				m_active_stop_orders.erase(iteror++);
			}
		}
		else
			iteror++;
	}
}


void BacktesterEngine::PutEvent(std::shared_ptr<Event>e)
{
	m_eventengine->Put(e);
}