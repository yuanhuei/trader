#ifndef	QCSTRUCTS_H
#define QCSTRUCTS_H

#include<string>
#include<iostream>
#include<vector>
#include<mutex>
#include<map>
#include"utils.hpp"
/*************************************/
#define PRICETYPE_LIMITPRICE "pricetypelimit"
#define PRICETYPE_MARKETPRICE "pricetypemarket"
#define PRICETYPE_FAK "FAK"
#define PRICETYPE_FOK "FOK"
#define PRICETYPE_STOP "STOP"
#define PRICETYPE_REQUEST "REQUEST_PRICE"

#define DIRECTION_NONE "directionnone"
#define DIRECTION_LONG "directionlong"
#define DIRECTION_SHORT "directionshort"
#define DIRECTION_UNKNOWN "unknow"
#define DIRECTION_NET "directionnet"

#define OFFSET_NONE "offsetnone"
#define OFFSET_OPEN "offsetopen"
#define OFFSET_CLOSE "offsetclose"
#define OFFSET_CLOSETODAY "offsetclosetoday"
#define OFFSET_CLOSEYESTERDAY "offsetcloseyesterday"
#define OFFSET_UNKNOWN "offsetunknow"

#define EXCHANGE_SHFE "SHFE"  //�Ϻ��ڻ�������
#define EXCHANGE_INE "INE"    //�Ϻ�������Դ��������

#define STATUS_ALLTRADED "orderalltraded"
#define STATUS_PARTTRADED "orderparttraded"
#define STATUS_NOTRADED "ordernottraded"

#define STATUS_CANCELLED "ordercanceld"
#define STATUS_WAITING "orderwaiting"
#define STATUS_SUBMITTING "ordersubmitting"
#define STATUS_TRIGGED "ordertrigged"
#define STATUS_REGECTED "orderrejected"

#define BAR_MODE "barmode"
#define TICK_MODE "tickmode"

typedef enum
{
	MINUTE = 0,
	HOUR = 1,
	DAILY = 2,
	WEEKLY = 3,
	TICK = 4
}Interval;

enum Mode
{
	RealMode = 0,
	BacktestMode = 1
};

struct SubscribeReq
{
	std::string symbol;
	std::string exchange;
	std::string productClass;
	std::string currency;
	std::string expiry;
	double strikePrice;
	std::string optionType;
};

struct OrderReq
{
	std::string symbol;
	std::string exchange;
	double price;
	double volume;
	std::string priceType;
	std::string direction;
	std::string offset;
	std::string productClass;
	std::string currency;
	std::string expiry;
	double strikePrice;
	std::string optionType;
	std::string strateyName;
};

struct StopOrderReq
{
	std::string symbol;
	std::string exchange;
	std::string  strategyName;
	std::string orderID;

	double price;
	double volume;
	std::string priceType;
	std::string direction;
	std::string offset;
	std::string productClass;
	std::string currency;
	std::string expiry;
	double strikePrice;
	std::string optionType;
	std::string strateyName;
};

struct CancelOrderReq
{
	std::string symbol;
	std::string exchange;
	std::string orderID;
	std::string frontID;
	std::string sessionID;
};

struct Portfolio_Result_Data
{

	double	maxCapital;
	double	drawdown;
	double	Winning;	//ƽ��ȫ��ӯ��
	double	Losing;		//ƽ��ȫ������
	double  totalwinning; //ƽ�־�ӯ��  winning-losing
	double  lastdayTotalwinning;
	double  delta;
	int totalResult;

	double holdingwinning;//�ֲ�ӯ��
	double holdingposition;
	double holdingprice;

	double holding_and_totalwinning;	//totalwinning+holdingwinning

	double portfolio_winning;			//���в��� ��Լ��ӯ��
};

class BaseData  //����
{
public:
	BaseData(std::string type) :m_datatype(type)
	{}
	std::string GetDataType()
	{
		return m_datatype;
	}
private:
	std::string m_datatype;
};

