#include"daytrade1.h"

/*
日内交易
*/
StrategyTemplate* CreateStrategy(CTAAPI *ctamanager)
{
	//创建策略
	StrategyTemplate *strategy = new Daytrade1(ctamanager);
	g_DAYTRADE1_v.push_back(strategy);
	return strategy;
}

int ReleaseStrategy()//多品种要删除多次对象
{
	//释放策略对象
	for (std::vector<StrategyTemplate*>::iterator it = g_DAYTRADE1_v.begin(); it != g_DAYTRADE1_v.end(); it++)
	{
		if ((*it) != nullptr)
		{
			//删除指针
			delete *it;
			*it = nullptr;
		}
	}
	return 0;
}


Daytrade1::Daytrade1(CTAAPI *ctamanager) :StrategyTemplate(ctamanager)
{
	//基本参数
	m_ctamanager = ctamanager;
	trademode = BAR_MODE;
	tickDbName = "CTPTickDb";
	BarDbName = "CTPMinuteDb";
	gatewayname = "CTP";
	initDays = 10;
	unitLimit = 2;


	/*****************夜盘收盘平仓*************************/
	std::string ninetoeleven[] = { "bu", "rb", "hc", "ru" };//9点到11点的合约列表
	std::string ninetohalfeleven[] = { "p", "j", "m", "y", "a", "b", "jm", "i", "SR", "CF", "RM", "MA", "ZC", "FG", "OI" };//9点到11点半的合约
	std::string ninetoone[] = { "cu", "al", "zn", "pb", "sn", "ni" };//9点到1点的合约列表
	std::string ninetohalftwo[] = { "ag", "au" };//9点到2点半的合约
	for (int i = 0; i < sizeof(ninetoeleven) / sizeof(ninetoeleven[0]); ++i)
	{
		m_ninetoeleven.insert(ninetoeleven[i]);
	}
	for (int i = 0; i < sizeof(ninetohalfeleven) / sizeof(ninetohalfeleven[0]); ++i)
	{
		m_ninetohalfeleven.insert(ninetohalfeleven[i]);
	}
	for (int i = 0; i < sizeof(ninetoone) / sizeof(ninetoone[0]); ++i)
	{
		m_ninetoone.insert(ninetoone[i]);
	}
	for (int i = 0; i < sizeof(ninetohalftwo) / sizeof(ninetohalftwo[0]); ++i)
	{
		m_ninetohalftwo.insert(ninetohalftwo[i]);
	}
}
//TICK

void Daytrade1::onInit()
{
	StrategyTemplate::onInit();
	/*************************************************/
	unitLimit = 2;

	putEvent();
}


