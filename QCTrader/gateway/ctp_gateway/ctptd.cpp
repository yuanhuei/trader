#include"ctptd.h"
#include"ctpgateway.h"
#include<io.h>
#include<direct.h>

CTPTD::CTPTD(CTPGateway* CTPGateway, std::string gatewayname)
{
	m_ctpgateway = CTPGateway;
	m_gatewayname = gatewayname;
	m_connectionStatus = false;
	m_loginStatus = false;
	m_reqID = 0;
	m_orderRef = 0;

}
CTPTD::~CTPTD()
{

}

void CTPTD::OnFrontConnected()
{
	m_connectionStatus = true;
	m_ctpgateway->write_log("交易服务器连接成功");
	if (m_authCode.length() != 0)
		authenticate();
	else
		login();
	
}

void CTPTD::authenticate()
{
	CThostFtdcReqAuthenticateField req;
	m_reqID++;
	strncpy(req.BrokerID, m_brokerID. c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.UserID, m_userID.c_str(), sizeof(req.UserID) - 1);
	strncpy(req.AuthCode, m_authCode.c_str(), sizeof(req.AuthCode) - 1);
	strncpy(req.AppID, m_appID.c_str(), sizeof(req.AppID) - 1);
	strncpy(req.UserProductInfo, m_productInfo.c_str(), sizeof(req.UserProductInfo) - 1);
	
	//if(m_productInfo.length()!=0)
		//strncpy(req.UserProductInfo, m_productInfo.c_str(), sizeof(req.UserProductInfo) - 1);

	m_tdapi->ReqAuthenticate(&req, m_reqID);
	
}

void CTPTD::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo))
	{
		auth_status = false;
		m_ctpgateway->write_log("交易服务器授权认证成功");
		login();
	}
	else
		m_ctpgateway->write_error("交易服务器授权验证失败", pRspInfo);
}

void CTPTD::OnFrontDisconnected(int nReason)
{
	m_connectionStatus = false;
	m_loginStatus = false;
	m_ctpgateway->write_log("交易服务器断开,原因:" + std::to_string(nReason));


}

void CTPTD::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//验证无问题
{
	if (pRspUserLogin == nullptr)
	{
		m_ctpgateway->write_log("返回空指针");
		return;
	}
	if (!IsErrorRspInfo(pRspInfo))
	{
		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;
		m_loginStatus = true;
		//m_ctpgateway->ctptdconnected = true;
		m_ctpgateway->write_log("交易服务器登录完成");

		//确认结算信息
		CThostFtdcSettlementInfoConfirmField myreq;
		strncpy(myreq.BrokerID, m_brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
		strncpy(myreq.InvestorID, m_userID.c_str(), sizeof(myreq.InvestorID) - 1);
		m_reqID++;
		m_tdapi->ReqSettlementInfoConfirm(&myreq, m_reqID);
	}
	else
	{
		//返回错误信息
		m_ctpgateway->write_error("交易服务器登录失败", pRspInfo);
		m_login_failed = true;

	}
}

void CTPTD::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//验证无问题
{
	if (pUserLogout == nullptr)
	{
		m_ctpgateway->write_log("返回空指针");
		return;
	}
	//登出回报
	if (!IsErrorRspInfo(pRspInfo))
	{
		m_loginStatus = false;
		//m_ctpgateway->ctptdconnected = false;
		m_ctpgateway->write_log("交易服务器登出完成");
	}
	else
	{
		//返回错误信息
		m_ctpgateway->write_error("交易服务器登出失败", pRspInfo);

	}
}

void CTPTD::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//验证无问题
{
	//错误发单柜台
	if (pInputOrder == nullptr)
	{
		 m_ctpgateway->write_log("返回空指针");
		return;
	}
	m_ctpgateway->write_error("交易委托失败", pRspInfo);
}

void CTPTD::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	if (pInputOrder == nullptr)
	{
		//空指针
		m_ctpgateway->write_log("返回空指针");
		return;
	}
	//发单错误回报(交易所)
	m_ctpgateway->write_error("交易委托失败", pRspInfo);


}

void CTPTD::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//验证无问题
{
	//撤单错误回报(柜台)
	if (pInputOrderAction == nullptr)
	{
		m_ctpgateway->write_log("返回空指针");
		return;
	}
	m_ctpgateway->write_error("交易撤单失败", pRspInfo);

}

