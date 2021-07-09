
#include"CtaEngine.h"
#include"Strategies/turtlebreak.h"
#include <json/json.h>
#include<qstring.h>

#include <iostream>  
#include <fstream>  
#include"./cta_strategy/strategies/BollChannelStrategy.h"

extern mongoc_uri_t* g_uri;
extern mongoc_client_pool_t* g_pool;


typedef StrategyTemplate* (*Dllfun)(CtaEngine*);
typedef int(*Release)();

CtaEngine::CtaEngine(Gatewaymanager* gatewaymanager, EventEngine* eventengine, riskmanager* riskmanager)
{
	m_eventengine = eventengine;
	m_gatewaymanager = gatewaymanager;
	m_riskmanager = riskmanager;
	m_connectstatus = false;//CTP连接



	//加载策略
	loadStrategy();
	registerEvent();

	m_portfolio = new Portfolio(eventengine, gatewaymanager);

	//is_LoadStrategy = false;
}
CtaEngine::~CtaEngine()
{


	std::map<std::string, StrategyTemplate*>::iterator iter;
	for (iter = m_strategymap.begin(); iter != m_strategymap.end(); iter++)
	{
		StrategyTemplate* strP = iter->second;
		if (strP!=nullptr)
		{
			//删除指针
			delete strP;
			strP = nullptr;
		}

	}
	/*
	for (std::map<std::string, HINSTANCE>::iterator it = dllmap.begin(); it != dllmap.end(); it++)
	{
		Release result = (Release)GetProcAddress(it->second, "ReleaseStrategy");//析构策略
		result();
	}*/
	delete m_portfolio;
}
//读取策略配置文件
void CtaEngine::ReadStrategyConfFileJson()
{
	Json::Reader reader;
	Json::Value root;

	//从文件中读取，保证当前文件有demo.json文件  
	std::ifstream in("./Strategy/cta_strategy_setting.json", std::ios::binary);

	if (!in.is_open())
	{
		writeCtaLog("打开策略配置文件失败");
		return;
	}

	if (reader.parse(in, root))
	{
		writeCtaLog("打开策略配置文件成功");
		for (int i = 0; i < root.size(); i++)
		{
			//读取策略名称和合约名称

			std::string StrategyName = root[i]["strategy_name"].asString();
			std::string vt_symbol = root[i]["vt_symbol"].asString();
			std::string ClassName = root[i]["class_name"].asString();
			if ((StrategyName.length() < 1 || vt_symbol.length()) < 1 || ClassName.length() < 1)
			{
				this->writeCtaLog("配置文件策略信息不全");
				return;
			}

			//读取策略配置信息 
			std::map<std::string, float> settingMap;
			Json::Value::Members members;
			members = root[i]["setting"].getMemberNames();
			//std::vector<std::string> settingKeys= root["setting"].getMemberNames();
			for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // 遍历每个key
			{
				std::string strKey = *iterMember;
				float fValue = root[i]["setting"][strKey.c_str()].asFloat();
				/*
				if (root[i]["setting"][strKey.c_str()].isString())
				{
					fValue = root[i]["setting"][strKey.c_str()].asString();
				}
				else
					fValue = root[i]["setting"][strKey.c_str()].asFloat();
				*/
				//if(fValue.ist)
				settingMap.insert({ strKey,  fValue });

			}
			//插入到策略配置map中
			m_strategyConfigInfo_map[StrategyName + "_" + vt_symbol + "_" + ClassName] = settingMap;
		}

	}
	else
	{
		this->writeCtaLog("解析策略配置文件失败");
	}

	in.close();
}
//读取策略数据文件
void CtaEngine::ReadStrategyDataJson(std::string strfileName)
{
	Json::Reader reader;
	Json::Value root;

	//从文件中读取，保证当前文件有demo.json文件  
	std::ifstream in(strfileName, std::ios::binary);

	if (!in.is_open())
	{
		this->writeCtaLog("打开策略数据文件失败");
		return;
	}

	if (reader.parse(in, root))
	{
		this->writeCtaLog("打开策略数据文件成功");
		for (int i = 0; i < root.size(); i++)
		{
			//读取策略名称和合约名称

			std::string StrategyName = root[i]["strategy_name"].asString();
			std::string vt_symbol = root[i]["vt_symbol"].asString();
			std::string ClassName = root[i]["class_name"].asString();
			if ((StrategyName.length() < 1 || vt_symbol.length()) < 1 || ClassName.length() < 1)
			{
				this->writeCtaLog("策略数据文件信息不全");
				return;
			}

			//读取策略数据信息 
			std::map<std::string, float> settingMap;
			Json::Value::Members members;
			members = root[i]["setting"].getMemberNames();
			//std::vector<std::string> settingKeys= root["setting"].getMemberNames();
			for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // 遍历每个key
			{
				std::string strKey = *iterMember;
				float fValue = root[i]["setting"][strKey.c_str()].asFloat();
				/*
				if (root[i]["setting"][strKey.c_str()].isString())
				{
					fValue = root[i]["setting"][strKey.c_str()].asString();
				}
				else
					fValue = root[i]["setting"][strKey.c_str()].asFloat();
				*/
				//if(fValue.ist)
				settingMap.insert({ strKey,  fValue });

			}
			//插入到策略数据map中
			m_strategyData_map[StrategyName + "_" + vt_symbol + "_" + ClassName] = settingMap;
		}

	}
	else
	{
	this->writeCtaLog("解析策略数据文件失败");
	}

	in.close();
}
//写变量文件
void CtaEngine::WriteStrategyDataJson(std::map<std::string, std::string>dataMap,std::string fileName)
{


	Json::Value root;

	//根节点属性
	std::map<std::string, std::string>::iterator iter;
	for (iter = dataMap.begin(); iter != dataMap.end(); iter++)
	{
		std::string varName = iter->first;
		std::string varValue = iter->second;
		root[varName] = Json::Value(varValue);
	}
	
	Json::StyledWriter sw;

	//输出到文件
	std::ofstream os;
	std::string file = "./Strategy/cta_strategy_data_" + fileName + ".json";
	os.open(file);
	os << sw.write(root);
	os.close();


}