void Daytrade1::onTick(TickData Tick)
{

	putEvent();

	int tickMinute = Tick.getminute();
	int tickHour = Tick.gethour();
	m_algorithm->checkPositions_Tick(&Tick);
	m_hourminutemtx.lock();
	if ((tickMinute != m_minute) || tickHour != m_hour)
	{
		if (m_bar.close != 0)
		{
			if (!((m_hour == 11 && m_minute == 30) || (m_hour == 15 && m_minute == 00) || (m_hour == 10 && m_minute == 15)))
			{
				onBar(m_bar);
			}
		}
		BarData bar;
		bar.symbol = Tick.symbol;
		bar.exchange = Tick.exchange;
		bar.open = Tick.lastprice;
		bar.high = Tick.lastprice;
		bar.low = Tick.lastprice;
		bar.close = Tick.lastprice;
		bar.highPrice = Tick.highPrice;
		bar.lowPrice = Tick.lowPrice;
		bar.upperLimit = Tick.upperLimit;
		bar.lowerLimit = Tick.lowerLimit;
		bar.openInterest = Tick.openInterest;
		bar.openPrice = Tick.openPrice;
		bar.volume = Tick.volume;
		bar.date = Tick.date;
		bar.time = Tick.time;
		bar.unixdatetime = Tick.unixdatetime;

		m_bar = bar;
		m_minute = tickMinute;
		m_hour = tickHour;
	}
	else
	{
		m_bar.high = std::max(m_bar.high, Tick.lastprice);
		m_bar.low = std::min(m_bar.low, Tick.lastprice);
		m_bar.close = Tick.lastprice;
		m_bar.volume = Tick.volume;
		m_bar.highPrice = Tick.highPrice;
		m_bar.lowPrice = Tick.lowPrice;
	}
	lastprice = Tick.lastprice;
	m_hourminutemtx.unlock();

}
//BAR
void Daytrade1::onBar(BarData Bar)
{

	if (TradingMode == BacktestMode)
	{
		//提前跟踪变动，以防return截断
		m_VarPlotmtx.lock();

		m_indicatorPlot["buysellsignal"] = std::to_string(buysellsignal);
		m_VarPlot["stopsignal"] = std::to_string(stopsignal);
		m_indicatorPlot["pos"] = Utils::doubletostring(getpos(Bar.symbol));
		m_VarPlotmtx.unlock();
	}

	m_hour = Bar.gethour();
	m_minute = Bar.getminute();

	if (close_vector.size() > 100)
	{
		close_vector.erase(close_vector.begin());
	}
	close_vector.push_back(Bar.close);
	//最后一根K线不进行交易
	if (m_hour == 14 && m_minute == 59)
	{
		return;
	}

	if (m_ninetoeleven.find(Utils::regMySymbol(Bar.symbol)) != m_ninetoeleven.end())
	{
		if (m_hour == 22 && m_minute == 59)
		{
			return;
		}
	}
	else if (m_ninetohalfeleven.find(Utils::regMySymbol(Bar.symbol)) != m_ninetohalfeleven.end())
	{
		if (m_hour == 23 && m_minute == 29)
		{
			return;

		}
	}

	if (m_ninetoone.find(Utils::regMySymbol(Bar.symbol)) != m_ninetoone.end())
	{
		if (m_hour == 00 && m_minute == 59)
		{
			return;
		}
	}
	else if (m_ninetohalftwo.find(Utils::regMySymbol(Bar.symbol)) != m_ninetohalftwo.end())
	{
		if (m_hour == 2 && m_minute == 29)
		{
			return;
		}
	}





	//入场
	if ((Bar.gethour() == 9 && Bar.getminute() == 0) || (Bar.gethour() == 21 && Bar.getminute() == 0))
	{
		TodayOpen = Bar.open;
	}

	//开仓

	buysellsignal = ((Bar.open - TodayOpen) / TodayOpen)*1000;
	stopsignal = EntryPrice*(1000 - stoploss) / 1000;

	if ((Bar.gethour() == 9 && Bar.getminute() == 04) || (Bar.gethour() == 21 && Bar.getminute() == 04))
	{
		if ((Bar.open - TodayOpen) / TodayOpen > (range / 1000))
		{
			m_algorithm->set_supposedPos(Bar.symbol, 1);
		}

		if ((Bar.open - TodayOpen) / TodayOpen < (-1 * range / 1000))
		{
			m_algorithm->set_supposedPos(Bar.symbol, -1);
		}
	}

	if (getpos(Bar.symbol) == 1 && close_vector[close_vector.size() - 2] < EntryPrice*(1000 - stoploss) / 1000)
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}

	if (getpos(Bar.symbol) == -1 && close_vector[close_vector.size() - 2]> EntryPrice*(1000 + stoploss) / 1000)
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}

	//收盘平仓
	if (m_hour == 14 && m_minute == 58)
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}

	if (m_ninetoeleven.find(Utils::regMySymbol(Bar.symbol)) != m_ninetoeleven.end())
	{
		if (m_hour == 22 && m_minute == 58)
		{
			m_algorithm->set_supposedPos(Bar.symbol, 0);
		}
	}
	else if (m_ninetohalfeleven.find(Utils::regMySymbol(Bar.symbol)) != m_ninetohalfeleven.end())
	{
		if (m_hour == 23 && m_minute == 28)
		{
			m_algorithm->set_supposedPos(Bar.symbol, 0);
		}
	}

	if (m_ninetoone.find(Utils::regMySymbol(Bar.symbol)) != m_ninetoone.end())
	{
		if (m_hour == 00 && m_minute == 58)
		{
			m_algorithm->set_supposedPos(Bar.symbol, 0);
		}
	}
	else if (m_ninetohalftwo.find(Utils::regMySymbol(Bar.symbol)) != m_ninetohalftwo.end())
	{
		if (m_hour == 2 && m_minute == 28)
		{
			m_algorithm->set_supposedPos(Bar.symbol, 0);
		}
	}

	if (TradingMode == BacktestMode)
	{
		m_algorithm->setTradingMode(BacktestMode);
		m_algorithm->checkPositions_Bar(&Bar);
	}
	putEvent();
}

