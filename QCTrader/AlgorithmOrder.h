#ifndef ALGORITHMORDER_H
#define ALGORITHMORDER_H
#include<map>
#include"qcstructs.h"
#include"utils.hpp"
#include"cta_strategy/StrategyTemplate.h"
class StrategyTemplate;
enum stopmode
{
	trailingStop = 1,
	timeStop = 2,
	trailingandtimeStop = 3,
	winAndLoseStop = 4
};

class algorithmOrder
{
public:
	//�㷨�µ�
	algorithmOrder(double unitLimit,enum Mode TradingMode, StrategyTemplate *strategy_ptr);
	void setunitLimit(int unitLimit);
	void setTradingMode(Mode TradingMode);
	void checkPositions_Tick(const TickData *Tick);
	void checkPositions_Bar(const BarData *Bar);
	void set_supposedPos(std::string symbol, double volume);
	//����ֹ��Ӧ�÷��ڿ��ֵ�λ��
	void setStop_timeandTrailing(const BarData *bar, int seconds, double tralingpercent, std::string longshortCondition);//����ʱ�������ֹ��
	void setStop_tralingLose(const BarData *bar, double tralingPercent, std::string longshortCondition);  //���ø���ֹ��
	void setStop_time(const BarData *bar, int seconds);										    //����ʱ��ֹ��
	void setStop_timeandTrailing(const BarData *bar, int seconds, double tralingpercent, std::string longshortCondition, double lastopenprice);   //����ֹӯֹ��
	bool checkStop(const BarData *bar);															//ִ��ֹ��
	bool checkStop(const TickData *Tick);
private:
	std::map<std::string, double>m_supposedPos;  //����ʵ���㷨����
	std::mutex m_supposedPosmtx;
	enum Mode m_TradingMode;					//�ز⻹��ʵ��
	StrategyTemplate* m_strategy_ptr;
	double m_unitLimit;
	int stopMode;						//ֹ��ʽ
	//ֹ��۸�
	int waitCount=0;
	int waitCountLimit=0;
	double lastOpenPrice=0;
	double intraTradeHigh=0;
	double intraTradeLow=9999999999999;
	double longTrailingPercent;
	double shortTrailingPercent;
	double longWinTrailingPercent;
	double shortWinTrailingPercent;
	//���౨����
	std::mutex m_orderIDvmtx;
	std::map<std::string, std::vector<std::string>> orderID_Vector_Map;			//����symbol����
};
#endif