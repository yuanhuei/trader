#include "MainWindow.h"
#include"CTPConnectWidgets.h"
#include"event_engine/eventengine.h"
#include"gateway/gatewaymanager.h"
#include"utility.h"
#include"CTAStrategyManager.h"
#include"qwidget.h"
#include<qglobal.h>
#include"./cta_strategy/CtaEngine.h"
#include"risk_manager/riskmanager.h"
#include"BacktesterManager.h"
#include"cta_backtester/BacktesterEngine.h"
#include"ContractQueryManager.h"



#include"MongoCxx.h"
#include"../include/libmongoc-1.0/mongoc.h"
#include"../include/libbson-1.0/bson.h"

//MONGOC �̳߳�
mongoc_uri_t* g_uri;
mongoc_client_pool_t* g_pool;
//��ʼ��MONGODB

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	mongoc_init();													//1
	g_uri = mongoc_uri_new("mongodb://localhost:27017/");			//2
	// �����ͻ��˳�
	g_pool = mongoc_client_pool_new(g_uri);

	setUI();
	LoadEngine();
	
}

MainWindow::~MainWindow()
{
	delete m_TradeSubmitTableModel;;
	delete m_OrderSubmitTableModel;;
	delete m_AccountModel;
	delete m_PositionModel;
	delete m_SymbolSubscribedTableModel;


	m_gatewaymanager->exit();
	delete m_riskmanager;
	delete m_ctaEngine;
	delete m_ctaBacktesterManager;
	delete m_gatewaymanager;
	delete m_eventengine;

	mongoc_client_pool_destroy(g_pool);
	mongoc_uri_destroy(g_uri);
	mongoc_cleanup();
}

