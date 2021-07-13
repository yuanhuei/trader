#pragma once
#ifndef CTPTD_H
#define CTPTD_H

#include<string>
#include"../../qcstructs.h"
#include"CTPAPI/ThostFtdcUserApiDataType.h"
#include"CTPAPI/ThostFtdcTraderApi.h"


class CTPGateway;
class   CTPTD :public CThostFtdcTraderSpi
{
public:
	CTPTD(CTPGateway* CTPGateway, std::string gatewayname);
	~CTPTD();
	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();
	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	virtual void OnFrontDisconnected(int nReason);
	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�ǳ�������Ӧ
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///Ͷ���߽�����ȷ����Ӧ
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Լ��Ӧ
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ�ʽ��˻���Ӧ
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ���ֲ߳���Ӧ
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///����¼��������Ӧ
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///��������������Ӧ
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	///����֪ͨ
	virtual void OnRtnOrder(CThostFtdcOrderField* pOrder);
	///�ɽ�֪ͨ
	virtual void OnRtnTrade(CThostFtdcTradeField* pTrade);
	///����¼�����ر�
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo);
	// �Ƿ��յ��ɹ�����Ӧ

	///������������ر�
	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo);

	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);

	bool IsFlowControl(int iResult);

	void connect(std::string userID, std::string password, std::string brokerID, std::string address,std::string authcode="", std::string appid="", std::string productinfo="");
	//void connect(std::string userID, std::string password, std::string brokerID, std::string address);

	void login();

	void logout();

	void qryAccount();

	void qryPosition();

	std::string sendOrder(OrderReq& req);

	void cancelOrder(CancelOrderReq& req);

	void close();

	void authenticate();

	///�ͻ�����֤��Ӧ
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

private:
	CThostFtdcTraderApi* m_tdapi;
	CTPGateway* m_ctpgateway;
	std::string m_gatewayname;
	std::atomic_int m_reqID;  //������
	int m_orderRef; std::mutex m_orderRefmtx;//�������

	std::atomic_bool m_connectionStatus;
	std::atomic_bool m_loginStatus;
	std::atomic_bool m_login_failed = false;
	std::atomic_bool auth_status=false;

	std::string m_userID;//�˺����� �����̵�ַ����9999 1080 ��������ַtcp://xxx.xxx.xxx.xxx
	std::string m_password;
	std::string m_brokerID;
	std::string m_address;
	
	std::string m_authCode;
	std::string m_appID;
	std::string m_productInfo;

	int m_frontID;//ǰ�û����
	int m_sessionID;//�Ự���

	//
	std::map<std::string, std::shared_ptr<Event_Position>>m_posBufferMap;		std::mutex m_positionbuffermtx;//����ֲ�
public:
	std::map<std::string, std::string>m_symbolExchangeMap;//��Լ�ͽ�������map,ֻ����д������Ҫ����
	std::map<std::string, int>m_symbolSizeMap;//��Լ�ͺ�Լ������map
};

#endif // !CTPTD_H