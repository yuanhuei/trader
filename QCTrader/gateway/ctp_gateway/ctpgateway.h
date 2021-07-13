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
class CTPTD;										//��ǰ����
class CTPMD;										//��ǰ����
class EventEngine;
class  CTPGateway : public QCGateway//�̳г�����JSGATEWAY
{
public:
	explicit CTPGateway(EventEngine* eventengine, std::string gatewayname = "CTP");
	~CTPGateway();
	//����״̬
	std::atomic_bool ctpmdconnected;
	std::atomic_bool ctptdconnected;

	//��������
	//�����ļ�����
	void connect();											
	//�Ի��������������
	void connect(std::string userID, std::string password, std::string brokerID, std::string mdaddress, std::string tdaddress, std::string authcode, std::string appid, std::string productinfo);

	void subscribe(SubscribeReq& subscribeReq);				//����
	std::string sendOrder(OrderReq& req);					//����
	void cancelOrder(CancelOrderReq& req);					//����
	void qryAccount();										//���˻�
	void qryPosition();										//��ֲ�
	void close();											//�˳�
	void initQuery();										//��ʼ����ѯѭ��
	void query();											//��ѯ
	std::shared_ptr<Event_Order>getorder(std::string orderID);		//��ȡorder
	//����ά��
	std::map<std::string, std::shared_ptr<Event_Order>>m_ordermap; std::mutex m_ordermapmtx;//���ί�е�map

	void write_error(std::string msg, CThostFtdcRspInfoField* pRspInfo);//��ʽ��������Ϣ�����û����write_log
	std::string GetExchangeName(std::string strSymbol);
	int  GetSymbolSize(std::string strSymbol);

private:
	//�¼�����
	//EventEngine* m_eventengine = nullptr;
	//����ͽ���
	CTPMD* m_MDSPI;
	CTPTD* m_TDSPI;
	//�ӿ���
	//std::string m_gatewayname;
	//��ѯ��ر���
	std::atomic_int m_qrycount=0;
	const int m_maxqry = 4;									//����4���ٴ�0��ʼ
	std::atomic_bool m_qryEnabled=false;                    //�������������Կ�ʼ��ѯ�˻��Ͳ�λ�ˡ�
};
#endif