/*
void CTPTD::OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo)
{
	if (pOrderAction == nullptr)
	{
		//空指针
		return;
	}
	//撤单错误回报(交易所)

	m_ctpgateway->write_error("交易撤单失败", pRspInfo);
}
*/

//结算单确认回调函数,在里面开始发送合约查询
void CTPTD::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//验证无问题
{
	
	if (pSettlementInfoConfirm == nullptr)
	{
		return;
	}
	m_ctpgateway->write_log("结算单确认完成");
	
	m_reqID++;
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	while (true)
	{
		if (m_connectionStatus == false)
		{
			return;
		}
		if (m_tdapi != nullptr)
		{
			int iResult = m_tdapi->ReqQryInstrument(&req, m_reqID);//请求查询合约
			if (iResult==-2 || iResult==-3)
			{
				m_ctpgateway->write_log("查询合约请求流控限制");
				std::this_thread::sleep_for(std::chrono::seconds(5)); //失败后等待5秒再发送

			}
			else if(iResult==-1)
			{
				m_ctpgateway->write_log("表示网络连接失败");
				std::this_thread::sleep_for(std::chrono::seconds(5));//失败后等待5秒再发送

			}
			else if (iResult == 0)
			{
				m_ctpgateway->write_log("查询合约请求发送");
				break;

			}
		}
	}
}

//账户查询回调函数
void CTPTD::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//验证无问题
{
	if (pTradingAccount == nullptr)
	{
		return;
	}
	std::shared_ptr<Event_Account>e = std::make_shared<Event_Account>();
	e->gatewayname = "CTP";
	e->accountid = pTradingAccount->AccountID;
	e->preBalance = pTradingAccount->PreBalance;
	e->available = pTradingAccount->Available;
	e->commission = pTradingAccount->Commission;
	e->margin = pTradingAccount->CurrMargin;
	e->closeProfit = pTradingAccount->CloseProfit;
	e->positionProfit = pTradingAccount->PositionProfit;

	e->balance = (pTradingAccount->PreBalance - pTradingAccount->PreCredit - pTradingAccount->PreMortgage +
		pTradingAccount->Mortgage - pTradingAccount->Withdraw + pTradingAccount->Deposit +
		pTradingAccount->CloseProfit + pTradingAccount->PositionProfit + pTradingAccount->CashIn -
		pTradingAccount->Commission);
	m_ctpgateway->onAccount(e);
}

//持仓查询回调函数
void CTPTD::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	
	if (pInvestorPosition == nullptr)
	{
		//空指针
		return;
	}
	std::string symboldirection = pInvestorPosition->InstrumentID;//合约名
	symboldirection += pInvestorPosition->PosiDirection;          //合约+多空方向作为键，Event_Position为值
	std::unique_lock<std::mutex>lck(m_positionbuffermtx);         //加锁
	if (m_posBufferMap.find(symboldirection) == m_posBufferMap.end())//如果找不到这个合约则创建一个
	{
		std::shared_ptr<Event_Position>e = std::make_shared<Event_Position>();
		m_posBufferMap.insert(std::pair<std::string, std::shared_ptr<Event_Position>>(symboldirection, e));//创建一个键和值
	}
	m_posBufferMap[symboldirection]->gatewayname = "CTP";
	m_posBufferMap[symboldirection]->symbol = pInvestorPosition->InstrumentID;
	if (pInvestorPosition->PosiDirection == '2')
	{
		m_posBufferMap[symboldirection]->direction = DIRECTION_LONG;
	}
	else if (pInvestorPosition->PosiDirection == '3')
	{
		m_posBufferMap[symboldirection]->direction = DIRECTION_SHORT;
	}

	std::string exchange = m_symbolExchangeMap[pInvestorPosition->InstrumentID];

