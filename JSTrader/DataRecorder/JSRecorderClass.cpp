//这里是接口抽象类
#include"JSRecordClass.h"
JSRecordClass::JSRecordClass(EventEngine *eventengine, std::string gatewayname)
{
	m_ptr_eventengine = eventengine;
	m_gatewayname = gatewayname;
}
void JSRecordClass::onTick(std::shared_ptr<Event_Tick>e)
{
	m_ptr_eventengine->Put(e);
}
void JSRecordClass::onContract(std::shared_ptr<Event_Contract>e)
{
	m_ptr_eventengine->Put(e);
}
void JSRecordClass::onError(std::shared_ptr<Event_Error>e)
{
	m_ptr_eventengine->Put(e);
}
void JSRecordClass::onLog(std::shared_ptr<Event_Log>e)
{
	m_ptr_eventengine->Put(e);
}
