#pragma once
#ifndef CTPMD_H
#define CTPMD_H

#include<string>
#include<stdio.h>
#include<set>
#include<atomic>
#include"../../qcstructs.h"
#include"CTPAPI/ThostFtdcMdApi.h"
#include"CTPAPI/ThostFtdcTraderApi.h"


class CTPGateway;

class CTPMD : public  CThostFtdcMdSpi
{
public:
	CTPMD(CTPGateway* CTPGateway, std::string gatewayname);
	~CTPMD();
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

	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//����Ӧ����Ӧ
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�������֪ͨ
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

	//�жϴ���ĺ���
	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
	//��������

	//���Ӻ���
	void connect(std::string userID, std::string password, std::string brokerID, std::string address);
	//��¼����
	void login();
	//���ĺ���,����subscribeMarketData
	void subscribe(SubscribeReq& subscribeReq);
	//�˳���¼
	void logout();

	void subscribeMarketData(std::string instrumentID);//���ĺ�Լ������m_mdapi->SubscribeMarketData
	
	void close();
	//�˳��߳�
private:
	CThostFtdcMdApi* m_mdapi;
	CTPGateway* m_ctpgateway;
	std::string m_gatewayname;
	std::atomic_int m_reqID;

	std::atomic_bool m_connectionStatus;
	std::atomic_bool m_loginStatus;

	std::string m_userID;							//�˺����� �����̵�ַ����9999 1080 ��������ַtcp://xxx.xxx.xxx.xxx
	std::string m_password;
	std::string m_brokerID;
	std::string m_address;

	std::set<std::string>m_subscribedSymbols;		//���ĳɹ��ĺ�Լ  ��ʱδ��ȡ��ֻ�洢��������Ҫ����

	//���˷ǽ���ʱ��εĴ�������Tick
	std::set<std::string> m_ninetoeleven;
	std::set<std::string> m_ninetohalfeleven;
	std::set<std::string> m_ninetoone;
	std::set<std::string> m_ninetohalftwo;

};

#endif 