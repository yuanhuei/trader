#ifndef DAYTRADE1_H
#define DAYTRADE1_H
#ifdef  DAYTRADE1_EXPORTS
#define DAYTRADE1_API __declspec(dllexport)
#else
#define DAYTRADE1_API __declspec(dllimport)
#endif
#include"StrategyTemplate.h"
extern "C" DAYTRADE1_API StrategyTemplate * CreateStrategy(CTAAPI *ctamanager);//创建策略
extern "C" DAYTRADE1_API int ReleaseStrategy();//释放策略
std::vector<StrategyTemplate*>g_DAYTRADE1_v;//全局指针

#include<vector>
#include<algorithm>
#include<regex>
#include"talib.h"
#include"libbson-1.0\bson.h"
#include"libmongoc-1.0\mongoc.h"
class DAYTRADE1_API Daytrade1 : public StrategyTemplate
{
public:
	Daytrade1(CTAAPI *ctamanager);
	~Daytrade1();

	void onInit();
	//TICK
	void onTick(TickData Tick);
	//BAR
	void onBar(BarData Bar);

	//报单回调
	void onOrder(std::shared_ptr<Event_Order>e);
	//成交回调
	void onTrade(std::shared_ptr<Event_Trade>e);

	virtual void savepostomongo();
	virtual void loadposfrommongo();

	//参数
	double unit_volume;


	//指标
	std::mutex m_HLCmtx;
	std::vector<double>close_vector;
	std::vector<double>high_vector;
	std::vector<double>low_vector;


	double TodayOpen;
	 
	double EntryPrice;

	double buysellsignal;

	double stopsignal;

	const double range = 5;
	const double stoploss = 15;


	std::set<std::string> m_ninetoeleven;
	std::set<std::string> m_ninetohalfeleven;
	std::set<std::string> m_ninetoone;
	std::set<std::string> m_ninetohalftwo;
	BarData m_5Bar;
	//缓存用
	double lastprice;				//更新界面
	//更新界面
	void putEvent();
};
#endif