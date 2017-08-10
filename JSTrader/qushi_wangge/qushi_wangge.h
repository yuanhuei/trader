#ifndef QUSHI_WANGGE_H
#define QUSHI_WANGGE_H
#ifdef  QUSHI_WANGGE_EXPORTS
#define QUSHI_WANGGE_API __declspec(dllexport)
#else
#define QUSHI_WANGGE_API __declspec(dllimport)
#endif
#include"StrategyTemplate.h"
extern "C" QUSHI_WANGGE_API StrategyTemplate * CreateStrategy(CTAAPI *ctamanager);//创建策略
extern "C" QUSHI_WANGGE_API int ReleaseStrategy();//释放策略
std::vector<StrategyTemplate*>g_qushi_wangge_v;//全局指针

#include<vector>
#include<algorithm>
#include<regex>
#include"talib.h"
#include"libbson-1.0\bson.h"
#include"libmongoc-1.0\mongoc.h"
class QUSHI_WANGGE_API qushi_wangge : public StrategyTemplate
{
public:
	qushi_wangge(CTAAPI *ctamanager);
	~qushi_wangge();

	void onInit();
	//TICK
	void onTick(TickData Tick);
	//BAR
	void onBar(BarData Bar);
	void on10Bar(BarData Bar);
	//报单回调
	void onOrder(std::shared_ptr<Event_Order>e);
	//成交回调
	void onTrade(std::shared_ptr<Event_Trade>e);

	//参数
	double unit_volume;


	//指标
	std::mutex m_HLCmtx;
	std::vector<double>close_vector;
	std::vector<double>high_vector;
	std::vector<double>low_vector;
	std::vector<double>close10_vector;
	bool trade;
	double lastHour;
	double upperBorder;
	double lowerBorder;
	double wg1;
	double wg2;
	
	double delta;
	double openPrice;
	double recentPrice;
	double rsi;
	double count;
	double todayStart;
	double filter;
	double cdp;
	double ah;
	double nh;
	double nl;
	double al;
	bool todayEntry;

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