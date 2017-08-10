#ifndef CTPGATEWAY_H
#define CTPGATEWAY_H
#ifdef  CTPGATEWAY_EXPORTS
#define CTPGATEWAY_API __declspec(dllexport)
#else
#define CTPGATEWAY_API __declspec(dllimport)
#endif
#include<map>
#include<atomic>
#include<io.h>
#include<direct.h>
#include<fstream>
#include<memory>
#include"CTPAPI/ThostFtdcMdApi.h"
#include"CTPAPI/ThostFtdcTraderApi.h"
#include"JSgateway.hpp"
#include"eventengine.h"
#include"md.h"
#include"td.h"
class CTPTD;										//超前声明
class CTPMD;										//超前声明
class CTPGATEWAY_API CTPGateway : public JSGateway//继承抽象类JSGATEWAY
{
public:
	explicit CTPGateway(EventEngine *eventengine, std::string gatewayname = "CTP");
	~CTPGateway();
	//连接状态
	std::atomic_bool ctpmdconnected;
	std::atomic_bool ctptdconnected;
	//主动函数
	void connect();											//连接
	void subscribe(SubscribeReq& subscribeReq);				//订阅
	std::string sendOrder(OrderReq & req);					//报单
	void cancelOrder(CancelOrderReq & req);					//撤单
	void qryAccount();										//查账户
	void qryPosition();										//查持仓
	void close();											//退出
	void initQuery();										//初始化查询循环
	void query();											//查询
	std::shared_ptr<Event_Order>getorder(std::string orderID);		//获取order
	//订单维护
	std::map<std::string, std::shared_ptr<Event_Order>>m_ordermap; std::mutex m_ordermapmtx;//存放委托的map

private:
	//事件驱动
	EventEngine * m_eventengine = nullptr;
	//行情和交易
	CTPMD *m_MDSPI;
	CTPTD *m_TDSPI;
	//接口名
	std::string m_gatewayname;
	//查询相关变量
	std::atomic_int m_qrycount;
	const int m_maxqry =4;									//超过4就再从0开始
	std::atomic_bool m_qryEnabled;
};
#endif