//加载策略
void CtaEngine::loadStrategy()
{

	//读取策略配置文件（存放策略配置参数，包括策略名，合约，类名还有配置参数
	ReadStrategyConfFileJson();
	//读取变量文件信息，这里的文件名称包含策略名和合约，与StrategyTemplate::sync_data()写的文件名称要一致
	std::map<std::string, std::map<std::string, float>>::iterator iter;
	for (iter = m_strategyConfigInfo_map.begin(); iter!=m_strategyConfigInfo_map.end(); iter++)
	{
		QString str = QString::fromStdString(iter->first).section("_", 0, 1);
		//循环读取策略数据文件（存放策略变量,例如仓位）
		std::string strfileName = "./Strategy/cta_strategy_data_" + str.toStdString() + ".json";
		
		ReadStrategyDataJson(strfileName);

	}


	if (m_strategyConfigInfo_map.size() == 0)
	{
		this->writeCtaLog("无策略在配置文件中");
		return;

	}
	else//根据策略配置文件生成策略集,加载策略配置和策略变量
	{
		std::set<std::string> symbolSet;//把策略中所关联的合约都放到一个set里面，后面会统一订阅合约
		std::map<std::string, std::map<std::string, float>>::iterator it;
		for (it = m_strategyConfigInfo_map.begin(); it != m_strategyConfigInfo_map.end(); it++)
		{
			QString strStrategy = QString::fromStdString(it->first).section("_", 0, 0);
			QString strSymbol = QString::fromStdString(it->first).section("_", 1, 1);
			QString strClassname = QString::fromStdString(it->first).section("_", 2, 2);
			std::string strStrategyName = strStrategy.toStdString();
			std::string strSymbolName = strSymbol.toStdString();
			std::string strClassName = strClassname.toStdString();
			std::map<std::string, float>settingMap = it->second;

			symbolSet.insert(strSymbolName);
			//生成策略
			StrategyTemplate* strategy_ptr;
			HINSTANCE his;
			if (strStrategyName.find(".dll")!= strStrategyName.npos)//策略通过DLL文件提供
			{
				std::string strategy = "./Strategy/" + strStrategyName;//策略DLL文件
				his = LoadLibraryA(strategy.c_str());//加载一个策略
				if (his == NULL)
				{
					//没有加载进来DLL
					std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
					e->msg = "无法读取策略" + strategy;
					m_eventengine->Put(e);

					return;
				}
				Dllfun dll = (Dllfun)GetProcAddress(his, "CreateStrategy");//创建策略
				if (dll == NULL)
				{
					//没有加载进来DLL
					std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
					e->msg = "无法创建策略函数" + strategy;
					m_eventengine->Put(e);
					FreeLibrary(his);
					return;
				}
				strategy_ptr = dll(this);
				std::string strName = strStrategyName + "_" + strSymbolName;
				dllmap.insert(std::pair<std::string, HINSTANCE>(strName, his));

			}
			else//通过在源代码中提供的策略
			{
				if (strStrategyName == "BollChannelStrategy")
					strategy_ptr = new BollChannelStrategy(this, strStrategyName, strSymbolName);
				else
				{
					this->writeCtaLog("没有相关的策略提供");
					return;
				}

			}



			//赋值参数给策略中的strategeData,策略中必须生成strategeData,里面包含了配置参数和变量参数，下面是根据配置文件更新。
			for (std::map<std::string, float>::iterator it = settingMap.begin(); it != settingMap.end(); it++)
			{
				//遍历parameter
				std::string value = std::to_string(it->second);
				strategy_ptr->updateParam(it->first.c_str(), value.c_str());
			}
			//赋值变量给策略中的strategeData
			if (m_strategyData_map.find(iter->first) != m_strategyData_map.end())
			{
				std::map<std::string, float>varMap = m_strategyData_map[iter->first];//变量map
				for (std::map<std::string, float>::iterator it = varMap.begin(); it != varMap.end(); it++)
				{
					//遍历var
					std::string value = std::to_string(it->second);
					strategy_ptr->updateVar(it->first.c_str(), value.c_str());
				}

			}
			/*
			for (std::map<std::string, std::map<std::string, float>>::iterator it = m_strategyData_map.begin(); it != m_strategyData_map.end(); it++)
			{
				std::string str = it->first;
				if (str == (strStrategyName + "_" + strSymbolName))//找到对应的策略合约
				{
					std::map<std::string, float>varMap = it->second;//变量map
					for (std::map<std::string, float>::iterator it = varMap.begin(); it != varMap.end(); it++)
					{
						//遍历var
						std::string value = std::to_string(it->second);
						strategy_ptr->updateVar(it->first.c_str(), value.c_str());
					}
				}

			}*/
			//把读取到的配置和变量值通过strategeData更新到策略的变量上去
			strategy_ptr->updateSetting();

			//插入pos_map
			//strategy_ptr->checkSymbol(strSymbolName.c_str());
			/*
			for (std::vector<std::string>::iterator iter = symbol_v.begin(); iter != symbol_v.end(); iter++)
			{
				strategy_ptr->checkSymbol((*iter).c_str());
			}

			if (strategy_ptr->getparam("name") == "Null")
			{
				this->writeCtaLog("策略中有一个没有给策略起名！", strategy_ptr->gatewayname);
			}
			else
			{*/
			//strategy_ptr->putEvent();
			//strategy_ptr->putGUI();

			m_tickstrategymtx.lock();
			//for (std::vector<std::string>::iterator iter = symbol_v.begin(); iter != symbol_v.end(); iter++)
			//{
			//更新m_tickstrategymap
			if (m_tickstrategymap.find(strSymbolName) == m_tickstrategymap.end())
			{
				//没有合约存在map中，创建一个
				std::vector<StrategyTemplate*>strategy_v;
				strategy_v.push_back(strategy_ptr);
				m_tickstrategymap.insert(std::pair<std::string, std::vector<StrategyTemplate*>>(strSymbolName, strategy_v));
			}
			else//已经有合约在其中，加入其中
			{
				m_tickstrategymap[strSymbolName].push_back(strategy_ptr);
			}
			//}
			m_tickstrategymtx.unlock();
			//插入策略m_strategymap,   key是策略名+合约名+类名，值是之前new出来的 指向策略对象的指针
			//
			std::string strName = strStrategyName + "_" + strSymbolName+"_"+ strClassName;
			//m_strategymap.insert(std::pair<std::string, StrategyTemplate*>(strategy_ptr->getparam("name"), strategy_ptr));//策略名和策略
			
			//生成策略集
			m_strategymap.insert(std::pair<std::string, StrategyTemplate*>(strName, strategy_ptr));
		}

			//订阅合约
		std::set<std::string>::iterator iter;
		for (iter=symbolSet.begin();iter!=symbolSet.end();iter++)
		{
			std::string strSymbolName = *iter;
			std::shared_ptr<Event_Contract>contract = m_gatewaymanager->getContract(strSymbolName);
			SubscribeReq req;
			req.symbol = contract->symbol;
			req.exchange = contract->exchange;
			/*
			if (strategy_ptr->getparam("currency") != "Null" && strategy_ptr->getparam("productClass") != "Null")
			{
				req.currency = strategy_ptr->getparam("currency");
				req.productClass = strategy_ptr->getparam("productClass");
			}*/
			m_gatewaymanager->subscribe(req, "CTP");// strategy_ptr->getparam("gatewayname"));
		}

	}


}
//初始化
void CtaEngine::initStrategy(std::string name)
{
	m_portfolio->calculate();
	m_strategymtx.lock();
	if (m_strategymap.find(name) != m_strategymap.end())
	{
		//判断策略加载进来了
		StrategyTemplate* temp = m_strategymap[name];
		if (temp->inited == false)
		{
			temp->onInit();
		}
		else
		{
			this->writeCtaLog("请勿重复初始化", temp->gatewayname);
		}
	}
	else
	{
		this->writeCtaLog("策略实例不存在", "");
	}
	m_strategymtx.unlock();
}
//开始策略
void CtaEngine::startStrategy(std::string name)
{
	m_strategymtx.lock();
	if (m_strategymap.find(name) != m_strategymap.end())
	{
		StrategyTemplate* temp = m_strategymap[name];

		if (temp->trading == false && temp->inited == true)
		{
			temp->onStart();
		}
		else if (temp->inited == false)
		{
			this->writeCtaLog("策略还没初始化");
		}
	}
	else
	{
		this->writeCtaLog("策略实例不存在");
	}
	m_strategymtx.unlock();
}
//停止策略
void CtaEngine::stopStrategy(std::string name)
{
	m_strategymtx.lock();
	if (m_strategymap.find(name) != m_strategymap.end())
	{
		StrategyTemplate* temp = m_strategymap[name];
		if (temp->trading == true)
		{
			temp->onStop();
			//撤单
			temp->cancelallorder();
		}
	}
	else
	{
		this->writeCtaLog("策略实例不存在", "");
	}
	m_strategymtx.unlock();
}
//算法交易
void CtaEngine::changesupposedpos(std::string strategyname, std::string symbol, double pos)
{
	std::unique_lock<std::mutex>lck(m_strategymtx);
	m_strategymap[strategyname]->m_algorithm->set_supposedPos(symbol, pos);
}