/*对于非上期 / 能源的交易所，合约的昨仓YdPosition和今仓TodayPosition在一条记录里面，而上期 / 能源是分成了两条记录
具体参考网页https://mp.weixin.qq.com/s?__biz=Mzg5MjEwNDEwMQ==&mid=2247483885&idx=1&sn=771b9a43ac413837a9cd37665c9cd5ef&chksm=cfc27d27f8b5f431f959381e30ff53b2a4be24ca1bcfb82e60a2c71c538432c59a2709b1e4cb&scene=178&cur_album_id=1545992091560968194#rd
*/
	if (exchange == EXCHANGE_SHFE|| exchange==EXCHANGE_INE )//上期 / 能源交易所
	{
		//  两条记录中昨仓的记录，记录中的position是针对昨仓和今仓的，不是真实的总持仓
		if (pInvestorPosition->YdPosition)
		{
			m_posBufferMap[symboldirection]->ydPosition = pInvestorPosition->Position;
			m_posBufferMap[symboldirection]->ydPositionCost = pInvestorPosition->PositionCost;
		}
		else
		{
			m_posBufferMap[symboldirection]->todayPosition = pInvestorPosition->Position;
			m_posBufferMap[symboldirection]->todayPositionCost = pInvestorPosition->PositionCost;
		}

		m_posBufferMap[symboldirection]->position = m_posBufferMap[symboldirection]->ydPosition + m_posBufferMap[symboldirection]->todayPosition;

		if (m_posBufferMap[symboldirection]->todayPosition || m_posBufferMap[symboldirection]->ydPosition)//昨仓和今仓有一个非0
		{
			//计算持仓成本，按昨仓和今仓合纵计算
			m_posBufferMap[symboldirection]->price = (m_posBufferMap[symboldirection]->ydPositionCost + m_posBufferMap[symboldirection]->todayPositionCost) / ((m_posBufferMap[symboldirection]->todayPosition + m_posBufferMap[symboldirection]->ydPosition) * m_symbolSizeMap[pInvestorPosition->InstrumentID]);
		}
		else//空仓
		{
			m_posBufferMap[symboldirection]->price = 0;
		}
	}
	else//其他交易所
	{
		m_posBufferMap[symboldirection]->position = pInvestorPosition->Position;
		m_posBufferMap[symboldirection]->ydPosition = 0;//无昨仓的概念

		if (m_posBufferMap[symboldirection]->position)
		{
			m_posBufferMap[symboldirection]->price = pInvestorPosition->PositionCost / (m_posBufferMap[symboldirection]->position * m_symbolSizeMap[pInvestorPosition->InstrumentID]);
		}
		else
		{
			m_posBufferMap[symboldirection]->price = 0;
		}
	}
	if (bIsLast == true)
	{
		//如果是最后一笔数据
		for (std::map<std::string, std::shared_ptr<Event_Position>>::iterator it = m_posBufferMap.begin(); it != m_posBufferMap.end(); it++)
		{
			//循环推入position
			m_ctpgateway->onPosition(it->second);
		}
	}
}

//发送合约查询的回调函数
void CTPTD::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//验证无问题
{
	if (pInstrument == nullptr)
	{
		//空指针
		return;
	}
	//合约查询
	std::shared_ptr<Event_Contract>e = std::make_shared<Event_Contract>();
	e->gatewayname = m_gatewayname;
	e->symbol = std::string(pInstrument->InstrumentID);
	e->exchange = pInstrument->ExchangeID;
	e->name = pInstrument->InstrumentName;
	e->optionType = pInstrument->OptionsType;
	e->priceTick = pInstrument->PriceTick;
	e->productClass = pInstrument->ProductClass;
	e->size = pInstrument->VolumeMultiple;
	e->strikePrice = pInstrument->StrikePrice;
	e->underlyingSymbol = pInstrument->UnderlyingInstrID;

	//获取之后放入两个map中，一个是合约和交易所map，一个是合约与合约价格乘数map
	m_symbolExchangeMap.insert(std::pair<std::string, std::string>(e->symbol, e->exchange));
	m_symbolSizeMap.insert(std::pair<std::string, int>(e->symbol, e->size));
	m_ctpgateway->onContract(e);
	if (bIsLast)
	{
		std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		e->gatewayname = m_gatewayname;
		e->msg = "合约查询完成";
		m_ctpgateway->onLog(e);
		m_ctpgateway->initQuery();
	}
}

