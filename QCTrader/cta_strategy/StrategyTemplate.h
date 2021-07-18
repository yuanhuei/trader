#ifndef STRATEGYTEMPLATE_H
#define STRATEGYTEMPLATE_H
#include<map>
#include<set>
#include<memory>
#include<atomic>
#include<vector>
#include"../qcstructs.h"
#include"CtaEngine.h"
#include"json11.hpp"
#include"AlgorithmOrder.h"
#include"MongoCxx.h"
#include"../include/libmongoc-1.0/mongoc.h"
#include"../include/libbson-1.0/bson.h"



class algorithmOrder;
class MongoCxx;
class CtaEngine;
class BaseEngine;


class StrategyData
{
public:
	void insertparam(std::string key, std::string value);					//�������
	void insertvar(std::string key, std::string value);						//�������
	void setparam(std::string key, std::string value);//���ò�����ֵ
	void setvar(std::string key, std::string value);//���ñ�����ֵ
	std::string getparam(std::string);										//��ȡ����
	std::string getvar(std::string);										//��ȡ����
	std::map<std::string, std::string>getallparam();						//��ȡ����
	std::map<std::string, std::string>getallvar();							//��ȡ����
private:
	std::mutex m_mtx;					//������
	//�����������ֵ����string��ʽ����д���Ե�ʱ��һ���������ֵ��������Ҫ�Լ���һ��ת��
	std::map<std::string, std::string>m_parammap;//�����б� 
	std::map<std::string, std::string>m_varmap;//�����б�
};

class StrategyTemplate
{
public:
	StrategyTemplate(BaseEngine* ctaEngine,std::string strategyName,std::string symbol);
	virtual ~StrategyTemplate();
	/******************************���Բ����ͱ���*********************************************/
	//��������
	std::string gatewayname;				//CTP
	//��������
	std::string m_symbol;					//���׵ĺ�Լ
	std::string m_exchange;						//��Լ������
	std::string m_strategyName;             //��������

	std::string trademode;					//����ģʽbar����tick����
	bool inited;							//��ʼ������
	bool trading;							//���׿���
	int m_Pos;                                //��λ
	int initDays;							//������ʷ���ݵ�����

	//�㷨���ײ��ֱ���						Ĭ��ֵ�����Բ��޸��ڲ���ʵ���е���setlimit��������������
	double unitLimit;						//�㷨�µ�
	enum Mode TradingMode;				//�ز⻹��ʵ��

	/******************************CTAMANAGER���Ʋ���***********************************************/
	//��ʼ��
	virtual void onInit();
	//��ʼ 
	virtual void onStart();
	//ֹͣ
	virtual void onStop();
	//��Ҫ����Ĳ��Ժ���ʵ�֣��������ļ���ȡ�õ�ֵ��ֵ�������о���ı���
	virtual void updateSetting()=0;


	//��������ֵ
	void checkparam(const char* paramname, const char* paramvalue);

	//���²����ͱ�����ֵ
	void updateParam(const char* paramname, const char* paramvalue);
	void updateVar(const char* paramname, const char* paramvalue);
	//��pos��ֵ
	void checkSymbol(const char* symbolname);
	//��ȡ������ֵ
	std::string getparam(std::string paramname);
	//���ز������ݵ�����
	void putGUI();
	//���²���������
	virtual void putEvent();
	//tradeevent���³ֲ�
	void setPos(int pos);
	void changeposmap(std::string symbol, double pos);

	void sync_data();//���������ݱ��浽json�ļ�,һ����ֹͣ���ԣ����׵��ɹ����˳�ʱ����
	/*******************************ʵ�ֲ�����Ҫ����**************************************************/
	//TICK
	virtual void onTick(TickData Tick);
	//BAR
	virtual void onBar(BarData Bar);
	//�����ص�
	virtual void onOrder(std::shared_ptr<Event_Order>e);
	//�ɽ��ص�
	virtual void onTrade(std::shared_ptr<Event_Trade>e);

	virtual void onStopOrder(std::shared_ptr<Event_StopOrder>e);
	//��������

	//����bStopOrderֹͣ�� ���� �޼۵���CTP��֧��ֹͣ����ʵ���˱���ֹͣ���Ĺ���
	std::vector<std::string> buy(double price, double volume,bool bStopOrder = false);
	//ƽ��
	std::vector<std::string> sell(double price, double volume, bool bStopOrder = false);
	//����
	std::vector<std::string> sellshort(double price, double volume, bool bStopOrder = false);
	//ƽ��
	std::vector<std::string> buycover(double price, double volume, bool bStopOrder = false);

	//�ܱ�����ƽ��������
	std::vector<std::string> sendOrder(bool bStopOrder, std::string strDirection, std::string strOffset, double price, double volume);
	//�����е���ֹͣ����ʱʹ��
	void cancelAllOrder();
	//��������
	void cancelOrder(std::string orderID, std::string gatewayname);


	//��ȡm_Pos
	int getpos();

	//�㷨����
	double getpos(std::string symbol);							//���㷨����ģ���ȡ�ֲ��ⲿ�ӿ�
	
	std::map<std::string, double>getposmap();					//��ȡȫ���ֲ֣����㷨����������

	//�ṩ��backtestengine���ⲿ�ӿ�
	//std::map<std::string, std::string> GetVarPlotMap();
	//std::map<std::string, std::string> GetIndicatorMap();


	//�㷨����
	algorithmOrder *m_algorithm;
public:
	//��ȡ��ʷ����
	std::vector<TickData>loadTick(std::string symbol, int days);
	std::vector<BarData>loadBar(std::string symbol, int days);

	//����mongocxxģ�屣��Ͷ�ȡmongodb���Ѿ�ûʹ���ˣ�ʹ��json�ļ��������
	//virtual void savepostomongo();
	//virtual void loadposfrommongo();
	MongoCxx *m_MongoCxx;

	//CTA������
	BaseEngine* m_ctaEngine;

	//Bar����                                   tick�����Բ��� �� �����bar�ľ�Ҫ��
	std::mutex m_hourminutemtx;					//�����ڼ̳еĲ���ֱ������Щ������ʡ�������� 
	int m_minute;
	int m_hour;
	BarData m_bar;

	//���������б�
	StrategyData *m_strategydata;


	std::map<std::string, double>m_pos_map;				//�ֲ�
	std::mutex m_Pos_mapmtx;

	//OrderList
	std::mutex m_orderlistmtx;							//������
	std::vector<std::string>m_orderlist;				//��ͨ�����б�
};

#endif