void CtaEngine::initallStrategy()
{
	m_strategymtx.lock();
	for (std::map<std::string, StrategyTemplate*>::iterator it = m_strategymap.begin(); it != m_strategymap.end(); it++)
	{
		StrategyTemplate* temp = it->second;
		if (temp->inited == false)
		{
			temp->onInit();
		}
		else
		{
			this->writeCtaLog("请勿重复初始化", temp->gatewayname);
		}
	}
	m_strategymtx.unlock();
}
//启动所有策略
void CtaEngine::startallStrategy()
{
	m_strategymtx.lock();
	for (std::map<std::string, StrategyTemplate*>::iterator it = m_strategymap.begin(); it != m_strategymap.end(); it++)
	{
		StrategyTemplate* temp = it->second;
		if (temp->trading == false && temp->inited == true)
		{
			temp->onStart();
		}
		else
		{
			this->writeCtaLog("策略未初始化");
		}
	}
	m_strategymtx.unlock();
}
//停止所有策略
void CtaEngine::stopallStrategy()
{
	m_strategymtx.lock();
	for (std::map<std::string, StrategyTemplate*>::iterator it = m_strategymap.begin(); it != m_strategymap.end(); it++)
	{
		StrategyTemplate* temp = it->second;
		if (temp->trading == true)
		{
			temp->onStop();

		}
	}
	m_strategymtx.unlock();

	//撤单
	m_orderStrategymtx.lock();
	for (std::map<std::string, StrategyTemplate*>::iterator it = m_orderStrategymap.begin(); it != m_orderStrategymap.end(); it++)
	{
		cancelOrder(it->first, "CTP");
	}
	m_orderStrategymtx.unlock();
}

