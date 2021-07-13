#include"StrategyTemplate.h"
#include"CtaEngine.h"
#include<cstdlib>
#include<string>
#include"CTAAPI.h"
#include"utility.h"
extern mongoc_uri_t* g_uri;
extern mongoc_client_pool_t* g_pool;

StrategyTemplate::StrategyTemplate(CTAAPI*ctaEngine,std::string strategyName, std::string symbol)
{
	m_ctaEngine = ctaEngine;
	gatewayname = "CTP";
	inited = false;
	trading = false;
	TradingMode = RealMode;
	unitLimit = 2;
	m_Pos = 0;

	m_strategyName = strategyName;
	m_symbol = symbol;
	//m_exchange= symbol_vt

	m_strategydata = new StrategyData();
	m_strategydata->insertvar("inited", Utils::booltostring(inited));
	m_strategydata->insertvar("trading", Utils::booltostring(trading));
	m_strategydata->insertvar("pos", "0");

	m_algorithm = new algorithmOrder(unitLimit, TradingMode, this);
	m_MongoCxx = new MongoCxx(g_pool);
}

StrategyTemplate::~StrategyTemplate()
{
	sync_data();
	delete m_strategydata;
	delete m_MongoCxx;
}
void StrategyTemplate::updateSetting()
{}
void StrategyTemplate::sync_data()
{
	std::string strFileName = m_strategyName + "_" + m_symbol;
	Global_FUC::WriteStrategyDataJson(m_strategydata->getallvar(), strFileName);
}
void StrategyTemplate::setPos(int pos)
{
	m_Pos = pos;
	m_strategydata->setvar("pos", std::to_string(pos));//变量表中的pos也需要更新
}
void StrategyTemplate::changeposmap(std::string symbol, double pos)
{
	m_Pos_mapmtx.lock();
	m_pos_map[symbol] = pos;
	m_Pos_mapmtx.unlock();
}

//初始化
void StrategyTemplate::onInit()
{

	//默认使用bar 需要使用tick自己修改
	std::string strategyname = m_strategyName;
	m_ctaEngine->writeCtaLog("策略初始化" + strategyname);
	std::vector<BarData>datalist = loadBar(m_symbol, initDays);
	for (std::vector<BarData>::iterator it = datalist.begin(); it != datalist.end(); it++)
	{
		onBar(*it);
	}
	/*
	std::vector<std::string>symbol_v = Utils::split(m_strategydata->getparam("symbol"), ",");
	if (trademode == BAR_MODE)
	{
		std::vector<BarData>alldatalist;
		for (std::vector<std::string>::iterator it = symbol_v.begin(); it != symbol_v.end(); it++)
		{
			std::vector<BarData>datalist = loadBar(*it, initDays);
			for (std::vector<BarData>::iterator it = datalist.begin(); it != datalist.end(); it++)
			{
				alldatalist.push_back(*it);
			}
		}

		std::sort(alldatalist.begin(), alldatalist.end(), BarGreater());

		for (std::vector<BarData>::iterator it = alldatalist.begin(); it != alldatalist.end(); it++)
		{
			onBar(*it);
		}
	}
	else if (trademode == TICK_MODE)
	{
		std::vector<TickData>alldatalist;
		for (std::vector<std::string>::iterator it = symbol_v.begin(); it != symbol_v.end(); it++)
		{
			std::vector<TickData>datalist = loadTick(*it, initDays);
			for (std::vector<TickData>::iterator it = datalist.begin(); it != datalist.end(); it++)
			{
				alldatalist.push_back(*it);
			}
		}

		std::sort(alldatalist.begin(), alldatalist.end(), TickGreater());

		for (std::vector<TickData>::iterator it = alldatalist.begin(); it != alldatalist.end(); it++)
		{
			onTick(*it);
		}
	}
	//loadposfrommongo();  //读取持仓
	std::map<std::string, double>map = getposmap();
	for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
	{
		m_algorithm->set_supposedPos(iter->first, iter->second);
	}

	*/
	inited = true;
	//putEvent();
}
//开始 
void StrategyTemplate::onStart()
{
	std::string strategyname = m_strategyName;
	m_ctaEngine->writeCtaLog("策略开始" + strategyname);
	trading = true;
	putEvent();
}
//停止
void StrategyTemplate::onStop()
{
	std::string strategyname = m_strategyName;
	m_ctaEngine->writeCtaLog("策略停止" + strategyname);
	trading = false;
	putEvent();
	//savepostomongo();
	sync_data();
}
//给参数赋值
void StrategyTemplate::checkparam(const char* paramname, const char* paramvalue)
{
	m_strategydata->insertparam(paramname, paramvalue);

}

