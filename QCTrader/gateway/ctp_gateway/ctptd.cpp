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
	m_ctpgateway->write_log("���׷��������ӳɹ�");
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
		m_ctpgateway->write_log("���׷�������Ȩ��֤�ɹ�");
		login();
	}
	else
		m_ctpgateway->write_error("���׷�������Ȩ��֤ʧ��", pRspInfo);
}

void CTPTD::OnFrontDisconnected(int nReason)
{
	m_connectionStatus = false;
	m_loginStatus = false;
	m_ctpgateway->write_log("���׷������Ͽ�,ԭ��:" + std::to_string(nReason));


}

void CTPTD::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//��֤������
{
	if (pRspUserLogin == nullptr)
	{
		m_ctpgateway->write_log("���ؿ�ָ��");
		return;
	}
	if (!IsErrorRspInfo(pRspInfo))
	{
		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;
		m_loginStatus = true;
		//m_ctpgateway->ctptdconnected = true;
		m_ctpgateway->write_log("���׷�������¼���");

		//ȷ�Ͻ�����Ϣ
		CThostFtdcSettlementInfoConfirmField myreq;
		strncpy(myreq.BrokerID, m_brokerID.c_str(), sizeof(myreq.BrokerID) - 1);
		strncpy(myreq.InvestorID, m_userID.c_str(), sizeof(myreq.InvestorID) - 1);
		m_reqID++;
		m_tdapi->ReqSettlementInfoConfirm(&myreq, m_reqID);
	}
	else
	{
		//���ش�����Ϣ
		m_ctpgateway->write_error("���׷�������¼ʧ��", pRspInfo);
		m_login_failed = true;

	}
}

void CTPTD::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//��֤������
{
	if (pUserLogout == nullptr)
	{
		m_ctpgateway->write_log("���ؿ�ָ��");
		return;
	}
	//�ǳ��ر�
	if (!IsErrorRspInfo(pRspInfo))
	{
		m_loginStatus = false;
		//m_ctpgateway->ctptdconnected = false;
		m_ctpgateway->write_log("���׷������ǳ����");
	}
	else
	{
		//���ش�����Ϣ
		m_ctpgateway->write_error("���׷������ǳ�ʧ��", pRspInfo);

	}
}
//����¼������¼�����ʱ��Ӧ��ӦOnRspOrderInsert��OnErrRtnOrderInsert����ȷʱ��Ӧ�ر�OnRtnOrder��OnRtnTrade��
void CTPTD::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//��֤������
{
	//���󷢵���̨
	if (pInputOrder == nullptr)
	{
		 m_ctpgateway->write_log("���ؿ�ָ��");
		return;
	}
	m_ctpgateway->write_error("����ί��ʧ��", pRspInfo);
}

void CTPTD::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	if (pInputOrder == nullptr)
	{
		//��ָ��
		m_ctpgateway->write_log("���ؿ�ָ��");
		return;
	}
	//��������ر�(������)
	m_ctpgateway->write_error("����ί��ʧ��", pRspInfo);


}

void CTPTD::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//��֤������
{
	//��������ر�(��̨)
	if (pInputOrderAction == nullptr)
	{
		m_ctpgateway->write_log("���ؿ�ָ��");
		return;
	}
	m_ctpgateway->write_error("���׳���ʧ��", pRspInfo);

}

/*
void CTPTD::OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo)
{
	if (pOrderAction == nullptr)
	{
		//��ָ��
		return;
	}
	//��������ر�(������)

	m_ctpgateway->write_error("���׳���ʧ��", pRspInfo);
}
*/

//���㵥ȷ�ϻص�����,�����濪ʼ���ͺ�Լ��ѯ
void CTPTD::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//��֤������
{
	
	if (pSettlementInfoConfirm == nullptr)
	{
		return;
	}
	m_ctpgateway->write_log("���㵥ȷ�����");
	
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
			int iResult = m_tdapi->ReqQryInstrument(&req, m_reqID);//�����ѯ��Լ
			if (iResult==-2 || iResult==-3)
			{
				m_ctpgateway->write_log("��ѯ��Լ������������");
				std::this_thread::sleep_for(std::chrono::seconds(5)); //ʧ�ܺ�ȴ�5���ٷ���

			}
			else if(iResult==-1)
			{
				m_ctpgateway->write_log("��ʾ��������ʧ��");
				std::this_thread::sleep_for(std::chrono::seconds(5));//ʧ�ܺ�ȴ�5���ٷ���

			}
			else if (iResult == 0)
			{
				m_ctpgateway->write_log("��ѯ��Լ������");
				break;

			}
		}
	}
}

