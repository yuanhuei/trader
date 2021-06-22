#include"ctpmd.h"
#include"../qcgateway.hpp"
#include"../ctp_gateway/ctpgateway.h"
#include<io.h>
#include<direct.h>

CTPMD::CTPMD(CTPGateway* CTPGateway, std::string gatewayname)//����֤û����
{
	m_ctpgateway = CTPGateway;
	m_gatewayname = gatewayname;

	std::string ninetoeleven[] = { "bu", "rb", "hc", "ru" };//9�㵽11��ĺ�Լ�б�
	std::string ninetohalfeleven[] = { "p", "j", "m", "y", "a", "b", "jm", "i", "SR", "CF", "RM", "MA", "ZC", "FG", "OI" };//9�㵽11���ĺ�Լ
	std::string ninetoone[] = { "cu", "al", "zn", "pb", "sn", "ni" };//9�㵽1��ĺ�Լ�б�
	std::string ninetohalftwo[] = { "ag", "au" };//9�㵽2���ĺ�Լ
	for (int i = 0; i < sizeof(ninetoeleven) / sizeof(ninetoeleven[0]); ++i)
	{
		m_ninetoeleven.insert(ninetoeleven[i]);
	}
	for (int i = 0; i < sizeof(ninetohalfeleven) / sizeof(ninetohalfeleven[0]); ++i)
	{
		m_ninetohalfeleven.insert(ninetohalfeleven[i]);
	}
	for (int i = 0; i < sizeof(ninetoone) / sizeof(ninetoone[0]); ++i)
	{
		m_ninetoone.insert(ninetoone[i]);
	}
	for (int i = 0; i < sizeof(ninetohalftwo) / sizeof(ninetohalftwo[0]); ++i)
	{
		m_ninetohalftwo.insert(ninetohalftwo[i]);
	}
}

CTPMD::~CTPMD()//����֤û����
{

}

void CTPMD::OnFrontConnected()//����֤û����
{
	//�������ӳɹ�
	m_connectionStatus = true;
	std::shared_ptr<Event_Log>e(new Event_Log);
	e->msg = "������������ӳɹ�";
	e->gatewayname = m_gatewayname;
	m_ctpgateway->onLog(e);
	login();
}
void CTPMD::OnFrontDisconnected(int nReason)//����֤û����
{
	//�������ӶϿ�
	m_connectionStatus = false;
	m_loginStatus = false;
	m_ctpgateway->ctpmdconnected = false;
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->gatewayname = m_gatewayname;
	e->msg = "������������ӶϿ�";
	m_ctpgateway->onLog(e);
}

void CTPMD::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//����֤û����
{
	std::shared_ptr<Event_Error>e = std::make_shared<Event_Error>();
	e->errorMsg = pRspInfo->ErrorMsg;
	e->errorID = pRspInfo->ErrorID;
	e->gatewayname = m_gatewayname;
	m_ctpgateway->onError(e);
}

void CTPMD::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//����֤û����
{
	//��¼�ر�
	if (!IsErrorRspInfo(pRspInfo))
	{
		//��¼�ɹ�
		m_loginStatus = true;
		m_ctpgateway->ctpmdconnected = true;
		std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		e->gatewayname = m_gatewayname;
		e->msg = "�����������¼���";
		m_ctpgateway->onLog(e);
	}
	else
	{
		std::shared_ptr<Event_Error>e = std::make_shared<Event_Error>();
		e->errorMsg = pRspInfo->ErrorMsg;
		e->errorID = pRspInfo->ErrorID;
		e->gatewayname = m_gatewayname;
		m_ctpgateway->onError(e);
	}
}

void CTPMD::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//����֤û����
{
	//�ǳ��ر�
	if (!IsErrorRspInfo(pRspInfo))
	{
		m_loginStatus = false;
		m_ctpgateway->ctpmdconnected = false;
		std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		e->gatewayname = m_gatewayname;
		e->msg = "����������ǳ����";
		m_ctpgateway->onLog(e);
	}
	else
	{
		std::shared_ptr<Event_Error>e = std::make_shared<Event_Error>();
		e->errorMsg = pRspInfo->ErrorMsg;
		e->errorID = pRspInfo->ErrorID;
		e->gatewayname = m_gatewayname;
		m_ctpgateway->onError(e);
	}
}


