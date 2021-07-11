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
struct UnitResult
{
	double  totalwinning = 0;
	double  lastdayTotalwinning;
	double  delta;
	double	maxCapital = 0;
	double	drawdown = 0;
	double	Winning = 0;
	double	Losing = 0;
	int totalResult = 0;

	double holdingwinning = 0;//�ֲ�ӯ��
	double holdingposition = 0;
	double holdingprice = 0;
};


class DailyTradingResult
{
public:
	DailyTradingResult(QDate date, double price);
	//DailyTradingResult(double entryPrice, std::string entryDt, double exitPrice,
		//std::string exitDt, double volume,double size);
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

typedef std::map<std::string, UnitResult> Result;//key�Ǻ�Լ value��һ�������λ

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
	std::map<std::string, std::shared_ptr<Event_Order>>m_Ordermap;			std::mutex m_ordermapmtx;	//���еĵ�������ɾ��
	std::map<std::string, std::shared_ptr<Event_Order>>m_WorkingOrdermap;								//�����еĵ�   ��workingordermap��һ����

	int m_trade_count = 0;
	//trades = {}
	std::map<std::string, std::shared_ptr<Event_Trade>>m_tradeMap;//�ɽ���					//std::mutex m_tradelistmtx;//���㻺��
	std::map<std::string, Result>m_result;											std::mutex m_resultmtx;			//������

	std::vector<BarData> vector_history_data;


	//logs = []

	std::map<QDate, std::shared_ptr<DailyTradingResult>> m_daily_resultMap;
	//daily_df = None
	//m_result_df;
	std::map < std::string, std::string> m_result_statistics;

	void calculate_result();
	std::map<std::string, double> calculate_statistics(bool bOutput = false);

	//�ṩ��Strategytemplate���õĽӿ�
	std::vector<std::string> sendOrder(std::string symbol, std::string orderType, double price, double volume, StrategyTemplate* Strategy);
	void cancelOrder(std::string orderID, std::string gatewayname);
	//void writeCtaLog(std::string msg, std::string gatewayname);
	void PutEvent(std::shared_ptr<Event>e);
	std::vector<TickData> loadTickbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<BarData> loadBarbyDateTime(std::string symbol, QDateTime startDay, QDateTime endDay);
	std::vector<TickData> loadTick(std::string symbol, int days);
	std::vector<BarData> loadBar(std::string symbol, int days);

	EventEngine* m_eventengine;
	void InitUI();

	//MONGOC �̳߳�
	//mongoc_uri_t* m_uri;
	//mongoc_client_pool_t* m_pool;

	//�ز⺯��
	std::vector<BarData> LoadHistoryData();				//��ȡ�ز��õ���ʷ����
	void update_daily_close(float price);



	void processTickEvent(std::shared_ptr<Event>e);					//����tick�¼�
	void processBarEvent(std::shared_ptr<Event>e);					//����bar�¼�
	void CrossLimitOrder(const TickData& data);
	void CrossLimitOrder(const BarData& data);
	void Settlement(std::shared_ptr<Event_Trade>etrade, std::map<std::string, StrategyTemplate*>orderStrategymap);
	void RecordCapital(const TickData& data);
	void RecordCapital(const BarData& data);
	void RecordPNL(const TickData& data);
	void RecordPNL(const BarData& data);

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
	std::atomic_int m_limitorderCount;						//�˹�ORDERID
	std::atomic_int  m_tradeCount;							//�ɽ�����
	std::string m_backtestmode;								//barģʽ����tickģʽ
	std::atomic<time_t> m_datetime;							//�Ա�ʱ��
	std::atomic_int working_worker; 	std::mutex m_workermutex;	//ͬ����
	std::condition_variable m_cv;
	std::map<std::string, double>symbol_size;						//��Լ����������Ӳ�̶�ȡ	ֻ����д�����ü���
	std::map<std::string, double>symbol_rate;						//��Լ����������Ӳ�̶�ȡ	ֻ����д�����ü���
	//std::map<std::string, double>m_slippage;
	std::map<std::string, HINSTANCE>dllmap;																//��Ų���dll����		ֻ����д�����ü���

	//key ��OrderID  value �ǲ��Զ��� ��;��Ϊ�˱�֤�������������Է���ȥ��  �ɽ��ر�����ֲ���ȷ���ض�Ӧ�Ĳ����ϣ��Է�������Խ���ͬһ����Լ����BUG
	std::map<std::string, StrategyTemplate*>m_orderStrategymap;	            std::mutex m_orderStrategymtx;
	std::map<std::string, StrategyTemplate*>m_strategymap;					std::mutex m_strategymtx;	//��������Ͳ��Զ���ָ��
	//key ��Լ����value ����ָ��(ʵ��)vector  �����Ѳ�ͬ�ĺ�Լtick���͵�ÿһ�����Զ���
	std::map<std::string, std::vector<StrategyTemplate*>>m_tickstrategymap;	std::mutex m_tickstrategymtx;

	std::set<std::string>m_backtestsymbols;									std::mutex	m_backtestsymbolmtx;//Ҫ�ز�ĺ�Լ�б� ������ȡ����
	std::vector<TickData>m_Tickdatavector;									std::mutex m_HistoryDatamtx;//��ʷ���ݼ���
	std::vector<BarData>m_Bardatavector;

	//�ز�ʱ��
	time_t startDatetime;
	time_t endDatetime;

};

