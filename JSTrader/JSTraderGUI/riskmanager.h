#ifndef RISKMANAGER_H
#define RISKMANAGER_H
#include"eventengine.h"
#include"utils.hpp"
#include<io.h>
#include<direct.h>
#include<fstream>
class riskmanager
{
	
public:
	riskmanager( EventEngine *eventengine)
	{
		eventengine_ptr = eventengine;
		active = false;
		orderFlowLimit = 0;
		orderFlowCount = 0;
		orderFlowClear = 0;
		orderFlowTimer = 0;
		orderSizeLimit = 0;
		tradeCount = 0;
		tradeLimit = 0;
		orderCancelLimit;
		workingOrderLimit = 0;
		loadSetting();
		registerEvent();
	}
	void loadSetting();
	void registerEvent();
	void onLog(std::shared_ptr<Event>e);
	void updateTrade(std::shared_ptr<Event>e);
	void updateOrder(std::shared_ptr<Event>e);
	void updateTimer(std::shared_ptr<Event>e);
	bool checkRisk(OrderReq req);
	void clearOrderFlowCount();
	void clearTradeCount();
	void setOrderFlowLimit(int n);
	void setOrderFlowClearTime(int n);
	void setOrderSizeLimit(int n);
	void setTradeLimit(int n);
	void setWorkingOrderLimit(int n);
	void setOrderCancelLimit(int n);
	bool switchEngineStatus();
	void saveSetting();

	bool active;
	std::atomic_int orderFlowLimit;

	std::atomic_int orderFlowClear;
	std::atomic_int orderFlowTimer;

	std::atomic_int orderSizeLimit;


	std::atomic_int tradeLimit;

	std::atomic_int orderCancelLimit;
	std::map<std::string, int>orderCancelMap;			std::mutex orderCancelMapMtx;

	std::atomic_int workingOrderLimit;

private:
	EventEngine *eventengine_ptr;
	std::atomic_int orderFlowCount;
	std::atomic_int tradeCount;


};

#endif