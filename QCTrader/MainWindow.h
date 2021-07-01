#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
//#include"event_engine/eventengine.h"
//#include"gateway/gatewaymanager.h"
#include<qstandarditemmodel.h>
#include"qcstructs.h"

class EventEngine;
class Gatewaymanager;

struct UpdatePriceTableData
{
	std::string symbol;
	std::string exchange;
	std::string gatewayname;
	std::string date;//����
	std::string time;//ʱ��

	//�ɽ�����
	double lastprice;//���³ɽ���
	double openInterest;//�ֲ���

	double upperLimit;//��ͣ
	double lowerLimit;//��ͣ
	double bidprice1;
	double askprice1;
};



struct UpdateStrategyData
{
	std::string strategyname;
	std::map<std::string, std::string>parammap;
	std::map<std::string, std::string>varmap;
};

struct LoadStrategyData
{
	std::string strategyname;
	std::map<std::string, std::string>parammap;
	std::map<std::string, std::string>varmap;
};

struct PositionData
{
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

struct AccountData
{
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

struct PortfolioData
{
	std::string dllname;
	std::string strategyname;
	std::string symbol;
	Portfolio_Result_Data Portfoliodata;
	std::vector<int>strategyrows;
};

struct LogData
{
	std::string msg;//log��Ϣ
	std::string gatewayname; //�ӿ���
	std::string logTime;//ʱ��

};


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

	void setUI();
	void LoadEngine();
	void RegEvent();

	//�¼��ص�����
	void OnLogUpdate(std::shared_ptr<Event>e);
	void onAccountUpdate(std::shared_ptr<Event>e);
	void onPositionUpdate(std::shared_ptr<Event>e);
	void onPriceTableUpdate(std::shared_ptr<Event>e);
	//�����źźͲ�
	void ConnectSignalAndSlot();
	//��־��¼
	void write_log(std::string msg, std::string gateway_name);

	void UpdateSymbolBox(UpdatePriceTableData data);

signals:
	void UpdateLogSignal(LogData data);
	//void LoadStrategySignal(LoadStrategyData data);
	//void UpdateStrategySignal(UpdateStrategyData data);
	void UpdateAccountSignal(AccountData data);
	void UpdatePositionSignal(PositionData  data);
	//void UpdatePortfolioSignal(PortfolioData data);
	void UpdatePriceTableSignal(UpdatePriceTableData data);
	//void UpdatePortfolioWinning(QString winning);

private:
	Ui::MainWindow ui;

private slots:
	void menu_ctp_connect();
	void menu_exit();
	void symbol_ReturnPressed();
	void UpdateAccountBox(AccountData data);
	void UpdatePositionBox(PositionData data);
	void UpdateLogTable(LogData data);
	void UpdateTickTable(UpdatePriceTableData data);

public:
	//�������������ָ��
	EventEngine* m_eventengine;//�¼���������
	Gatewaymanager* m_gatewaymanager;//�ӿڹ�����
	//riskmanager* m_riskmanager;//���չ�����
	//CTAmanager* m_ctamanager;//cta������


    //model
	QStandardItemModel* m_AccountModel;
	QStandardItemModel* m_PositionModel;
	QStandardItemModel* m_SymbolSubscribedTableModel;
};