//委托成功回报函数
void CTPTD::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	if (pOrder == nullptr)
	{
		//空指针
		return;
	}
	//报单编号不断累加
	const char* neworderref = pOrder->OrderRef;
	m_orderRefmtx.lock();
	m_orderRef = std::max(atoi(neworderref), m_orderRef);
	m_orderRefmtx.unlock();

	//从回报中获取订单最新状态
	std::shared_ptr<Event_Order>e = std::make_shared<Event_Order>();
	e->symbol = pOrder->InstrumentID;
	e->exchange = pOrder->ExchangeID;
	e->orderID = pOrder->OrderRef;
	e->gatewayname = "CTP";
	if (pOrder->Direction == '0')
	{
		//多
		e->direction = DIRECTION_LONG;
	}
	else if (pOrder->Direction == '1')
	{
		//空
		e->direction = DIRECTION_SHORT;
	}
	else
	{
		//未知
		e->direction = DIRECTION_UNKNOWN;
	}

	if (pOrder->CombOffsetFlag[0] == '0')
	{
		//开
		e->offset = OFFSET_OPEN;
	}
	else if (pOrder->CombOffsetFlag[0] == '1')
	{
		e->offset = OFFSET_CLOSE;
	}
	else if (pOrder->CombOffsetFlag[0] == '3')
	{
		e->offset = OFFSET_CLOSETODAY;
	}
	else if (pOrder->CombOffsetFlag[0] == '4')
	{
		e->offset = OFFSET_CLOSEYESTERDAY;
	}
	else
	{
		e->offset = OFFSET_UNKNOWN;
	}

	if (pOrder->OrderStatus == THOST_FTDC_OST_AllTraded)
	{
		//全成交
		e->status = STATUS_ALLTRADED;
	}
	else if (pOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing)
	{
		//部分成交
		e->status = THOST_FTDC_OST_PartTradedQueueing;
	}
	else if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
	{
		//未成交
		e->status = THOST_FTDC_OST_NoTradeQueueing;
	}
	else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
	{
		//撤销
		e->status = STATUS_CANCELLED;
	}
	else
	{
		//未知
		e->status = THOST_FTDC_OST_Unknown;
	}

	e->price = pOrder->LimitPrice;
	e->totalVolume = pOrder->VolumeTotalOriginal;
	e->tradedVolume = pOrder->VolumeTraded;
	e->orderTime = pOrder->InsertTime;
	e->cancelTime = pOrder->CancelTime;
	e->frontID = pOrder->FrontID;
	e->sessionID = pOrder->SessionID;

	//维护订单状态，订单通过m_ordermap维护，其中key为orderID,orderID来自于OrderRef，
	if (e->status == STATUS_ALLTRADED || e->status == STATUS_CANCELLED)//订单成交或者取消，从map中删除
	{
		std::unique_lock<std::mutex>lck(m_ctpgateway->m_ordermapmtx);
		if (m_ctpgateway->m_ordermap.find(e->orderID) != m_ctpgateway->m_ordermap.end())
		{
			m_ctpgateway->m_ordermap.erase(e->orderID);
		}
	}
	else//其他状况更新订单状态
	{
		std::unique_lock<std::mutex>lck(m_ctpgateway->m_ordermapmtx);
		if (m_ctpgateway->m_ordermap.find(e->orderID) != m_ctpgateway->m_ordermap.end())//新订单，插入
		{
			m_ctpgateway->m_ordermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(e->orderID, e));
		}
		else//老订单更新状态
		{
			m_ctpgateway->m_ordermap[e->orderID] = e;
		}
	}

	m_ctpgateway->onOrder(e);
}
//针对用户请求的出错通知
void CTPTD::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo == nullptr)
	{
		//空指针
		return;
	}
	m_ctpgateway->write_error("用户请求出错", pRspInfo);

}

void CTPTD::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	if (pTrade == nullptr)
	{
		//空指针
		return;
	}
	std::shared_ptr<Event_Trade>e = std::make_shared<Event_Trade>();
	e->gatewayname = m_gatewayname;
	e->symbol = pTrade->InstrumentID;
	e->exchange = pTrade->ExchangeID;
	e->tradeID = pTrade->TradeID;
	e->orderID = pTrade->OrderRef;
	if (pTrade->Direction == THOST_FTDC_D_Buy)
	{
		//多
		e->direction = DIRECTION_LONG;
	}
	else if (pTrade->Direction == THOST_FTDC_D_Sell)
	{
		//空
		e->direction = DIRECTION_SHORT;
	}
	else
	{
		//未知
		e->direction = DIRECTION_UNKNOWN;
	}

	if (pTrade->OffsetFlag == THOST_FTDC_OF_Open)
	{
		//开
		e->offset = OFFSET_OPEN;
	}
	else if (pTrade->OffsetFlag == THOST_FTDC_OF_Close )
	{
		e->offset = OFFSET_CLOSE;
	}
	else if (pTrade->OffsetFlag == THOST_FTDC_OF_CloseToday)
	{
		e->offset = OFFSET_CLOSETODAY;
	}
	else if (pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday)
	{
		e->offset = OFFSET_CLOSEYESTERDAY;
	}
	else
	{
		e->offset = OFFSET_UNKNOWN;
	}
	e->price = pTrade->Price;
	e->volume = pTrade->Volume;
	e->tradeTime = Utils::getCurrentSystemDate() + pTrade->TradeTime;
	m_ctpgateway->onTrade(e);
}

