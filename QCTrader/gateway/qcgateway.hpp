#ifndef QCGATEWAY_H
#define QCGATEWAY_H
//�����࣬�����нӿ���̳�
#include<string>
#include"../qcstructs.h"
#include"../event_engine/eventengine.h"
class QCGateway
{
public:
	QCGateway::QCGateway(EventEngine* eventengine, std::string gatewayname)
	{
		m_eventengine = eventengine;
		m_gatewayname = gatewayname;
	}
	void QCGateway::onTick(std::shared_ptr<Event_Tick>e)
	{
		m_eventengine->Put(e);
	}
	void QCGateway::onTrade(std::shared_ptr<Event_Trade>e)
	{
		m_eventengine->Put(e);
	}
	void QCGateway::onOrder(std::shared_ptr<Event_Order>e)
	{
		m_eventengine->Put(e);
	}
	void QCGateway::onPosition(std::shared_ptr<Event_Position>e)
	{
		m_eventengine->Put(e);
	}
	void QCGateway::onAccount(std::shared_ptr<Event_Account>e)
	{
		m_eventengine->Put(e);
	}
	void QCGateway::onError(std::shared_ptr<Event_Error>e)
	{
		m_eventengine->Put(e);
	}
	void QCGateway::onLog(std::shared_ptr<Event_Log>e)
	{
		m_eventengine->Put(e);
	}

	void QCGateway::onContract(std::shared_ptr<Event_Contract>e)
	{
		m_eventengine->Put(e);
	}
	void QCGateway::write_log(std::string msg)
	{
		std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
		e->gatewayname = m_gatewayname;
		e->msg = msg;
		this->onLog(e);
	}
	virtual void connect() = 0;																					//����
	virtual void subscribe(SubscribeReq& subscribeReq) = 0;														//����
	virtual std::string sendOrder(OrderReq& req) = 0;															//����
	virtual void cancelOrder(CancelOrderReq& req) = 0;															//����
	virtual void qryAccount() = 0;																				//��ѯ�˻��ʽ�
	virtual void qryPosition() = 0;																				//��ѯ�ֲ�
	virtual void close() = 0;																					//�Ͽ�API
	virtual std::string GetExchangeName(std::string strSymbol) =0;
	virtual int  GetSymbolSize(std::string strSymbol) =0;
public:
	EventEngine* m_eventengine;
	std::string m_gatewayname;
};
#endif