//�˻���ѯ�ص�����
void CTPTD::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//��֤������
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

//�ֲֲ�ѯ�ص�����
void CTPTD::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//�����ѯʱ�ͻ�û���κγֲּ�¼��APIҲ��ص��ú�����ֻ��pInvestorPositionָ��Ϊ�գ�bIsLast�ֶ�Ϊtrue��
	if (pInvestorPosition == nullptr)
	{
		//��ָ��
		//m_ctpgateway->write_log("����Ϊ�գ��޲�λ");
		return;
	}
	std::string symboldirection = pInvestorPosition->InstrumentID;//��Լ��
	symboldirection += pInvestorPosition->PosiDirection;          //��Լ+��շ�����Ϊ����Event_PositionΪֵ
	std::unique_lock<std::mutex>lck(m_positionbuffermtx);         //����
	if (m_posBufferMap.find(symboldirection) == m_posBufferMap.end())//����Ҳ��������Լ�򴴽�һ��
	{
		std::shared_ptr<Event_Position>e = std::make_shared<Event_Position>();
		m_posBufferMap.insert(std::pair<std::string, std::shared_ptr<Event_Position>>(symboldirection, e));//����һ������ֵ
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

/*���ڷ����� / ��Դ�Ľ���������Լ�����YdPosition�ͽ��TodayPosition��һ����¼���棬������ / ��Դ�Ƿֳ���������¼
����ο���ҳhttps://mp.weixin.qq.com/s?__biz=Mzg5MjEwNDEwMQ==&mid=2247483885&idx=1&sn=771b9a43ac413837a9cd37665c9cd5ef&chksm=cfc27d27f8b5f431f959381e30ff53b2a4be24ca1bcfb82e60a2c71c538432c59a2709b1e4cb&scene=178&cur_album_id=1545992091560968194#rd
*/
	if (exchange == EXCHANGE_SHFE|| exchange==EXCHANGE_INE )//���� / ��Դ������
	{
		//  ������¼����ֵļ�¼����¼�е�position�������ֺͽ�ֵģ�������ʵ���ֲܳ�
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

		if (m_posBufferMap[symboldirection]->todayPosition || m_posBufferMap[symboldirection]->ydPosition)//��ֺͽ����һ����0
		{
			//����ֲֳɱ�������ֺͽ�ֺ��ݼ���
			m_posBufferMap[symboldirection]->price = (m_posBufferMap[symboldirection]->ydPositionCost + m_posBufferMap[symboldirection]->todayPositionCost) / ((m_posBufferMap[symboldirection]->todayPosition + m_posBufferMap[symboldirection]->ydPosition) * m_symbolSizeMap[pInvestorPosition->InstrumentID]);
		}
		else//�ղ�
		{
			m_posBufferMap[symboldirection]->price = 0;
		}
	}
	else//����������
	{
		m_posBufferMap[symboldirection]->position = pInvestorPosition->Position;
		m_posBufferMap[symboldirection]->ydPosition = 0;//����ֵĸ���

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
		//��������һ������
		for (std::map<std::string, std::shared_ptr<Event_Position>>::iterator it = m_posBufferMap.begin(); it != m_posBufferMap.end(); it++)
		{
			//ѭ������position
			m_ctpgateway->onPosition(it->second);
		}
	}
}

//���ͺ�Լ��ѯ�Ļص�����
void CTPTD::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//��֤������
{
	if (pInstrument == nullptr)
	{
		//��ָ��
		return;
	}
	//��Լ��ѯ
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

	//��ȡ֮���������map�У�һ���Ǻ�Լ�ͽ�����map��һ���Ǻ�Լ���Լ�۸����map
	m_symbolExchangeMap.insert(std::pair<std::string, std::string>(e->symbol, e->exchange));
	m_symbolSizeMap.insert(std::pair<std::string, int>(e->symbol, e->size));
	m_ctpgateway->onContract(e);
	if (bIsLast)
	{
		std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		e->gatewayname = m_gatewayname;
		e->msg = "��Լ��ѯ���";
		m_ctpgateway->onLog(e);
		m_ctpgateway->initQuery();
	}
}

