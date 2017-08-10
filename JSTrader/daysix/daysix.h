#ifndef DAYSIX_H
#define DAYSIX_H
#ifdef  DAYSIX_EXPORTS
#define DAYSIX_API __declspec(dllexport)
#else
#define DAYSIX_API __declspec(dllimport)
#endif
#include"StrategyTemplate.h"
extern "C" DAYSIX_API StrategyTemplate * CreateStrategy(CTAAPI *ctamanager);//创建策略
extern "C" DAYSIX_API int ReleaseStrategy();//释放策略
std::vector<StrategyTemplate*>g_daysix_v;//全局指针

#include<vector>
#include<algorithm>
#include<regex>
#include"talib.h"
#include"libbson-1.0\bson.h"
#include"libmongoc-1.0\mongoc.h"
class DAYSIX_API daysix : public StrategyTemplate
{
public:
	daysix(CTAAPI *ctamanager);
	~daysix();

	void onInit();
	//TICK
	void onTick(TickData Tick);
	//BAR
	void onBar(BarData Bar);
	void on5Bar(BarData Bar);
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
  
	std::vector<double>close_50_vector;
 
	double pos=0;

	double pb6;
	double pb4;
	int volumelimit;
	double high_100;
	double low_100;
 
	double holdingProfit;
	double highProfit;

	double holdingPrice;
	double openPrice;

	double intraTrailingHigh;
	double intraTrailingLow;

	bool rehold;
	double lastHoldingVolume;
	double lastHoldingPrice;

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