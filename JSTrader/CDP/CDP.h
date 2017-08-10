#ifndef CDP_H
#define CDP_H
#ifdef  CDP_EXPORTS
#define CDP_API __declspec(dllexport)
#else
#define CDP_API __declspec(dllimport)
#endif
#include"StrategyTemplate.h"
extern "C" CDP_API StrategyTemplate * CreateStrategy(CTAAPI *ctamanager);//创建策略
extern "C" CDP_API int ReleaseStrategy();//释放策略
std::vector<StrategyTemplate*>g_CDP_v;//全局指针

#include<vector>
#include<algorithm>
#include<regex>
#include"talib.h"
#include"libbson-1.0\bson.h"
#include"libmongoc-1.0\mongoc.h"
class CDP_API CDP : public StrategyTemplate
{
public:
	CDP(CTAAPI *ctamanager);
	~CDP();

	void onInit();
	//TICK
	void onTick(TickData Tick);
	//BAR
	void onBar(BarData Bar);

	//10Bar
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

	std::vector<double>close_10min_vector;
	double dayHIGH;
	double dayLOW;
	double dayClose;

	double cdp;
	double ah;
	double nh;
	double nl;
	double al;

	double todayEntry;

	std::set<std::string> m_ninetoeleven;
	std::set<std::string> m_ninetohalfeleven;
	std::set<std::string> m_ninetoone;
	std::set<std::string> m_ninetohalftwo;
	BarData Bar_10min;

	//缓存用
	double lastprice;				//更新界面
	//更新界面
	void putEvent();
};
#endif