/******************处理函数***************************/
void CtaEngine::procecssTickEvent(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Tick> etick = std::static_pointer_cast<Event_Tick>(e);
	TickData tick;
	tick.askprice1 = etick->askprice1;
	tick.askprice2 = etick->askprice2;
	tick.askprice3 = etick->askprice3;
	tick.askprice4 = etick->askprice4;
	tick.askprice5 = etick->askprice5;
	tick.askvolume1 = etick->askvolume1;
	tick.askvolume2 = etick->askvolume2;
	tick.askvolume3 = etick->askvolume3;
	tick.askvolume4 = etick->askvolume4;
	tick.askvolume5 = etick->askvolume5;
	tick.bidprice1 = etick->bidprice1;
	tick.bidprice2 = etick->bidprice2;
	tick.bidprice3 = etick->bidprice3;
	tick.bidprice4 = etick->bidprice4;
	tick.bidprice5 = etick->bidprice5;
	tick.bidvolume1 = etick->bidvolume1;
	tick.bidvolume2 = etick->bidvolume2;
	tick.bidvolume3 = etick->bidvolume3;
	tick.bidvolume4 = etick->bidvolume4;
	tick.bidvolume5 = etick->bidvolume5;
	tick.date = etick->date;
	tick.exchange = etick->exchange;
	tick.gatewayname = etick->gatewayname;
	tick.highPrice = etick->highPrice;
	tick.lastprice = etick->lastprice;
	tick.lowerLimit = etick->lowerLimit;
	tick.lowPrice = etick->lowPrice;
	tick.openInterest = etick->openInterest;
	tick.openPrice = etick->openPrice;
	tick.preClosePrice = etick->preClosePrice;
	tick.symbol = etick->symbol;
	tick.time = etick->time;
	tick.unixdatetime = Utils::getsystemunixdatetime(tick.time, "ms");
	tick.upperLimit = etick->upperLimit;
	tick.volume = etick->volume;

	m_tickstrategymtx.lock();
	if (m_tickstrategymap.find(etick->symbol) != m_tickstrategymap.end())
	{
		//有的
		for (std::vector<StrategyTemplate*>::iterator it = m_tickstrategymap[etick->symbol].begin(); it != m_tickstrategymap[etick->symbol].end(); it++)
		{
			//循环所有策略实例指针
			if ((*it)->inited == true)
			{
				(*it)->onTick(tick);
			}
		}
	}
	m_tickstrategymtx.unlock();
}