class TickData :public BaseData
{
public:
	int getminute()
	{
		std::string timecopy = time;
		std::vector<std::string>v;
		char* pch = strtok(const_cast<char*>(timecopy.c_str()), ":.");
		while (pch != NULL)
		{
			v.push_back(pch);
			pch = strtok(NULL, ":.");
		}
		if (v.size() == 4)
		{
			return atoi(v[1].c_str());//���ط���
		}
		else
		{
			std::cout << "�޷����ط��ӣ����ʱ��������⣡" << std::endl;
		}
	}
	int gethour()
	{
		std::string timecopy = time;
		std::vector<std::string>v;
		char* pch = strtok(const_cast<char*>(timecopy.c_str()), ":.");
		while (pch != NULL)
		{
			v.push_back(pch);
			pch = strtok(NULL, ":.");
		}
		if (v.size() == 4)
		{
			return atoi(v[0].c_str());//����Сʱ
		}
		else
		{
			std::cout << "�޷�����Сʱ�����ʱ��������⣡" << std::endl;
		}
	}
	TickData() :BaseData("tick")
	{};
	std::string symbol;
	std::string exchange;
	std::string gatewayname;
	//�ɽ�����
	double lastprice;//���³ɽ���
	double volume;//�ܳɽ���
	double openInterest;//�ֲ���
	long long unixdatetime;	//ʱ�������ȡmongodb����ʱ������1000
	std::string date;//����
	std::string time;//ʱ��
	//��������
	double openPrice;//���տ�
	double highPrice;//���ո�
	double lowPrice;//���յ�
	double preClosePrice;//����

	double upperLimit;//��ͣ
	double lowerLimit;//��ͣ
	//�嵵����
	double bidprice1;
	double bidprice2;
	double bidprice3;
	double bidprice4;
	double bidprice5;

	double askprice1;
	double askprice2;
	double askprice3;
	double askprice4;
	double askprice5;

	double bidvolume1;
	double bidvolume2;
	double bidvolume3;
	double bidvolume4;
	double bidvolume5;

	double askvolume1;
	double askvolume2;
	double askvolume3;
	double askvolume4;
	double askvolume5;
};

class BarData :public BaseData
{
public:
	int getminute()
	{
		std::string timecopy = time;
		std::vector<std::string>v;
		char* pch = strtok(const_cast<char*>(timecopy.c_str()), ":.");
		while (pch != NULL)
		{
			v.push_back(pch);
			pch = strtok(NULL, ":.");
		}
		if (v.size() == 4||v.size()==3)
		{
			return atoi(v[1].c_str());//���ط���
		}
		else
		{
			std::cout << "�޷����ط��ӣ����ʱ��������⣡" << std::endl;
		}
	}
	int gethour()
	{
		std::string timecopy = time;
		std::vector<std::string>v;
		char* pch = strtok(const_cast<char*>(timecopy.c_str()), ":.");
		while (pch != NULL)
		{
			v.push_back(pch);
			pch = strtok(NULL, ":.");
		}
		if (v.size() == 4||v.size()==3)
		{
			return atoi(v[0].c_str());//����Сʱ
		}
		else
		{
			std::cout << "�޷�����Сʱ�����ʱ��������⣡" << std::endl;
		}
	}

	BarData() :BaseData("bar")
	{};
	std::string symbol;
	std::string exchange;
	double open = 0;
	double high = 0;
	double low = 999999;
	double close = 0;

	std::string date;
	std::string time;
	std::string datetime;
	long long unixdatetime;	//ʱ�������ȡmongodb����ʱ������1000

	double openPrice = 0;//���տ�
	double highPrice = 0;//���ո�
	double lowPrice = 999999;//���յ�
	double preClosePrice = 0;//����

	double upperLimit = 0;//��ͣ
	double lowerLimit = 0;//��ͣ

	double volume = 0;
	double openInterest = 0;
	Interval interval; //��ʾ�Ƿ��ӣ�Сʱ��������
};

class DailyBar :public BaseData
{
public:
	int getminute()
	{
		std::string timecopy = time;
		std::vector<std::string>v;
		char* pch = strtok(const_cast<char*>(timecopy.c_str()), ":.");
		while (pch != NULL)
		{
			v.push_back(pch);
			pch = strtok(NULL, ":.");
		}
		if (v.size() == 4)
		{
			return atoi(v[1].c_str());//���ط���
		}
		else
		{
			std::cout << "�޷����ط��ӣ����ʱ��������⣡" << std::endl;
		}
	}
	int gethour()
	{
		std::string timecopy = time;
		std::vector<std::string>v;
		char* pch = strtok(const_cast<char*>(timecopy.c_str()), ":.");
		while (pch != NULL)
		{
			v.push_back(pch);
			pch = strtok(NULL, ":.");
		}
		if (v.size() == 4)
		{
			return atoi(v[0].c_str());//����Сʱ
		}
		else
		{
			std::cout << "�޷�����Сʱ�����ʱ��������⣡" << std::endl;
		}
	}
	DailyBar() :BaseData("dailybar")
	{};
	std::string symbol;
	std::string exchange;
	double open;
	double high;
	double low;
	double close;

