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
	//算法下单
	algorithmOrder(double unitLimit,enum Mode TradingMode, StrategyTemplate *strategy_ptr);
	void setunitLimit(int unitLimit);
	void setTradingMode(Mode TradingMode);
	void checkPositions_Tick(const TickData *Tick);
	void checkPositions_Bar(const BarData *Bar);
	void set_supposedPos(std::string symbol, double volume);
	//设置止损应该放在开仓的位置
	void setStop_timeandTrailing(const BarData *bar, int seconds, double tralingpercent, std::string longshortCondition);//设置时间与跟踪止损
	void setStop_tralingLose(const BarData *bar, double tralingPercent, std::string longshortCondition);  //设置跟踪止损
	void setStop_time(const BarData *bar, int seconds);										    //设置时间止损
	void setStop_timeandTrailing(const BarData *bar, int seconds, double tralingpercent, std::string longshortCondition, double lastopenprice);   //设置止盈止损
	bool checkStop(const BarData *bar);															//执行止损
	bool checkStop(const TickData *Tick);
private:
	std::map<std::string, double>m_supposedPos;  //用来实现算法交易
	std::mutex m_supposedPosmtx;
	enum Mode m_TradingMode;					//回测还是实盘
	StrategyTemplate* m_strategy_ptr;
	double m_unitLimit;
	int stopMode;						//止损方式
	//止损价格
	int waitCount=0;
	int waitCountLimit=0;
	double lastOpenPrice=0;
	double intraTradeHigh=0;
	double intraTradeLow=9999999999999;
	double longTrailingPercent;
	double shortTrailingPercent;
	double longWinTrailingPercent;
	double shortWinTrailingPercent;
	//分类报撤单
	std::mutex m_orderIDvmtx;
	std::map<std::string, std::vector<std::string>> orderID_Vector_Map;			//按照symbol分类
};
#endif