void CtaEngine::processOrderEvent(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Order> eOrder = std::static_pointer_cast<Event_Order>(e);
	m_orderStrategymtx.lock();
	if (m_orderStrategymap.find(eOrder->orderID) != m_orderStrategymap.end())
	{
		m_orderStrategymap[eOrder->orderID]->onOrder(eOrder);
	}
	m_orderStrategymtx.unlock();
}

void CtaEngine::processTradeEvent(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Trade> eTrade = std::static_pointer_cast<Event_Trade>(e);

	m_portfolio->calculate_memory(eTrade, m_orderStrategymap);//计算portfolio


	//更新策略内部的pos数据
	m_orderStrategymtx.lock();
	if (m_orderStrategymap.find(eTrade->orderID) != m_orderStrategymap.end())
	{
		if (eTrade->direction == DIRECTION_LONG)
		{
			m_orderStrategymap[eTrade->orderID]->setPos( m_orderStrategymap[eTrade->orderID]->getpos() + eTrade->volume);
		}
		else
		{
			m_orderStrategymap[eTrade->orderID]->setPos( m_orderStrategymap[eTrade->orderID]->getpos() - eTrade->volume);
		}
		m_orderStrategymap[eTrade->orderID]->onTrade(eTrade);

		//保存交易记录
		savetraderecord(m_orderStrategymap[eTrade->orderID]->m_strategyName, eTrade);
		//保存仓位信息到配置文件
		m_orderStrategymap[eTrade->orderID]->sync_data();
	}
	m_orderStrategymtx.unlock();
	

	//更新持仓m_symbolpositionbuffer
	m_tickstrategymtx.lock();
	if (m_tickstrategymap.find(eTrade->symbol) != m_tickstrategymap.end())
	{
		m_symbolpositionmtx.lock();
		PositionBuffer* bufferPtr;
		if (m_symbolpositionbuffer.find(eTrade->symbol) != m_symbolpositionbuffer.end())
		{
			bufferPtr = &m_symbolpositionbuffer[eTrade->symbol];
		}
		else
		{
			PositionBuffer positionbuffer;
			bufferPtr = &positionbuffer;
		}
		if (eTrade->direction == DIRECTION_LONG)//多
		{
			if (eTrade->offset == OFFSET_OPEN)
			{
				bufferPtr->longposition += eTrade->volume;
				bufferPtr->longtodayposition += eTrade->volume;
			}
			else if (eTrade->offset == OFFSET_CLOSETODAY)
			{
				bufferPtr->shortposition -= eTrade->volume;
				bufferPtr->shorttodayposition -= eTrade->volume;
			}
			else
			{
				bufferPtr->shortposition -= eTrade->volume;
				bufferPtr->shortydposition -= eTrade->volume;

			}
		}
		else if (eTrade->direction == DIRECTION_SHORT)//空
		{
			if (eTrade->offset == OFFSET_OPEN)
			{
				bufferPtr->shortposition += eTrade->volume;
				bufferPtr->shorttodayposition += eTrade->volume;
			}
			else if (eTrade->offset == OFFSET_CLOSETODAY)
			{
				bufferPtr->longposition -= eTrade->volume;
				bufferPtr->longtodayposition -= eTrade->volume;
			}
			else
			{
				bufferPtr->longposition -= eTrade->volume;
				bufferPtr->longydposition -= eTrade->volume;
			}
		}
		if (m_symbolpositionbuffer.find(eTrade->symbol) != m_symbolpositionbuffer.end())
		{
			//有的话直接更新
			m_symbolpositionbuffer[eTrade->symbol] = *bufferPtr;
		}
		else
		{
			//没有的话插入一个
			m_symbolpositionbuffer.insert(std::pair<std::string, PositionBuffer>(eTrade->symbol, *bufferPtr));
		}
		m_symbolpositionmtx.unlock();
	}
	m_tickstrategymtx.unlock();
}

