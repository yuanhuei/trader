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
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason);

	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//订阅应答响应
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

	//判断错误的函数
	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
	//主动函数

	//连接函数
	void connect(std::string userID, std::string password, std::string brokerID, std::string address);
	//登录函数
	void login();
	//订阅函数,调用subscribeMarketData
	void subscribe(SubscribeReq& subscribeReq);
	//退出登录
	void logout();

	void subscribeMarketData(std::string instrumentID);//订阅合约，调用m_mdapi->SubscribeMarketData
	
	void close();
	//退出线程
private:
	CThostFtdcMdApi* m_mdapi;
	CTPGateway* m_ctpgateway;
	std::string m_gatewayname;
	std::atomic_int m_reqID;

	std::atomic_bool m_connectionStatus;
	std::atomic_bool m_loginStatus;

	std::string m_userID;							//账号密码 经纪商地址例如9999 1080 服务器地址tcp://xxx.xxx.xxx.xxx
	std::string m_password;
	std::string m_brokerID;
	std::string m_address;

	std::set<std::string>m_subscribedSymbols;		//订阅成功的合约  暂时未读取，只存储，还不需要加锁

	//过滤非交易时间段的错误行情Tick
	std::set<std::string> m_ninetoeleven;
	std::set<std::string> m_ninetohalfeleven;
	std::set<std::string> m_ninetoone;
	std::set<std::string> m_ninetohalftwo;

};

#endif 