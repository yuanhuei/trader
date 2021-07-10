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

TradingResult::TradingResult(double entryPrice, std::string entryDt, double exitPrice, 
	std::string exitDt, double volume, double rate, double slippage, double size)
{
	//����
	m_entryPrice = entryPrice;  //���ּ۸�
	m_exitPrice = exitPrice;    // ƽ�ּ۸�
	m_entryDt = entryDt;        // ����ʱ��datetime
	m_exitDt = exitDt;          // ƽ��ʱ��
	m_volume = volume;			//���������� + / -������
	m_turnover = (entryPrice + exitPrice) * size * abs(volume);   //�ɽ����
	m_commission = m_turnover * rate;                                // �����ѳɱ�
	m_slippage = slippage * 2 * size * abs(volume);                        // ����ɱ�
	m_pnl = ((m_exitPrice - m_entryPrice) * volume * size - m_commission - m_slippage);  //��ӯ��
}
TradingResult::TradingResult(double entryPrice, std::string entryDt, double exitPrice,
	std::string exitDt, double volume, double size)
{
	//����
	m_entryPrice = entryPrice;  //���ּ۸�
	m_exitPrice = exitPrice;    // ƽ�ּ۸�
	m_entryDt = entryDt;        // ����ʱ��datetime
	m_exitDt = exitDt;          // ƽ��ʱ��
	m_volume = volume;			//���������� + / -������
	m_pnl = ((m_exitPrice - m_entryPrice) * volume * size);  //��ӯ��
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
	m_iInterval = iInterval;
	m_rate = rate;
	m_fSlippage = slippage;
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
//���ɲ���ʵ��

	if (m_strategy_classname == "BollChannelStrategy")
		m_strategy = new BollChannelStrategy(this, m_strategyName, m_symbol);

	//��ֵ�����������е�strategeData,�����б�������strategeData,������������ò����ͱ��������������Ǹ��������ļ����¡�
	for (std::map<std::string, float>::iterator it =m_settingMap.begin(); it != m_settingMap.end(); it++)
	{
		//����parameter
		std::string value = std::to_string(it->second);
		m_strategy->updateParam(it->first.c_str(), value.c_str());
	}
	m_strategy->updateSetting();//���²���ʵ���������ò�����ֵ

//��������

	 LoadHistoryData();

//��������
	 
	 //std::vector<BarData>datalist = loadBar(m_symbol, initDays);
	 for (std::vector<BarData>::iterator it = vector_history_data.begin(); it != vector_history_data.end(); it++)
	 {
		 m_strategy->onBar(*it);
		 if (m_strategy->inited)
		 {
			 m_strategy->trading = true;//��ʼ����ɺ�ʼִ�в���
			 writeCtaLog("���Գ�ʼ����ɣ���ʼ�ز����");
		 }
		
	 }
	 

//����ͳ�ƽ��


//���ͻز�����ź�
	writeCtaLog("���Իز����");
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
	//auto targetday = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (days * 24 * 3600);//��ȡ��ǰ��ϵͳʱ��
	auto start = startDay.toTime_t();
	auto end = endDay.toTime_t();

	mongoc_cursor_t* cursor;
	bson_error_t error;
	const bson_t* doc;


	bson_t parent;
	bson_t child;
	mongoc_collection_t* collection;
	bson_init(&parent);
	//��ѯbson
	BSON_APPEND_UTF8(&parent, "symbol", symbol.c_str());
	BSON_APPEND_DOCUMENT_BEGIN(&parent, "datetime", &child);
	BSON_APPEND_TIME_T(&child, "$gt", start);//$gt����ĳ��ʱ�䣬"$lt"С��ĳ��ʱ��
	BSON_APPEND_TIME_T(&child, "$lt", end);//$gt����ĳ��ʱ�䣬"$lt"С��ĳ��ʱ��
	bson_append_document_end(&parent, &child);


	char* str = bson_as_json(&parent, NULL);
	//	printf("\n%s\n", str);

	// �ӿͻ��˳��л�ȡһ���ͻ���
	mongoc_client_t* client = mongoc_client_pool_pop(g_pool);																		//ȡһ��mongo����

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
			mongoc_client_pool_push(g_pool, client);																						//�Ż�һ��mongo����
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

		bardata.openPrice = json["openPrice"].number_value();//���տ�
		bardata.highPrice = json["highPrice"].number_value();//���ո�
		bardata.lowPrice = json["lowPrice"].number_value();//���յ�
		bardata.preClosePrice = json["preClosePrice"].number_value();//����

		bardata.upperLimit = json["upperLimit"].number_value();//��ͣ
		bardata.lowerLimit = json["lowerLimit"].number_value();//��ͣ

		bardata.openInterest = json["openInterest"].number_value();//�ֲ�

		datavector.push_back(bardata);

		//		printf("%s\n", str);
		bson_free(str);
	}

	if (mongoc_cursor_error(cursor, &error)) {
		fprintf(stderr, "An error occurred: %s\n", error.message);
	}

	mongoc_cursor_destroy(cursor);
	mongoc_client_pool_push(g_pool, client);																						//�Ż�һ��mongo����
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
	//auto targetday = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (days * 24 * 3600);//��ȡ��ǰ��ϵͳʱ��
	auto start = startDay.toTime_t();
	auto end = endDay.toTime_t();

	mongoc_cursor_t* cursor;
	bson_error_t error;
	const bson_t* doc;


	// �ӿͻ��˳��л�ȡһ���ͻ���
	mongoc_client_t* client = mongoc_client_pool_pop(g_pool);																//ȡһ��mongo����

	bson_t parent;
	bson_t child;
	mongoc_collection_t* collection;
	bson_init(&parent);
	//��ѯbson
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
			mongoc_client_pool_push(g_pool, client);																						//�Ż�һ��mongo����
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

		tickdata.openPrice = json["openPrice"].number_value();//���տ�
		tickdata.highPrice = json["highPrice"].number_value();//���ո�
		tickdata.lowPrice = json["lowPrice"].number_value();//���յ�
		tickdata.preClosePrice = json["preClosePrice"].number_value();//����

		tickdata.upperLimit = json["upperLimit"].number_value();//��ͣ
		tickdata.lowerLimit = json["lowerLimit"].number_value();//��ͣ

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
	mongoc_client_pool_push(g_pool, client);																						//�Ż�һ��mongo����
	return datavector;


}

std::vector<TickData> BacktesterEngine::loadTick(std::string symbol, int days)
{
	//QDateTime startDay
	QDateTime endDay = QDateTime::currentDateTime();
	QDateTime startDay = endDay.addDays(0 - days);

	std::vector<TickData> tickData = loadTickbyDateTime(symbol, startDay, endDay);
	return tickData;

}
std::vector<BarData> BacktesterEngine::loadBar(std::string symbol, int days)
{
	//QDateTime startDay
	QDateTime endDay = QDateTime::currentDateTime();
	QDateTime startDay = endDay.addDays(0 - days);
	return loadBarbyDateTime(symbol, startDay, endDay);
}


//�ṩ��Strategytemplate
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
		req->direction = DIRECTION_LONG;//����
		req->offset = OFFSET_OPEN;//����
		//
		//����
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
		req->direction = DIRECTION_SHORT;//ƽ��
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
		//����
		req->direction = DIRECTION_SHORT;
		req->offset = OFFSET_OPEN;
		//����

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
		//ƽ��
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
			//������Ч
			if (!(order->status == STATUS_ALLTRADED || order->status == STATUS_CANCELLED))
			{
				//�ɳ���״̬
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