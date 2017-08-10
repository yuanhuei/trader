#include"CDP.h"


/*
CDP短线策略
*/
StrategyTemplate* CreateStrategy(CTAAPI *ctamanager)
{
	//创建策略
	StrategyTemplate *strategy = new CDP(ctamanager);
	g_CDP_v.push_back(strategy);
	return strategy;
}

int ReleaseStrategy()//多品种要删除多次对象
{
	//释放策略对象
	for (std::vector<StrategyTemplate*>::iterator it = g_CDP_v.begin(); it != g_CDP_v.end(); it++)
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


CDP::CDP(CTAAPI *ctamanager) :StrategyTemplate(ctamanager)
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

void CDP::onInit()
{
	StrategyTemplate::onInit();
	/*************************************************/
	unitLimit = 2;

	putEvent();
}


void CDP::onTick(TickData Tick)
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
void CDP::onBar(BarData Bar)
{
	int tmphour = (Bar.gethour());
	int tmpminute = (Bar.getminute() + 1);

	if (TradingMode == BacktestMode)
	{
		//提前跟踪变动，以防return截断
		m_VarPlotmtx.lock();
		//m_VarPlot["upper"] = Utils::doubletostring(upperBorder);
		//m_VarPlot["lower"] = Utils::doubletostring(lowerBorder);
		//m_indicatorPlot["pos"] = Utils::doubletostring(getpos(Bar.symbol));
		//m_indicatorPlot["cdp"] = Utils::doubletostring(cdp);
		//m_indicatorPlot["rsi"] = Utils::doubletostring(rsi);
		//m_indicatorPlot["100"] = Utils::doubletostring(count);
		m_VarPlotmtx.unlock();
	}

	if (tmpminute % 10 == 0)
	{
		if (this->Bar_10min.close != 0)
		{
				on10Bar(this->Bar_10min);
		}
		BarData Bar_10min;
		Bar_10min.symbol = Bar.symbol;
		Bar_10min.exchange = Bar.exchange;
		Bar_10min.open = Bar.open;
		Bar_10min.high = Bar.high;
		Bar_10min.low = Bar.low;
		Bar_10min.close = Bar.close;
		Bar_10min.highPrice = Bar.highPrice;
		Bar_10min.lowPrice = Bar.lowPrice;
		Bar_10min.upperLimit = Bar.upperLimit;
		Bar_10min.lowerLimit = Bar.lowerLimit;
		Bar_10min.openInterest = Bar.openInterest;
		Bar_10min.openPrice = Bar.openPrice;
		Bar_10min.volume = Bar.volume;
		Bar_10min.date = Bar.date;
		Bar_10min.time = Bar.time;
		Bar_10min.unixdatetime = Bar.unixdatetime;
		this->Bar_10min = Bar_10min;
	}
	else
	{
		Bar_10min.high = std::max(Bar_10min.high, Bar.high);
		Bar_10min.low = std::min(Bar_10min.low, Bar.low);
		Bar_10min.close = Bar.close;
		Bar_10min.volume += Bar.volume;
		Bar_10min.highPrice = Bar.highPrice;
		Bar_10min.lowPrice = Bar.lowPrice;
	}

	std::unique_lock<std::mutex>lck(m_HLCmtx);
	if (high_vector.size()>225)
	{
		high_vector.erase(high_vector.begin());
		close_vector.erase(close_vector.begin());
		low_vector.erase(low_vector.begin());
	}

	high_vector.push_back(Bar.high);
	close_vector.push_back(Bar.close);
	low_vector.push_back(Bar.low);

	if (close_vector.size() < 225)
	{
		return;
	}
	
	if (tmphour == 9 && tmpminute == 01)
	{
		std::vector<double>::iterator biggest375 = std::max_element(std::begin(close_vector), std::end(close_vector));
		std::vector<double>::iterator smallest375 = std::min_element(std::begin(close_vector), std::end(close_vector));
		dayHIGH = *biggest375;
		dayLOW = *smallest375;
		dayClose = close_vector[close_vector.size() - 2];

		cdp = (dayHIGH + dayLOW + 2 * dayClose) / 4;
		ah = cdp + (dayHIGH - dayLOW);
		nh = cdp * 2 - dayLOW;
		nl = cdp * 2 - dayHIGH;
		al = cdp - (dayHIGH - dayLOW);
		todayEntry = dayHIGH - dayLOW>20;
	}

	m_algorithm->checkStop(&Bar);


	if (Bar.gethour() == 14 && Bar.getminute() == 58)
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}

	//回测用发单函数
	if (TradingMode == BacktestMode)
	{
		m_algorithm->setTradingMode(BacktestMode);
		m_algorithm->checkPositions_Bar(&Bar);
	}

	putEvent();
}
void CDP::on10Bar(BarData Bar)
{
	if (close_10min_vector.size() >= 100)
	{
		close_10min_vector.erase(close_10min_vector.begin());
	}
	
	close_10min_vector.push_back(Bar.close);

	if (close_10min_vector.size() < 99)
	{
		return;
	}

	double rsi = TALIB::RSI(close_10min_vector, 80).back();



	if (getpos(Bar.symbol) == 0 && todayEntry == true && rsi < 80 && cdp > 0 && Bar.gethour() < 14)
	{
		if (Bar.close > ah)
		{
			m_algorithm->set_supposedPos(Bar.symbol, 1);
			m_algorithm->setStop_tralingLose(&Bar, 0.9,"long");
		}
	}
	else if (getpos(Bar.symbol) == 0 && todayEntry == true && rsi > 20 && cdp > 0 && Bar.gethour() < 14)
	{
		if (Bar.close < al)
		{
			m_algorithm->set_supposedPos(Bar.symbol, -1);
			m_algorithm->setStop_tralingLose(&Bar, 0.9, "short");
		}
	}

	if (getpos(Bar.symbol)>0 && (Bar.close< al))
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}


	if (getpos(Bar.symbol) < 0 && (Bar.close > ah))
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}

	if (getpos(Bar.symbol) > 0 && Bar.close > nh)
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}

	if (getpos(Bar.symbol) < 0 && Bar.close < nl)
	{
		m_algorithm->set_supposedPos(Bar.symbol, 0);
	}
 
}

//报单回调
void CDP::onOrder(std::shared_ptr<Event_Order>e)
{


}
//成交回调
void CDP::onTrade(std::shared_ptr<Event_Trade>e)
{
	 
}

//更新参数到界面
void CDP::putEvent()
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