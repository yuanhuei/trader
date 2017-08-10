#ifndef __MD_H__
#define __MD_H__
#include<string>
#include<set>
#include"ThostFtdcMdApi.h"
#include<direct.h>
#include<io.h>
#include"structs.h"
#include"eventengine.h"
#include"JSRecordClass.h"
class CTPMD :public CThostFtdcMdSpi,public JSRecordClass
{
public:
	CTPMD(EventEngine *eventengine, std::string gatewayname);
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
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	//判断错误的函数
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	//////////////////主动函数

	void connect();

    void connectmd(std::string userID, std::string password, std::string brokerID, std::string address);
	//登录函数
	virtual void login();
	//请求登录
	virtual void subscribe(SubscribeReq& subscribeReq);

	virtual void logout();

	void subscribeMarketData(std::string instrumentID);
	//订阅
	virtual void close();
	//退出线程
private:
	CThostFtdcMdApi* m_ptr_mdapi;
	std::string m_gatewayname;
	int m_reqID;
	bool m_connectionStatus = false;
	bool m_loginStatus = false;
	std::set<std::string>m_subscribedSymbols;
	std::string m_userID;//账号密码 经纪商地址例如9999 1080 服务器地址tcp://xxx.xxx.xxx.xxx
	std::string m_password;
	std::string m_brokerID;
	std::string m_address;
	std::set<std::string> *m_ptr_subscribedSymbols = nullptr;
};
#endif