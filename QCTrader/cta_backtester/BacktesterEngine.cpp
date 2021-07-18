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
	m_close_price = price;
	m_date = date;
}
DailyTradingResult::~DailyTradingResult()
{
}

void DailyTradingResult::add_trade(Event_Trade trade)
{

	m_Dailytrades.push_back(trade);
}

void DailyTradingResult::calculate_pnl (float pre_close,float start_pos,int size,float rate,float slippage)
{
	m_start_pos = start_pos;
	m_end_pos = m_start_pos;

	if (pre_close != -1)//����Ϊ-1��ʾm_daily_resultMap�ĵ�һ�죬��֪��֮ǰ�����̼ۣ���λҲΪ0������Ϊ0
	{
		m_pre_close = pre_close;
		m_holding_pnl = m_start_pos * (m_close_price - m_pre_close) * size;
	}
	else
		m_holding_pnl = 0;


	

	//Trading pnl is the pnl from new trade during the day
	m_trade_count = m_Dailytrades.size();
	for (int i = 0; i < m_trade_count; i++)
	{
		int pos_change;
		if (m_Dailytrades[i].direction == DIRECTION_LONG)
			pos_change = m_Dailytrades[i].volume;
		else
			pos_change = -m_Dailytrades[i].volume;

		m_trading_pnl += pos_change * (m_close_price - m_Dailytrades[i].price) * size;

		m_turnover += m_Dailytrades[i].volume * size * m_Dailytrades[i].price;
		m_slippage += m_Dailytrades[i].volume * size * slippage;
		m_commission += m_turnover * rate;

		m_end_pos += pos_change;

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

void BacktesterEngine:: ResetData()
{
	m_stop_order_count = 0;
	m_limit_order_count = 0;
	m_tradeCount = 0;

	m_stop_orders.clear();
	m_active_stop_orders.clear();
	m_limit_orders.clear();
	m_active_limit_orders.clear();
	m_tradeMap.clear();
	vector_history_data.clear();
	m_daily_resultMap.clear();
	m_result_statistics.clear();

	return;

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
	//��ֹ���߳��˳���,���̻߳�û�˳�
	if(m_thread.joinable())
		m_thread.detach();
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
	// bool bTrading = false;
	 for (int i=0;i< vector_history_data.size();i++)
	 {
		 if (m_strategy->trading)
		 {
			 //m_barDate = vector_history_data[i].date;//��ֵ��ǰ����bar��ʱ���m_barDate���㴫�ݲ���
			 //m_datetime = QDateTime::fromString(QString::fromStdString(vector_history_data[i].datetime),"yyyy-MM-dd hh:mm:ss");
			 cross_limit_order(vector_history_data[i]);//���۸��Ƿ񴥷�֮ǰ��order
			 cross_stop_order(vector_history_data[i]);//���۸��Ƿ񴥷�֮ǰ��order

			 //m_strategy->onBar(vector_history_data[i]);//���ò��Ե�onBar��������bar����
			 //update_daily_close(vector_history_data[i].close);//����m_daily_results map�����̼ۣ�Ϊ���������׼��
		 }
		 else
		 {
			 if (m_strategy->inited)
			 {
				 m_strategy->trading = true;//��ʼ����ɺ�ʼִ�в���
				 writeCtaLog("���Գ�ʼ����ɣ���ʼ�ز����");
				 //bTrading = true;
			 }

		 }
		 m_barDate = vector_history_data[i].date;//��ֵ��ǰ����bar��ʱ���m_barDate���㴫�ݲ���
		 m_datetime = QDateTime::fromString(QString::fromStdString(vector_history_data[i].date + " " + vector_history_data[i].time), "yyyy-MM-dd hh:mm:ss");

		 m_strategy->onBar(vector_history_data[i]);//���ò��Ե�onBar��������bar����
		 update_daily_close(vector_history_data[i].close);//����m_daily_results map�����̼ۣ�Ϊ���������׼��

	 }
	 if (m_strategy->inited == false)
	 {
		 writeCtaLog("���Գ�ʼ���޷���ɣ����ܻز�");
		 return;
	 }
	 

//����ͳ�ƽ��
	 calculate_result();
	 calculate_statistics(true);

	// Clear thread object handler.
	//self.thread = None
 //���ͻز�����ź�
	 std::shared_ptr<Event_TesterFinished>e = std::make_shared<Event_TesterFinished>();
	 m_eventengine->Put(e);

	writeCtaLog("���Իز����");
}


void BacktesterEngine::update_daily_close(float price)
{
	QDate date;
	date=QDate::fromString(QString::fromStdString(m_barDate), "yyyy-MM-dd");//���ַ���ʱ��ת����QDate����
	std::map <QDate , std::shared_ptr<DailyTradingResult> > ::iterator iter = m_daily_resultMap.find(date);
	if (iter!= m_daily_resultMap.end())//�Ѿ������������
	{
		//����closeprice
		iter->second->m_close_price = price;
	}
	else
	{
		m_daily_resultMap[date] = std::make_shared<DailyTradingResult>(date, price);
	}

}

void BacktesterEngine::calculate_result()
{
	writeCtaLog("��ʼ�������ն���ӯ��");

	if (m_tradeMap.size() == 0)
	{
		writeCtaLog("�ɽ���¼Ϊ�գ��޷�����");
		return;	
	}
	//����trapmap�����ɽ���¼�������ڼ��뵽m_daily_resultMap��
	for (std::map<std::string, std::shared_ptr<Event_Trade>>::iterator iter = m_tradeMap.begin(); iter!=m_tradeMap.end(); iter++)
	{
		std::string tradetime = iter->second->tradeTime;
		QDate date;
		date=QDateTime::fromString(QString::fromStdString(tradetime), "yyyy-MM-dd").date();
		
		m_daily_resultMap[date]->add_trade(*iter->second);
	}

	// Calculate daily result by iteration.��ʼ���۸񣬲�λ
	int	pre_close = -1;
	int start_pos = 0;
	//����ÿ�յ��������ʧ
	for(std::map<QDate, std::shared_ptr<DailyTradingResult>>::iterator iter= m_daily_resultMap.begin();iter!= m_daily_resultMap.end();iter++)
	{
		iter->second->calculate_pnl(pre_close, start_pos, m_size,  m_rate, m_slippage);
		pre_close=iter->second->m_close_price;
		start_pos=iter->second->m_end_pos;
	}
}

void BacktesterEngine::calculate_statistics(bool bOutput )
{
	m_result_statistics["start_date"] = m_startDay.toString("yyyy-MM-dd").toStdString();
	m_result_statistics["end_date"] = m_endDay.toString("yyyy-MM-dd").toStdString();
	m_result_statistics["total_days"] = std::to_string(m_daily_resultMap.size());

	int iProfit_days = 0;
	int iLoss_days = 0;
	double preBalance =m_capital;
	double max_drawdown=0;
	double max_ddpercent = 0;
	double total_commission = 0;
	double	total_slippage = 0;
	double	total_turnover = 0;
	double total_trade_count = 0;
	double daily_return = 0, total_return=0;
	for (auto &iter: m_daily_resultMap)
	{
		if (iter.second->m_net_pnl > 0)
			iProfit_days++;
		else if (iter.second->m_net_pnl < 0)
			iLoss_days++;
		iter.second->m_balance += iter.second->m_net_pnl+ preBalance;
		iter.second->m_highlevel = std::max(preBalance, iter.second->m_balance);//֮ǰ����߾�ֵ
		iter.second->m_return = iter.second->m_net_pnl / preBalance;//�����������
		iter.second->m_drawdown = iter.second->m_balance - iter.second->m_highlevel;//���������ߵ�ĵĻز�
		iter.second->m_ddpercent = iter.second->m_drawdown / iter.second->m_highlevel;//�����������ߵİٷֱȻز�
		preBalance = iter.second->m_balance;

		max_drawdown = std::min(iter.second->m_drawdown, max_drawdown);//�Ǹ�����������min
		max_ddpercent = std::min(iter.second->m_ddpercent, max_ddpercent);

		total_commission = iter.second->m_commission + total_commission;
		total_slippage = iter.second->m_slippage + total_slippage;
		total_turnover = iter.second->m_turnover + total_turnover;
		total_trade_count = iter.second->m_Dailytrades.size() + total_trade_count;

		total_return = iter.second->m_return + total_return;
	}
	int totalDays = m_daily_resultMap.size();
	double lastBalance = preBalance;//�����preBalance������forѭ�����濴�������һ��banlance��
	m_result_statistics["profit_days"] = std::to_string(iProfit_days);

	m_result_statistics["loss_days"] = std::to_string(iLoss_days);
	m_result_statistics["start_balance"] = std::to_string(m_capital);
	m_result_statistics["end_balance"] = std::to_string(lastBalance);


	m_result_statistics["max_drawdown"] = std::to_string(max_drawdown);
	std::string strMax_ddpercent = std::to_string(max_ddpercent * 100) + "%";
	m_result_statistics["max_ddpercent"] = strMax_ddpercent;
		//m_result_statistics["max_drawdown_end"] =

	m_result_statistics["total_net_pnl"] = std::to_string(preBalance - m_capital);
	m_result_statistics["daily_net_pnl"] = std::to_string((preBalance - m_capital)/ totalDays);

	m_result_statistics["total_commission"] = std::to_string(total_commission);
	m_result_statistics["daily_commission"] = std::to_string(total_commission/ totalDays);

	m_result_statistics["total_slippage"] = std::to_string(total_slippage);
	m_result_statistics["total_turnover"] = std::to_string(total_turnover);
	m_result_statistics["daily_slippage"] = std::to_string(total_slippage/ totalDays);
	m_result_statistics["daily_turnover"] = std::to_string(total_turnover/ totalDays);

	m_result_statistics["total_trade_count"] = std::to_string(total_trade_count);
	m_result_statistics["daily_trade_count"] = std::to_string(total_trade_count/ totalDays);


	total_return = (lastBalance - m_capital) / m_capital;
	double annual_return = (lastBalance - m_capital)*240 /( m_capital* totalDays);
	daily_return = total_return / totalDays;
	std::string strTotal_return = std::to_string(total_return*100)+"%";
	std::string strAnnual_return = std::to_string(annual_return *100)+"%";
	std::string strDaily_return = std::to_string(daily_return *100)+"%";

	m_result_statistics["total_return"] = strTotal_return;
	m_result_statistics["annual_return"] = strAnnual_return;
	m_result_statistics["daily_return"] = strDaily_return;
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

	collection = mongoc_client_get_collection(client, DATABASE_NAME, BARCOLLECTION_NAME);

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
		bardata.open = json["open_price"].number_value();
		bardata.high = json["high_price"].number_value();
		bardata.low = json["low_price"].number_value();
		bardata.close = json["close_price"].number_value();
		bardata.volume = json["volume"].number_value();

		json11::Json::object datetime = json["datetime"].object_items();
		bardata.unixdatetime = datetime["$date"].number_value() / 1000;
		//std::string strdatetime = json["datetime"].;
		time_t t;  //��ʱ��  
		tm* local; //����ʱ��   
		//tm* gmt;   //��������ʱ��  
		char buf[128] = { 0 };

		t = bardata.unixdatetime-8*60*60; //����time(&t);//��ȡĿǰ��ʱ��  
		local = localtime(&t); //תΪ����ʱ��  
		strftime(buf, 64, "%Y-%m-%d %H:%M:%S", local);
		//std::cout << buf << std::endl;
		QString qString = QString::fromStdString(buf);
		QStringList strList = qString.split(" ");
		//bardata.date = qString.section(" ", 0, 0).toStdString();
		//bardata.time = qString.section(" ", 1, 1).toStdString();
		bardata.datetime = qString.toStdString();
		bardata.date = strList[0].toStdString();
		bardata.time = strList[1].toStdString();


		//QDateTime qDateTime = QDateTime::fromString(qString, "yyyy-MM-dd hh:mm:ss");

		//bardata.date = qDateTime.date().toString().toStdString();// json["date"].string_value();
		//bardata.time = qDateTime.time().toString().toStdString();// json["time"].string_value();
		/*
		bardata.openPrice = json["openPrice"].number_value();//���տ�
		bardata.highPrice = json["highPrice"].number_value();//���ո�
		bardata.lowPrice = json["lowPrice"].number_value();//���յ�
		bardata.preClosePrice = json["preClosePrice"].number_value();//����

		bardata.upperLimit = json["upperLimit"].number_value();//��ͣ
		bardata.lowerLimit = json["lowerLimit"].number_value();//��ͣ
		*/
		bardata.openInterest = json["open_interest"].number_value();//�ֲ�

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
	//BSON_APPEND_DOCUMENT_BEGIN(&parent, "datetime", &child);
	//BSON_APPEND_TIME_T(&child, "$gt", start);
	//BSON_APPEND_TIME_T(&child, "$lt", end);

	//bson_append_document_end(&parent, &child);


	char* str = bson_as_json(&parent, NULL);
	//	printf("\n%s\n", str);

	collection = mongoc_client_get_collection(client, DATABASE_NAME, TICKCOLLECTION_NAME);

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


//�ṩ��Strategytemplate
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

	//����
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
	ptr_order->tradedVolume = volume;
	ptr_order->status = STATUS_SUBMITTING;
	ptr_order->orderTime = m_barDate;
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
	ptr_stop_order->tradedVolume = volume;
	ptr_stop_order->orderTime = m_barDate;

	std::string orderID = std::to_string(m_limit_order_count);
	ptr_stop_order->orderID = orderID;

	m_active_stop_orders.insert(std::pair <std::string, std::shared_ptr<Event_StopOrder>>(orderID, ptr_stop_order));

	m_stop_orders.insert(std::pair<std::string, std::shared_ptr<Event_StopOrder>>(orderID, ptr_stop_order));

	//m_stragegyOrderMap[pStrategy->m_strategyName].push_back(orderID);����m_stragegyOrderMap�б���
	//pStrategy->onStopOrder(ptr_stop_order);

	std::vector<std::string> orderidVector;
	orderidVector.push_back(orderID);

	//PutEvent(ptr_stop_order);
	return orderidVector;

}
void BacktesterEngine::cross_limit_order(const BarData& data)
{
	double long_cross_price, short_cross_price, long_best_price, short_best_price,trade_price,pos_change;
	bool bLong_cross=false, bShort_cross=false;

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

		if (limitOrder->status == STATUS_SUBMITTING)//�������send_limit_order���ָ�ֵ��Ӧ
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
			//����������������
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
		ptr_trade->tradeTime = m_barDate;// m_datetime.toString().toStdString();
		ptr_trade->price = limitOrder->price;
	

		m_tradeMap[ptr_trade->tradeID] = ptr_trade;

		m_strategy->setPos(m_strategy->getpos() + pos_change);
		m_strategy->onTrade(ptr_trade);
		//���������Ĵ�������ɾ������ڵ�
		m_active_limit_orders.erase(iter++);
	}

}
void BacktesterEngine::cross_stop_order(const BarData& data)
{
	double long_cross_price, short_cross_price, long_best_price, short_best_price, trade_price, pos_change;
	bool bLong_cross=false, bShort_cross=false;

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
			//����������������
			iter++;
			continue;
		}
		std::shared_ptr<Event_Order> ptr_order = std::make_shared<Event_Order>();
		ptr_order->symbol = stopOrder->symbol;
		ptr_order->exchange = stopOrder->exchange;
		ptr_order->direction = stopOrder->direction;
		ptr_order->offset = stopOrder->offset;
		m_limit_order_count++;
		ptr_order->orderID = std::to_string(m_limit_order_count);
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
		ptr_trade->tradeTime = m_barDate;// m_datetime.toString().toStdString();
		ptr_trade->price = stopOrder->price;

		m_tradeMap[ptr_trade->tradeID] = ptr_trade;

		stopOrder->status = STATUS_TRIGGED;

		m_strategy->onStopOrder(stopOrder);
		m_strategy->onOrder(ptr_order);

		m_strategy->setPos(m_strategy->getpos() + pos_change);
		m_strategy->onTrade(ptr_trade);

		//���������Ĵ�������ɾ������ڵ�
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
			//������Ч
			if (!(ptr_order->status == STATUS_ALLTRADED || ptr_order->status == STATUS_CANCELLED))
			{
				//�ɳ���״̬
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
			//������Ч
			if (!(ptr_order->status == STATUS_ALLTRADED || ptr_order->status == STATUS_CANCELLED))
			{
				//�ɳ���״̬
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
void BacktesterEngine::cancelAllOrder(std::string strStragetyName)
{
	std::map<std::string, std::shared_ptr<Event_Order>>::iterator iter;
	for(iter= m_active_limit_orders.begin();iter!= m_active_limit_orders.end();)
	{
		std::shared_ptr<Event_Order>ptr_order =iter->second;
		if (ptr_order != nullptr)
		{
			//������Ч
			//if (!(ptr_order->status == STATUS_ALLTRADED || ptr_order->status == STATUS_CANCELLED))
			//{
				//�ɳ���״̬
				m_limit_orders[ptr_order->orderID]->status = STATUS_CANCELLED;
				m_active_limit_orders.erase(iter++);
			//}
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
			//������Ч
			//if (!(ptr_order->status == STATUS_ALLTRADED || ptr_order->status == STATUS_CANCELLED))
			//{
				//�ɳ���״̬
				m_stop_orders[ptr_order->orderID]->status = STATUS_CANCELLED;
				m_active_stop_orders.erase(iteror++);
			//}
		}
		else
			iteror++;
	}
}


void BacktesterEngine::PutEvent(std::shared_ptr<Event>e)
{
	m_eventengine->Put(e);
}