//更新参数的值
void StrategyTemplate::updateParam(const char* paramname, const char* paramvalue)
{

	m_strategydata->setparam(paramname, paramvalue);
}

//更新变量的值
void StrategyTemplate::updateVar(const char* paramname, const char* paramvalue)
{

	m_strategydata->setvar(paramname, paramvalue);
}
//TICK
void StrategyTemplate::onTick(TickData Tick)
{
	putEvent();
}
//BAR
void StrategyTemplate::onBar(BarData Bar)
{
	putEvent();
}
//报单回调
void StrategyTemplate::onOrder(std::shared_ptr<Event_Order>e)
{
	putEvent();
}
//成交回调
void StrategyTemplate::onTrade(std::shared_ptr<Event_Trade>e)
{
	putEvent();
}
void StrategyTemplate::onStopOrder(std::shared_ptr<Event_StopOrder>e)
{
	putEvent();

 }
//做多
std::vector<std::string> StrategyTemplate::buy(double price, double volume,bool bStopOrder )
{
	return sendOrder(bStopOrder,DIRECTION_LONG, OFFSET_OPEN, price, volume);

}
//平多
std::vector<std::string> StrategyTemplate::sell(double price, double volume, bool bStopOrder )
{
	return sendOrder(bStopOrder, DIRECTION_SHORT, OFFSET_CLOSE, price, volume);
}
//做空
std::vector<std::string> StrategyTemplate::sellshort(double price, double volume, bool bStopOrder )
{
	return sendOrder(bStopOrder, DIRECTION_SHORT, OFFSET_OPEN, price, volume);
}
//平空
std::vector<std::string> StrategyTemplate::buycover(double price, double volume, bool bStopOrder )
{
	return sendOrder(bStopOrder, DIRECTION_LONG, OFFSET_CLOSE, price, volume);
}

//总报单开平函数公用
std::vector<std::string> StrategyTemplate::sendOrder(bool bStopOrder, std::string strDirection, std::string strOffset, double price, double volume)
{
	std::vector<std::string>orderIDv;
	if (trading == true)
	{
		orderIDv = m_ctaEngine->sendOrder(bStopOrder,m_symbol, strDirection, strOffset ,price, volume, this);
		for (std::vector<std::string>::iterator it = orderIDv.begin(); it != orderIDv.end(); it++)
		{
			m_orderlistmtx.lock();
			m_orderlist.push_back(*it);
			m_orderlistmtx.unlock();
		}
		return orderIDv;
	}
	return orderIDv;
}

//撤掉这个策略所有单
void StrategyTemplate::cancelallorder()
{
	m_orderlistmtx.lock();
	for (std::vector<std::string>::iterator iter = m_orderlist.begin(); iter != m_orderlist.end(); iter++)
	{
		cancelOrder(*iter, gatewayname);
	}
	m_orderlistmtx.unlock();
}

//撤交易所的单
void StrategyTemplate::cancelOrder(std::string orderID, std::string gatewayname)
{
	m_ctaEngine->cancelOrder(orderID, gatewayname);
}

//读取历史数据
std::vector<TickData>StrategyTemplate::loadTick(std::string symbol, int days)
{
	return m_ctaEngine->loadTick(symbol, days);
}
std::vector<BarData>StrategyTemplate::loadBar(std::string symbol, int days)
{
	return m_ctaEngine->loadBar( symbol, days);
}

//获取参数的值
std::string StrategyTemplate::getparam(std::string paramname)
{
	return m_strategydata->getparam(paramname);
}

//更新参数到界面
void StrategyTemplate::putEvent()
{

	/*
	//更新仓位
	std::map<std::string, double>map = getposmap();
	for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
	{
		m_strategydata->insertvar(("pos_" + iter->first), Utils::doubletostring(iter->second));
	}
	//将参数和变量传递到界面上去
	*/
	std::shared_ptr<Event_UpdateStrategy>e = std::make_shared<Event_UpdateStrategy>();
	e->parammap = m_strategydata->getallparam();
	e->varmap = m_strategydata->getallvar();
	e->strategyname =m_strategyName;
	m_ctaEngine->PutEvent(e);
}

