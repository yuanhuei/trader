#include"ctpmd.h"
#include"../qcgateway.hpp"
#include"../ctp_gateway/ctpgateway.h"
#include<io.h>
#include<direct.h>

CTPMD::CTPMD(CTPGateway* CTPGateway, std::string gatewayname)//已验证没问题
{
	m_ctpgateway = CTPGateway;
	m_gatewayname = gatewayname;

	std::string ninetoeleven[] = { "bu", "rb", "hc", "ru" };//9点到11点的合约列表
	std::string ninetohalfeleven[] = { "p", "j", "m", "y", "a", "b", "jm", "i", "SR", "CF", "RM", "MA", "ZC", "FG", "OI" };//9点到11点半的合约
	std::string ninetoone[] = { "cu", "al", "zn", "pb", "sn", "ni" };//9点到1点的合约列表
	std::string ninetohalftwo[] = { "ag", "au" };//9点到2点半的合约
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

CTPMD::~CTPMD()//已验证没问题
{

}

void CTPMD::OnFrontConnected()//已验证没问题
{
	//行情连接成功
	m_connectionStatus = true;
	std::shared_ptr<Event_Log>e(new Event_Log);
	e->msg = "行情服务器连接成功";
	e->gatewayname = m_gatewayname;
	m_ctpgateway->onLog(e);
	login();
}
void CTPMD::OnFrontDisconnected(int nReason)//已验证没问题
{
	//行情连接断开
	m_connectionStatus = false;
	m_loginStatus = false;
	m_ctpgateway->ctpmdconnected = false;
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->gatewayname = m_gatewayname;
	e->msg = "行情服务器连接断开";
	m_ctpgateway->onLog(e);
}

void CTPMD::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//已验证没问题
{
	std::shared_ptr<Event_Error>e = std::make_shared<Event_Error>();
	e->errorMsg = pRspInfo->ErrorMsg;
	e->errorID = pRspInfo->ErrorID;
	e->gatewayname = m_gatewayname;
	m_ctpgateway->onError(e);
}

void CTPMD::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//已验证没问题
{
	//登录回报
	if (!IsErrorRspInfo(pRspInfo))
	{
		//登录成功
		m_loginStatus = true;
		m_ctpgateway->ctpmdconnected = true;
		std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		e->gatewayname = m_gatewayname;
		e->msg = "行情服务器登录完成";
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

void CTPMD::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//已验证没问题
{
	//登出回报
	if (!IsErrorRspInfo(pRspInfo))
	{
		m_loginStatus = false;
		m_ctpgateway->ctpmdconnected = false;
		std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		e->gatewayname = m_gatewayname;
		e->msg = "行情服务器登出完成";
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


void CTPMD::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)//已验证没问题
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

	auto nowtime = Utils::getsystemunixdatetime(Tick->time, "s");//数据包时间
	auto nowtime2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string symbol = Utils::regMySymbol(Tick->symbol);

	if (m_ninetoeleven.find(symbol) != m_ninetoeleven.end())
	{
		if (!(((nowtime >= Utils::timetounixtimestamp(9, 0, 0) && nowtime <= Utils::timetounixtimestamp(15, 0, 0))) || ((nowtime >= Utils::timetounixtimestamp(21, 0, 0)) && (nowtime <= Utils::timetounixtimestamp(23, 0, 0)))))
		{
			//在9点到11点里头的对立事件则返回
			return;
		}
		else
		{
			//数据包的时间是在交易时间段的在判断一下真实时间是否也在交易时间段
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
		//如果只是日盘没有夜盘的合约，就判断一下发过来的时间是否是在日盘交易时间之内
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

bool CTPMD::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)//已验证没问题
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	return bResult;
}

void CTPMD::connect(std::string userID, std::string password, std::string brokerID, std::string address)//已验证没问题
{
	//连接
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
			//不存在这个目录需要创建
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

void CTPMD::subscribe(SubscribeReq& subscribeReq)//已验证没问题
{
	if (m_loginStatus == true)
	{
		subscribeMarketData(subscribeReq.symbol);
	}
	m_subscribedSymbols.insert(subscribeReq.symbol);
}

void CTPMD::subscribeMarketData(std::string instrumentID)//已验证没问题
{
	char* buffer = (char*)instrumentID.c_str();
	char* myreq[1] = { buffer };
	this->m_mdapi->SubscribeMarketData(myreq, 1);
}

void CTPMD::login()//已验证没问题
{
	//登录
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

void CTPMD::close()//已验证没问题
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
