#pragma once
#pragma once
#ifndef CTAENGINE_H
#define CTAENGINE_H

#include"json11.hpp"
#include"utils.hpp"
#include"../event_engine/eventengine.h"
#include"../risk_manager/riskmanager.h"
#include"../cta_strategy/StrategyTemplate.h"
#include"../gateway/gatewaymanager.h"
#include"../portfolio.h"
#include<functional>
#include<cstdio>
//#include <windows.h>
//#include<minwindef.h>
#include"MongoCxx.h"
#include"../include/libmongoc-1.0/mongoc.h"
#include"../include/libbson-1.0/bson.h"
//#include "stdafx.h"
#include<BaseEngine.h>
//#include "second.h"
#define CTAORDER_BUY "ctaorderbuy"
#define CTAORDER_SELL "ctaordersell"
#define CTAORDER_SHORT "ctaordershort"
#define CTAORDER_COVER "ctaordercover"

#define DATABASE_NAME "test"
#define TICKCOLLECTION_NAME "db_tick_data"
#define  BARCOLLECTION_NAME  "db_bar_data"
//class EventEngine;
//class Gatewaymanager;
//class riskmanager;
class StrategyTemplate;
//class mongoc_uri_t;
//class mongoc_client_pool_t;
class Portfolio;

//typedef StrategyTemplate* (*Dllfun)(CtaEngine*);
//typedef int(*Release)();
/*******************************************/
class PositionBuffer//��������ֲ�
{
public:
	std::string symbol;//��Լ

	//��ͷ
	double longposition = 0;
	double longtodayposition = 0;
	double longydposition = 0;
	//��ͷ
	double shortposition = 0;
	double shorttodayposition = 0;
	double shortydposition = 0;
};

class CtaEngine :public BaseEngine
{
public:
	//CTA������
	CtaEngine(Gatewaymanager* gatewaymanager, EventEngine* eventengine, riskmanager* riskmanager);
	~CtaEngine();

	//�������躯��
	//std::vector<std::string> sendOrder(std::string symbol, std::string orderType, double price, double volume, StrategyTemplate* Strategy);
	std::vector<std::string>sendOrder(bool bStopOrder,std::string symbol, std::string strDirection, std::string strOffset, double price, double volume, StrategyTemplate* Strategy);

	void cancelOrder(std::string orderID,std::string gatewayName="CTP");
	void cancelAllOrder(std::string strStragetyName);
	void cancel_local_stop_order(std::string orderID);

	void writeCtaLog(std::string msg);

	void PutEvent(std::shared_ptr<Event>e);

	std::vector<TickData> loadTick(std::string symbol, int days);
	std::vector<BarData> loadBar(std::string symbol, int days);


	//������������躯��
	void loadStrategy();
	void initStrategy(std::string name);												//��ʼ��
	void startStrategy(std::string name);												//��ʼ����
	void stopStrategy(std::string name);												//ֹͣ����
	void initallStrategy();															//��ʼ�����в���
	void startallStrategy();															//�������в���
	void stopallStrategy();															//ֹͣ���в���
	
	void changesupposedpos(std::string name, std::string symbol, double pos);			//ͨ�����������Ƴֲ�

	//��������
	//void savetraderecord(std::string strategyname, std::shared_ptr<Event_Trade>etrade);

	void ReadStrategyConfFileJson();
	void ReadStrategyDataJson(std::string strfileName);
	void WriteStrategyDataJson(std::map<std::string, std::string>,std::string fileName);

	//���úͱ������������ļ���ȡ��ŵ�����map�У���һ��map��key��StrategyName + "__" + vt_symbol + "__" + ClassName] 
	//�ڶ���map��key ��StrategyName + "__" + vt_symbol
	std::map<std::string, std::map<std::string, float>> m_strategyConfigInfo_map;
	std::map<std::string, std::map<std::string, std::string>> m_strategyData_map;
private:
	EventEngine* m_eventengine;
	Gatewaymanager* m_gatewaymanager;
	riskmanager* m_riskmanager;

	//bool is_LoadStrategy;

	//����  ��   ������   ���̰߳�ȫ
	//key ��OrderID  value �ǲ��Զ��� ��;��Ϊ�˱�֤�������������Է���ȥ��  �ɽ��ر�����ֲ���ȷ���ض�Ӧ�Ĳ����ϣ��Է�������Խ���ͬһ����Լ����BUG
	std::map<std::string, StrategyTemplate*>m_orderStrategymap;		std::mutex m_orderStrategymtx;
	//���ԺͲ����µĶ�����map,
	std::map<std::string, std::vector<std::string>> m_stragegyOrderMap; std::mutex m_stragegyOrderMapmtx;

	//key ��Լ�� value �ֲֶ���   ��������ÿһ����Լ�Ľ������Ƕ���
	std::map<std::string, PositionBuffer>m_symbolpositionbuffer;	std::mutex m_symbolpositionmtx;

	//key ��Լ���������������Ƶ�����l2109.INE)��value ����ָ��(ʵ��)vector  ���ڽ��ܵ�tick����ʱ�򣬿��ٵ����͵�ÿһ�����Զ���
	std::map<std::string, std::vector<StrategyTemplate*>>m_tickstrategymap;	std::mutex m_tickstrategymtx;
	//key ������+��Լ��, value Ϊ����ָ��    �����ѽ���ѡ�еĲ����� ��Ӧ�ĵĲ��Զ�������
	std::map<std::string, StrategyTemplate*>m_strategymap;			std::mutex m_strategymtx;
public://���map������Ҫ��CTA���Խ����϶�ȡ������public��
	//orderid:: orderReq
	std::map<std::string, std::shared_ptr<Event_StopOrder>> m_stop_order_map; std::mutex m_stop_order_mtx; 
private:
	int m_stop_order_count=1;//Ϊ������ID

	void check_stop_order(TickData tickDate);
	std::vector<std::string>sendStopOrder(std::string symbol, std::string strDirection,std::string strOffset, double price, double volume, StrategyTemplate* Strategy);

	//������
	void procecssTickEvent(std::shared_ptr<Event>e);
	void processOrderEvent(std::shared_ptr<Event>e);
	void processStopOrderEvent(std::shared_ptr<Event>e);

	void processTradeEvent(std::shared_ptr<Event>e);
	void processPositionEvent(std::shared_ptr<Event>e);

	//ע�ᴦ����
	void registerEvent();

	//DLL�洢
	std::map<std::string, HINSTANCE>dllmap;//��Ų���dll����

	//�Զ�����
	std::atomic_bool m_connectstatus;
	void autoConnect(std::shared_ptr<Event>e);

public:

	//portfolio
	Portfolio* m_portfolio;
};
#endif