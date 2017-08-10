#include"daysix.h"


/*
黑色系加仓策略
*/
StrategyTemplate* CreateStrategy(CTAAPI *ctamanager)
{
	//创建策略
	StrategyTemplate *strategy = new daysix(ctamanager);
	g_daysix_v.push_back(strategy);
	return strategy;
}

int ReleaseStrategy()//多品种要删除多次对象
{
	//释放策略对象
	for (std::vector<StrategyTemplate*>::iterator it = g_daysix_v.begin(); it != g_daysix_v.end(); it++)
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


daysix::daysix(CTAAPI *ctamanager) :StrategyTemplate(ctamanager)
{
	//基本参数
	m_ctamanager = ctamanager;
	trademode = BAR_MODE;
	tickDbName = "CTPTickDb";
	BarDbName = "CTPMinuteDb";
	gatewayname = "CTP";
	initDays = 10;
	unitLimit = 2;
	volumelimit = 22;
	holdingPrice = 0;

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

void daysix::onInit()
{
	StrategyTemplate::onInit();
	/*************************************************/
	unitLimit = 2;

	putEvent();
}


void daysix::onTick(TickData Tick)
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
	//仓位控制
	if (holdingPrice == 0)
	{
		holdingPrice = Tick.lastprice;
	}
	holdingProfit = (Tick.lastprice - holdingPrice)*pos*atoi(getparam("size").c_str());
	highProfit = std::max(highProfit, holdingProfit);
	if (getpos(Tick.symbol) == 1 || getpos(Tick.symbol) == 0 || getpos(Tick.symbol) == -1)
	{
		highProfit = 0;
	}
	if (highProfit != 0)
	{
		if (holdingProfit < -1000)
		{
			if (pos > 0)
			{
				pos = 1;
				m_ctamanager->writeCtaLog("TICK多头止损", "daysix");
			}
			else if (pos < 0)
			{
				pos = -1;
				m_ctamanager->writeCtaLog("TICK空头止损", "daysix");
			}
		}
	}

	if (pos == 0 || pos == 1 || pos == -1)
	{
		m_algorithm->set_supposedPos(Tick.symbol, 0);
	}
	else if (pos>1)
	{
		m_algorithm->set_supposedPos(Tick.symbol, pos - 1);
	}
	else if (pos < -1)
	{
		m_algorithm->set_supposedPos(Tick.symbol, pos + 1);
	}

}
//BAR
void daysix::onBar(BarData Bar)
{

	if (TradingMode == BacktestMode)
	{
		//提前跟踪变动，以防return截断
		m_hour = Bar.gethour();
		m_minute = Bar.getminute()+1;
		m_VarPlotmtx.lock();
		m_VarPlot["pb2"] = Utils::doubletostring(pb4);
		m_VarPlot["pb6"] = Utils::doubletostring(pb6);
		m_VarPlot["holdingprice"] = Utils::doubletostring(holdingPrice);
		m_VarPlot["high100"] = Utils::doubletostring(high_100);
		m_VarPlot["low100"] = Utils::doubletostring(low_100);
		m_indicatorPlot["pos"] = Utils::doubletostring(pos);
		m_indicatorPlot["profit"] = Utils::doubletostring(holdingProfit);
		m_VarPlotmtx.unlock();
	}

	m_algorithm->checkStop(&Bar);
	m_hour = Bar.gethour();
	m_minute = Bar.getminute();


	

	if (pos > 0 && Bar.close<pb6 - 3)
	{
		pos = -1;
		m_ctamanager->writeCtaLog("PB6转向空", "daysix");
	}
	if (pos < 0 && Bar.close>pb6 + 3)
	{
		pos = 1;
		m_ctamanager->writeCtaLog("PB6转向多", "daysix");
	}

	if (std::abs(pb6 - pb4) <= 10)
	{
		if (pos > 1 && Bar.close<pb6)
		{
			pos = 1;
			m_ctamanager->writeCtaLog("PB6-PB4绝对值小于等于10个点 pb6平转向", "daysix");
		}
		if (pos < -1 && Bar.close>pb6)
		{
			pos = -1;
			m_ctamanager->writeCtaLog("PB6-PB4绝对值小于等于10个点 pb6平转向", "daysix");
		}
	}
	else
	{
		if (pos> 1 && Bar.close<pb4)
		{
			pos = int(pos / 2);
			m_ctamanager->writeCtaLog("PB6-PB4绝对值大于10个点 在pb4减半", "daysix");
		}
		if (getpos(Bar.symbol) < -1 && Bar.close>pb4)
		{
			pos = int(pos / 2);
			m_ctamanager->writeCtaLog("PB6-PB4绝对值大于10个点 在pb4减半", "daysix");
		}
	}
	

	

	//缓存5分钟线

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
	else if ((m_ninetohalfeleven.find(Utils::regMySymbol(Bar.symbol)) == m_ninetohalfeleven.end()) &&
		(m_ninetoeleven.find(Utils::regMySymbol(Bar.symbol)) == m_ninetoeleven.end()) &&
		(m_ninetoone.find(Utils::regMySymbol(Bar.symbol)) == m_ninetoone.end()) &&
		(m_ninetohalftwo.find(Utils::regMySymbol(Bar.symbol)) == m_ninetohalftwo.end()))
	{
		if (m_hour == 14 && m_minute == 59)
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



	if (m_minute % 5 == 0)
	{
		if (m_5Bar.close != 0)
		{
			if (!((m_hour == 11 && m_minute == 30) || (m_hour == 15 && m_minute == 00) || (m_hour == 10 && m_minute == 15)))
			{
				on5Bar(m_5Bar);
			}
		}
		BarData bar_5min;
		bar_5min.symbol = Bar.symbol;
		bar_5min.exchange = Bar.exchange;
		bar_5min.open = Bar.open;
		bar_5min.high = Bar.high;
		bar_5min.low = Bar.low;
		bar_5min.close = Bar.close;
		bar_5min.highPrice = Bar.highPrice;
		bar_5min.lowPrice = Bar.lowPrice;
		bar_5min.upperLimit = Bar.upperLimit;
		bar_5min.lowerLimit = Bar.lowerLimit;
		bar_5min.openInterest = Bar.openInterest;
		bar_5min.openPrice = Bar.openPrice;
		bar_5min.volume = Bar.volume;
		bar_5min.date = Bar.date;
		bar_5min.time = Bar.time;
		bar_5min.unixdatetime = Bar.unixdatetime;

		m_5Bar = bar_5min;
	}
	else
	{
		m_5Bar.high = std::max(m_5Bar.high, Bar.high);
		m_5Bar.low = std::min(m_5Bar.low, Bar.low);
		m_5Bar.close = Bar.close;
		m_5Bar.volume += Bar.volume;
		m_5Bar.highPrice = Bar.highPrice;
		m_5Bar.lowPrice = Bar.lowPrice;
	}

	if (abs(pos) >= 10)
	{
		if (m_hour == 14 && m_minute == 58)
		{
			pos = 0;
			m_ctamanager->writeCtaLog("持仓大于等于10手，平仓", "daysix");
		}
	}

	if (m_ninetoeleven.find(Utils::regMySymbol(Bar.symbol)) != m_ninetoeleven.end())
	{
		if (Utils::getWeedDay(Bar.date) == 5)
		{
			if (m_hour == 22 && m_minute == 58)
			{
				pos = 0;
			}
		}
		if (abs(pos) >= 10)
		{
			if (m_hour == 22 && m_minute == 58)
			{
				pos = 0;
				m_ctamanager->writeCtaLog("持仓大于等于10手，平仓", "daysix");
			}
		}
	}
	else if (m_ninetohalfeleven.find(Utils::regMySymbol(Bar.symbol)) != m_ninetohalfeleven.end())
	{
		if (Utils::getWeedDay(Bar.date) == 5)
		{
			if (m_hour == 23 && m_minute == 28)
			{
				pos = 0;

			}
		}
		if (abs(pos) >= 10)
		{
			if (m_hour == 23 && m_minute == 28)
			{
				pos = 0;
				m_ctamanager->writeCtaLog("持仓大于等于10手，平仓", "daysix");
			}
		}
	}
	else if ((m_ninetohalfeleven.find(Utils::regMySymbol(Bar.symbol)) == m_ninetohalfeleven.end()) &&
		(m_ninetoeleven.find(Utils::regMySymbol(Bar.symbol)) == m_ninetoeleven.end()) &&
		(m_ninetoone.find(Utils::regMySymbol(Bar.symbol)) == m_ninetoone.end()) &&
		(m_ninetohalftwo.find(Utils::regMySymbol(Bar.symbol)) == m_ninetohalftwo.end()))
	{
		if (Utils::getWeedDay(Bar.date) == 5)
		{
			if (m_hour == 14 && m_minute == 58)
			{
				pos = 0;
			}
		}
	}


	if (m_ninetoone.find(Utils::regMySymbol(Bar.symbol)) != m_ninetoone.end())
	{
		if (Utils::getWeedDay(Bar.date) == 6)
		{
			if (m_hour == 00 && m_minute == 58)
			{
				pos = 0;
			}
		}
		if (abs(pos) >= 10)
		{
			if (m_hour == 00 && m_minute == 58)
			{
				pos = 0;
				m_ctamanager->writeCtaLog("持仓大于等于10手，平仓", "daysix");
			}
		}
	}
	else if (m_ninetohalftwo.find(Utils::regMySymbol(Bar.symbol)) != m_ninetohalftwo.end())
	{
		if (Utils::getWeedDay(Bar.date) == 6)
		{
			if (m_hour == 2 && m_minute == 28)
			{
				pos = 0;
			}
		}
		if (abs(pos) >= 10)
		{
			if (m_hour == 2 && m_minute == 28)
			{
				pos = 0;
				m_ctamanager->writeCtaLog("持仓大于等于10手，平仓", "daysix");
			}
		}
	}
	if (pos == 0 || pos == 1 || pos == -1)
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}
	else if (pos>1)
	{
		m_algorithm->set_supposedPos(Bar.symbol, pos - 1);
	}
	else if (pos < -1)
	{
		m_algorithm->set_supposedPos(Bar.symbol, pos + 1);
	}


	if (TradingMode == BacktestMode)
	{
		m_algorithm->setTradingMode(BacktestMode);
		m_algorithm->checkPositions_Bar(&Bar);
	}
	putEvent();
}

void daysix::on5Bar(BarData Bar)
{
	std::unique_lock<std::mutex>lck(m_HLCmtx);
	if (high_vector.size()>100)
	{
		high_vector.erase(high_vector.begin());
		close_vector.erase(close_vector.begin());
		low_vector.erase(low_vector.begin());
	}

	close_vector.push_back(Bar.close);
	high_vector.push_back(Bar.high);
	low_vector.push_back(Bar.low);


	if (close_50_vector.size() > 100)
	{
		close_50_vector.erase(close_50_vector.begin());
	}
	close_50_vector.push_back(Bar.close);

	std::vector<double>::iterator biggest100 = std::max_element(std::begin(close_50_vector), std::end(close_50_vector));
	std::vector<double>::iterator smallest100 = std::min_element(std::begin(close_50_vector), std::end(close_50_vector));


	if (close_vector.size() < 100)
	{
		return;
	}

	pb4 = (TALIB::MA(close_vector, TA_MAType_EMA, 12).back() + TALIB::MA(close_vector, TA_MAType_SMA, 12 * 2).back()
		+ TALIB::MA(close_vector, TA_MAType_SMA, 12 * 4).back()) / 3;
	pb6 = (TALIB::MA(close_vector, TA_MAType_EMA, 24).back() + TALIB::MA(close_vector, TA_MAType_SMA, 24 * 2).back()
		+ TALIB::MA(close_vector, TA_MAType_SMA, 24 * 4).back()) / 3;


	if (high_100 == 0)
	{
		high_100 = *biggest100;
		low_100 = *smallest100;
	}

	//单根5分钟BAR 亏1500以上
	if (pos > 0)
	{
		if (((Bar.open - Bar.close)*pos * 10) >= 1500)
		{
			pos = 1;
			m_ctamanager->writeCtaLog("单根五分钟BAR亏1500以上 多砍", "daysix");
		}
	}
	if (pos< 0)
	{
		if (((Bar.close - Bar.open)*(-pos) * 10) >= 1500)
		{
			pos = -1;
			m_ctamanager->writeCtaLog("单根五分钟BAR亏1500以上 空砍", "daysix");
		}
	}

	if (!((Bar.gethour() == 9 && Bar.getminute() == 0) || (Bar.gethour() == 9 && Bar.getminute() == 5) ||
		(Bar.gethour() == 21 && Bar.getminute() == 0) || (Bar.gethour() == 21 && Bar.getminute() == 5)))
	{


		if (Bar.close > pb6)
		{

			if (pos == 0 && Bar.close>pb6 + 3)
			{
				pos = 1;
				//m_algorithm->setStop_tralingLose(&Bar, 1.0, "long");

			}
			else if (pos == 1 && Bar.close > high_100)
			{
				pos = pos + 2;
			}
			else if (pos > 1 && pos < 19 && pos < volumelimit&&Bar.close > high_100)
			{
				if (pb4>pb6)
				{
					pos = pos + 3;
				}
				
				//m_algorithm->setStop_tralingLose(&Bar, 0.8, "long");
			}
			else if (pos >= 19 && pos < volumelimit&&Bar.close > high_100)
			{
				if (pb4 > pb6)
				{
					pos = pos + 1;
				}//m_algorithm->setStop_tralingLose(&Bar, 0.8, "long");
			}
		}
		else if (Bar.close < pb6)
		{

			if (pos== 0 && Bar.close<pb6 - 3)
			{
				pos = -1;
				//m_algorithm->setStop_tralingLose(&Bar, 1.0, "short");

			}
			else if (pos == -1 && Bar.close < low_100)
			{
				pos = pos - 1;
			}
			else if (pos < 0 && pos > -19 && std::abs(pos) < volumelimit&&Bar.close < low_100)
			{
				if (pb4<pb6)
				{
					pos = pos - 3;
				}//m_algorithm->setStop_tralingLose(&Bar, 0.8, "short");
			}
			else if (pos <= -19 && std::abs(pos) < volumelimit&&Bar.close < low_100)
			{
				if (pb4 < pb6)
				{
					pos = pos - 1;
				}//m_algorithm->setStop_tralingLose(&Bar, 0.8, "short");

			}
		}
	}

	if (std::abs(pos) >= 10)
	{
		if (pos> 0)
		{
			if (Bar.close - close_vector[close_vector.size() - 2]>15 * atoi(getparam("delta").c_str()))
			{
				pos = 1;
				m_ctamanager->writeCtaLog("大于等于10手，创新高止盈", "daysix");
			}
		}
		else
		{
			if (Bar.close - close_vector[close_vector.size() - 2]<-15* atoi(getparam("delta").c_str()))
			{
				pos = -1;
				m_ctamanager->writeCtaLog("大于等于10手，创新低止盈", "daysix");
			}
		}
	}


	if (std::abs(pos) >= 19)
	{
		if (pos> 0)
		{
			if (Bar.close < high_100 - 10 * atoi(getparam("delta").c_str()))
			{
				pos = 1;
			}
		}
		else
		{
			if (Bar.close > low_100 + 10 * atoi(getparam("delta").c_str()))
			{
				pos = -1;
			}
		}
	}

	if (pos == 0 || pos == 1 || pos == -1)
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}
	else if (pos>1)
	{
		m_algorithm->set_supposedPos(Bar.symbol, pos - 1);
	}
	else if (pos < -1)
	{
		m_algorithm->set_supposedPos(Bar.symbol, pos + 1);
	}


	high_100 = *biggest100;
	low_100 = *smallest100;

	//回测用发单函数
	if (TradingMode == BacktestMode)
	{
		m_algorithm->setTradingMode(BacktestMode);
		m_algorithm->checkPositions_Bar(&Bar);
	}
}
//报单回调
void daysix::onOrder(std::shared_ptr<Event_Order>e)
{


}
//成交回调
void daysix::onTrade(std::shared_ptr<Event_Trade>e)
{
	openPrice = e->price;
	if (e->offset == "offsetopen")
	{
		holdingPrice = ((abs(getpos(e->symbol)) - e->volume)*holdingPrice + e->volume*e->price) / abs(getpos(e->symbol));
	}
	else if (e->offset == "offsetclose")
	{
		holdingPrice = e->price;
	}
}

