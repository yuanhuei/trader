
#include"CtaEngine.h"
#include"Strategies/turtlebreak.h"
#include <json/json.h>
#include<qstring.h>

#include <iostream>  
#include <fstream>  
#include"./cta_strategy/strategies/BollChannelStrategy.h"
#include"../utility.h"
//#include <QCTrader/utility.cpp>


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
			std::string StrategyName = "";
			std::string vt_symbol = "";
			std::string ClassName = "";

			StrategyName = root[i]["strategy_name"].asString();
			vt_symbol = root[i]["vt_symbol"].asString();
			ClassName = root[i]["class_name"].asString();
			if ((StrategyName.length() < 1 || (vt_symbol.length()) < 1 || ClassName.length() < 1))
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

		//读取策略名称和合约名称
		std::string StrategyName = "";
		std::string vt_symbol = "";
		//std::string ClassName = "";
		QString qstrfileName = QString::fromStdString(strfileName);

		StrategyName = qstrfileName.section("_", 3, 3).toStdString();
		vt_symbol = qstrfileName.section("_",4,4).toStdString();


		std::map<std::string, std::string> settingMap;
		Json::Value::Members members;
		members = root.getMemberNames();
		//std::vector<std::string> settingKeys= root["setting"].getMemberNames();
		for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // 遍历每个key
		{
			std::string strKey = *iterMember;
			std::string  fValue = root[strKey.c_str()].asString();
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
		m_strategyData_map[StrategyName + "_" + vt_symbol] = settingMap;

		/*

		for (int i = 0; i < root.size(); i++)
		{
			//读取策略名称和合约名称

			std::string StrategyName = root[i]["strategy_name"].asString();
			std::string vt_symbol = root[i]["vt_symbol"].asString();
			std::string ClassName = root[i]["class_name"].asString();
			if ((StrategyName.length() < 1 || (vt_symbol.length()) < 1 || ClassName.length() < 1))
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
				
				//if(fValue.ist)
				settingMap.insert({ strKey,  fValue });

			}
			//插入到策略数据map中
			m_strategyData_map[StrategyName + "_" + vt_symbol + "_" + ClassName] = settingMap;
		
		}*/

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
	std::string file = "./Strategy/cta_strategy_data_" + fileName + "_.json";
	os.open(file);
	os << sw.write(root);
	os.close();


}