void CtaEngine::processPositionEvent(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Position> ePosition = std::static_pointer_cast<Event_Position>(e);
	m_tickstrategymtx.lock();
	if (m_tickstrategymap.find(ePosition->symbol) != m_tickstrategymap.end())
	{
		m_symbolpositionmtx.lock();
		if (m_symbolpositionbuffer.find(ePosition->symbol) != m_symbolpositionbuffer.end())
		{	//有就直接更新
			PositionBuffer positionbuffer;
			if (ePosition->direction == DIRECTION_LONG)//多
			{
				positionbuffer.longposition = ePosition->position;
				positionbuffer.longydposition = ePosition->ydPosition;
				positionbuffer.longtodayposition = ePosition->todayPosition;
				positionbuffer.shortposition = m_symbolpositionbuffer[ePosition->symbol].shortposition;
				positionbuffer.shortydposition = m_symbolpositionbuffer[ePosition->symbol].shortydposition;
				positionbuffer.shorttodayposition = m_symbolpositionbuffer[ePosition->symbol].shorttodayposition;
			}
			else if (ePosition->direction == DIRECTION_SHORT)//空
			{
				positionbuffer.longposition = m_symbolpositionbuffer[ePosition->symbol].longposition;
				positionbuffer.longydposition = m_symbolpositionbuffer[ePosition->symbol].longydposition;
				positionbuffer.longtodayposition = m_symbolpositionbuffer[ePosition->symbol].longtodayposition;
				positionbuffer.shortposition = ePosition->position;
				positionbuffer.shortydposition = ePosition->ydPosition;
				positionbuffer.shorttodayposition = ePosition->todayPosition;
			}
			m_symbolpositionbuffer[ePosition->symbol] = positionbuffer;
		}
		else
		{	//没有就创建一个再更新进去
			PositionBuffer positionbuffer;
			if (ePosition->direction == DIRECTION_LONG)//多
			{
				positionbuffer.longposition = ePosition->position;
				positionbuffer.longydposition = ePosition->ydPosition;
				positionbuffer.longtodayposition = ePosition->todayPosition;

			}
			else if (ePosition->direction == DIRECTION_SHORT)//空
			{
				positionbuffer.shortposition = ePosition->position;
				positionbuffer.shortydposition = ePosition->ydPosition;
				positionbuffer.shorttodayposition = ePosition->todayPosition;
			}
			m_symbolpositionbuffer.insert(std::pair<std::string, PositionBuffer>(ePosition->symbol, positionbuffer));
		}
		m_symbolpositionmtx.unlock();
	}
	m_tickstrategymtx.unlock();
}
/*
void CtaEngine::showLog(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Log> elog = std::static_pointer_cast<Event_Log>(e);
	if (elog->gatewayname == "CTP" && elog->msg == "行情服务器登录完成")
	{
		m_connectstatus = true;
		m_tickstrategymtx.lock();
		for (std::map<std::string, std::vector<StrategyTemplate*>>::iterator it = m_tickstrategymap.begin(); it != m_tickstrategymap.end(); it++)
		{
			std::shared_ptr<Event_Contract>contract = m_gatewaymanager->getContract(it->first);
			SubscribeReq req;
			req.symbol = contract->symbol;
			req.exchange = contract->exchange;
			m_gatewaymanager->subscribe(req, "CTP");
		}
		m_tickstrategymtx.unlock();
	}
}
*/
//设置了定时器，非交易时间断开连接，交易时间自动连接
void CtaEngine::autoConnect(std::shared_ptr<Event>e)
{
	auto nowtime2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	if (((nowtime2 > Utils::timetounixtimestamp(2, 31, 0)) && (nowtime2 < Utils::timetounixtimestamp(8, 50, 0)))
		|| ((nowtime2 > Utils::timetounixtimestamp(15, 01, 0)) && (nowtime2 < Utils::timetounixtimestamp(20, 50, 0))))
	{
		if (m_connectstatus == true)
		{
			m_connectstatus = false;

			writeCtaLog("CTP接口主动断开连接！", "CTP");
			m_gatewaymanager->close("CTP");
			m_riskmanager->clearTradeCount();
		}
	}
	else
	{
		//连接
		if (m_connectstatus == false)
		{
			m_connectstatus = true;
			writeCtaLog("CTP接口主动连接！", "CTP");

			m_gatewaymanager->connect("CTP");
		}
	}
}
/****************** 注册***************************/

