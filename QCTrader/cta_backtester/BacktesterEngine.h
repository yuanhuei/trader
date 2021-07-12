#pragma once
#include "CTAAPI.h"
#include<string>
#include<map>
#include<vector>
#include<set>
#include"qcstructs.h"
#include<QDateTime>

class StrategyTemplate;
class EventEngine;


class DailyTradingResult
{
public:
	DailyTradingResult(QDate date, double price);
	~DailyTradingResult();

	void add_trade(Event_Trade trade);
	void calculate_pnl(float pre_close, float start_pos, int size, float rate, float slippage);


	QDate date = date;
	double m_close_price;//�������̼�
	double	m_pre_close = 0;//ǰһ������̼�

	 std::vector<Event_Trade> m_trades;
	int m_trade_count = 0;

	int m_start_pos = 0;
	int m_end_pos = 0;

	double m_turnover = 0;
	double m_commission = 0;//������
	double m_slippage = 0;//����

	double	m_trading_pnl = 0;
	double	m_holding_pnl = 0;
	double	m_total_pnl = 0;
	double		m_net_pnl = 0;

	double	m_balance;//�ʽ�
	double m_highlevel;//�ʽ����ֵ
		
	double  m_return;//ÿ�ջر�
	double 	m_drawdown;//ÿ�ջز�
	double  m_ddpercent;//�ز�ٷֱ�
};


class BacktesterEngine :
    public CTAAPI
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
	float m_capital = 1000000;
	float risk_free= 0.02;
	//mode = BacktestingMode.BAR;
	//self.inverse = False
	std::string m_barDate;



	TickData m_tick;
	BarData m_bar;
	//self.datetime = None

	Interval m_iInterval=MINUTE;
	int m_days = 0;
	//self.callback = None

	int m_stop_order_count = 0;
	//stop_orders = {}
	//active_stop_orders = {}

	int m_limit_order_count = 0;
	//self.limit_orders = {}
	//self.active_limit_orders = {}
	std::map<std::string, std::shared_ptr<Event_Order>>m_Ordermap;				//���еĵ�������ɾ��
	std::map<std::string, std::shared_ptr<Event_Order>>m_WorkingOrdermap;		//�����еĵ�   

	//int m_trade_count = 0;
	std::map<std::string, std::shared_ptr<Event_Trade>>m_tradeMap;//�ɽ���					
	//std::map<std::string, Result>m_result;											

	std::vector<BarData> vector_history_data;



	std::map<QDate, std::shared_ptr<DailyTradingResult>> m_daily_resultMap;
	std::map < std::string, std::string> m_result_statistics;

	void calculate_result();
	std::map<std::string, double> calculate_statistics(bool bOutput = false);

	//�ṩ��Strategytemplate���õĽӿ�
	std::vector<std::string> sendOrder(std::string symbol, std::string strDirection, std::string strOffset, double price, double volume, StrategyTemplate* Strategy);
	void cancelOrder(std::string orderID, std::string gatewayname);

	void PutEvent(std::shared_ptr<Event>e);

	std::vector<TickData> loadTickbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<BarData> loadBarbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<TickData> loadTick(std::string symbol, int days);
	std::vector<BarData> loadBar(std::string symbol, int days);

	EventEngine* m_eventengine;
	//void InitUI();


	//�ز⺯��
	std::vector<BarData> LoadHistoryData();				//��ȡ�ز��õ���ʷ����
	void update_daily_close(float price);


	void CrossLimitOrder(const TickData& data);
	void CrossLimitOrder(const BarData& data);

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

	//�ز�ı���
	int m_limitorderCount=1;						//�˹�ORDERID
	int  m_tradeCount=0;							//�ɽ�����
	std::string m_backtestmode;								//barģʽ����tickģʽ
};

