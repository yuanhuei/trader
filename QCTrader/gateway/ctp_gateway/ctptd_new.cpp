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
	login();
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

	}
}

void CTPTD::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)//验证无问题
{
	if (pUserLogout == nullptr)
	{
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