void CtaEngine::registerEvent()
{
	m_eventengine->RegEvent(EVENT_TICK, std::bind(&CtaEngine::procecssTickEvent, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_ORDER, std::bind(&CtaEngine::processOrderEvent, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TRADE, std::bind(&CtaEngine::processTradeEvent, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_POSITION, std::bind(&CtaEngine::processPositionEvent, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TIMER, std::bind(&CtaEngine::autoConnect, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_LOG, std::bind(&CtaEngine::showLog, this, std::placeholders::_1));
}
/******************策略调用***************************/

std::vector<std::string>CtaEngine::sendOrder(std::string symbol, std::string orderType, double price, double volume, StrategyTemplate* Strategy)
{
	if (Strategy->trading == true)
	{
		OrderReq req;
		req.symbol = symbol;
		req.exchange = m_gatewaymanager->getContract(symbol)->exchange;
		req.price = price;
		req.volume = volume;

		if (m_riskmanager->checkRisk(req) == false)
		{
			std::vector<std::string>result;
			return result;
		}

		if (Strategy->getparam("currency") != "Null" && Strategy->getparam("productClass") != "Null")
		{
			req.currency = Strategy->getparam("currency");
			req.productClass = Strategy->getparam("productClass");
		}
		req.priceType = PRICETYPE_LIMITPRICE;//限价单
		if (orderType == CTAORDER_BUY)
		{
			req.direction = DIRECTION_LONG;//做多
			req.offset = OFFSET_OPEN;//开仓
			//
			//发单
			std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
			m_orderStrategymtx.lock();
			m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
			m_orderStrategymtx.unlock();
			writeCtaLog("策略" + Strategy->m_strategyName + "发出委托" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
			std::vector<std::string>result;
			result.push_back(orderID);
			return result;
		}
		else if (orderType == CTAORDER_SELL)
		{
			req.direction = DIRECTION_SHORT;//平多
			if (m_gatewaymanager->getContract(symbol)->exchange != EXCHANGE_SHFE)
			{
				req.offset = OFFSET_CLOSE;
				std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
				m_orderStrategymtx.lock();
				m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
				m_orderStrategymtx.unlock();
				writeCtaLog("策略" + Strategy->m_strategyName + "发出委托" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
				std::vector<std::string>result;
				result.push_back(orderID);
				return result;
			}
			else
			{
				//区分平今平昨
				//先判断昨仓够不够用
				if ((m_symbolpositionbuffer[symbol].longydposition - volume) >= 0)
				{
					//昨仓够用直接平昨
					req.offset = OFFSET_CLOSEYESTERDAY;
					std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
					m_orderStrategymtx.lock();
					m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
					m_orderStrategymtx.unlock();
					writeCtaLog("策略" + Strategy->m_strategyName + "发出委托(只平昨)" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
					std::vector<std::string>result;
					result.push_back(orderID);
					return result;
				}
				else if (m_symbolpositionbuffer[symbol].longydposition == 0)
				{
					req.offset = OFFSET_CLOSETODAY;
					std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
					m_orderStrategymtx.lock();
					m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
					m_orderStrategymtx.unlock();
					writeCtaLog("策略" + Strategy->m_strategyName + "发出委托(只平今)" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
					std::vector<std::string>result;
					result.push_back(orderID);
					return result;
				}
				else
				{
					//昨仓不够用，分开平，先平昨再平今
					req.offset = OFFSET_CLOSEYESTERDAY;
					req.volume = m_symbolpositionbuffer[symbol].longydposition;
					std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
					m_orderStrategymtx.lock();
					m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
					m_orderStrategymtx.unlock();
					std::vector<std::string>result;
					result.push_back(orderID);
					//再平今仓
					req.offset = OFFSET_CLOSETODAY;
					req.volume = volume - req.volume;
					orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
					m_orderStrategymtx.lock();
					m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
					m_orderStrategymtx.unlock();
					writeCtaLog("策略" + Strategy->m_strategyName + "发出委托(平今和昨)" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
					result.push_back(orderID);
					return result;
				}
			}
		}
		else if (orderType == CTAORDER_SHORT)
		{
			//做空
			req.direction = DIRECTION_SHORT;
			req.offset = OFFSET_OPEN;
			//发单
			std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
			m_orderStrategymtx.lock();
			m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
			m_orderStrategymtx.unlock();
			writeCtaLog("策略" + Strategy->m_strategyName + "发出委托" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
			std::vector<std::string>result;
			result.push_back(orderID);
			return result;
		}
		else if (orderType == CTAORDER_COVER)
		{
			//平空
			req.direction = DIRECTION_LONG;
			if (m_gatewaymanager->getContract(symbol)->exchange != EXCHANGE_SHFE)
			{
				req.offset = OFFSET_CLOSE;
				std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
				m_orderStrategymtx.lock();
				m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
				m_orderStrategymtx.unlock();
				writeCtaLog("策略" + Strategy->m_strategyName + "发出委托" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
				std::vector<std::string>result;
				result.push_back(orderID);
				return result;
			}
			else
			{
				//区分平今平昨
				//先判断昨仓够不够用
				if ((m_symbolpositionbuffer[symbol].shortydposition - volume) >= 0)
				{
					//只平昨
					req.offset = OFFSET_CLOSEYESTERDAY;
					std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
					m_orderStrategymtx.lock();
					m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
					m_orderStrategymtx.unlock();
					writeCtaLog("策略" + Strategy->m_strategyName + "发出委托(只平昨)" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
					std::vector<std::string>result;
					result.push_back(orderID);
					return result;
				}
				else if (m_symbolpositionbuffer[symbol].shortydposition == 0)
				{
					//只平今
					req.offset = OFFSET_CLOSETODAY;
					std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
					m_orderStrategymtx.lock();
					m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
					m_orderStrategymtx.unlock();
					writeCtaLog("策略" + Strategy->m_strategyName + "发出委托(只平今)" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
					std::vector<std::string>result;
					result.push_back(orderID);
					return result;
				}
				else
				{
					//平今也平昨
					//昨仓不够用，分开平，先平昨再平今
					req.offset = OFFSET_CLOSEYESTERDAY;
					req.volume = m_symbolpositionbuffer[symbol].shortydposition;
					std::string orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
					m_orderStrategymtx.lock();
					m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
					m_orderStrategymtx.unlock();
					std::vector<std::string>result;
					result.push_back(orderID);
					//再平今仓
					req.offset = OFFSET_CLOSETODAY;
					req.volume = volume - req.volume;
					orderID = m_gatewaymanager->sendOrder(req, m_gatewaymanager->getContract(symbol)->gatewayname);
					m_orderStrategymtx.lock();
					m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, Strategy));
					m_orderStrategymtx.unlock();
					writeCtaLog("策略" + Strategy->m_strategyName + "发出委托(平今和昨)" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price), Strategy->gatewayname);
					result.push_back(orderID);
					return result;
				}
			}
		}
	}
	std::vector<std::string>v;
	return v;
}

void CtaEngine::cancelOrder(std::string orderID, std::string gatewayname)
{
	std::shared_ptr<Event_Order>order = m_gatewaymanager->getorder(gatewayname, orderID);
	if (order != nullptr)
	{
		//报单有效
		if (!(order->status == STATUS_ALLTRADED || order->status == STATUS_CANCELLED))
		{
			//可撤单状态
			CancelOrderReq req;
			req.symbol = order->symbol;
			req.exchange = order->exchange;
			req.frontID = order->frontID;
			req.sessionID = order->sessionID;
			req.orderID = order->orderID;
			m_gatewaymanager->cancelOrder(req, order->gatewayname);
		}
	}
}

void CtaEngine::writeCtaLog(std::string msg, std::string gatewayname)
{
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->msg = msg;
	e->gatewayname = gatewayname;
	m_eventengine->Put(e);
}
void CtaEngine::writeCtaLog(std::string msg)
{
	writeCtaLog(msg, "BacktesterEngine");
}

void CtaEngine::PutEvent(std::shared_ptr<Event>e)
{
	m_eventengine->Put(e);
}

void CtaEngine::savetraderecord(std::string strategyname, std::shared_ptr<Event_Trade>etrade)
{
	//交易记录
	if (_access("./traderecord", 0) != -1)
	{
		std::fstream f;
		f.open("./traderecord/" + strategyname + ".csv", std::ios::app | std::ios::out);
		if (!f.is_open())
		{
			//如果打不开文件
			std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
			e->msg = "无法保存交易记录";
			e->gatewayname = "CTP";
			m_eventengine->Put(e);
			return;
		}
		std::string symbol = etrade->symbol;
		std::string direction = etrade->direction;
		std::string offset = etrade->offset;
		std::string tradetime = etrade->tradeTime;
		std::string volume = Utils::doubletostring(etrade->volume);
		std::string price = Utils::doubletostring(etrade->price);

		f << strategyname << "," << tradetime << "," << symbol << "," << direction << "," << offset << "," << price << "," << volume << "\n";

		f.close();
	}
}

std::vector<TickData> CtaEngine::loadTick( std::string symbol, int days)
{
	std::vector<TickData>datavector;
	if (symbol == " " || symbol == "")
	{
		return datavector;
	}
	const char* databasename = DATABASE_NAME;
	const char* collectionsname = TICKCOLLECTION_NAME;
	auto targetday = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (days * 24 * 3600);//获取当前的系统时间


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
	BSON_APPEND_TIME_T(&child, "$gt", targetday);
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
std::vector<BarData> CtaEngine::loadBar( std::string symbol, int days)
{
	std::vector<BarData>datavector;
	if (symbol == " " || symbol == "")
	{
		return datavector;
	}
	const char* databasename = DATABASE_NAME;
	const char* collectionsname = BARCOLLECTION_NAME;
	auto targetday = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (days * 24 * 3600);//获取当前的系统时间

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
	BSON_APPEND_TIME_T(&child, "$gt", targetday);//$gt大于某个时间，"$lt"小于某个时间
	bson_append_document_end(&parent, &child);


	char* str = bson_as_json(&parent, NULL);
	//	printf("\n%s\n", str);

	// 从客户端池中获取一个客户端
	mongoc_client_t* client = mongoc_client_pool_pop(g_pool);																				//取一个mongo连接

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
