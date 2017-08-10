//////下面是接口管理
#include"RecorderManager.h"
#include"CTPMD.h"
Recordermanager::Recordermanager(EventEngine *eventengine)
{
	m_ptr_eventengine = eventengine;
	Init();
}
Recordermanager::~Recordermanager()
{
	//析构
}
void Recordermanager::Init()
{
	//初始化所有接口
	//CTP
	std::shared_ptr<CTPMD>ctpgateway = std::make_shared<CTPMD>(m_ptr_eventengine, "CTP");
	m_gatewaymap.insert(std::pair<std::string, std::shared_ptr<JSRecordClass>>("CTP", ctpgateway));
	//LTS
	//.....
}
void Recordermanager::connect(std::string gatewayname)
{
	//连接接口
	m_gatewaymap[gatewayname]->connect();//连接指定的接口
}
void Recordermanager::subscribe(SubscribeReq req, std::string gatewayname)
{
	//订阅行情
	m_gatewaymap[gatewayname]->subscribe(req);//连接指定的接口
}

void Recordermanager::close(std::string gatewayname)
{
	//退出指定接口
	m_gatewaymap[gatewayname]->close();
}

void Recordermanager::exit()
{
	//关闭全部接口
	for (std::map<std::string, std::shared_ptr<JSRecordClass>>::iterator it = m_gatewaymap.begin(); it != m_gatewaymap.end(); it++)
	{
		it->second->close();
	}
}