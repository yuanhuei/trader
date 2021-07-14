#include"gatewaymanager.h"



Gatewaymanager::Gatewaymanager(EventEngine *eventengine)
{
	m_eventengine = eventengine;
	m_eventengine->RegEvent(EVENT_CONTRACT, std::bind(&Gatewaymanager::Process_contract_event, this, std::placeholders::_1));
	Init();
}

Gatewaymanager::~Gatewaymanager()
{
	exit();
}

void Gatewaymanager::Process_contract_event(std::shared_ptr<Event>e)
{
	
	//�����еĺ�Լ�浽���������
	std::unique_lock<std::mutex>lck(contractmutex);
	std::shared_ptr<Event_Contract> contract = std::static_pointer_cast<Event_Contract>(e);
	m_contractmap.insert(std::pair<std::string, std::shared_ptr<Event_Contract>>(contract->symbol, contract));

}
void Gatewaymanager::Init()
{

	//��ʼ�����нӿ�
	//CTP
	std::shared_ptr<CTPGateway>ctpgateway = std::make_shared<CTPGateway>(m_eventengine, "CTP");
	std::unique_lock<std::mutex>lck(gatewaymtx);
	m_gatewaymap.insert(std::pair<std::string, std::shared_ptr<QCGateway>>("CTP", ctpgateway));
	//IB
	//std::shared_ptr<>ctpgateway = std::make_shared<CTPGateway>(m_eventengine, "IB");
}
std::string Gatewaymanager::GetExchangeName(std::string strSymbol, std::string strGatewayname)
{
	std::unique_lock<std::mutex>lck(gatewaymtx);
	return m_gatewaymap[strGatewayname]->GetExchangeName(strSymbol);//����ָ���Ľӿ�

}
int  Gatewaymanager::GetSymbolSize(std::string strSymbol, std::string strGatewayname)
{
	std::unique_lock<std::mutex>lck(gatewaymtx);
	std::shared_ptr<QCGateway> ptr_gateway;
	ptr_gateway = m_gatewaymap[strGatewayname];
	int iSize = ptr_gateway->GetSymbolSize(strSymbol);
	return iSize;//����ָ���Ľӿ�
}


std::shared_ptr<Event_Contract>Gatewaymanager::getContract(std::string symbol)
{
	std::unique_lock<std::mutex>lck(contractmutex);
	if (m_contractmap.find(symbol) == m_contractmap.end())
	{
		return nullptr;
	}
	return m_contractmap[symbol];//���غ�Լ�Ķ���ָ��
}

std::shared_ptr<Event_Contract>Gatewaymanager::Find_Contract(std::string symbol)
{
	if (m_contractmap.find(symbol) != m_contractmap.end())
	{
		return m_contractmap[symbol];
	}
	else
	{
		return nullptr;
	}
}


std::map<std::string, std::shared_ptr<Event_Contract>> Gatewaymanager::getAllContract()
{
	return m_contractmap;
}

void Gatewaymanager::connect(std::string gatewayname)
{
	//���ӽӿ�
	std::unique_lock<std::mutex>lck(gatewaymtx);

	m_gatewaymap[gatewayname]->connect();//����ָ���Ľӿ�
}

void Gatewaymanager::subscribe(SubscribeReq req, std::string gatewayname)
{
	//��������
	std::unique_lock<std::mutex>lck(gatewaymtx);
	m_gatewaymap[gatewayname]->subscribe(req);//����ָ���Ľӿ�
}

std::string Gatewaymanager::sendOrder(OrderReq req, std::string gatewayname)
{
	//����
	std::unique_lock<std::mutex>lck(gatewaymtx);
	return m_gatewaymap[gatewayname]->sendOrder(req);//����ָ���ӿڷ���
}

void Gatewaymanager::cancelOrder(CancelOrderReq req, std::string gatewayname)
{
	//����
	std::unique_lock<std::mutex>lck(gatewaymtx);
	m_gatewaymap[gatewayname]->cancelOrder(req);//����ָ���ӿڳ���
}

void Gatewaymanager::qryAccont(std::string gatewayname)
{
	//���˻�
	std::unique_lock<std::mutex>lck(gatewaymtx);
	m_gatewaymap[gatewayname]->qryAccount();
}

void Gatewaymanager::qryPosition(std::string gatewayname)
{
	//���λ
	std::unique_lock<std::mutex>lck(gatewaymtx);
	m_gatewaymap[gatewayname]->qryPosition();
}
std::shared_ptr<Event_Order>Gatewaymanager::getorder(std::string gatewayname, std::string orderID)
{
	std::unique_lock<std::mutex>lck(gatewaymtx);
	if (gatewayname == "CTP")
	{
		std::shared_ptr<CTPGateway> ctpgateway = std::static_pointer_cast<CTPGateway>(m_gatewaymap[gatewayname]);
		return ctpgateway->getorder(orderID);
	}
	else
	{
		return nullptr;
	}
}

void Gatewaymanager::exit()
{
	std::unique_lock<std::mutex>lck(gatewaymtx);
	//�ر�ȫ���ӿ�
	if (m_gatewaymap.empty())
	{
		return;
	}
	for (std::map<std::string, std::shared_ptr<QCGateway>>::iterator it = m_gatewaymap.begin(); it != m_gatewaymap.end(); it++)
	{
		it->second->close();
	}
	m_gatewaymap.clear();
}

void Gatewaymanager::close(std::string gatewayname)
{
	std::unique_lock<std::mutex>lck(gatewaymtx);
	m_gatewaymap[gatewayname]->close();
}