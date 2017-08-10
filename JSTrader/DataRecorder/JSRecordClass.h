#ifndef __JSGATEWAY_H__
#define __JSGATEWAY_H__
//抽象类，给所有接口类继承
#include<string>
#include"structs.h"
#include"eventengine.h"
class JSRecordClass
{
public:
	JSRecordClass(EventEngine *eventengine, std::string gatewayname);
	void onTick(std::shared_ptr<Event_Tick>e);
	void onContract(std::shared_ptr<Event_Contract>e);
	void onError(std::shared_ptr<Event_Error>e);
	void onLog(std::shared_ptr<Event_Log>e);
	virtual void connect()=0;//连接
	virtual void subscribe(SubscribeReq& subscribeReq) = 0;//订阅
	virtual void close() = 0;//断开API
private:
	EventEngine *m_ptr_eventengine;
	std::string m_gatewayname;
};

#endif