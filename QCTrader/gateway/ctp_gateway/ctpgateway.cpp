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

CTPGateway::CTPGateway(EventEngine* eventengine, std::string gatewayname) :QCGateway(eventengine, "CTP")
{
	m_eventengine = eventengine;
	m_gatewayname = gatewayname;

	//��������ͽ��׵Ļص�����
	m_MDSPI = new CTPMD(this, gatewayname);
	m_TDSPI = new CTPTD(this, gatewayname);

	ctpmdconnected = false;
	ctptdconnected = false;
	m_qryEnabled = false;
	m_qrycount = 0;
	m_eventengine->RegEvent(EVENT_TIMER, std::bind(&CTPGateway::query, this));//ע�ᵽ�¼��������棬ѭ����ѯ
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

void CTPGateway::connect()
{
	//��������ͽ���
	//��ɨ���ĵ��µ��˺����������ļ�
	if (_access("./CTPGateway", 0) != -1)
	{
		std::fstream f;
		f.open("./CTPGateway/CTP_connect");
		if (!f.is_open())
		{
			//����򲻿��ļ�
			std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
			e->msg = "�޷���ȡ���������ļ�";
			e->gatewayname = m_gatewayname;
			this->onLog(e);
			return;
		}
		std::string line;
		std::map<std::string, std::string>configmap;
		while (!f.eof())
		{
			getline(f, line);
			std::string::size_type pos = line.find("=");//���յȺŷָ�
			configmap.insert(std::make_pair(line.substr(0, pos), line.substr(pos + 1, line.size() - 1)));
		}
		m_MDSPI->connect(configmap["userid"], configmap["password"], configmap["brokerid"], configmap["mdaddress"]);
		m_TDSPI->connect(configmap["userid"], configmap["password"], configmap["brokerid"], configmap["tdaddress"]);
	}
	else
	{
		std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		e->msg = "�޷���ȡ���������ļ�";
		e->gatewayname = m_gatewayname;
		this->onLog(e);
	}
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
	newMsg = msg + ",����" + std::to_string(pRspInfo->ErrorID) + ",��Ϣ" + pRspInfo->ErrorMsg;
	write_log(newMsg);

}