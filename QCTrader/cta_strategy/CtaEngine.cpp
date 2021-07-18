
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
	m_connectstatus = false;//CTP����



	//���ز���
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
			//ɾ��ָ��
			delete strP;
			strP = nullptr;
		}

	}
	delete m_portfolio;
}
//��ȡ���������ļ�

void CtaEngine::ReadStrategyConfFileJson()
{
	Json::Reader reader;
	Json::Value root;

	//���ļ��ж�ȡ����֤��ǰ�ļ���demo.json�ļ�  
	std::ifstream in("./Strategy/cta_strategy_setting.json", std::ios::binary);

	if (!in.is_open())
	{
		writeCtaLog("�򿪲��������ļ�ʧ��");
		return;
	}

	if (reader.parse(in, root))
	{
		writeCtaLog("�򿪲��������ļ��ɹ�");
		for (int i = 0; i < root.size(); i++)
		{
			//��ȡ�������ƺͺ�Լ����
			std::string StrategyName = "";
			std::string vt_symbol = "";
			std::string ClassName = "";

			StrategyName = root[i]["strategy_name"].asString();
			vt_symbol = root[i]["vt_symbol"].asString();
			ClassName = root[i]["class_name"].asString();
			if ((StrategyName.length() < 1 || (vt_symbol.length()) < 1 || ClassName.length() < 1))
			{
				this->writeCtaLog("�����ļ�������Ϣ��ȫ");
				return;
			}

			//��ȡ����������Ϣ 
			std::map<std::string, float> settingMap;
			Json::Value::Members members;
			members = root[i]["setting"].getMemberNames();
			//std::vector<std::string> settingKeys= root["setting"].getMemberNames();
			for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // ����ÿ��key
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
			//���뵽��������map��
			m_strategyConfigInfo_map[StrategyName + "_" + vt_symbol + "_" + ClassName] = settingMap;
		}

	}
	else
	{
		this->writeCtaLog("�������������ļ�ʧ��");
	}

	in.close();
}

//��ȡ���������ļ�
void CtaEngine::ReadStrategyDataJson(std::string strfileName)
{
	Json::Reader reader;
	Json::Value root;

	//���ļ��ж�ȡ����֤��ǰ�ļ���demo.json�ļ�  
	std::ifstream in(strfileName, std::ios::binary);

	if (!in.is_open())
	{
		this->writeCtaLog("�򿪲��������ļ�ʧ��");
		return;
	}

	if (reader.parse(in, root))
	{
		this->writeCtaLog("�򿪲��������ļ��ɹ�");

		//��ȡ�������ƺͺ�Լ����
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
		for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // ����ÿ��key
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
		//���뵽��������map��
		m_strategyData_map[StrategyName + "_" + vt_symbol] = settingMap;

		/*

		for (int i = 0; i < root.size(); i++)
		{
			//��ȡ�������ƺͺ�Լ����

			std::string StrategyName = root[i]["strategy_name"].asString();
			std::string vt_symbol = root[i]["vt_symbol"].asString();
			std::string ClassName = root[i]["class_name"].asString();
			if ((StrategyName.length() < 1 || (vt_symbol.length()) < 1 || ClassName.length() < 1))
			{
				this->writeCtaLog("���������ļ���Ϣ��ȫ");
				return;
			}

			//��ȡ����������Ϣ 
			std::map<std::string, float> settingMap;
			Json::Value::Members members;
			members = root[i]["setting"].getMemberNames();
			//std::vector<std::string> settingKeys= root["setting"].getMemberNames();
			for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // ����ÿ��key
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
			//���뵽��������map��
			m_strategyData_map[StrategyName + "_" + vt_symbol + "_" + ClassName] = settingMap;
		
		}*/

	}
	else
	{
	this->writeCtaLog("�������������ļ�ʧ��");
	}

	in.close();
}
//д�����ļ�
void CtaEngine::WriteStrategyDataJson(std::map<std::string, std::string>dataMap,std::string fileName)
{


	Json::Value root;

	//���ڵ�����
	std::map<std::string, std::string>::iterator iter;
	for (iter = dataMap.begin(); iter != dataMap.end(); iter++)
	{
		std::string varName = iter->first;
		std::string varValue = iter->second;
		root[varName] = Json::Value(varValue);
	}
	
	Json::StyledWriter sw;

	//������ļ�
	std::ofstream os;
	std::string file = "./Strategy/cta_strategy_data_" + fileName + "_.json";
	os.open(file);
	os << sw.write(root);
	os.close();


}

