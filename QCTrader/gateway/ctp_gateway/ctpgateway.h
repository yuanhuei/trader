#pragma once

#ifndef CTPGATEWAY_H
#define CTPGATEWAY_H

//#include<map>
//#include<atomic>
//#include<io.h>
//#include<direct.h>
//#include<fstream>
//#include<memory>
//#include"CTPAPI/ThostFtdcMdApi.h"
//#include"CTPAPI/ThostFtdcTraderApi.h"
#include"../qcgateway.hpp"
#include"CTPAPI/ThostFtdcMdApi.h"
//#include"../../event_engine/eventengine.h"
//#include"ctpmd.h"
//#include"ctptd.h"
class CTPTD;										//超前声明
class CTPMD;										//超前声明
class EventEngine;
class  CTPGateway : public QCGateway//继承抽象类JSGATEWAY
{
public:
	explicit CTPGateway(EventEngine* eventengine, std::string gatewayname = "CTP");
	~CTPGateway();
	//连接状态
	std::atomic_bool ctpmdconnected;
	std::atomic_bool ctptdconnected;

	//主动函数
	//配置文件连接
	void connect();											
	//对话框输入参数连接
	void connect(std::string userID, std::string password, std::string brokerID, std::string mdaddress, std::string tdaddress, std::string authcode, std::string appid, std::string productinfo);

	void subscribe(SubscribeReq& subscribeReq);				//订阅
	std::string sendOrder(OrderReq& req);					//报单
	void cancelOrder(CancelOrderReq& req);					//撤单
	void qryAccount();										//查账户
	void qryPosition();										//查持仓
	void close();											//退出
	void initQuery();										//初始化查询循环
	void query();											//查询
	std::shared_ptr<Event_Order>getorder(std::string orderID);		//获取order
	//订单维护
	std::map<std::string, std::shared_ptr<Event_Order>>m_ordermap; std::mutex m_ordermapmtx;//存放委托的map

	void write_error(std::string msg, CThostFtdcRspInfoField* pRspInfo);//格式化错误信息，调用基类的write_log

private:
	//事件驱动
	//EventEngine* m_eventengine = nullptr;
	//行情和交易
	CTPMD* m_MDSPI;
	CTPTD* m_TDSPI;
	//接口名
	//std::string m_gatewayname;
	//查询相关变量
	std::atomic_int m_qrycount=0;
	const int m_maxqry = 4;									//超过4就再从0开始
	std::atomic_bool m_qryEnabled=false;                    //条件就绪，可以开始轮询账户和仓位了。
};
#endif