void daysix::savepostomongo()
{
	//需要update
	bson_t *query;
	bson_t *update;
	query = BCON_NEW("strategyname", BCON_UTF8(m_strategydata->getparam("name").c_str()));
	std::map<std::string, double>map = getposmap();
	for (std::map<std::string, double>::iterator it = map.begin(); it != map.end(); it++)
	{
		update = BCON_NEW("$set", "{", BCON_UTF8(it->first.c_str()), BCON_DOUBLE(it->second), BCON_UTF8("holdingPrice"), BCON_DOUBLE(holdingPrice), "}");
		m_MongoCxx->updateData(query, update, "StrategyPos", "pos");
	}
}

void daysix::loadposfrommongo()
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
				holdingPrice = it->second.number_value();
			}
		}
	}
}

//更新参数到界面
void daysix::putEvent()
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
	m_strategydata->insertvar("pos", Utils::doubletostring(pos));
	m_strategydata->insertvar("pb6", Utils::doubletostring(pb6));
	m_strategydata->insertvar("pb4", Utils::doubletostring(pb4));
	m_strategydata->insertvar("100high", Utils::doubletostring(high_100));
	m_strategydata->insertvar("100low", Utils::doubletostring(low_100));
	m_strategydata->insertvar("holdingprofit", Utils::doubletostring(holdingProfit));
	m_strategydata->insertvar("highprofit", Utils::doubletostring(highProfit));
	//将参数和变量传递到界面上去
	std::shared_ptr<Event_UpdateStrategy>e = std::make_shared<Event_UpdateStrategy>();
	e->parammap = m_strategydata->getallparam();
	e->varmap = m_strategydata->getallvar();
	e->strategyname = m_strategydata->getparam("name");
	m_ctamanager->PutEvent(e);
}