//��UI���ɵĽ�����һЩ����
void  MainWindow::setUI()
{

	setWindowState(Qt::WindowMaximized);//���ô������
	setWindowTitle("QCTrade");

	/*
	QPalette pal(this->palette());
	pal.setColor(QPalette::Background, Qt::black); //���ñ�����ɫ
	this->setAutoFillBackground(true);
	this->setPalette(pal);
	*/
	//�����ʽ��
	m_AccountModel = new QStandardItemModel;
	QStringList accountheader;
	accountheader << str2qstr_new("�ӿ���") << str2qstr_new("�˻�ID") << str2qstr_new("���") << str2qstr_new("��ֵ") << str2qstr_new("����") << str2qstr_new("������") << str2qstr_new("��֤��") << str2qstr_new("ƽ��ӯ��") << str2qstr_new("�ֲ�ӯ��");
	m_AccountModel->setHorizontalHeaderLabels(accountheader);
	ui.tableView_2->setModel(m_AccountModel);
	ui.tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_2->setSelectionBehavior(QAbstractItemView::SelectRows);  //����ѡ��һ��  
	ui.tableView_2->setSelectionMode(QAbstractItemView::SingleSelection); //����ֻ��ѡ��һ�У����ܶ���ѡ��  
	ui.tableView_2->setAlternatingRowColors(true);

	//���óֱֲ�
	m_PositionModel = new QStandardItemModel;
	QStringList positionheader;
	positionheader << str2qstr_new("�ӿ���") << str2qstr_new("��Լ") << str2qstr_new("����") << str2qstr_new("��λ") << str2qstr_new("���") << str2qstr_new("�����ʽ�") << str2qstr_new("�ֲּ�");
	m_PositionModel->setHorizontalHeaderLabels(positionheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_3->setModel(m_PositionModel);
	ui.tableView_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_3->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_3->setSelectionBehavior(QAbstractItemView::SelectRows);  //����ѡ��һ��  
	ui.tableView_3->setSelectionMode(QAbstractItemView::SingleSelection); //����ֻ��ѡ��һ�У����ܶ���ѡ��  
	ui.tableView_3->setAlternatingRowColors(true);

	//�������鶩�ı�
	m_SymbolSubscribedTableModel = new QStandardItemModel;
	QStringList symbolheader;
	symbolheader << str2qstr_new("��Լ����") << str2qstr_new("������") << str2qstr_new("���¼�") << str2qstr_new("�ֲ���") << str2qstr_new("��ͣ") << str2qstr_new("��ͣ") << str2qstr_new("��һ��") << str2qstr_new("��һ��")<< str2qstr_new("ʱ��") << str2qstr_new("�ӿ�");
	m_SymbolSubscribedTableModel->setHorizontalHeaderLabels(symbolheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_4->setModel(m_SymbolSubscribedTableModel);
	ui.tableView_4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_4->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_4->setSelectionBehavior(QAbstractItemView::SelectRows);  //����ѡ��һ��  
	ui.tableView_4->setSelectionMode(QAbstractItemView::SingleSelection); //����ֻ��ѡ��һ�У����ܶ���ѡ��  
	ui.tableView_4->setAlternatingRowColors(true);

	//����ί�ж�����
	m_OrderSubmitTableModel = new QStandardItemModel;
	QStringList ordersubmitheader;
	ordersubmitheader << str2qstr_new("ί�к�") << str2qstr_new("��Դ") << str2qstr_new("��Լ����") << str2qstr_new("������") << str2qstr_new("����") << str2qstr_new("��ƽ") << str2qstr_new("�۸�") << str2qstr_new("������") << str2qstr_new("�ѳɽ�") << str2qstr_new("״̬")<< str2qstr_new("ʱ��")<< str2qstr_new("�ӿ�");
	m_OrderSubmitTableModel->setHorizontalHeaderLabels(ordersubmitheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_5->setModel(m_OrderSubmitTableModel);
	ui.tableView_5->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_5->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_5->setSelectionBehavior(QAbstractItemView::SelectRows);  //����ѡ��һ��  
	ui.tableView_5->setSelectionMode(QAbstractItemView::SingleSelection); //����ֻ��ѡ��һ�У����ܶ���ѡ��  
	ui.tableView_5->setAlternatingRowColors(true);

	//����ί�ж�����
	m_TradeSubmitTableModel = new QStandardItemModel;
	QStringList Tradesubmitheader;
	Tradesubmitheader << str2qstr_new("�ɽ���") << str2qstr_new("ί�к�") << str2qstr_new("��Լ����") << str2qstr_new("������")<< str2qstr_new("����") << str2qstr_new("��ƽ") << str2qstr_new("�۸�") << str2qstr_new("����") << str2qstr_new("ʱ��")<< str2qstr_new("�ӿ�");
	m_TradeSubmitTableModel->setHorizontalHeaderLabels(Tradesubmitheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_6->setModel(m_TradeSubmitTableModel);
	ui.tableView_6->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_6->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_6->setSelectionBehavior(QAbstractItemView::SelectRows);  //����ѡ��һ��  
	ui.tableView_6->setSelectionMode(QAbstractItemView::SingleSelection); //����ֻ��ѡ��һ�У����ܶ���ѡ��  
	ui.tableView_6->setAlternatingRowColors(true);


}


void MainWindow::LoadEngine()
{
	m_eventengine = new EventEngine;//�¼���������

	m_gatewaymanager = new Gatewaymanager(m_eventengine);//�ӿڹ�����

	m_riskmanager = new riskmanager(m_eventengine); //���չ�����

	m_ctaEngine = new CtaEngine(m_gatewaymanager, m_eventengine, m_riskmanager);//cta������

	m_backtesterEngine = new BacktesterEngine(m_eventengine);

	RegEvent();
	ConnectSignalAndSlot();
	m_eventengine->StartEngine();
}

void MainWindow::RegEvent()
{
	m_eventengine->RegEvent(EVENT_LOG,std::bind(&MainWindow::OnLogUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_ACCOUNT, std::bind(&MainWindow::onAccountUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_POSITION, std::bind(&MainWindow::onPositionUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TICK, std::bind(&MainWindow::onPriceTableUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_ORDER, std::bind(&MainWindow::onOrderTableUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TRADE, std::bind(&MainWindow::onTradeTableUpdate, this, std::placeholders::_1));

	//m_eventengine->RegEvent(EVENT_LOADSTRATEGY, std::bind(&MainWindow::onStrategyLoaded, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_UPDATESTRATEGY, std::bind(&MainWindow::onStrategyUpdate, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_UPDATEPORTFOLIO, std::bind(&MainWindow::onPortfolioUpdate, this, std::placeholders::_1));
}

void MainWindow::ConnectSignalAndSlot()
{
	//�źźͲۺ���
	//qRegisterMetaType<LoadStrategyData>("LoadStrategyData");//ע�ᵽԪϵͳ��
	//qRegisterMetaType<UpdateStrategyData>("UpdateStrategyData");//ע�ᵽԪϵͳ��
	//qRegisterMetaType<PortfolioData>("PortfolioData");//ע�ᵽԪϵͳ��

	qRegisterMetaType<PositionData>("PositionData");//ע�ᵽԪϵͳ��
	qRegisterMetaType<AccountData>("AccountData");//ע�ᵽԪϵͳ��
	qRegisterMetaType<std::string>("std::string");//ע�ᵽԪϵͳ��
	qRegisterMetaType<LogData>("LogData");//ע�ᵽԪϵͳ�� UpdatePriceTableData
	qRegisterMetaType<UpdatePriceTableData>("UpdatePriceTableData");
	qRegisterMetaType<UpdateOrderTableData>("UpdateOrderTableData");
	qRegisterMetaType<UpdateTradeTableData>("UpdateTradeTableData");


	connect(this, SIGNAL(UpdateLogSignal(LogData)), this, SLOT(UpdateLogTable(LogData)));
	connect(this, SIGNAL(UpdatePositionSignal(PositionData)), this, SLOT(UpdatePositionBox(PositionData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdateAccountSignal(AccountData)), this, SLOT(UpdateAccountBox(AccountData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdatePriceTableSignal(UpdatePriceTableData)), this, SLOT(UpdateTickTable(UpdatePriceTableData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdateOrDerTableSignal(UpdateOrderTableData)), this, SLOT(UpdateOrderTable(UpdateOrderTableData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdateTradeTableSignal(UpdateTradeTableData)), this, SLOT(UpdateTradeTable(UpdateTradeTableData)), Qt::QueuedConnection);

	//connect(this, SIGNAL(LoadStrategySignal(LoadStrategyData)), this, SLOT(CreateStrategyBox(LoadStrategyData)), Qt::QueuedConnection);
	//connect(this, SIGNAL(UpdateStrategySignal(UpdateStrategyData)), this, SLOT(UpdateStrategyBox(UpdateStrategyData)), Qt::QueuedConnection); 
	//connect(this, SIGNAL(UpdatePortfolioSignal(PortfolioData)), this, SLOT(UpdatePortfolioBox(PortfolioData)), Qt::QueuedConnection);
}


////////////////////////////////////////////////////�¼����������� On_��ͷ////////////
// 
//EVENT_LOG�¼��Ĵ�����������UpdateLog�źŸ�����
void MainWindow::OnLogUpdate(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Log> elog = std::static_pointer_cast<Event_Log>(e);
	//std::string msg = "�ӿ���:" + elog->gatewayname + "ʱ��:" + elog->logTime + "��Ϣ:" + elog->msg;
	LogData  logdata;
	logdata.gatewayname = elog->gatewayname;
	logdata.msg = elog->msg;
	logdata.logTime = elog->logTime;
	emit UpdateLogSignal(logdata);
}



void MainWindow::onAccountUpdate(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Account> eAccount = std::static_pointer_cast<Event_Account>(e);
	AccountData data;
	data.accountid = eAccount->accountid;
	data.available = eAccount->available;
	data.balance = eAccount->balance;
	data.closeProfit = eAccount->closeProfit;
	data.commission = eAccount->commission;
	data.gatewayname = eAccount->gatewayname;
	data.margin = eAccount->margin;
	data.positionProfit = eAccount->positionProfit;
	data.preBalance = eAccount->preBalance;
	emit  UpdateAccountSignal(data);
}

void MainWindow::onPositionUpdate(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Position> ePosition = std::static_pointer_cast<Event_Position>(e);
	PositionData data;
	data.direction = ePosition->direction;
	data.frozen = ePosition->frozen;
	data.gatewayname = ePosition->gatewayname;
	data.position = ePosition->position;
	data.price = ePosition->price;
	data.symbol = ePosition->symbol;
	data.todayPosition = ePosition->todayPosition;
	data.todayPositionCost = ePosition->todayPositionCost;
	data.ydPosition = ePosition->ydPosition;
	data.ydPositionCost = ePosition->ydPositionCost;
	emit  UpdatePositionSignal(data);
}


void MainWindow::onPriceTableUpdate(std::shared_ptr<Event>e)
{
	
	/**/std::shared_ptr<Event_Tick> eTick = std::static_pointer_cast<Event_Tick>(e);
	//UpdatePriceTableData data;
	std::shared_ptr<UpdatePriceTableData> data= std::make_shared< UpdatePriceTableData>();
	data->askprice1 = eTick->askprice1;
	data->bidprice1 = eTick->bidprice1;
	data->date = eTick->date;
	data->lastprice = eTick->lastprice;
	data->lowerLimit =eTick->lowerLimit;
	data->openInterest = eTick->openInterest;
	data->symbol =eTick->symbol;
	data->time = eTick->time;
	data->upperLimit = eTick->upperLimit;
	data->exchange = eTick->exchange;
	data->gatewayname = eTick->gatewayname;
	
	

	emit UpdatePriceTableSignal(*data);
}

void MainWindow::onOrderTableUpdate(std::shared_ptr<Event>e)
{
	UpdateOrderTableData data;
	std::shared_ptr<Event_Order> eTick = std::static_pointer_cast<Event_Order>(e);
	data.cancelTime = eTick->cancelTime;
	data.direction = eTick->direction;
	data.exchange = eTick->exchange;
	data.frontID = eTick->frontID;
	data.gatewayname = eTick->gatewayname;
	data.offset = eTick->offset;
	data.orderID = eTick->orderID;
	data.orderTime = eTick->orderTime;
	data.price = eTick->price;
	data.status = eTick->status;
	data.symbol = eTick->symbol;
	data.totalVolume = eTick->totalVolume;
	data.tradedVolume = eTick->tradedVolume;


	emit UpdateOrderTableSignal(data);

}

void MainWindow::onTradeTableUpdate(std::shared_ptr<Event>e)
{
	UpdateTradeTableData data;
	std::shared_ptr<Event_Trade> eTick = std::static_pointer_cast<Event_Trade>(e);
	data.tradeID = eTick->tradeID;
	data.direction = eTick->direction;
	data.exchange = eTick->exchange;
	data.gatewayname = eTick->gatewayname;
	data.offset = eTick->offset;
	data.orderID = eTick->orderID;
	data.tradeTime = eTick->tradeTime;
	data.price = eTick->price;
	data.symbol = eTick->symbol;
	data.volume = eTick->volume;


	emit UpdateTradeTableSignal(data);
}


//////////////////////////////////////�ۺ������� //////////////////////////////
// 
// 
// ����˵�CTP���� �Ĳۺ��� �����Ի���
void MainWindow::menu_ctp_connect()
{
	CTPConnectWidgets* e = new CTPConnectWidgets(this);
	e->show();

}
//����˵��˳���Ĳۺ���
void MainWindow::menu_exit()
{
	this->close();
}


// //�����Լ����س���Ĳۺ���
void MainWindow::symbol_ReturnPressed()
{

	SubscribeReq req;
	req.symbol = ui.lineEdit->text().toStdString();
	req.exchange = ui.comboBox->currentText().toStdString();
	if (req.symbol.length() > 0)
		m_gatewaymanager->subscribe(req, "CTP");

	bool bAdded = false;//��û�м�������۸��
	for (int i = 0; i < m_SymbolSubscribedTableModel->rowCount(); i++)
	{
		if (m_SymbolSubscribedTableModel->item(i, 0)->text().toStdString() != req.symbol)//�ж��Ƿ���һ����Լ���Ǿ͸�������
			bAdded = true;
	}
	if (bAdded==false)//��û���룬�ͼ���
	{
		int i = m_SymbolSubscribedTableModel->rowCount();
		m_SymbolSubscribedTableModel->setItem(i, 0, new QStandardItem(str2qstr_new(req.symbol)));
		m_SymbolSubscribedTableModel->setItem(i, 1, new QStandardItem(str2qstr_new(req.exchange)));
		//m_SymbolSubscribedTableModel->setItem(i, 2, new QStandardItem("0"));
		m_SymbolSubscribedTableModel->setItem(i, 3, new QStandardItem(QString::fromStdString((std::to_string(0)))));
		m_SymbolSubscribedTableModel->setItem(i, 4, new QStandardItem(QString::fromStdString((std::to_string(0)))));
		m_SymbolSubscribedTableModel->setItem(i, 5, new QStandardItem(QString::fromStdString((std::to_string(0)))));
		m_SymbolSubscribedTableModel->setItem(i, 6, new QStandardItem(QString::fromStdString((std::to_string(0)))));
		m_SymbolSubscribedTableModel->setItem(i, 7, new QStandardItem(QString::fromStdString((std::to_string(0)))));
		std::string DateTime = " ";
		m_SymbolSubscribedTableModel->setItem(i, 8, new QStandardItem(QString::fromStdString(DateTime)));
		m_SymbolSubscribedTableModel->setItem(i, 9, new QStandardItem("CTP"));


	}
}

void MainWindow::SendOrder_clicked()
{
	OrderReq req;

	req.exchange = ui.comboBox->currentText().toStdString();
	req.symbol = ui.lineEdit->text().toStdString();

	int direction = ui.comboBox_2->currentIndex();// currentText().toStdString();
	if (direction == 0)
		req.direction = DIRECTION_LONG;
	else if (direction ==1 )
		req.direction = DIRECTION_SHORT;
	else
		req.direction = DIRECTION_UNKNOWN;

	int offset = ui.comboBox_3->currentIndex();// Text().toStdString();
	if (offset == 0)
		req.offset = OFFSET_OPEN;
	else if (offset == 1)
		req.offset = OFFSET_CLOSE;
	else if (offset == 2)
		req.offset = OFFSET_CLOSETODAY;
	else if (offset == 3)
		req.offset = OFFSET_CLOSEYESTERDAY;
	else
		req.offset = OFFSET_NONE;

	int  pricetype = ui.comboBox_5->currentIndex();
	if (pricetype == 0)
		req.priceType = PRICETYPE_LIMITPRICE;
	else if (pricetype == 1)
		req.priceType = PRICETYPE_MARKETPRICE;
	else if (pricetype == 2)
		req.priceType = PRICETYPE_STOP;
	else if (pricetype == 3)
		req.priceType = PRICETYPE_FAK;
	else if (pricetype == 4)
		req.priceType = PRICETYPE_FOK;
	else
		req.priceType = PRICETYPE_REQUEST;

	req.price = ui.lineEdit_2->text().toDouble();
	req.volume = ui.lineEdit_3->text().toDouble();
	std::string gatewayname = ui.comboBox_6->currentText().toStdString();

	std::string orderRef;
	orderRef = m_gatewaymanager->sendOrder(req, gatewayname);
	
	this->write_log("��������,���Ϊ:" + orderRef, "MainWindow");
}
void MainWindow::UpdateOrderTable(UpdateOrderTableData data)
{
	//ordersubmitheader << str2qstr_new("ί�к�") << str2qstr_new("��Դ") << str2qstr_new("��Լ����") << str2qstr_new("������")<< str2qstr_new("����") << str2qstr_new("��ƽ") << str2qstr_new("�۸�") << str2qstr_new("������") << str2qstr_new("�ѳɽ�") << str2qstr_new("״̬") << str2qstr_new("����ʱ��") << str2qstr_new("����ʱ��")<< str2qstr_new("�ӿ�");
		//�������ݣ�����Ѿ����ھ͸��£����û�оͲ���
	for (int i = 0; i < m_OrderSubmitTableModel->rowCount(); i++)
	{
		if (m_OrderSubmitTableModel->item(i, 0)->text().toStdString() == data.symbol)//�ж��Ƿ���һ����Լ���Ǿ͸�������
		{
			m_OrderSubmitTableModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.orderID)));
			m_OrderSubmitTableModel->setItem(i, 1, new QStandardItem(QString::number(data.frontID)));
			m_OrderSubmitTableModel->setItem(i, 2, new QStandardItem(str2qstr_new(data.symbol)));
			m_OrderSubmitTableModel->setItem(i, 3, new QStandardItem(str2qstr_new(data.exchange)));
			m_OrderSubmitTableModel->setItem(i, 4, new QStandardItem(str2qstr_new(data.direction)));
			m_OrderSubmitTableModel->setItem(i, 5, new QStandardItem(str2qstr_new(data.offset)));
			m_OrderSubmitTableModel->setItem(i, 6, new QStandardItem(QString::number(data.price)));
			m_OrderSubmitTableModel->setItem(i, 7, new QStandardItem(QString::number(data.totalVolume)));
			m_OrderSubmitTableModel->setItem(i, 8, new QStandardItem(QString::number(data.tradedVolume)));
			m_OrderSubmitTableModel->setItem(i, 9, new QStandardItem(str2qstr_new(data.status)));
			m_OrderSubmitTableModel->setItem(i, 10, new QStandardItem(str2qstr_new(data.orderTime)));
			m_OrderSubmitTableModel->setItem(i, 11, new QStandardItem(str2qstr_new(data.cancelTime)));
			m_OrderSubmitTableModel->setItem(i, 12, new QStandardItem(str2qstr_new(data.gatewayname)));
			return;
		}
	}

	//û���ҵ��Ͳ���һ��
	int i = m_OrderSubmitTableModel->rowCount();
	m_OrderSubmitTableModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.orderID)));
	m_OrderSubmitTableModel->setItem(i, 1, new QStandardItem(QString::number(data.frontID)));
	m_OrderSubmitTableModel->setItem(i, 2, new QStandardItem(str2qstr_new(data.symbol)));
	m_OrderSubmitTableModel->setItem(i, 3, new QStandardItem(str2qstr_new(data.exchange)));
	m_OrderSubmitTableModel->setItem(i, 4, new QStandardItem(str2qstr_new(data.direction)));
	m_OrderSubmitTableModel->setItem(i, 5, new QStandardItem(str2qstr_new(data.offset)));
	m_OrderSubmitTableModel->setItem(i, 6, new QStandardItem(QString::number(data.price)));
	m_OrderSubmitTableModel->setItem(i, 7, new QStandardItem(QString::number(data.totalVolume)));
	m_OrderSubmitTableModel->setItem(i, 8, new QStandardItem(QString::number(data.tradedVolume)));
	m_OrderSubmitTableModel->setItem(i, 9, new QStandardItem(str2qstr_new(data.status)));
	m_OrderSubmitTableModel->setItem(i, 10, new QStandardItem(str2qstr_new(data.orderTime)));
	m_OrderSubmitTableModel->setItem(i, 11, new QStandardItem(str2qstr_new(data.cancelTime)));
	m_OrderSubmitTableModel->setItem(i, 12, new QStandardItem(str2qstr_new(data.gatewayname)));
}


void MainWindow::UpdateTradeTable(UpdateTradeTableData data)
{
	//ordersubmitheader << str2qstr_new("�ɽ���") << str2qstr_new("ί�к�") << str2qstr_new("��Լ����") << str2qstr_new("������")<< str2qstr_new("����") << str2qstr_new("��ƽ") << str2qstr_new("�۸�") << str2qstr_new("����") << str2qstr_new("ʱ��")<< str2qstr_new("�ӿ�");
	//�������ݣ�����Ѿ����ھ͸��£����û�оͲ���
	for (int i = 0; i < m_TradeSubmitTableModel->rowCount(); i++)
	{
		if (m_TradeSubmitTableModel->item(i, 0)->text().toStdString() == data.symbol)//�ж��Ƿ���һ����Լ���Ǿ͸�������
		{
			m_TradeSubmitTableModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.tradeID)));
			m_TradeSubmitTableModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.orderID)));
			m_TradeSubmitTableModel->setItem(i, 2, new QStandardItem(str2qstr_new(data.symbol)));
			m_TradeSubmitTableModel->setItem(i, 3, new QStandardItem(str2qstr_new(data.exchange)));
			m_TradeSubmitTableModel->setItem(i, 4, new QStandardItem(str2qstr_new(data.direction)));
			m_TradeSubmitTableModel->setItem(i, 5, new QStandardItem(str2qstr_new(data.offset)));
			m_TradeSubmitTableModel->setItem(i, 6, new QStandardItem(QString::number(data.price)));
			m_TradeSubmitTableModel->setItem(i, 7, new QStandardItem(QString::number(data.volume)));
			m_TradeSubmitTableModel->setItem(i, 8, new QStandardItem(str2qstr_new(data.tradeTime)));
			m_TradeSubmitTableModel->setItem(i, 9, new QStandardItem(str2qstr_new(data.gatewayname)));
			return;
		}
	}
	//û���ҵ��Ͳ���һ��
	int i = m_TradeSubmitTableModel->rowCount();
	m_TradeSubmitTableModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.tradeID)));
	m_TradeSubmitTableModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.orderID)));
	m_TradeSubmitTableModel->setItem(i, 2, new QStandardItem(str2qstr_new(data.symbol)));
	m_TradeSubmitTableModel->setItem(i, 3, new QStandardItem(str2qstr_new(data.exchange)));
	m_TradeSubmitTableModel->setItem(i, 4, new QStandardItem(str2qstr_new(data.direction)));
	m_TradeSubmitTableModel->setItem(i, 5, new QStandardItem(str2qstr_new(data.offset)));
	m_TradeSubmitTableModel->setItem(i, 6, new QStandardItem(QString::number(data.price)));
	m_TradeSubmitTableModel->setItem(i, 7, new QStandardItem(QString::number(data.volume)));
	m_TradeSubmitTableModel->setItem(i, 8, new QStandardItem(str2qstr_new(data.tradeTime)));
	m_TradeSubmitTableModel->setItem(i, 9, new QStandardItem(str2qstr_new(data.gatewayname)));

}