//ί�гɹ��ر�����
void CTPTD::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	if (pOrder == nullptr)
	{
		//��ָ��
		return;
	}
	//������Ų����ۼ�
	const char* neworderref = pOrder->OrderRef;
	m_orderRefmtx.lock();
	m_orderRef = std::max(atoi(neworderref), m_orderRef);
	m_orderRefmtx.unlock();

	//�ӻر��л�ȡ��������״̬
	std::shared_ptr<Event_Order>e = std::make_shared<Event_Order>();
	e->symbol = pOrder->InstrumentID;
	e->exchange = pOrder->ExchangeID;
	e->orderID = pOrder->OrderRef;
	e->gatewayname = "CTP";
	if (pOrder->Direction == '0')
	{
		//��
		e->direction = DIRECTION_LONG;
	}
	else if (pOrder->Direction == '1')
	{
		//��
		e->direction = DIRECTION_SHORT;
	}
	else
	{
		//δ֪
		e->direction = DIRECTION_UNKNOWN;
	}

	if (pOrder->CombOffsetFlag[0] == '0')
	{
		//��
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
		//ȫ�ɽ�
		e->status = STATUS_ALLTRADED;
	}
	else if (pOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing)
	{
		//���ֳɽ�
		e->status = THOST_FTDC_OST_PartTradedQueueing;
	}
	else if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing)
	{
		//δ�ɽ�
		e->status = THOST_FTDC_OST_NoTradeQueueing;
	}
	else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
	{
		//����
		e->status = STATUS_CANCELLED;
	}
	else
	{
		//δ֪
		e->status = THOST_FTDC_OST_Unknown;
	}

	e->price = pOrder->LimitPrice;
	e->totalVolume = pOrder->VolumeTotalOriginal;
	e->tradedVolume = pOrder->VolumeTraded;
	e->orderTime = pOrder->InsertTime;
	e->cancelTime = pOrder->CancelTime;
	e->frontID = pOrder->FrontID;
	e->sessionID = pOrder->SessionID;

	//ά������״̬������ͨ��m_ordermapά��������keyΪorderID,orderID������OrderRef��
	if (e->status == STATUS_ALLTRADED || e->status == STATUS_CANCELLED)//�����ɽ�����ȡ������map��ɾ��
	{
		std::unique_lock<std::mutex>lck(m_ctpgateway->m_ordermapmtx);
		if (m_ctpgateway->m_ordermap.find(e->orderID) != m_ctpgateway->m_ordermap.end())
		{
			m_ctpgateway->m_ordermap.erase(e->orderID);
		}
	}
	else//����״�����¶���״̬
	{
		std::unique_lock<std::mutex>lck(m_ctpgateway->m_ordermapmtx);
		if (m_ctpgateway->m_ordermap.find(e->orderID) != m_ctpgateway->m_ordermap.end())//�¶���������
		{
			m_ctpgateway->m_ordermap.insert(std::pair<std::string, std::shared_ptr<Event_Order>>(e->orderID, e));
		}
		else//�϶�������״̬
		{
			m_ctpgateway->m_ordermap[e->orderID] = e;
		}
	}

	m_ctpgateway->onOrder(e);
}
//����û�����ĳ���֪ͨ
void CTPTD::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo == nullptr)
	{
		//��ָ��
		return;
	}
	m_ctpgateway->write_error("�û��������", pRspInfo);

}

void CTPTD::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	if (pTrade == nullptr)
	{
		//��ָ��
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
		//��
		e->direction = DIRECTION_LONG;
	}
	else if (pTrade->Direction == THOST_FTDC_D_Sell)
	{
		//��
		e->direction = DIRECTION_SHORT;
	}
	else
	{
		//δ֪
		e->direction = DIRECTION_UNKNOWN;
	}

	if (pTrade->OffsetFlag == THOST_FTDC_OF_Open)
	{
		//��
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

//����¼�����ر�
/*
void CTPTD::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
	if (pInputOrder == nullptr)
	{
		//��ָ��
		return;
	}
	//��������ر�(������)

	m_ctpgateway->write_error("�����ύ����",pRspInfo);
};
*/
//��������ر�
void CTPTD::OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo)
{
	if (pOrderAction == nullptr)
	{
		//��ָ��
		return;
	}
	//��������ر�(������)
	if(IsErrorRspInfo(pRspInfo))
		m_ctpgateway->write_error("��������",pRspInfo);
}


bool CTPTD::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{//�������Ϊ 0 ʱ����ʾ�����ɹ�
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
			//���������Ŀ¼��Ҫ����
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

//�ύ����
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
		m_ctpgateway->write_log("�������ͳɹ�");
	else if(i==-1)
		m_ctpgateway->write_log("��������,��������ʧ��");
	else if(i==-2)
		m_ctpgateway->write_log("��������,δ�������󳬹������");
	else if(i==-3)
		m_ctpgateway->write_log("��������,ÿ�뷢�����������������");
	return std::to_string(m_orderRef);
}
//ȡ������
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
//��ѯ��λ
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
//��ѯ�˻���Ϣ
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