//报单回调
void Daytrade1::onOrder(std::shared_ptr<Event_Order>e)
{


}
//成交回调
void Daytrade1::onTrade(std::shared_ptr<Event_Trade>e)
{

	EntryPrice = e->price;
}

void Daytrade1::savepostomongo()
{

	////需要update
	//bson_t *query;
	//bson_t *update;
	//query = BCON_NEW("strategyname", BCON_UTF8(m_strategydata->getparam("name").c_str()));
	//std::map<std::string, double>map = getposmap();
	//for (std::map<std::string, double>::iterator it = map.begin(); it != map.end(); it++)
	//{
	//	update = BCON_NEW("$set", "{", BCON_UTF8(it->first.c_str()), BCON_DOUBLE(it->second), BCON_UTF8("holdingPrice"), BCON_DOUBLE(holdingPrice), "}");
	//	m_MongoCxx->updateData(query, update, "StrategyPos", "pos");
	//}
}

void Daytrade1::loadposfrommongo()
{
	std::vector<std::string>result;

	bson_t query;

	bson_init(&query);

	m_MongoCxx->append_utf8(&query, "strategyname", m_strategydata->getparam("name").c_str());

	result = m_MongoCxx->findData(&query, "StrategyPos", "pos");

	for (std::vector<std::string>::iterator iter = result.begin(); iter != result.end(); iter++)
	{
		std::string s = (*iter);
		std::string err;


		auto json_parsed = json11::Json::parse(s, err);
		if (!err.empty())
		{
			return;
		}
		auto amap = json_parsed.object_items();

		std::map<std::string, double>temp_posmap = getposmap();
		for (std::map<std::string, json11::Json>::iterator it = amap.begin(); it != amap.end(); it++)
		{
			if (temp_posmap.find(it->first) != temp_posmap.end())
			{
				changeposmap(it->first, it->second.number_value());
			}
			if (it->first == "holdingPrice")
			{
			//	holdingPrice = it->second.number_value();
			}
		}
	}
}

//更新参数到界面
void Daytrade1::putEvent()
{
	m_strategydata->insertvar("inited", Utils::booltostring(inited));
	m_strategydata->insertvar("trading", Utils::booltostring(trading));
	//更新仓位
	std::map<std::string, double>map = getposmap();
	for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
	{
		m_strategydata->insertvar(("pos_" + iter->first), Utils::doubletostring(iter->second));
	}

	m_strategydata->insertvar("lastprice", Utils::doubletostring(lastprice));

	//将参数和变量传递到界面上去
	std::shared_ptr<Event_UpdateStrategy>e = std::make_shared<Event_UpdateStrategy>();
	e->parammap = m_strategydata->getallparam();
	e->varmap = m_strategydata->getallvar();
	e->strategyname = m_strategydata->getparam("name");
	m_ctamanager->PutEvent(e);
}