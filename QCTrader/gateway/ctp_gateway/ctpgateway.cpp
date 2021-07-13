#include"CTPGateway.h"

#include<map>
#include<atomic>
#include<io.h>
#include<functional>
#include<fstream>
#include<memory>
#include"CTPAPI/ThostFtdcMdApi.h"
#include"CTPAPI/ThostFtdcTraderApi.h"
//#include"JSgateway.hpp"
#include"../../event_engine/eventengine.h"
#include"ctpmd.h"
#include"ctptd.h"

CTPGateway::CTPGateway(EventEngine* eventengine, std::string gatewayname) :QCGateway(eventengine, gatewayname)
{
	//m_eventengine = eventengine;
	//m_gatewayname = gatewayname;

	//创建行情和交易的回调对象
	m_MDSPI = new CTPMD(this, gatewayname);
	m_TDSPI = new CTPTD(this, gatewayname);

	ctpmdconnected = false;
	ctptdconnected = false;
	m_qryEnabled = false;
	m_qrycount = 0;
	m_eventengine->RegEvent(EVENT_TIMER, std::bind(&CTPGateway::query, this));//注册到事件驱动引擎，循环查询仓位和账户
}

CTPGateway::~CTPGateway()
{
	if (m_MDSPI != nullptr)
	{
		delete m_MDSPI;
	}
	if (m_TDSPI != nullptr)
	{
		delete m_TDSPI;
	}
}
std::string CTPGateway:: GetExchangeName(std::string strSymbol)
{
	std::string strExchange = "";
	if (m_TDSPI->m_symbolExchangeMap.find(strSymbol) != m_TDSPI->m_symbolExchangeMap.end())
		return m_TDSPI->m_symbolExchangeMap[strSymbol];
	return strExchange;

}
int CTPGateway:: GetSymbolSize(std::string strSymbol)
{
	int iSize = 0;
	if (m_TDSPI->m_symbolSizeMap.find(strSymbol) != m_TDSPI->m_symbolSizeMap.end())
		return m_TDSPI->m_symbolSizeMap[strSymbol];
	return iSize;
}

void CTPGateway::connect()
{
	//连接行情和交易
	//先扫描文档下的账号密码配置文件
	if (_access("./CTPGateway", 0) != -1)
	{
		std::fstream f;
		f.open("./CTPGateway/CTP_connect");
		if (!f.is_open())
		{
			//如果打不开文件
			write_log("无法读取本地配置文件");
			//std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
			//e->msg = "无法读取本地配置文件";
			//e->gatewayname = m_gatewayname;
			//this->onLog(e);
			return;
		}
		std::string line;
		std::map<std::string, std::string>configmap;
		while (!f.eof())
		{
			getline(f, line);
			std::string::size_type pos = line.find("=");//按照等号分隔
			configmap.insert(std::make_pair(line.substr(0, pos), line.substr(pos + 1, line.size() - 1)));
		}
		m_MDSPI->connect(configmap["username"], configmap["password"], configmap["brokerid"], configmap["mdipaddress"]);
		m_TDSPI->connect(configmap["username"], configmap["password"], configmap["brokerid"], configmap["tdipaddress"], configmap["authcode"], configmap["productname"], configmap["productinfo"]);
		write_log("连接请求已发送");

	}
	else
	{
		write_log("无法读取本地配置文件");
		//std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		//e->msg = "无法读取本地配置文件";
		//e->gatewayname = m_gatewayname;
		//this->onLog(e);
	}
}
void CTPGateway::connect(std::string userID, std::string password, std::string brokerID, std::string mdaddress, std::string tdaddress, std::string authcode, std::string appid, std::string productinfo)
{

	m_MDSPI->connect(userID,password,brokerID,mdaddress);
	m_TDSPI->connect(userID,password,brokerID,tdaddress,authcode,appid,productinfo);


}


void CTPGateway::subscribe(SubscribeReq& subscribeReq)
{
	m_MDSPI->subscribe(subscribeReq);
}

std::string CTPGateway::sendOrder(OrderReq& req)
{
	return m_TDSPI->sendOrder(req);
}

void CTPGateway::cancelOrder(CancelOrderReq& req)
{
	m_TDSPI->cancelOrder(req);
}

void CTPGateway::qryAccount()
{

	m_TDSPI->qryAccount();
}

void CTPGateway::qryPosition()
{
	m_TDSPI->qryPosition();
}

std::shared_ptr<Event_Order>CTPGateway::getorder(std::string orderID)
{
	std::unique_lock<std::mutex>lck(m_ordermapmtx);
	if (m_ordermap.find(orderID) != m_ordermap.end())
	{
		return m_ordermap[orderID];
	}
	else
	{
		return nullptr;
	}
}

void CTPGateway::close()
{
	m_qryEnabled = false;
	m_MDSPI->close();
	m_TDSPI->close();

}

void CTPGateway::initQuery()
{
	std::this_thread::sleep_for(std::chrono::seconds(5));
	m_qryEnabled = true;
}

void CTPGateway::query()
{
	if (m_qryEnabled == false)
	{
		return;
	}
	m_qrycount++;
	if (m_qrycount == 2)
	{
		qryAccount();
	}
	else if (m_qrycount == 4)
	{
		qryPosition();
	}
	else if (m_qrycount > m_maxqry)
	{
		m_qrycount = 0;
	}
}

void CTPGateway::write_error(std::string msg, CThostFtdcRspInfoField* pRspInfo)
{
	std::string newMsg;
	newMsg = msg + ",代码" + std::to_string(pRspInfo->ErrorID) + ",信息" + pRspInfo->ErrorMsg;
	write_log(newMsg);

}