#include"atr_hl.h"


/*
海龟交易法则：50周期通道，25周期卖出通道，每日加仓最多两次，一共加仓最多6次
*/
StrategyTemplate* CreateStrategy(CTAAPI *ctamanager)
{
	//创建策略
	StrategyTemplate *strategy = new atr_hl(ctamanager);
	g_atr_hl_v.push_back(strategy);
	return strategy;
}

int ReleaseStrategy()//多品种要删除多次对象
{
	//释放策略对象
	for (std::vector<StrategyTemplate*>::iterator it = g_atr_hl_v.begin(); it != g_atr_hl_v.end(); it++)
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


atr_hl::atr_hl(CTAAPI *ctamanager) :StrategyTemplate(ctamanager)
{
	//基本参数
	m_ctamanager = ctamanager;
	trademode = BAR_MODE;
	tickDbName = "CTPTickDb";
	BarDbName = "CTPMinuteDb";
	gatewayname = "CTP";
	initDays = 10;
	unitLimit = 2;
	atrget = 0.5;

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

void atr_hl::onInit()
{
	StrategyTemplate::onInit();
	/*************************************************/
	unitLimit = 2;

	putEvent();
}


void atr_hl::onTick(TickData Tick)
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


}
//BAR
void atr_hl::onBar(BarData Bar)
{
	if (TradingMode == BacktestMode)
	{
		m_hour = Bar.gethour();
		m_minute = Bar.getminute() + 1;
		//提前跟踪变动，以防return截断
		m_VarPlotmtx.lock();
		m_VarPlot["myPrice"] = Utils::doubletostring(myPrice);
		m_VarPlot["up"] = Utils::doubletostring(UP);
		m_VarPlot["down"] = Utils::doubletostring(DOWN);
		m_VarPlot["dayStart"] = Utils::doubletostring(dayStart);
		m_indicatorPlot["pos"] = Utils::doubletostring(getpos(Bar.symbol));
		m_VarPlotmtx.unlock();
	}
	if (m_hour == 9 && m_minute == 01)
	{
		if (high_vector.size() < 67)
		{
			return;
		}
		dayStart = Bar.open;

		std::vector<double>::iterator biggest100 = std::max_element(std::begin(high_vector), std::end(high_vector));
		std::vector<double>::iterator smallest100 = std::min_element(std::begin(low_vector), std::end(low_vector));
		PP = *biggest100 - *smallest100;
		UP = dayStart + PP*atrget;
		DOWN = dayStart - PP*atrget;
	}

	//缓存5分钟线
	m_algorithm->checkStop(&Bar);
	m_hour = Bar.gethour();
	m_minute = Bar.getminute();



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
		m_minute = m_minute;
		m_hour = m_hour;
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


	if (TradingMode == BacktestMode)
	{
		m_algorithm->setTradingMode(BacktestMode);
		m_algorithm->checkPositions_Bar(&Bar);
	}


	putEvent();
}

void atr_hl::on5Bar(BarData Bar)
{
	std::unique_lock<std::mutex>lck(m_HLCmtx);
	if (high_vector.size()>67)
	{
		high_vector.erase(high_vector.begin());
		close_vector.erase(close_vector.begin());
		low_vector.erase(low_vector.begin());
	}
	if (!((Bar.gethour() == 9 && Bar.getminute() == 0) || (Bar.gethour() == 9 && Bar.getminute() == 5) ||
		(Bar.gethour() == 21 && Bar.getminute() == 0) || (Bar.gethour() == 21 && Bar.getminute() == 5)))
	{
		close_vector.push_back(Bar.close);
		high_vector.push_back(Bar.high);
		low_vector.push_back(Bar.low);
	}
	if (high_vector.size() < 67)
	{
		return;
	}

	

	if (getpos(Bar.symbol) != 1 && Bar.high > UP)
	{
		myPrice = UP;
		if (Bar.open > myPrice)
		{
			myPrice = Bar.open;
		}
		m_algorithm->set_supposedPos(Bar.symbol, getpos(Bar.symbol) + 1);
		return;
	}
	if (getpos(Bar.symbol) != 1 && Bar.low < DOWN)
	{
		myPrice = DOWN;
		if (Bar.open > myPrice)
		{
			myPrice = Bar.open;
		}
		m_algorithm->set_supposedPos(Bar.symbol, getpos(Bar.symbol) - 1);
		return;
	}
	
	//回测用发单函数
	if (TradingMode == BacktestMode)
	{
		m_algorithm->setTradingMode(BacktestMode);
		m_algorithm->checkPositions_Bar(&Bar);
	}
}
//报单回调
void atr_hl::onOrder(std::shared_ptr<Event_Order>e)
{


}
//成交回调
void atr_hl::onTrade(std::shared_ptr<Event_Trade>e)
{
	openPrice = e->price;
}

//更新参数到界面
void atr_hl::putEvent()
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