// 
// //���¼۸���ʾ�Լ���Լ���ı�
void MainWindow::UpdateTickTable(UpdatePriceTableData data)
{
	//data.exchange = "DCN";// eTick->exchange;
	//data.gatewayname = "CTP";// eTick->gatewayname;
	/*data.askprice1 = 1;
	data.bidprice1 =1;
	data.date ="2010-1-1";
	data.lastprice = 1;
	data.lowerLimit =1;
	data.openInterest =1;
	data.symbol ="l2109";
	data.time = "21:21:21";
//	data.upperLimit =1;
	data.exchange ="DCN";
	data.gatewayname = "CTP";*/
	std::string strSymbol = ui.lineEdit->text().toStdString();
	if (data.symbol == strSymbol)
	{
		ui.label_7->setText(QString::fromStdString((std::to_string(data.lastprice))));//�������¼�
		ui.label_2->setText(QString::fromStdString((std::to_string(data.bidprice1))));//������һ��
		ui.label_3->setText(QString::fromStdString((std::to_string(data.askprice1))));//������һ��
		ui.label_5->setText(QString::fromStdString((std::to_string(data.upperLimit))));//������ͣ��
		ui.label_9->setText(QString::fromStdString((std::to_string(data.lowerLimit))));//���õ�ͣ��
		if (ui.checkBox->isChecked() == true) //����۸�ĸ�ѡ��ѡ��ҲҪ��������۸�
		{
			ui.lineEdit_2->setText(QString::fromStdString((std::to_string(data.lastprice))));
		}
	}
	UpdateSymbolBox(data);

}
//�������鶩�ı�
void MainWindow::UpdateSymbolBox(UpdatePriceTableData data)
{

		//	���б� << str2qstr_new("��Լ����") << str2qstr_new("������") << str2qstr_new("���¼�") << str2qstr_new("�ֲ���") << str2qstr_new("��ͣ") << str2qstr_new("��ͣ") << str2qstr_new("��һ��") << str2qstr_new("��һ��")<< str2qstr_new("ʱ��") << str2qstr_new("�ӿ�");


	//�������ݣ�����Ѿ����ھ͸��£����û�оͲ���
	for (int i = 0; i < m_SymbolSubscribedTableModel->rowCount(); i++)
	{
		if (m_SymbolSubscribedTableModel->item(i, 0)->text().toStdString() == data.symbol)//�ж��Ƿ���һ����Լ���Ǿ͸�������
		{
			m_SymbolSubscribedTableModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.symbol)));
			m_SymbolSubscribedTableModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.exchange)));
			m_SymbolSubscribedTableModel->setItem(i, 2, new QStandardItem(QString::fromStdString((std::to_string(data.lastprice)))));
			m_SymbolSubscribedTableModel->setItem(i, 3, new QStandardItem(QString::fromStdString((std::to_string(data.openInterest)))));
			m_SymbolSubscribedTableModel->setItem(i, 4, new QStandardItem(QString::fromStdString((std::to_string(data.upperLimit)))));
			m_SymbolSubscribedTableModel->setItem(i, 5, new QStandardItem(QString::fromStdString((std::to_string(data.lowerLimit)))));
			m_SymbolSubscribedTableModel->setItem(i, 6, new QStandardItem(QString::fromStdString((std::to_string(data.askprice1)))));
			m_SymbolSubscribedTableModel->setItem(i, 7, new QStandardItem(QString::fromStdString((std::to_string(data.bidprice1)))));
			std::string DateTime = data.date + " " + data.time;
			m_SymbolSubscribedTableModel->setItem(i, 8, new QStandardItem(QString::fromStdString(DateTime)));
			m_SymbolSubscribedTableModel->setItem(i, 9, new QStandardItem(QString::fromStdString(data.gatewayname)));
			return;
		}
	}


	//û���ҵ��Ͳ���һ��
	int i = m_SymbolSubscribedTableModel->rowCount();
	m_SymbolSubscribedTableModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.symbol)));
	m_SymbolSubscribedTableModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.exchange)));
	m_SymbolSubscribedTableModel->setItem(i, 2, new QStandardItem(QString::fromStdString((std::to_string(data.lastprice)))));
	m_SymbolSubscribedTableModel->setItem(i, 3, new QStandardItem(QString::fromStdString((std::to_string(data.openInterest)))));
	m_SymbolSubscribedTableModel->setItem(i, 4, new QStandardItem(QString::fromStdString((std::to_string(data.upperLimit)))));
	m_SymbolSubscribedTableModel->setItem(i, 5, new QStandardItem(QString::fromStdString((std::to_string(data.lowerLimit)))));
	m_SymbolSubscribedTableModel->setItem(i, 6, new QStandardItem(QString::fromStdString((std::to_string(data.askprice1)))));
	m_SymbolSubscribedTableModel->setItem(i, 7, new QStandardItem(QString::fromStdString((std::to_string(data.bidprice1)))));
	std::string DateTime = data.date + " " + data.time;
	m_SymbolSubscribedTableModel->setItem(i, 8, new QStandardItem(QString::fromStdString(DateTime)));
	m_SymbolSubscribedTableModel->setItem(i, 9, new QStandardItem(QString::fromStdString(data.gatewayname)));

	

}