	std::string date;
	std::string time;
	long long unixdatetime;//���뼶UNIXʱ���

	double volume;
	double openInterest;
};

/*********************************�¼��궨��**********************************************/
#define EVENT_QUIT "equit"//�˳��¼�
#define EVENT_TIMER "etimer"//ѭ��1�����¼����������ϵĲ�ֲ֣���ֹCTP����
#define EVENT_TICK "etick"
#define EVENT_TRADE "etrade"
#define EVENT_ORDER "eorder"
#define EVENT_STOP_ORDER "stoporder"
#define EVENT_POSITION "ePosition"
#define EVENT_ACCOUNT "eAccount"
#define EVENT_CONTRACT "eContract"
#define EVENT_ERROR "eError"
#define EVENT_LOG "elog"
#define EVENT_LOADSTRATEGY "eloadstrategy"
#define EVENT_UPDATESTRATEGY "eupdatestrategy"
#define EVENT_UPDATEPORTFOLIO "eupdateportfolio"
//�ز��¼�
#define EVENT_BACKTEST_TICK "ebacktesttick"
#define EVENT_BACKTEST_BAR "ebacktestbar"
#define EVENT_CTABACKTESTERFINISHED "backtessterfinished"

///////////////////////////////////////


/**********************************�¼�����*****************************************/
class  Event //�����¼�����
{
public:
	Event(std::string type) :m_eventtype(type)
	{}
	std::string GetEventType()
	{
		return m_eventtype;
	}
private:
	std::string m_eventtype;
};
class  Event_TesterFinished :public Event
{
public:
	Event_TesterFinished() :Event(EVENT_CTABACKTESTERFINISHED)
	{}
};

class  Event_Exit :public Event
{
public:
	Event_Exit() :Event(EVENT_QUIT)
	{}
};

class   Event_Timer :public Event
{
public:
	Event_Timer() :Event(EVENT_TIMER)
	{}
};

class   Event_Tick :public Event
{
public:
	Event_Tick(): Event(EVENT_TICK)
	{}
	//����
	std::string symbol;
	std::string exchange;
	std::string gatewayname;
	//�ɽ�����
	double lastprice;//���³ɽ���
	double volume;//�ܳɽ���
	double openInterest;//�ֲ���
	std::string date;//����
	std::string time;//ʱ��
	//��������
	double openPrice;//���տ�
	double highPrice;//���ո�
	double lowPrice;//���յ�
	double preClosePrice;//����

	double upperLimit;//��ͣ
	double lowerLimit;//��ͣ
	//�嵵����
	double bidprice1;
	double bidprice2;
	double bidprice3;
	double bidprice4;
	double bidprice5;

	double askprice1;
	double askprice2;
	double askprice3;
	double askprice4;
	double askprice5;

	double bidvolume1;
	double bidvolume2;
	double bidvolume3;
	double bidvolume4;
	double bidvolume5;

	double askvolume1;
	double askvolume2;
	double askvolume3;
	double askvolume4;
	double askvolume5;

};

class   Event_Trade :public Event
{
public:
	Event_Trade() :Event(EVENT_TRADE)
	{}
	//������
	std::string symbol;
	std::string exchange;
	std::string tradeID;   //���ױ��
	std::string orderID;  //�������
	std::string gatewayname;
	//�ɽ����
	std::string direction;//����
	std::string offset; //�ɽ���ƽ��
	double price;//�ɽ��۸�
	double volume;//�ɽ���
	std::string tradeTime;//�ɽ�ʱ��
};

class   Event_Order :public Event
{
public:
	Event_Order() :Event(EVENT_ORDER)
	{}
	//������
	std::string symbol;
	std::string exchange;
	std::string orderID;//�������
	std::string gatewayname;
	//�������
	std::string direction;//����
	std::string offset;//��ƽ����
	double price; //�����۸�
	double totalVolume;//��������
	double tradedVolume;//�ɽ�����
	std::string status;//����״̬

