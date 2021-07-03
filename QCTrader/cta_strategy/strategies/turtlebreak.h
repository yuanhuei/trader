#ifndef TURTLEBREAK_H
#define TURTLEBREAK_H
#ifdef TURTLEBREAK_EXPORTS
#define TURTLEBREAK_API __declspec(dllexport)
#else
#define TURTLEBREAK_API __declspec(dllimport)
#endif
#include"StrategyTemplate.h"
extern "C" TURTLEBREAK_API StrategyTemplate * CreateStrategy(CTAAPI *ctamanager);//��������
extern "C" TURTLEBREAK_API int ReleaseStrategy();//�ͷŲ���
std::vector<StrategyTemplate*>g_turtlebreak_v;//ȫ��ָ��

#include<vector>
#include<algorithm>
#include<regex>
#include"talib.h"
#include"libbson-1.0\bson.h"
#include"libmongoc-1.0\mongoc.h"
class TURTLEBREAK_API turtlebreak : public StrategyTemplate
{
public:
	turtlebreak(CTAAPI *ctamanager);
	~turtlebreak();

	void onInit();
	//TICK
	void onTick(TickData Tick);
	//BAR
	void onBar(BarData Bar);
	void on5Bar(BarData Bar);
	//�����ص�
	void onOrder(std::shared_ptr<Event_Order>e);
	//�ɽ��ص�
	void onTrade(std::shared_ptr<Event_Trade>e);

	//ָ��
	std::mutex m_HLCmtx;
	std::vector<double>close_vector;
	std::vector<double>high_vector;
	std::vector<double>low_vector;

	std::vector<double>close_10_vector;
	double preclose;
	double startPoint;
	double intraHigh;
	double intraLow;
	bool opened;
	double openedlala;
	int volumelimit;
	double high_100;
	double low_100;
	double high_10;
	double low_10;
	double width;

	double openPrice;

	double holdingPrice;
	double holdingProfit;
	double highProfit;

	std::set<std::string> m_ninetoeleven;
	std::set<std::string> m_ninetohalfeleven;
	std::set<std::string> m_ninetoone;
	std::set<std::string> m_ninetohalftwo;
	BarData m_5Bar;
	//������
	double lastprice;				//���½���
	//���½���
	void putEvent();
};
#endif