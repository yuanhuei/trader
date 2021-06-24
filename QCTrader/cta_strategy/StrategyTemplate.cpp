#include"StrategyTemplate.h"

StrategyTemplate::StrategyTemplate(CTAmanager*ctamanager)
{
	m_ctamanager = ctamanager;
	tickDbName = "";
	BarDbName = "";
	gatewayname = "";
	inited = false;
	trading = false;
	TradingMode = RealMode;
	unitLimit = 2;
	m_strategydata = new StrategyData();
	m_algorithm = new algorithmOrder(unitLimit, TradingMode, this);
	m_MongoCxx = new MongoCxx(ctamanager->m_pool);
}

StrategyTemplate::~StrategyTemplate()
{
	delete m_strategydata;
	delete m_MongoCxx;
}

void StrategyTemplate::changeposmap(std::string symbol, double pos)
{
	m_Pos_mapmtx.lock();
	m_pos_map[symbol] = pos;
	m_Pos_mapmtx.unlock();
}

//��ʼ��
void StrategyTemplate::onInit()
{
	//Ĭ��ʹ��bar ��Ҫʹ��tick�Լ��޸�
	std::string strategyname = getparam("name");
	m_ctamanager->writeCtaLog("���Գ�ʼ��" + strategyname, gatewayname);
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
	loadposfrommongo();  //��ȡ�ֲ�
	std::map<std::string, double>map = getposmap();
	for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
	{
		m_algorithm->set_supposedPos(iter->first, iter->second);
	}
	inited = true;
	putEvent();
}
//��ʼ 
void StrategyTemplate::onStart()
{
	std::string strategyname = getparam("name");
	m_ctamanager->writeCtaLog("���Կ�ʼ" + strategyname, gatewayname);
	trading = true;
	putEvent();
}
//ֹͣ
void StrategyTemplate::onStop()
{
	std::string strategyname = getparam("name");
	m_ctamanager->writeCtaLog("����ֹͣ" + strategyname, gatewayname);
	trading = false;
	putEvent();
	savepostomongo();
}
//��������ֵ
void StrategyTemplate::checkparam(const char* paramname, const char* paramvalue)
{
	m_strategydata->insertparam(paramname, paramvalue);
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
//�����ص�
void StrategyTemplate::onOrder(std::shared_ptr<Event_Order>e)
{
	putEvent();
}
//�ɽ��ص�
void StrategyTemplate::onTrade(std::shared_ptr<Event_Trade>e)
{
	putEvent();
}

//����
std::vector<std::string> StrategyTemplate::buy(std::string symbol, double price, double volume)
{
	return sendOrder(symbol, CTAORDER_BUY, price, volume);
}
//ƽ��
std::vector<std::string> StrategyTemplate::sell(std::string symbol, double price, double volume)
{
	return sendOrder(symbol, CTAORDER_SELL, price, volume);
}
//����
std::vector<std::string> StrategyTemplate::sellshort(std::string symbol, double price, double volume)
{
	return sendOrder(symbol, CTAORDER_SHORT, price, volume);
}
//ƽ��
std::vector<std::string> StrategyTemplate::buycover(std::string symbol, double price, double volume)
{
	return sendOrder(symbol, CTAORDER_COVER, price, volume);
}

//�ܱ�����ƽ��������
std::vector<std::string> StrategyTemplate::sendOrder(std::string symbol, std::string orderType, double price, double volume)
{
	std::vector<std::string>orderIDv;
	if (trading == true)
	{
		orderIDv = m_ctamanager->sendOrder(symbol, orderType, price, volume, this);
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

//��������������е�
void StrategyTemplate::cancelallorder()
{
	m_orderlistmtx.lock();
	for (std::vector<std::string>::iterator iter = m_orderlist.begin(); iter != m_orderlist.end(); iter++)
	{
		cancelOrder(*iter, gatewayname);
	}
	m_orderlistmtx.unlock();
}

//���������ĵ�
void StrategyTemplate::cancelOrder(std::string orderID, std::string gatewayname)
{
	m_ctamanager->cancelOrder(orderID, gatewayname);
}

//��ȡ��ʷ����
std::vector<TickData>StrategyTemplate::loadTick(std::string symbol, int days)
{
	return m_ctamanager->loadTick(BarDbName, symbol, days);
}
std::vector<BarData>StrategyTemplate::loadBar(std::string symbol, int days)
{
	return m_ctamanager->loadBar(BarDbName, symbol, days);
}

//��ȡ������ֵ
std::string StrategyTemplate::getparam(std::string paramname)
{
	return m_strategydata->getparam(paramname);
}

//���²���������
void StrategyTemplate::putEvent()
{
	m_strategydata->insertvar("inited", Utils::booltostring(inited));
	m_strategydata->insertvar("trading", Utils::booltostring(trading));
	//���²�λ
	std::map<std::string, double>map = getposmap();
	for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
	{
		m_strategydata->insertvar(("pos_" + iter->first), Utils::doubletostring(iter->second));
	}
	//�������ͱ������ݵ�������ȥ
	std::shared_ptr<Event_UpdateStrategy>e = std::make_shared<Event_UpdateStrategy>();
	e->parammap = m_strategydata->getallparam();
	e->varmap = m_strategydata->getallvar();
	e->strategyname = m_strategydata->getparam("name");
	m_ctamanager->PutEvent(e);
}

void StrategyTemplate::putGUI()
{
	//���͵�GUI
	std::shared_ptr<Event_LoadStrategy>e = std::make_shared<Event_LoadStrategy>();
	e->parammap = m_strategydata->getallparam();
	e->varmap = m_strategydata->getallvar();
	e->strategyname = m_strategydata->getparam("name");
	m_ctamanager->PutEvent(e);
}

void StrategyTemplate::checkSymbol(const char* symbolname)
{
	//��ȡ
	changeposmap(symbolname, 0);
}

void StrategyTemplate::savepostomongo()
{
	//��Ҫupdate
	bson_t *query;
	bson_t *update;
	query = BCON_NEW("strategyname", BCON_UTF8(m_strategydata->getparam("name").c_str()));
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
		}
	}
}

double StrategyTemplate::getpos(std::string symbol)
{
	double position;
	m_Pos_mapmtx.lock();
	if (m_pos_map.find(symbol) != m_pos_map.end())
	{
		position = m_pos_map[symbol];		//�ֲ�
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

