#include"StrategyTemplate.h"
#include"CtaEngine.h"
#include<cstdlib>
#include<string>
#include"BaseEngine.h"
#include"utility.h"
extern mongoc_uri_t* g_uri;
extern mongoc_client_pool_t* g_pool;

StrategyTemplate::StrategyTemplate(BaseEngine*ctaEngine,std::string strategyName, std::string symbol)
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

void StrategyTemplate::sync_data()
{
	std::string strFileName = m_strategyName + "_" + m_symbol;
	Global_FUC::WriteStrategyDataJson(m_strategydata->getallvar(), strFileName);
}
void StrategyTemplate::setPos(int pos)
{
	m_Pos = pos;
	m_strategydata->setvar("pos", std::to_string(pos));//�������е�posҲ��Ҫ����
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
	std::string strategyname = m_strategyName;
	m_ctaEngine->writeCtaLog("���Գ�ʼ��" + strategyname);
	std::vector<BarData>datalist = loadBar(m_symbol, initDays);
	for (std::vector<BarData>::iterator it = datalist.begin(); it != datalist.end(); it++)
	{
		onBar(*it);
	}
}
//��ʼ 
void StrategyTemplate::onStart()
{
	std::string strategyname = m_strategyName;
	m_ctaEngine->writeCtaLog("���Կ�ʼ" + strategyname);
	trading = true;
	putEvent();
}
//ֹͣ
void StrategyTemplate::onStop()
{
	std::string strategyname = m_strategyName;
	m_ctaEngine->writeCtaLog("����ֹͣ" + strategyname);
	trading = false;
	putEvent();
	//savepostomongo();
	sync_data();
}
//��������ֵ
void StrategyTemplate::checkparam(const char* paramname, const char* paramvalue)
{
	m_strategydata->insertparam(paramname, paramvalue);

}

//���²�����ֵ
void StrategyTemplate::updateParam(const char* paramname, const char* paramvalue)
{

	m_strategydata->setparam(paramname, paramvalue);
}

//���±�����ֵ
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
void StrategyTemplate::onStopOrder(std::shared_ptr<Event_StopOrder>e)
{
	putEvent();

 }
//����
std::vector<std::string> StrategyTemplate::buy(double price, double volume,bool bStopOrder )
{
	return sendOrder(bStopOrder,DIRECTION_LONG, OFFSET_OPEN, price, volume);

}
//ƽ��
std::vector<std::string> StrategyTemplate::sell(double price, double volume, bool bStopOrder )
{
	return sendOrder(bStopOrder, DIRECTION_SHORT, OFFSET_CLOSE, price, volume);
}
//����
std::vector<std::string> StrategyTemplate::sellshort(double price, double volume, bool bStopOrder )
{
	return sendOrder(bStopOrder, DIRECTION_SHORT, OFFSET_OPEN, price, volume);
}
//ƽ��
std::vector<std::string> StrategyTemplate::buycover(double price, double volume, bool bStopOrder )
{
	return sendOrder(bStopOrder, DIRECTION_LONG, OFFSET_CLOSE, price, volume);
}

//�ܱ�����ƽ��������
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

//��������������е�
void StrategyTemplate::cancelAllOrder()
{
	/*
	m_orderlistmtx.lock();
	for (std::vector<std::string>::iterator iter = m_orderlist.begin(); iter != m_orderlist.end(); iter++)
	{
		cancelOrder(*iter, gatewayname);
	}
	m_orderlistmtx.unlock();
	*/
	m_ctaEngine->cancelAllOrder(m_strategyName);
}

//���������ĵ�
void StrategyTemplate::cancelOrder(std::string orderID, std::string gatewayname)
{
	m_ctaEngine->cancelOrder(orderID, gatewayname);
}

//��ȡ��ʷ����
std::vector<TickData>StrategyTemplate::loadTick(std::string symbol, int days)
{
	return m_ctaEngine->loadTick(symbol, days);
}
std::vector<BarData>StrategyTemplate::loadBar(std::string symbol, int days)
{
	return m_ctaEngine->loadBar( symbol, days);
}

//��ȡ������ֵ
std::string StrategyTemplate::getparam(std::string paramname)
{
	return m_strategydata->getparam(paramname);
}

//���²���������
void StrategyTemplate::putEvent()
{

	/*
	//���²�λ
	std::map<std::string, double>map = getposmap();
	for (std::map<std::string, double>::iterator iter = map.begin(); iter != map.end(); iter++)
	{
		m_strategydata->insertvar(("pos_" + iter->first), Utils::doubletostring(iter->second));
	}
	//�������ͱ������ݵ�������ȥ
	*/
	std::shared_ptr<Event_UpdateStrategy>e = std::make_shared<Event_UpdateStrategy>();
	e->parammap = m_strategydata->getallparam();
	e->varmap = m_strategydata->getallvar();
	e->strategyname =m_strategyName;
	m_ctaEngine->PutEvent(e);
}

void StrategyTemplate::putGUI()
{
	//���͵�GUI
	std::shared_ptr<Event_LoadStrategy>e = std::make_shared<Event_LoadStrategy>();
	e->parammap = m_strategydata->getallparam();
	e->varmap = m_strategydata->getallvar();
	e->strategyname = m_strategyName;
	m_ctaEngine->PutEvent(e);
}

void StrategyTemplate::checkSymbol(const char* symbolname)
{
	//��ȡ
	changeposmap(symbolname, 0);
}
/*
void StrategyTemplate::savepostomongo()
{
	//��Ҫupdate
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
	//�ҵ��͸��£��Ҳ���Ͳ���
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