void StrategyTemplate::putGUI()
{
	//推送到GUI
	std::shared_ptr<Event_LoadStrategy>e = std::make_shared<Event_LoadStrategy>();
	e->parammap = m_strategydata->getallparam();
	e->varmap = m_strategydata->getallvar();
	e->strategyname = m_strategyName;
	m_ctaEngine->PutEvent(e);
}

void StrategyTemplate::checkSymbol(const char* symbolname)
{
	//读取
	changeposmap(symbolname, 0);
}
/*
void StrategyTemplate::savepostomongo()
{
	//需要update
	bson_t *query;
	bson_t *update;
	query = BCON_NEW("strategyname", BCON_UTF8(m_strategyName.c_str()));
	std::map<std::string, double>map = getposmap();
	for (std::map<std::string, double>::iterator it = map.begin(); it != map.end(); it++)
	{
		update = BCON_NEW("$set", "{", BCON_UTF8(it->first.c_str()), BCON_DOUBLE(it->second), "}");
		m_MongoCxx->updateData(query, update, "StrategyPos", "pos");
	}
}

void StrategyTemplate::loadposfrommongo()
{
	std::vector<std::string>result;

	bson_t query;

	bson_init(&query);

	m_MongoCxx->append_utf8(&query, "strategyname", m_strategyName.c_str());

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
		}
	}
}
*/
int StrategyTemplate::getpos()
{
	return m_Pos;

}

double StrategyTemplate::getpos(std::string symbol)
{
	double position;
	m_Pos_mapmtx.lock();
	if (m_pos_map.find(symbol) != m_pos_map.end())
	{
		position = m_pos_map[symbol];		//持仓
	}
	else
	{
		position = 0;
	}
	m_Pos_mapmtx.unlock();
	return position;
}

std::map<std::string, double> StrategyTemplate::getposmap()
{
	m_Pos_mapmtx.lock();
	std::map<std::string, double>map;
	std::copy(m_pos_map.begin(), m_pos_map.end(), std::inserter(map, map.begin()));
	m_Pos_mapmtx.unlock();
	return map;
}
/*
std::map<std::string, std::string> StrategyTemplate::GetVarPlotMap()
{
	std::unique_lock<std::mutex>lck(m_VarPlotmtx);
	return m_VarPlot;
}

std::map<std::string, std::string> StrategyTemplate::GetIndicatorMap()
{
	std::unique_lock<std::mutex>lck(m_VarPlotmtx);
	return m_indicatorPlot;
}
*/
void StrategyData::insertparam(std::string key, std::string value)
{
	m_mtx.lock();
	m_parammap[key] = value;
	m_mtx.unlock();
}

void StrategyData::insertvar(std::string key, std::string value)
{
	m_mtx.lock();
	m_varmap[key] = value;
	m_mtx.unlock();
}

std::string StrategyData::getparam(std::string key)
{
	std::string value;
	m_mtx.lock();
	if (m_parammap.find(key) != m_parammap.end())
	{
		value = m_parammap[key];
	}
	else
	{
		value = "Null";
	}
	m_mtx.unlock();
	return value;
}

void StrategyData::setparam(std::string key,std::string value)
{
	//找到就更新，找不大就插入
	m_mtx.lock();
	//if (m_parammap.find(key) != m_parammap.end())
	//{
		 m_parammap[key]=value;
	//}

	m_mtx.unlock();
}


void StrategyData::setvar(std::string key,std::string value)
{

	m_mtx.lock();
	//if (m_varmap.find(key) != m_varmap.end())
	//{
		m_varmap[key]=value;
	//}

	m_mtx.unlock();
}

std::string StrategyData::getvar(std::string key)
{
	std::string value;
	m_mtx.lock();
	if (m_varmap.find(key) != m_varmap.end())
	{
		value = m_varmap[key];
	}
	else
	{
		value = "Null";
	}
	m_mtx.unlock();
	return value;
}

std::map<std::string, std::string>StrategyData::getallparam()
{
	m_mtx.lock();
	std::map<std::string, std::string>map;
	std::copy(m_parammap.begin(), m_parammap.end(), std::inserter(map, map.begin()));
	m_mtx.unlock();
	return map;
}

std::map<std::string, std::string>StrategyData::getallvar()
{
	m_mtx.lock();
	std::map<std::string, std::string>map;
	std::copy(m_varmap.begin(), m_varmap.end(), std::inserter(map, map.begin()));
	m_mtx.unlock();
	return map;
}

