#ifndef PORTFOLIO_H
#define PORTFOLIO_H
#include<map>
#include<mutex>

#include"cta_strategy/StrategyTemplate.h"
#include"gateway/gatewaymanager.h"
#include"event_engine/eventengine.h"
#include"./cta_backtester/BacktesterEngine.h"
class StrategyTemplate;

class TradingResult
{
public:
	TradingResult(double entryPrice, std::string entryDt, double exitPrice, std::string exitDt, double volume, double size);
	double m_entryPrice;
	double m_exitPrice;
	std::string m_entryDt;
	std::string m_exitDt;
	double m_volume;
	double m_pnl;
};

struct UnitResult
{
	double  totalwinning;
	double  lastdayTotalwinning;
	double  delta;
	double	maxCapital;
	double	drawdown;
	double	Winning;
	double	Losing;
	int totalResult;

	double holdingwinning;//�ֲ�ӯ��
	double holdingposition;
	double holdingprice;
};



typedef std::map<std::string, UnitResult> Result;//key�Ǻ�Լ value��һ�������λ
class Portfolio
{
public:
	Portfolio(EventEngine *eventengine, Gatewaymanager *gatewaymanager);
	void calculate();																											//����Ӳ��ƽ��ӯ������Ϣ
	void calculate_memory(std::shared_ptr<Event_Trade>etrade, std::map<std::string, StrategyTemplate*>orderStrategymap);		//����ʵʱƽ��ӯ��
	void updateportfolio(std::shared_ptr<Event>e);																				//������ϵ����棬ͬʱ����ֲ�ӯ��
	void recordNetValue(std::shared_ptr<Event>e);	
	void writelog(std::string log);																								//��¼��ֵ
private:
	bool netValueRecorded;
	std::mutex m_portfoliomtx;
	bool is_read;
	std::map<std::string, std::map<std::string, std::vector<Event_Trade>>>m_tradelist;							//Ӳ��ӯ��
	std::map<std::string, std::map<std::string, std::vector<Event_Trade>>>m_tradelist_memory;					std::mutex m_tradelistmtx;//ʵʱ
	std::map<std::string, Result>m_result;//��������key�ǲ��ԣ�value��map �� key�Ǻ�Լ value�ǽ��
	//key�ǲ�������value��map
	//�ڶ���map key �� ��Լ�� value��vector
	//vector��TradeEvent

	EventEngine *m_eventengine;
	Gatewaymanager *m_gatewaymanager;
};
#endif