//报单录入错误回报
/*
void CTPTD::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	if (pInputOrder == nullptr)
	{
		//空指针
		return;
	}
	//发单错误回报(交易所)

	m_ctpgateway->write_error("订单提交错误",pRspInfo);
};
*/
//撤单错误回报
void CTPTD::OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo)
{
	if (pOrderAction == nullptr)
	{
		//空指针
		return;
	}
	//撤单错误回报(交易所)
	if(IsErrorRspInfo(pRspInfo))
		m_ctpgateway->write_error("撤单错误",pRspInfo);
}


bool CTPTD::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{//错误代码为 0 时，表示操作成功
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	return bResult;
}


bool CTPTD::IsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}


void CTPTD::connect(std::string userID, std::string password, std::string brokerID, std::string address, std::string authcode, std::string appid, std::string productinfo)
{
	m_userID = userID;
	m_password = password;
	m_brokerID = brokerID;
	m_address = address;
	m_authCode = authcode;
	m_appID = appid;
	m_productInfo = productinfo;
	if (m_connectionStatus == false)
	{
		if (_access("./temp", 0) == -1)
		{
			_mkdir("./temp");
			//不存在这个目录需要创建
		}
		m_tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi("./temp/CTPtd");
		m_tdapi->RegisterSpi(this);
		std::string tcpaddress = "tcp://" + m_address;
		char pAddress[40];
		strcpy(pAddress, tcpaddress.c_str());
		m_tdapi->RegisterFront(pAddress);
		m_tdapi->SubscribePublicTopic(THOST_TERT_QUICK);
		m_tdapi->SubscribePrivateTopic(THOST_TERT_QUICK);
		m_tdapi->Init();
	
	}
	else
	{
		authenticate();
		//if (m_loginStatus == false)
		//{
		//	login();
		//}
	}

}