//加载策略
void CtaEngine::loadStrategy()
{//有两种文件读取，需要注意，一种是策略配置文件，所有的策略都在一个文件里面./Strategy/cta_strategy_setting.json，
	//另一种是策略变量数据文件，每种策略对应一个，在目录.\Strategy\cta_strategy_vardata下，
	//命名规则为cta_strategy_data_" + str.toStdString() + ".json" str是策略名加策略类名

	//读取策略配置文件（存放策略配置参数，包括策略名，合约，类名还有配置参数
	//ReadStrategyConfFileJson();
	m_strategyConfigInfo_map = Global_FUC::ReadStrategyConfFileJson("./Strategy/cta_strategy_setting.json",this);
	
	std::map<std::string, std::map<std::string, float>>::iterator iter;
	for (iter = m_strategyConfigInfo_map.begin(); iter!=m_strategyConfigInfo_map.end(); iter++)
	{
		QString str = QString::fromStdString(iter->first).section("_", 0, 1);


		//循环读取策略数据文件（存放策略变量,例如仓位）
		std::string strfileName = "./Strategy/cta_strategy_data_" + str.toStdString() + "_.json";
		//读取变量文件信息，这里的文件名称包含策略名和合约，与StrategyTemplate::sync_data()写的文件名称要一致
		ReadStrategyDataJson(strfileName);

	}


	if (m_strategyConfigInfo_map.size() == 0)
	{
		this->writeCtaLog("无策略在配置文件中");
		return;

	}
	else//根据策略配置文件生成策略集,加载策略配置和策略变量
	{
		std::set<std::string> symbolSet;//把策略中所关联的合约都放到一个set里面
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
					e->msg = "通过dll加载策略失败" + strategy;
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



			//赋值配置参数给策略中的strategeData,策略中必须生成strategeData,里面包含了配置参数和变量参数，下面是根据配置文件更新。
			for (std::map<std::string, float>::iterator it = settingMap.begin(); it != settingMap.end(); it++)
			{
				//遍历parameter
				std::string value = std::to_string(it->second);
				strategy_ptr->updateParam(it->first.c_str(), value.c_str());
			}
			//赋值变量参数给策略中的strategeData
			if (m_strategyData_map.size()> 0)
			{
				//QString strTemp = QString::fromStdString(iter->first);
				//strTemp = QString::
				std::string strKey = strStrategyName + "_" + strSymbolName;
				if (m_strategyData_map.find(strKey) != m_strategyData_map.end())
				{
					std::map<std::string, std::string>varMap = m_strategyData_map[strKey];//变量map
					for (std::map<std::string, std::string>::iterator it = varMap.begin(); it != varMap.end(); it++)
					{
						//遍历var
						std::string value = it->second;
						strategy_ptr->updateVar(it->first.c_str(), value.c_str());
					}

				}
			}
			//把读取到的配置和变量值通过strategeData更新到策略的变量上去
			strategy_ptr->updateSetting();


			m_tickstrategymtx.lock();
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
			m_strategymtx.lock();
			m_strategymap.insert(std::pair<std::string, StrategyTemplate*>(strName, strategy_ptr));
			m_strategymtx.unlock();
		}

			//订阅合约
		/*
		std::set<std::string>::iterator iter;
		for (iter=symbolSet.begin();iter!=symbolSet.end();iter++)
		{
			std::string strSymbolName = *iter;
			std::shared_ptr<Event_Contract>contract = m_gatewaymanager->getContract(strSymbolName);
			SubscribeReq req;
			req.symbol = contract->symbol;
			req.exchange = contract->exchange;
			
			if (strategy_ptr->getparam("currency") != "Null" && strategy_ptr->getparam("productClass") != "Null")
			{
				req.currency = strategy_ptr->getparam("currency");
				req.productClass = strategy_ptr->getparam("productClass");
			}
			m_gatewaymanager->subscribe(req, "CTP");// strategy_ptr->getparam("gatewayname"));
		}
		*/
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
			//订阅该策略针对合约的行情
			std::shared_ptr<Event_Contract>contract = m_gatewaymanager->Find_Contract(temp->m_symbol);
			if(contract!=nullptr)
			{
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
				writeCtaLog("订阅策略的合约行情" + req.symbol = contract->symbol);
			}
			else
			{
				writeCtaLog("获取合约信息失败");
			}
		}
		else
		{
			this->writeCtaLog("请勿重复初始化");
		}
	}
	else
	{
		this->writeCtaLog("策略实例不存在");
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
			temp->cancelAllOrder();
		}
	}
	else
	{
		this->writeCtaLog("策略实例不存在");
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
			this->writeCtaLog("请勿重复初始化");
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

void CtaEngine::cancelAllOrder(std::string strStragetyName)
{
	m_stragegyOrderMapmtx.lock();
	if (m_stragegyOrderMap[strStragetyName].size() >= 1)
	{
		for (int i = 1; i < m_stragegyOrderMap[strStragetyName].size(); i++)
			cancelOrder(m_stragegyOrderMap[strStragetyName][i]);
	}
	m_stragegyOrderMapmtx.unlock();
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
void CtaEngine::processStopOrderEvent(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_StopOrder> eOrder = std::static_pointer_cast<Event_StopOrder>(e);
	m_orderStrategymtx.lock();
	if (m_orderStrategymap.find(eOrder->orderID) != m_orderStrategymap.end())
	{
		m_orderStrategymap[eOrder->orderID]->onStopOrder(eOrder);
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
		Global_FUC::savetraderecord(m_orderStrategymap[eTrade->orderID]->m_strategyName, eTrade,m_eventengine);
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

			writeCtaLog("CTP接口主动断开连接！");
			m_gatewaymanager->close("CTP");
			m_riskmanager->clearTradeCount();
		}
	}
	else
	{
		//连接
		if (m_connectstatus == false)
		{
			//如果没有策略初始化完成，先不连接，
			bool bInited = false;
			m_strategymtx.lock();
			for (std::map<std::string, StrategyTemplate*>::iterator iter = m_strategymap.begin(); iter != m_strategymap.end(); iter++)
			{
				if (iter->second->inited == true)
					bInited = true;
			}
			m_strategymtx.unlock();
			if (bInited == false)
				return;

			m_connectstatus = true;
			writeCtaLog("CTP接口主动连接！");

			m_gatewaymanager->connect("CTP");
		}
	}
}
/****************** 注册***************************/

void CtaEngine::registerEvent()
{
	m_eventengine->RegEvent(EVENT_TICK, std::bind(&CtaEngine::procecssTickEvent, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_ORDER, std::bind(&CtaEngine::processOrderEvent, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_STOP_ORDER, std::bind(&CtaEngine::processStopOrderEvent, this, std::placeholders::_1));

	m_eventengine->RegEvent(EVENT_TRADE, std::bind(&CtaEngine::processTradeEvent, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_POSITION, std::bind(&CtaEngine::processPositionEvent, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TIMER, std::bind(&CtaEngine::autoConnect, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_LOG, std::bind(&CtaEngine::showLog, this, std::placeholders::_1));
}
/******************策略调用***************************/

std::vector<std::string>CtaEngine::sendOrder(bool bStopOrder, std::string symbol, std::string strDirection,std::string strOffset,double price, double volume, StrategyTemplate* pStrategy)
{
	if (pStrategy->trading == true)
	{
		OrderReq req;
		req.symbol = symbol;
		req.exchange = m_gatewaymanager->GetExchangeName(symbol, "CTP");// getContract(symbol)->exchange;
		req.price = price;
		req.volume = volume;
		/*
		if (m_riskmanager->checkRisk(req) == false)
		{
			std::vector<std::string>result;
			return result;
		}
		
		if (Strategy->getparam("currency") != "Null" && Strategy->getparam("productClass") != "Null")
		{
			req.currency = Strategy->getparam("currency");
			req.productClass = Strategy->getparam("productClass");
		}*/
		if (bStopOrder)
		{
			return sendStopOrder(symbol, strDirection, strOffset, price, volume, pStrategy);
		}
		else
		{
			req.priceType = PRICETYPE_LIMITPRICE;//限价单
			//发单
			std::string orderID = m_gatewaymanager->sendOrder(req,"CTP");

			m_orderStrategymtx.lock();
			m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, pStrategy));
			m_orderStrategymtx.unlock();

			writeCtaLog("策略" + pStrategy->m_strategyName + "发出委托" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price));
			std::vector<std::string>result;
			result.push_back(orderID);
			return result;
		}


	}
	else
	{
		std::vector<std::string>v;
		return v;
	}
}
void CtaEngine::cancel_local_stop_order(std::string orderID)
{
	std::shared_ptr<Event_StopOrder> ptr_stop_order;
	m_stop_order_mtx.lock();
	std::map<std::string, std::shared_ptr<Event_StopOrder>>::iterator iter= m_stop_order_map.find(orderID);
	if (iter != m_stop_order_map.end())
	{
		ptr_stop_order = iter->second;
		m_stop_order_map.erase(iter);
	}
	m_stop_order_mtx.unlock();
	ptr_stop_order->status = STATUS_CANCELLED;


	m_stragegyOrderMapmtx.lock();
	std::vector<std::string>::iterator it = m_stragegyOrderMap[ptr_stop_order->strategyName].begin();
	for (it; it!=m_stragegyOrderMap[ptr_stop_order->strategyName].end();)
	{
		if ((*it) == orderID)
			it = m_stragegyOrderMap[ptr_stop_order->strategyName].erase(it);
		else
			iter++;

	}
	m_stragegyOrderMapmtx.unlock();	
	
	PutEvent(ptr_stop_order);

}


void CtaEngine::cancelOrder(std::string orderID, std::string gatewayName)
{
	if (orderID.find("stop_order_id") != orderID.npos)//是停止单
	{
		cancel_local_stop_order(orderID);
		return;
	}
	//非停止单，直接提交给交易所
	std::shared_ptr<Event_Order>order = m_gatewaymanager->getorder("CTP", orderID);
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
			m_gatewaymanager->cancelOrder(req, "CTP");
		}
	}
}
std::vector<std::string>CtaEngine::sendStopOrder(std::string symbol, std::string strDirection, std::string strOffset, double price, double volume, StrategyTemplate* pStrategy)
{
	
	std::shared_ptr<Event_StopOrder> ptr_stop_order=std::make_shared<Event_StopOrder>();

	ptr_stop_order->symbol = symbol;
	ptr_stop_order->direction = strDirection;
	ptr_stop_order->offset = strOffset;
	ptr_stop_order->price = price;
	ptr_stop_order->totalVolume = volume;
	ptr_stop_order->strategyName = pStrategy->m_strategyName;
	m_stop_order_count++;
	std::string orderID = "stop_order_id：" + std::to_string(m_stop_order_count);//加了stop_order_id：用来区分
	//stop_order.orderID
	m_stop_order_mtx.lock();
	m_stop_order_map.insert(std::pair <std::string, std::shared_ptr<Event_StopOrder>> (orderID, ptr_stop_order));
	m_stop_order_mtx.unlock();

	m_orderStrategymtx.lock();
	m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, pStrategy));
	m_orderStrategymtx.unlock();

	m_stragegyOrderMapmtx.lock();
	m_stragegyOrderMap[pStrategy->m_strategyName].push_back(orderID);//不在m_stragegyOrderMap中保存
	m_stragegyOrderMapmtx.unlock();

	//pStrategy->onStopOrder(ptr_stop_order); PutEvent后被调用了，这里不用调用
	std::vector<std::string> orderidVector;
	orderidVector.push_back(orderID);
	
	PutEvent(ptr_stop_order);
	return orderidVector;
}

