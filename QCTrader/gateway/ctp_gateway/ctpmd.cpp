#include"ctpmd.h"
#include"ctpgateway.h"
#include<io.h>
#include<direct.h>

CTPMD::CTPMD(CTPGateway* CTPGateway, std::string gatewayname)
{
	m_ctpgateway = CTPGateway;
	m_gatewayname = gatewayname;

}
CTPMD::~CTPMD()
{

}
void CTPMD::OnFrontConnected()
{
	m_connectionStatus = true;
	m_ctpgateway->write_log("行情服务器连接成功");
	login();
}

void CTPMD::OnFrontDisconnected(int nReason)
{
	m_connectionStatus = false;
	m_ctpgateway->write_log("行情服务器连接断开，原因" + std::to_string(nReason));

}

void CTPMD::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (IsErrorRspInfo(pRspInfo) == true)
	{
		m_ctpgateway->write_error("行情服务器登陆失败", pRspInfo);
	}
	else
	{
		m_loginStatus = true;
		m_ctpgateway->write_log("行情服务器登陆成功");
	}
}

void CTPMD::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//已验证没问题
{
	//登出回报
	if (!IsErrorRspInfo(pRspInfo))
	{
		m_loginStatus = false;
		m_ctpgateway->ctpmdconnected = false;

		m_ctpgateway->write_log("行情服务器登出完成");
	}
	else
	{

		m_ctpgateway->write_error("行情服务器登出出错",pRspInfo);
	}
}

void CTPMD::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{

	m_ctpgateway->write_error("行情接口报错", pRspInfo);

}

void CTPMD::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID == 0)
	{
		m_ctpgateway->write_log("行情订阅成功");
		//插入到已订阅合约合集中
		std::string strInstrument = pSpecificInstrument->InstrumentID;
		m_subscribedSymbols.insert(strInstrument);
		return;
	}
	m_ctpgateway->write_error("行情订阅失败", pRspInfo);

}

void CTPMD::subscribe(SubscribeReq& subscribeReq)
{
	if (m_loginStatus == true)
	{
		this->subscribeMarketData(subscribeReq.symbol);
	}
}


void CTPMD::subscribeMarketData(std::string instrumentID)
{
	//const char* buffer = instrumentID.c_str();
	//char[20] newbuffer;
	//strcpy(newbuffer, buffer);
	char* myreq[1];
	myreq[0] =(char*) instrumentID.c_str();
	this->m_mdapi->SubscribeMarketData(myreq, 1);
}

void CTPMD::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
	
	//	Callback of tick data update.
		
	//	# Filter data update with no timestamp
	if (strlen(pDepthMarketData->UpdateTime) == 0)
		return;

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
	
	if (pDepthMarketData->AskVolume2 != 0 || pDepthMarketData->BidVolume2 != 0)
	{

		Tick->bidprice2 = pDepthMarketData->BidPrice2;
		Tick->askprice2 = pDepthMarketData->AskPrice2;
		Tick->bidvolume2 = pDepthMarketData->BidVolume2;
		Tick->askvolume2 = pDepthMarketData->AskVolume2;

		Tick->bidprice3 = pDepthMarketData->BidPrice3;
		Tick->askprice3 = pDepthMarketData->AskPrice3;
		Tick->bidvolume3 = pDepthMarketData->BidVolume3;
		Tick->askvolume3 = pDepthMarketData->AskVolume3;

		Tick->bidprice4 = pDepthMarketData->BidPrice4;
		Tick->askprice4 = pDepthMarketData->AskPrice4;
		Tick->bidvolume4 = pDepthMarketData->BidVolume4;
		Tick->askvolume4 = pDepthMarketData->AskVolume4;

		Tick->bidprice5 = pDepthMarketData->BidPrice5;
		Tick->askprice5 = pDepthMarketData->AskPrice5;
		Tick->bidvolume5 = pDepthMarketData->BidVolume5;
		Tick->askvolume5 = pDepthMarketData->AskVolume5;
	}

	m_ctpgateway->onTick(Tick);
}

void CTPMD::connect(std::string userID, std::string password, std::string brokerID, std::string address)
{
	m_userID = userID;
	m_password = password;
	m_brokerID = brokerID;
	m_address = address;

	if (m_connectionStatus == false)
	{
		if (_access("./temp/", 0) == -1)
		{
			_mkdir("/temp/");
		}
		m_mdapi = CThostFtdcMdApi::CreateFtdcMdApi("./temp/CTPmd");
		m_mdapi->RegisterSpi(this);
		std::string tcpaddress = "tcp://" + m_address;
		char pAddress[40];
		strcpy(pAddress, tcpaddress.c_str());
		m_mdapi->RegisterFront(pAddress);
		m_mdapi->Init();
	}
	else
	{
		if (m_loginStatus == false)
			login();
	}

}

void CTPMD::login()
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


void CTPMD::close()
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

bool CTPMD::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)//已验证没问题
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo!=NULL) && (pRspInfo->ErrorID != 0));
	return bResult;
}