//���ز���
void CtaEngine::loadStrategy()
{//�������ļ���ȡ����Ҫע�⣬һ���ǲ��������ļ������еĲ��Զ���һ���ļ�����./Strategy/cta_strategy_setting.json��
	//��һ���ǲ��Ա��������ļ���ÿ�ֲ��Զ�Ӧһ������Ŀ¼.\Strategy\cta_strategy_vardata�£�
	//��������Ϊcta_strategy_data_" + str.toStdString() + ".json" str�ǲ������Ӳ�������

	//��ȡ���������ļ�����Ų������ò�������������������Լ�������������ò���
	//ReadStrategyConfFileJson();
	m_strategyConfigInfo_map = Global_FUC::ReadStrategyConfFileJson("./Strategy/cta_strategy_setting.json",this);
	
	std::map<std::string, std::map<std::string, float>>::iterator iter;
	for (iter = m_strategyConfigInfo_map.begin(); iter!=m_strategyConfigInfo_map.end(); iter++)
	{
		QString str = QString::fromStdString(iter->first).section("_", 0, 1);


		//ѭ����ȡ���������ļ�����Ų��Ա���,�����λ��
		std::string strfileName = "./Strategy/cta_strategy_data_" + str.toStdString() + "_.json";
		//��ȡ�����ļ���Ϣ��������ļ����ư����������ͺ�Լ����StrategyTemplate::sync_data()д���ļ�����Ҫһ��
		ReadStrategyDataJson(strfileName);

	}


	if (m_strategyConfigInfo_map.size() == 0)
	{
		this->writeCtaLog("�޲����������ļ���");
		return;

	}
	else//���ݲ��������ļ����ɲ��Լ�,���ز������úͲ��Ա���
	{
		std::set<std::string> symbolSet;//�Ѳ������������ĺ�Լ���ŵ�һ��set����
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
			//���ɲ���
			StrategyTemplate* strategy_ptr;
			HINSTANCE his;
			if (strStrategyName.find(".dll")!= strStrategyName.npos)//����ͨ��DLL�ļ��ṩ
			{
				std::string strategy = "./Strategy/" + strStrategyName;//����DLL�ļ�
				his = LoadLibraryA(strategy.c_str());//����һ������
				if (his == NULL)
				{
					//û�м��ؽ���DLL
					std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
					e->msg = "ͨ��dll���ز���ʧ��" + strategy;
					m_eventengine->Put(e);

					return;
				}
				Dllfun dll = (Dllfun)GetProcAddress(his, "CreateStrategy");//��������
				if (dll == NULL)
				{
					//û�м��ؽ���DLL
					std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
					e->msg = "�޷��������Ժ���" + strategy;
					m_eventengine->Put(e);
					FreeLibrary(his);
					return;
				}
				strategy_ptr = dll(this);
				std::string strName = strStrategyName + "_" + strSymbolName;
				dllmap.insert(std::pair<std::string, HINSTANCE>(strName, his));

			}
			else//ͨ����Դ�������ṩ�Ĳ���
			{
				if (strStrategyName == "BollChannelStrategy")
					strategy_ptr = new BollChannelStrategy(this, strStrategyName, strSymbolName);
				else
				{
					this->writeCtaLog("û����صĲ����ṩ");
					return;
				}

			}



			//��ֵ���ò����������е�strategeData,�����б�������strategeData,������������ò����ͱ��������������Ǹ��������ļ����¡�
			for (std::map<std::string, float>::iterator it = settingMap.begin(); it != settingMap.end(); it++)
			{
				//����parameter
				std::string value = std::to_string(it->second);
				strategy_ptr->updateParam(it->first.c_str(), value.c_str());
			}
			//��ֵ���������������е�strategeData
			if (m_strategyData_map.size()> 0)
			{
				//QString strTemp = QString::fromStdString(iter->first);
				//strTemp = QString::
				std::string strKey = strStrategyName + "_" + strSymbolName;
				if (m_strategyData_map.find(strKey) != m_strategyData_map.end())
				{
					std::map<std::string, std::string>varMap = m_strategyData_map[strKey];//����map
					for (std::map<std::string, std::string>::iterator it = varMap.begin(); it != varMap.end(); it++)
					{
						//����var
						std::string value = it->second;
						strategy_ptr->updateVar(it->first.c_str(), value.c_str());
					}

				}
			}
			//�Ѷ�ȡ�������úͱ���ֵͨ��strategeData���µ����Եı�����ȥ
			strategy_ptr->updateSetting();


			m_tickstrategymtx.lock();
			//����m_tickstrategymap
			if (m_tickstrategymap.find(strSymbolName) == m_tickstrategymap.end())
			{
				//û�к�Լ����map�У�����һ��
				std::vector<StrategyTemplate*>strategy_v;
				strategy_v.push_back(strategy_ptr);
				m_tickstrategymap.insert(std::pair<std::string, std::vector<StrategyTemplate*>>(strSymbolName, strategy_v));
			}
			else//�Ѿ��к�Լ�����У���������
			{
				m_tickstrategymap[strSymbolName].push_back(strategy_ptr);
			}
			//}
			m_tickstrategymtx.unlock();
			//�������m_strategymap,   key�ǲ�����+��Լ��+������ֵ��֮ǰnew������ ָ����Զ����ָ��
			//
			std::string strName = strStrategyName + "_" + strSymbolName+"_"+ strClassName;
			//m_strategymap.insert(std::pair<std::string, StrategyTemplate*>(strategy_ptr->getparam("name"), strategy_ptr));//�������Ͳ���
			
			//���ɲ��Լ�
			m_strategymtx.lock();
			m_strategymap.insert(std::pair<std::string, StrategyTemplate*>(strName, strategy_ptr));
			m_strategymtx.unlock();
		}

			//���ĺ�Լ
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
//��ʼ��
void CtaEngine::initStrategy(std::string name)
{
	m_portfolio->calculate();
	m_strategymtx.lock();
	if (m_strategymap.find(name) != m_strategymap.end())
	{
		//�жϲ��Լ��ؽ�����
		StrategyTemplate* temp = m_strategymap[name];
		if (temp->inited == false)
		{
			temp->onInit();
			//���ĸò�����Ժ�Լ������
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
				writeCtaLog("���Ĳ��Եĺ�Լ����" + req.symbol = contract->symbol);
			}
			else
			{
				writeCtaLog("��ȡ��Լ��Ϣʧ��");
			}
		}
		else
		{
			this->writeCtaLog("�����ظ���ʼ��");
		}
	}
	else
	{
		this->writeCtaLog("����ʵ��������");
	}
	m_strategymtx.unlock();
}
//��ʼ����
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
			this->writeCtaLog("���Ի�û��ʼ��");
		}
	}
	else
	{
		this->writeCtaLog("����ʵ��������");
	}
	m_strategymtx.unlock();
}
//ֹͣ����
void CtaEngine::stopStrategy(std::string name)
{
	m_strategymtx.lock();
	if (m_strategymap.find(name) != m_strategymap.end())
	{
		StrategyTemplate* temp = m_strategymap[name];
		if (temp->trading == true)
		{
			temp->onStop();
			//����
			temp->cancelAllOrder();
		}
	}
	else
	{
		this->writeCtaLog("����ʵ��������");
	}
	m_strategymtx.unlock();
}
//�㷨����
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
			this->writeCtaLog("�����ظ���ʼ��");
		}
	}
	m_strategymtx.unlock();
}
//�������в���
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
			this->writeCtaLog("����δ��ʼ��");
		}
	}
	m_strategymtx.unlock();
}
//ֹͣ���в���
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

	//����
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