//�����˻��ʽ��
void MainWindow::UpdateAccountBox(AccountData data)
{

//	accountheader << str2qstr_new("�ӿ���") << str2qstr_new("�˻�ID") << str2qstr_new("���") << str2qstr_new("��ֵ") << str2qstr_new("����") << str2qstr_new("������") << str2qstr_new("��֤��") << str2qstr_new("ƽ��ӯ��") << str2qstr_new("�ֲ�ӯ��");

	//�������ݣ�����Ѿ����ھ͸��£����û�оͲ���
	for (int i = 0; i < m_AccountModel->rowCount(); i++)
	{
		if (m_AccountModel->item(i, 0)->text().toStdString() == data.gatewayname)//�жϱ��Ľӿں�����ӿ��Ƿ�һ��
		{
			m_AccountModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
			m_AccountModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.accountid)));
			m_AccountModel->setItem(i, 2, new QStandardItem(QString("%1").arg(data.preBalance, 0, 'f', 3)));
			m_AccountModel->setItem(i, 3, new QStandardItem(QString("%1").arg(data.balance, 0, 'f', 3)));
			m_AccountModel->setItem(i, 4, new QStandardItem(QString("%1").arg(data.available, 0, 'f', 3)));
			m_AccountModel->setItem(i, 5, new QStandardItem(QString("%1").arg(data.commission, 0, 'f', 3)));
			m_AccountModel->setItem(i, 6, new QStandardItem(QString("%1").arg(data.margin, 0, 'f', 3)));
			m_AccountModel->setItem(i, 7, new QStandardItem(QString("%1").arg(data.closeProfit, 0, 'f', 3)));
			m_AccountModel->setItem(i, 8, new QStandardItem(QString("%1").arg(data.positionProfit, 0, 'f', 3)));
			return;
		}
	}

	//���һ���˻�û�ҵ��Ͳ���һ��
	int i = m_AccountModel->rowCount();
	m_AccountModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
	m_AccountModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.accountid)));
	m_AccountModel->setItem(i, 2, new QStandardItem(QString("%1").arg(data.preBalance, 0, 'f', 3)));
	m_AccountModel->setItem(i, 3, new QStandardItem(QString("%1").arg(data.balance, 0, 'f', 3)));
	m_AccountModel->setItem(i, 4, new QStandardItem(QString("%1").arg(data.available, 0, 'f', 3)));
	m_AccountModel->setItem(i, 5, new QStandardItem(QString("%1").arg(data.commission, 0, 'f', 3)));
	m_AccountModel->setItem(i, 6, new QStandardItem(QString("%1").arg(data.margin, 0, 'f', 3)));
	m_AccountModel->setItem(i, 7, new QStandardItem(QString("%1").arg(data.closeProfit, 0, 'f', 3)));
	m_AccountModel->setItem(i, 8, new QStandardItem(QString("%1").arg(data.positionProfit, 0, 'f', 3)));

	

}