void CTPTD::login()
{
	if (m_userID.empty() == false && m_password.empty() == false && m_brokerID.empty() == false)
	{
		CThostFtdcReqUserLoginField myreq;
		strncpy(myreq.BrokerID, m_brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
		strncpy(myreq.UserID, m_userID.c_str(), sizeof(myreq.UserID) - 1);
		strncpy(myreq.Password, m_password.c_str(), sizeof(myreq.Password) - 1);
		m_reqID++;
		m_tdapi->ReqUserLogin(&myreq, m_reqID);
	}
}

//提交订单
std::string CTPTD::sendOrder(OrderReq& req)
{
	std::unique_lock<std::mutex>lck(m_orderRefmtx);
	m_orderRef += 1;
	m_reqID += 1;
	CThostFtdcInputOrderField myreq;
	memset(&myreq, 0, sizeof(myreq));

	strncpy(myreq.BrokerID, m_brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
	strncpy(myreq.InvestorID, m_userID.c_str(), sizeof(myreq.InvestorID) - 1);
	strncpy(myreq.OrderRef, std::to_string(m_orderRef).c_str(), sizeof(myreq.OrderRef) - 1);
	strncpy(myreq.InstrumentID, req.symbol.c_str(), sizeof(myreq.InstrumentID) - 1);
	strncpy(myreq.UserID, m_userID.c_str(), sizeof(myreq.UserID) - 1);

	myreq.LimitPrice = req.price;
	myreq.VolumeTotalOriginal = req.volume;
	if (req.priceType == PRICETYPE_LIMITPRICE)
	{
		myreq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
	}
	if (req.priceType == PRICETYPE_MARKETPRICE)
	{
		myreq.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
	}

	if (req.direction == DIRECTION_LONG)
	{
		myreq.Direction = THOST_FTDC_D_Buy;
	}
	if (req.direction == DIRECTION_SHORT)
	{
		myreq.Direction = THOST_FTDC_D_Sell;
	}

	if (req.offset == OFFSET_OPEN)
	{
		myreq.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	}
	if (req.offset == OFFSET_CLOSE)
	{
		myreq.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
	}
	if (req.offset == OFFSET_CLOSETODAY)
	{
		myreq.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
	}
	if (req.offset == OFFSET_CLOSEYESTERDAY)
	{
		myreq.CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;
	}

	myreq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	myreq.ContingentCondition = THOST_FTDC_CC_Immediately;
	myreq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	myreq.IsAutoSuspend = 0;
	myreq.TimeCondition = THOST_FTDC_TC_GFD;
	myreq.VolumeCondition = THOST_FTDC_VC_AV;
	myreq.MinVolume = 1;

	if (req.priceType == PRICETYPE_FAK)
	{
		myreq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		myreq.TimeCondition = THOST_FTDC_TC_IOC;
		myreq.VolumeCondition = THOST_FTDC_VC_AV;
	}
	if (req.priceType == PRICETYPE_FOK)
	{
		myreq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		myreq.TimeCondition = THOST_FTDC_TC_IOC;
		myreq.VolumeCondition = THOST_FTDC_VC_CV;
	}

	int i = m_tdapi->ReqOrderInsert(&myreq, m_reqID);
	if (i == 0)
		m_ctpgateway->write_log("订单发送成功");
	else if(i==-1)
		m_ctpgateway->write_log("订单发送,网络连接失败");
	else if(i==-2)
		m_ctpgateway->write_log("订单发送,未处理请求超过许可数");
	else if(i==-3)
		m_ctpgateway->write_log("订单发送,每秒发送请求数超过许可数");
	return std::to_string(m_orderRef);
}
//取消订单
void CTPTD::cancelOrder(CancelOrderReq& req)
{
	m_reqID += 1;
	CThostFtdcInputOrderActionField myreq = CThostFtdcInputOrderActionField();
	memset(&myreq, 0, sizeof(myreq));

	strncpy(myreq.BrokerID, m_brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
	strncpy(myreq.InvestorID, m_userID.c_str(), sizeof(myreq.InvestorID) - 1);
	strncpy(myreq.InstrumentID, req.symbol.c_str(), sizeof(myreq.InstrumentID) - 1);
	strncpy(myreq.ExchangeID, req.exchange.c_str(), sizeof(myreq.ExchangeID) - 1);
	strncpy(myreq.OrderRef, req.orderID.c_str(), sizeof(myreq.OrderRef) - 1);
	myreq.FrontID = m_frontID;
	myreq.SessionID = m_sessionID;

	myreq.ActionFlag = THOST_FTDC_AF_Delete;

	m_tdapi->ReqOrderAction(&myreq, m_reqID);
}
//查询仓位
void CTPTD::qryPosition()
{
	m_reqID++;
	CThostFtdcQryInvestorPositionField req = CThostFtdcQryInvestorPositionField();
	memset(&req, 0, sizeof(req));
	strncpy(req.BrokerID, m_brokerID.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, m_userID.c_str(), sizeof(req.InvestorID) - 1);
	if (m_tdapi != nullptr)
	{
		m_tdapi->ReqQryInvestorPosition(&req, m_reqID);
	}
}
//查询账户信息
void CTPTD::qryAccount()
{
	m_reqID++;
	CThostFtdcQryTradingAccountField req = CThostFtdcQryTradingAccountField();
	memset(&req, 0, sizeof(req));
	strncpy(req.BrokerID, m_brokerID.c_str(), sizeof(req.BrokerID) - 1);
	strncpy(req.InvestorID, m_userID.c_str(), sizeof(req.InvestorID) - 1);
	if (m_tdapi != nullptr)
	{
		m_tdapi->ReqQryTradingAccount(&req, m_reqID);
	}
}

void CTPTD::logout()
{
	CThostFtdcUserLogoutField myreq;
	strncpy(myreq.BrokerID, m_brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
	strncpy(myreq.UserID, m_userID.c_str(), sizeof(myreq.UserID) - 1);
	m_reqID++;
	m_tdapi->ReqUserLogout(&myreq, m_reqID);
}


void CTPTD::close()
{
	if (m_tdapi != NULL)
	{
		logout();
		m_tdapi->RegisterSpi(NULL);
		m_tdapi->Release();
		m_tdapi = NULL;
		m_connectionStatus = false;
	}
}