/******************������***************************/
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
		//�е�
		for (std::vector<StrategyTemplate*>::iterator it = m_tickstrategymap[etick->symbol].begin(); it != m_tickstrategymap[etick->symbol].end(); it++)
		{
			//ѭ�����в���ʵ��ָ��
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

	m_portfolio->calculate_memory(eTrade, m_orderStrategymap);//����portfolio


	//���²����ڲ���pos����
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

		//���潻�׼�¼
		Global_FUC::savetraderecord(m_orderStrategymap[eTrade->orderID]->m_strategyName, eTrade,m_eventengine);
		//�����λ��Ϣ�������ļ�
		m_orderStrategymap[eTrade->orderID]->sync_data();
	}
	m_orderStrategymtx.unlock();
	

	//���³ֲ�m_symbolpositionbuffer
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
		if (eTrade->direction == DIRECTION_LONG)//��
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
		else if (eTrade->direction == DIRECTION_SHORT)//��
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
			//�еĻ�ֱ�Ӹ���
			m_symbolpositionbuffer[eTrade->symbol] = *bufferPtr;
		}
		else
		{
			//û�еĻ�����һ��
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
		{	//�о�ֱ�Ӹ���
			PositionBuffer positionbuffer;
			if (ePosition->direction == DIRECTION_LONG)//��
			{
				positionbuffer.longposition = ePosition->position;
				positionbuffer.longydposition = ePosition->ydPosition;
				positionbuffer.longtodayposition = ePosition->todayPosition;
				positionbuffer.shortposition = m_symbolpositionbuffer[ePosition->symbol].shortposition;
				positionbuffer.shortydposition = m_symbolpositionbuffer[ePosition->symbol].shortydposition;
				positionbuffer.shorttodayposition = m_symbolpositionbuffer[ePosition->symbol].shorttodayposition;
			}
			else if (ePosition->direction == DIRECTION_SHORT)//��
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
		{	//û�оʹ���һ���ٸ��½�ȥ
			PositionBuffer positionbuffer;
			if (ePosition->direction == DIRECTION_LONG)//��
			{
				positionbuffer.longposition = ePosition->position;
				positionbuffer.longydposition = ePosition->ydPosition;
				positionbuffer.longtodayposition = ePosition->todayPosition;

			}
			else if (ePosition->direction == DIRECTION_SHORT)//��
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
//�����˶�ʱ�����ǽ���ʱ��Ͽ����ӣ�����ʱ���Զ�����
void CtaEngine::autoConnect(std::shared_ptr<Event>e)
{


	auto nowtime2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	if (((nowtime2 > Utils::timetounixtimestamp(2, 31, 0)) && (nowtime2 < Utils::timetounixtimestamp(8, 50, 0)))
		|| ((nowtime2 > Utils::timetounixtimestamp(15, 01, 0)) && (nowtime2 < Utils::timetounixtimestamp(20, 50, 0))))
	{
		if (m_connectstatus == true)
		{
			m_connectstatus = false;

			writeCtaLog("CTP�ӿ������Ͽ����ӣ�");
			m_gatewaymanager->close("CTP");
			m_riskmanager->clearTradeCount();
		}
	}
	else
	{
		//����
		if (m_connectstatus == false)
		{
			//���û�в��Գ�ʼ����ɣ��Ȳ����ӣ�
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
			writeCtaLog("CTP�ӿ��������ӣ�");

			m_gatewaymanager->connect("CTP");
		}
	}
}
/****************** ע��***************************/

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
/******************���Ե���***************************/

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
			req.priceType = PRICETYPE_LIMITPRICE;//�޼۵�
			//����
			std::string orderID = m_gatewaymanager->sendOrder(req,"CTP");

			m_orderStrategymtx.lock();
			m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, pStrategy));
			m_orderStrategymtx.unlock();

			writeCtaLog("����" + pStrategy->m_strategyName + "����ί��" + symbol + req.direction + Utils::doubletostring(volume) + " @ " + Utils::doubletostring(price));
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
	if (orderID.find("stop_order_id") != orderID.npos)//��ֹͣ��
	{
		cancel_local_stop_order(orderID);
		return;
	}
	//��ֹͣ����ֱ���ύ��������
	std::shared_ptr<Event_Order>order = m_gatewaymanager->getorder("CTP", orderID);
	if (order != nullptr)
	{
		//������Ч
		if (!(order->status == STATUS_ALLTRADED || order->status == STATUS_CANCELLED))
		{
			//�ɳ���״̬
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
	std::string orderID = "stop_order_id��" + std::to_string(m_stop_order_count);//����stop_order_id����������
	//stop_order.orderID
	m_stop_order_mtx.lock();
	m_stop_order_map.insert(std::pair <std::string, std::shared_ptr<Event_StopOrder>> (orderID, ptr_stop_order));
	m_stop_order_mtx.unlock();

	m_orderStrategymtx.lock();
	m_orderStrategymap.insert(std::pair<std::string, StrategyTemplate*>(orderID, pStrategy));
	m_orderStrategymtx.unlock();

	m_stragegyOrderMapmtx.lock();
	m_stragegyOrderMap[pStrategy->m_strategyName].push_back(orderID);//����m_stragegyOrderMap�б���
	m_stragegyOrderMapmtx.unlock();

	//pStrategy->onStopOrder(ptr_stop_order); PutEvent�󱻵����ˣ����ﲻ�õ���
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
	//���׼�¼
	if (_access("./traderecord", 0) != -1)
	{
		std::fstream f;
		f.open("./traderecord/" + strategyname + ".csv", std::ios::app | std::ios::out);
		if (!f.is_open())
		{
			//����򲻿��ļ�
			std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
			e->msg = "�޷����潻�׼�¼";
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
	auto targetday = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (days * 24 * 3600);//��ȡ��ǰ��ϵͳʱ��


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
std::vector<BarData> CtaEngine::loadBar( std::string symbol, int days)
{
	std::vector<BarData>datavector;
	if (symbol == " " || symbol == "")
	{
		return datavector;
	}
	const char* databasename = DATABASE_NAME;
	const char* collectionsname = BARCOLLECTION_NAME;
	auto targetday = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) - (days * 24 * 3600);//��ȡ��ǰ��ϵͳʱ��

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
	BSON_APPEND_TIME_T(&child, "$gt", targetday);//$gt����ĳ��ʱ�䣬"$lt"С��ĳ��ʱ��
	bson_append_document_end(&parent, &child);


	char* str = bson_as_json(&parent, NULL);
	//	printf("\n%s\n", str);

	// �ӿͻ��˳��л�ȡһ���ͻ���
	mongoc_client_t* client = mongoc_client_pool_pop(g_pool);																				//ȡһ��mongo����

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
			mongoc_client_pool_push(g_pool, client);																						//�Ż�һ��mongo����
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

		bardata.openPrice = json["openPrice"].number_value();//���տ�
		bardata.highPrice = json["highPrice"].number_value();//���ո�
		bardata.lowPrice = json["lowPrice"].number_value();//���յ�
		bardata.preClosePrice = json["preClosePrice"].number_value();//����

		bardata.upperLimit = json["upperLimit"].number_value();//��ͣ
		bardata.lowerLimit = json["lowerLimit"].number_value();//��ͣ

		bardata.openInterest = json["openInterest"].number_value();//�ֲ�

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