//���³ֱֲ�
void MainWindow::UpdatePositionBox(PositionData data)
{
	std::string direction = "";
	if (data.direction == DIRECTION_SHORT)
	{
		direction = "��";
	}
	else if (data.direction == DIRECTION_LONG)
	{
		direction = "��";
	}
	for (int i = 0; i < m_PositionModel->rowCount(); i++)
	{
		if (m_PositionModel->item(i, 1)->text().toLocal8Bit().toStdString() == data.symbol && m_PositionModel->item(i, 2)->text().toLocal8Bit().toStdString() == direction)//�жϱ��Ľӿں�����ӿ��Ƿ�һ��
		{
		//	positionheader << str2qstr_new("�ӿ���") << str2qstr_new("��Լ") << str2qstr_new("����") << str2qstr_new("��λ") << str2qstr_new("���") << str2qstr_new("�����ʽ�") << str2qstr_new("�ֲּ�");

			m_PositionModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
			m_PositionModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.symbol)));
			m_PositionModel->setItem(i, 2, new QStandardItem(str2qstr_new(direction)));
			m_PositionModel->setItem(i, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
			m_PositionModel->setItem(i, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
			m_PositionModel->setItem(i, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
			m_PositionModel->setItem(i, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));
			return;
		}

	}
	//�Ҳ����Ͳ���һ��
	int i = m_PositionModel->rowCount();
	m_PositionModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
	m_PositionModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.symbol)));
	m_PositionModel->setItem(i, 2, new QStandardItem(str2qstr_new(direction)));
	m_PositionModel->setItem(i, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
	m_PositionModel->setItem(i, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
	m_PositionModel->setItem(i, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
	m_PositionModel->setItem(i, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));

	
}

//������־����
void MainWindow::UpdateLogTable(LogData data)
{
	//��ȡ��EVENT_LOG�¼�����MainWindowͳһ�ַ���־����ͬ�Ĵ��ڣ���������־����mainwindow,ctastrategymanager,backtestermanager
	if ( data.gatewayname.find("CtaEngine") != data.gatewayname.npos)
	{
		if (m_ctaStrategyDailog != NULL)
			m_ctaStrategyDailog->UpdateLogTable(data);

	}
	else if (data.gatewayname.find("BacktesterEngine") != data.gatewayname.npos)
	{
		if (m_ctaBacktesterManager != NULL)
			m_ctaBacktesterManager->UpdateLogTable(data);

	}
	else//���͵�MainWindow
	{
		int rowCount = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(rowCount);
		ui.tableWidget->setItem(rowCount, 0, new QTableWidgetItem(str2qstr_new(data.logTime)));
		ui.tableWidget->setItem(rowCount, 1, new QTableWidgetItem(str2qstr_new(data.msg)));
		ui.tableWidget->setItem(rowCount, 2, new QTableWidgetItem(str2qstr_new(data.gatewayname)));


	}

}





//mainwindow��ļ�¼��־
void MainWindow::write_log(std::string msg, std::string gateway_name)
{
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->gatewayname = gateway_name;
	e->msg = msg;

	m_eventengine->Put(e);
}


void MainWindow::menu_contractQueryclicked()
{
	if (m_ContractQueryManager == nullptr)
		m_ContractQueryManager = new ContractQueryManager(this);
	m_ContractQueryManager->show();


}
void MainWindow::menu_CTAStrategy()
{
	if(m_ctaStrategyDailog== nullptr)
		 m_ctaStrategyDailog = new CTAStrategyManager(this);
	m_ctaStrategyDailog->show();
}
void MainWindow::menu_CTABacktest()
{
	if (m_ctaBacktesterManager == NULL)
		 m_ctaBacktesterManager = new BacktesterManager(this);

	m_ctaBacktesterManager->show();

}