void CTPMD::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)//����֤û����
{
	std::shared_ptr<Event_Tick>Tick = std::make_shared<Event_Tick>();
	Tick->gatewayname = m_gatewayname;
	Tick->symbol = pDepthMarketData->InstrumentID;
	Tick->exchange = pDepthMarketData->ExchangeID;
	Tick->lastprice = pDepthMarketData->LastPrice;
	Tick->volume = pDepthMarketData->Volume;
	Tick->openInterest = pDepthMarketData->OpenInterest;
	Tick->time = std::string(pDepthMarketData->UpdateTime) + "." + std::to_string(pDepthMarketData->UpdateMillisec / 100);
	Tick->date = pDepthMarketData->TradingDay;

	Tick->openPrice = pDepthMarketData->OpenPrice;
	Tick->highPrice = pDepthMarketData->HighestPrice;
	Tick->lowPrice = pDepthMarketData->LowestPrice;
	Tick->preClosePrice = pDepthMarketData->PreClosePrice;
	Tick->upperLimit = pDepthMarketData->UpperLimitPrice;
	Tick->lowerLimit = pDepthMarketData->LowerLimitPrice;
	Tick->bidprice1 = pDepthMarketData->BidPrice1;
	Tick->askprice1 = pDepthMarketData->AskPrice1;
	Tick->bidvolume1 = pDepthMarketData->BidVolume1;
	Tick->askvolume1 = pDepthMarketData->AskVolume1;

	auto nowtime = Utils::getsystemunixdatetime(Tick->time, "s");//���ݰ�ʱ��
	auto nowtime2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string symbol = Utils::regMySymbol(Tick->symbol);

	if (m_ninetoeleven.find(symbol) != m_ninetoeleven.end())
	{
		if (!(((nowtime >= Utils::timetounixtimestamp(9, 0, 0) && nowtime <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime <= Utils::timetounixtimestamp(23, 0, 0)))))
		{
			//��9�㵽11����ͷ�Ķ����¼��򷵻�
			return;
		}
		else
		{
			//���ݰ���ʱ�����ڽ���ʱ��ε����ж�һ����ʵʱ���Ƿ�Ҳ�ڽ���ʱ���
			if (!(((nowtime2 >= Utils::timetounixtimestamp(9, 0, 0) && nowtime2 <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime2 >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime2 <= Utils::timetounixtimestamp(23, 0, 0)))))
			{
				return;
			}
		}
	}
	else if (m_ninetohalfeleven.find(symbol) != m_ninetohalfeleven.end())
	{
		if (!(((nowtime >= Utils::timetounixtimestamp(9, 0, 0) && nowtime <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime <= Utils::timetounixtimestamp(23, 30, 0)))))
		{
			return;
		}
		else
		{
			if (!(((nowtime2 >= Utils::timetounixtimestamp(9, 0, 0) && nowtime2 <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime2 >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime2 <= Utils::timetounixtimestamp(23, 30, 0)))))
			{
				return;
			}
		}

	}
	else if (m_ninetoone.find(symbol) != m_ninetoone.end())
	{
		if (!(((nowtime >= Utils::timetounixtimestamp(9, 0, 0) && nowtime <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime <= Utils::timetounixtimestamp(24, 0, 0))) || (nowtime <= Utils::timetounixtimestamp(1, 0, 0))))
		{
			return;
		}
		else
		{
			if (!(((nowtime2 >= Utils::timetounixtimestamp(9, 0, 0) && nowtime2 <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime2 >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime2 <= Utils::timetounixtimestamp(24, 0, 0))) || (nowtime2 <= Utils::timetounixtimestamp(1, 0, 0))))
			{
				return;
			}
		}
	}
	else if (m_ninetohalftwo.find(symbol) != m_ninetohalftwo.end())
	{
		if (!(((nowtime >= Utils::timetounixtimestamp(9, 0, 0) && nowtime <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime <= Utils::timetounixtimestamp(24, 0, 0))) || (nowtime <= Utils::timetounixtimestamp(2, 30, 0))))
		{
			return;
		}
		else
		{
			if (!(((nowtime2 >= Utils::timetounixtimestamp(9, 0, 0) && nowtime2 <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime2 >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime2 <= Utils::timetounixtimestamp(24, 0, 0))) || (nowtime2 <= Utils::timetounixtimestamp(2, 30, 0))))
			{
				return;
			}
		}
	}
	else
	{
		//���ֻ������û��ҹ�̵ĺ�Լ�����ж�һ�·�������ʱ���Ƿ��������̽���ʱ��֮��
		if (!(nowtime >= Utils::timetounixtimestamp(9, 0, 0) && nowtime <= Utils::timetounixtimestamp(15, 0, 0)))
		{
			return;
		}
		else
		{
			if (!(nowtime2 >= Utils::timetounixtimestamp(9, 0, 0) && nowtime2 <= Utils::timetounixtimestamp(15, 0, 0)))
			{
				return;
			}
		}
	}
	m_ctpgateway->onTick(Tick);
}