void CtaEngine::check_stop_order(TickData tickData)
{
	m_stop_order_mtx.lock();
	for (auto &iter : m_stop_order_map)
	{
		std::shared_ptr<Event_StopOrder> stopOrder = iter.second;
		std::string orderID = iter.first;
		bool long_triggered = false;
		bool short_triggered = false;
		if (stopOrder->symbol != tickData.symbol)
			continue;
		if (stopOrder->direction == DIRECTION_LONG && tickData.lastprice > stopOrder->price)
			long_triggered = true;
		if (stopOrder->direction == DIRECTION_SHORT && tickData.lastprice <= stopOrder->price)
			short_triggered = true;

		if (long_triggered || short_triggered)
		{
			StrategyTemplate* pStrategy = m_strategymap[stopOrder->strategyName];
			/*To get excuted immediately after stop order is
				# triggered, use limit price if available, otherwise
				# use ask_price_5 or bid_price_5*/
			double price;
			if (stopOrder->direction == DIRECTION_LONG)
				if (tickData.upperLimit > 0)
					price = tickData.upperLimit;
				else
					price = tickData.askprice5;
			else
				if (tickData.lowerLimit > 0)
					price = tickData.upperLimit;
				else
					price = tickData.askprice5;

			std::vector<std::string> orderReturn=sendOrder(false,stopOrder->symbol, stopOrder->direction, stopOrder->offset, price, stopOrder->totalVolume,  pStrategy);

			if (orderReturn.size() > 0)
			{
				m_stop_order_map.erase(orderID);
				//m_orderStrategymap.erase(orderID);

				stopOrder->status = STATUS_TRIGGED;
				stopOrder->orderID = orderReturn[0];
				//pStrategy->onStopOrder(stopOrder);

				PutEvent(stopOrder);
			}

		}
	}
	m_stop_order_mtx.unlock();

}


void CtaEngine::writeCtaLog(std::string msg)
{
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->msg = msg;
	e->gatewayname = "CtaEngine";
	m_eventengine->Put(e);
}

void CtaEngine::PutEvent(std::shared_ptr<Event>e)
{
	m_eventengine->Put(e);
}
/*
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
*/
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
