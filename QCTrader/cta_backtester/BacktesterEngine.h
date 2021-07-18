#pragma once
#include "BaseEngine.h"
#include<string>
#include<map>
#include<vector>
#include<set>
#include"qcstructs.h"
#include<QDateTime>
#include "../cta_strategy/StrategyTemplate.h"

class StrategyTemplate;
class EventEngine;


class DailyTradingResult
{
public:
	DailyTradingResult(QDate date, double price);
	~DailyTradingResult();

	void add_trade(Event_Trade trade);
	void calculate_pnl(float pre_close, float start_pos, int size, float rate, float slippage);


	QDate m_date;
	double m_close_price;//�������̼�
	double	m_pre_close = 0;//ǰһ������̼�

	 std::vector<Event_Trade> m_Dailytrades;
	int m_trade_count = 0;

	int m_start_pos = 0;
	int m_end_pos = 0;

	double m_turnover = 0;
	double m_commission = 0;//������
	double m_slippage = 0;//����

	double	m_trading_pnl = 0;//�����¿��ֵ����̵�PNL
	double	m_holding_pnl = 0;//���쿪ʼ�Ĳ�λ����������õ�PNL
	double	m_total_pnl = 0;
	double		m_net_pnl = 0;

	double	m_balance=0;//�ʽ�
	double m_highlevel=0;//�ʽ����ֵ
		
	double  m_return=0;//ÿ�ջر�
	double 	m_drawdown=0;//ÿ�����֮ǰ��ߵ�Ļس�
	double  m_ddpercent=0;//�س��ٷֱ�
};


class BacktesterEngine :
    public BaseEngine
{
public:
	BacktesterEngine(EventEngine* eventengine);
	~BacktesterEngine();


	StrategyTemplate* m_strategy=nullptr;
	std::map<std::string, float>  m_settingMap;

	std::string m_strategy_classname;
	std::string m_strategyName;
	std::string vt_symbol;
	std::string	m_symbol;

	std::string  exchange;
	QDate m_startDay;
	QDate	m_endDay;
	float	m_rate = 0;
	float	m_slippage = 0;
	float	m_size = 1;
	float m_pricetick = 0;
	float m_capital = 100000;
	float risk_free= 0.02;
	//self.inverse = False
	std::string m_barDate;



	TickData m_tick;
	BarData m_bar;
	QDateTime m_datetime;

	Interval m_iInterval=MINUTE;
	int m_days = 0;
	// 
	//barģʽ����tickģʽ
	std::string m_backtestmode = BAR_MODE;								

	int m_stop_order_count = 0;//����order ID����
	int m_limit_order_count = 0;
	std::map<std::string, std::shared_ptr<Event_StopOrder>>m_stop_orders;				//���еĵ�������ɾ��
	std::map<std::string, std::shared_ptr<Event_StopOrder>>m_active_stop_orders;		   //�����еĵ�   

	std::map<std::string, std::shared_ptr<Event_Order>>m_limit_orders;				//���еĵ�������ɾ��
	std::map<std::string, std::shared_ptr<Event_Order>>m_active_limit_orders;		//�����еĵ�   

	int  m_tradeCount = 0;							//�ɽ�����

	std::map<std::string, std::shared_ptr<Event_Trade>>m_tradeMap;//�ɽ���					

	std::vector<BarData> vector_history_data; //���ص���ʷ����



	std::map <QDate , std::shared_ptr<DailyTradingResult >> m_daily_resultMap; //���ݳɽ�������ÿ��ӯ�����
	std::map < std::string, std::string> m_result_statistics; //��������ͳ������

	void ResetData();//���²�����Ҫ����һ������
	void calculate_result();
	void calculate_statistics(bool bOutput = false);

	//�ṩ��Strategytemplate���õĽӿ�
	std::vector<std::string> sendOrder(bool bStopOrder,std::string symbol, std::string strDirection, std::string strOffset, double price, double volume, StrategyTemplate* Strategy);
	void cancelOrder(std::string orderID, std::string gatewayname);
	void cancel_stop_order(std::string orderID, std::string gatewayname);
	void cancel_limit_order(std::string orderID, std::string gatewayname);
	void cancelAllOrder(std::string strStragetyName);

	void cross_limit_order(const BarData& data);
	void cross_stop_order(const BarData& data);
	std::vector<std::string> send_limit_order(std::string symbol, std::string strDirection, std::string strOffset, 
		double price, double volume, StrategyTemplate* Strategy);
	std::vector<std::string> send_stop_order(std::string symbol, std::string strDirection, std::string strOffset, 
		double price, double volume, StrategyTemplate* Strategy);

	void PutEvent(std::shared_ptr<Event>e);//�����¼�

	std::vector<TickData> loadTickbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<BarData> loadBarbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<TickData> loadTick(std::string symbol, int days);
	std::vector<BarData> loadBar(std::string symbol, int days);

	EventEngine* m_eventengine;


	//�ز⺯��
	std::vector<BarData> LoadHistoryData();				//��ȡ�ز��õ���ʷ����
	void update_daily_close(float price);



	void StartBacktesting(
		std::string strStrategyName,
		std::string strStrategyClassName,
		std::string strSymbol,
		Interval iInterval,
		QDate startDay,
		QDate	endDay,
		float rate,
		float slippage,
		float contractsize,
		float pricetick,
		float capital,
		std::map<std::string, float>  ctaStrategyMap);
	void runBacktesting();
	void writeCtaLog(std::string msg);

};