	std::string orderTime;//����ʱ��
	std::string cancelTime;//����ʱ��

	int frontID;//ǰ�û����
	int sessionID;//���ӱ��


};
class   Event_StopOrder :public Event
{
public:
	Event_StopOrder() :Event(EVENT_STOP_ORDER)
	{}
	//������
	std::string symbol;
	std::string exchange;
	std::string orderID;//�������
	std::string gatewayname;
	//�������
	std::string direction;//����
	std::string offset;//��ƽ����
	double price; //�����۸�
	double totalVolume;//��������
	double tradedVolume;//�ɽ�����
	std::string status;//����״̬

	std::string orderTime;//����ʱ��
	std::string cancelTime;//����ʱ��

	int frontID;//ǰ�û����
	int sessionID;//���ӱ��
	std::string strategyName;

};

class   Event_Contract :public Event
{
public:
	Event_Contract() :Event(EVENT_CONTRACT)
	{}
	std::string symbol;
	std::string exchange;
	std::string name;                   //��Լ������
	std::string gatewayname;
	std::string productClass;           //��Լ����
	int size;                           //��Լ��С
	double priceTick;					//��Լ��С�۸�Tick

	double strikePrice;					//��Ȩ��
	std::string underlyingSymbol;       //������Լ����
	std::string optionType;				//��Ȩ����
};

class   Event_Position :public Event
{
public:
	Event_Position() :Event(EVENT_POSITION)
	{
		position = 0;
		todayPosition = 0;
		ydPosition = 0;
		todayPositionCost = 0;
		ydPositionCost = 0;
		price = 0;
		frozen = 0;
	}
	std::string symbol;
	std::string direction;
	std::string gatewayname;
	double position;
	double todayPosition;
	double ydPosition;
	double todayPositionCost;
	double ydPositionCost;
	double price;
	double frozen;
};

class   Event_Account :public Event
{
public:
	Event_Account() :Event(EVENT_ACCOUNT)
	{}
	std::string gatewayname;
	std::string accountid;
	double preBalance;//�����˻����㾻ֵ
	double balance;//�˻���ֵ
	double available;//�����ʽ�
	double commission;//����������
	double margin;//��֤��ռ��
	double closeProfit;//ƽ��ӯ��
	double positionProfit;//�ֲ�ӯ��
};

class   Event_Error :public Event
{
public:
	Event_Error() :Event(EVENT_ERROR)
	{}
	std::string errorID;//�������
	std::string errorMsg;//������Ϣ
	std::string additionalInfo;//������Ϣ
	std::string gatewayname;
	std::string errorTime = Utils::getCurrentSystemTime();
};

class   Event_Log :public Event
{
public:
	Event_Log() :Event(EVENT_LOG)
	{}
	std::string msg;//log��Ϣ
	std::string gatewayname;
	std::string logTime = Utils::getCurrentSystemTime();
};

class   Event_UpdateStrategy :public Event
{
public:
	Event_UpdateStrategy() :Event(EVENT_UPDATESTRATEGY)
	{}
	std::string strategyname;
	std::map<std::string, std::string>parammap;
	std::map<std::string, std::string>varmap;
};

class   Event_UpdatePortfolio :public Event
{
public:
	Event_UpdatePortfolio() :Event(EVENT_UPDATEPORTFOLIO)
	{}
	std::string strategyname;
	std::string symbol;
	time_t datetime;
	Portfolio_Result_Data Portfoliodata;
	std::vector<int>strategyrows;
};

class   Event_LoadStrategy :public Event
{
public:
	Event_LoadStrategy() :Event(EVENT_LOADSTRATEGY)
	{}
	std::string strategyname;
	std::map<std::string, std::string>parammap;
	std::map<std::string, std::string>varmap;
};

class   Event_Backtest_Tick :public Event
{
public:
	Event_Backtest_Tick() :Event(EVENT_BACKTEST_TICK)
	{}
	TickData tick;
};

class   Event_Backtest_Bar :public Event
{
public:
	Event_Backtest_Bar() :Event(EVENT_BACKTEST_BAR)
	{}
	BarData bar;
};
#endif