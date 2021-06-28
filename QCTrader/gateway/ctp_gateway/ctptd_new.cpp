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
	login();
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

	}
}

void CTPTD::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//��֤������
{
	if (pUserLogout == nullptr)
	{
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