bool CTPMD::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)//����֤û����
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	return bResult;
}

void CTPMD::connect(std::string userID, std::string password, std::string brokerID, std::string address)//����֤û����
{
	//����
	m_userID = userID;
	m_password = password;
	m_brokerID = brokerID;
	m_address = address;
	//
	if (m_connectionStatus == false)
	{
		if (_access("./temp/", 0) == -1)
		{
			_mkdir("./temp/");
			//���������Ŀ¼��Ҫ����
		}
		m_mdapi = CThostFtdcMdApi::CreateFtdcMdApi("./temp/CTPmd");
		m_mdapi->RegisterSpi(this);
		m_mdapi->RegisterFront((char*)m_address.c_str());
		m_mdapi->Init();
	}
	else
	{
		if (m_loginStatus == true)
		{
			login();
		}
	}
}

void CTPMD::subscribe(SubscribeReq& subscribeReq)//����֤û����
{
	if (m_loginStatus == true)
	{
		subscribeMarketData(subscribeReq.symbol);
	}
	m_subscribedSymbols.insert(subscribeReq.symbol);
}

void CTPMD::subscribeMarketData(std::string instrumentID)//����֤û����
{
	char* buffer = (char*)instrumentID.c_str();
	char* myreq[1] = { buffer };
	this->m_mdapi->SubscribeMarketData(myreq, 1);
}

void CTPMD::login()//����֤û����
{
	//��¼
	if (m_userID.empty() == false && m_password.empty() == false && m_brokerID.empty() == false)
	{
		CThostFtdcReqUserLoginField myreq;
		strncpy(myreq.BrokerID, m_brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
		strncpy(myreq.UserID, m_userID.c_str(), sizeof(myreq.UserID) - 1);
		strncpy(myreq.Password, m_password.c_str(), sizeof(myreq.Password) - 1);
		m_reqID++;
		m_mdapi->ReqUserLogin(&myreq, m_reqID);
	}
}

void CTPMD::logout()
{
	CThostFtdcUserLogoutField myreq;
	strncpy(myreq.BrokerID, m_brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
	strncpy(myreq.UserID, m_userID.c_str(), sizeof(myreq.UserID) - 1);
	m_reqID++;
	m_mdapi->ReqUserLogout(&myreq, m_reqID);
}

void CTPMD::close()//����֤û����
{
	if (m_mdapi != NULL)
	{
		logout();
		m_mdapi->RegisterSpi(NULL);
		m_mdapi->Release();
		m_mdapi = NULL;
		m_connectionStatus = false;
	}
}
