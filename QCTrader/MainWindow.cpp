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

//MONGOC 线程池
mongoc_uri_t* g_uri;
mongoc_client_pool_t* g_pool;
//初始化MONGODB

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	mongoc_init();													//1
	g_uri = mongoc_uri_new("mongodb://localhost:27017/");			//2
	// 创建客户端池
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
	//delete m_riskmanager;
	delete m_ctaEngine;
	delete m_ctaBacktesterManager;
	delete m_gatewaymanager;
	delete m_eventengine;

	mongoc_client_pool_destroy(g_pool);
	mongoc_uri_destroy(g_uri);
	mongoc_cleanup();
}

//对UI生成的界面做一些设置
void  MainWindow::setUI()
{

	setWindowState(Qt::WindowMaximized);//设置窗口最大化
	setWindowTitle("QCTrade");

	/*
	QPalette pal(this->palette());
	pal.setColor(QPalette::Background, Qt::black); //设置背景黑色
	this->setAutoFillBackground(true);
	this->setPalette(pal);
	*/
	//设置资金表
	m_AccountModel = new QStandardItemModel;
	QStringList accountheader;
	accountheader << str2qstr_new("接口名") << str2qstr_new("账户ID") << str2qstr_new("昨结") << str2qstr_new("净值") << str2qstr_new("可用") << str2qstr_new("手续费") << str2qstr_new("保证金") << str2qstr_new("平仓盈亏") << str2qstr_new("持仓盈亏");
	m_AccountModel->setHorizontalHeaderLabels(accountheader);
	ui.tableView_2->setModel(m_AccountModel);
	ui.tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_2->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	ui.tableView_2->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	ui.tableView_2->setAlternatingRowColors(true);

	//设置持仓表
	m_PositionModel = new QStandardItemModel;
	QStringList positionheader;
	positionheader << str2qstr_new("接口名") << str2qstr_new("合约") << str2qstr_new("方向") << str2qstr_new("仓位") << str2qstr_new("昨仓") << str2qstr_new("冻结资金") << str2qstr_new("持仓价");
	m_PositionModel->setHorizontalHeaderLabels(positionheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_3->setModel(m_PositionModel);
	ui.tableView_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_3->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_3->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	ui.tableView_3->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	ui.tableView_3->setAlternatingRowColors(true);

	//设置行情订阅表
	m_SymbolSubscribedTableModel = new QStandardItemModel;
	QStringList symbolheader;
	symbolheader << str2qstr_new("合约代码") << str2qstr_new("交易所") << str2qstr_new("最新价") << str2qstr_new("持仓量") << str2qstr_new("涨停") << str2qstr_new("跌停") << str2qstr_new("买一价") << str2qstr_new("卖一价")<< str2qstr_new("时间") << str2qstr_new("接口");
	m_SymbolSubscribedTableModel->setHorizontalHeaderLabels(symbolheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_4->setModel(m_SymbolSubscribedTableModel);
	ui.tableView_4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_4->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_4->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	ui.tableView_4->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	ui.tableView_4->setAlternatingRowColors(true);

	//设置委托订单表
	m_OrderSubmitTableModel = new QStandardItemModel;
	QStringList ordersubmitheader;
	ordersubmitheader << str2qstr_new("委托号") << str2qstr_new("来源") << str2qstr_new("合约代码") << str2qstr_new("交易所") << str2qstr_new("方向") << str2qstr_new("开平") << str2qstr_new("价格") << str2qstr_new("总数量") << str2qstr_new("已成交") << str2qstr_new("状态")<< str2qstr_new("时间")<< str2qstr_new("接口");
	m_OrderSubmitTableModel->setHorizontalHeaderLabels(ordersubmitheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_5->setModel(m_OrderSubmitTableModel);
	ui.tableView_5->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_5->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_5->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	ui.tableView_5->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	ui.tableView_5->setAlternatingRowColors(true);

	//设置委托订单表
	m_TradeSubmitTableModel = new QStandardItemModel;
	QStringList Tradesubmitheader;
	Tradesubmitheader << str2qstr_new("成交号") << str2qstr_new("委托号") << str2qstr_new("合约代码") << str2qstr_new("交易所")<< str2qstr_new("方向") << str2qstr_new("开平") << str2qstr_new("价格") << str2qstr_new("数量") << str2qstr_new("时间")<< str2qstr_new("接口");
	m_TradeSubmitTableModel->setHorizontalHeaderLabels(Tradesubmitheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_6->setModel(m_TradeSubmitTableModel);
	ui.tableView_6->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_6->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_6->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	ui.tableView_6->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	ui.tableView_6->setAlternatingRowColors(true);


}


void MainWindow::LoadEngine()
{
	m_eventengine = new EventEngine;//事件驱动引擎

	m_gatewaymanager = new Gatewaymanager(m_eventengine);//接口管理器

	//m_riskmanager = new riskmanager(m_eventengine); //风险管理器

	m_ctaEngine = new CtaEngine(m_gatewaymanager, m_eventengine, m_riskmanager);//cta管理器

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
	//信号和槽函数
	//qRegisterMetaType<LoadStrategyData>("LoadStrategyData");//注册到元系统中
	//qRegisterMetaType<UpdateStrategyData>("UpdateStrategyData");//注册到元系统中
	//qRegisterMetaType<PortfolioData>("PortfolioData");//注册到元系统中

	qRegisterMetaType<PositionData>("PositionData");//注册到元系统中
	qRegisterMetaType<AccountData>("AccountData");//注册到元系统中
	qRegisterMetaType<std::string>("std::string");//注册到元系统中
	qRegisterMetaType<LogData>("LogData");//注册到元系统中 UpdatePriceTableData
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


////////////////////////////////////////////////////事件处理函数部分 On_开头////////////
// 
//EVENT_LOG事件的处理函数，发送UpdateLog信号给窗口
void MainWindow::OnLogUpdate(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Log> elog = std::static_pointer_cast<Event_Log>(e);
	//std::string msg = "接口名:" + elog->gatewayname + "时间:" + elog->logTime + "信息:" + elog->msg;
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


//////////////////////////////////////槽函数部分 //////////////////////////////
// 
// 
// 点击菜单CTP连接 的槽函数 弹出对话框
void MainWindow::menu_ctp_connect()
{
	CTPConnectWidgets* e = new CTPConnectWidgets(this);
	e->show();

}
//点击菜单退出后的槽函数
void MainWindow::menu_exit()
{
	this->close();
}


// //输入合约代码回车后的槽函数
void MainWindow::symbol_ReturnPressed()
{

	SubscribeReq req;
	req.symbol = ui.lineEdit->text().toStdString();
	req.exchange = ui.comboBox->currentText().toStdString();
	if (req.symbol.length() > 0)
		m_gatewaymanager->subscribe(req, "CTP");

	bool bAdded = false;//还没有加入行情价格表
	for (int i = 0; i < m_SymbolSubscribedTableModel->rowCount(); i++)
	{
		if (m_SymbolSubscribedTableModel->item(i, 0)->text().toStdString() != req.symbol)//判断是否是一个合约，是就更新数据
			bAdded = true;
	}
	if (bAdded==false)//还没加入，就加入
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
	
	this->write_log("订单发送,编号为:" + orderRef, "MainWindow");
}
void MainWindow::UpdateOrderTable(UpdateOrderTableData data)
{
	//ordersubmitheader << str2qstr_new("委托号") << str2qstr_new("来源") << str2qstr_new("合约代码") << str2qstr_new("交易所")<< str2qstr_new("方向") << str2qstr_new("开平") << str2qstr_new("价格") << str2qstr_new("总数量") << str2qstr_new("已成交") << str2qstr_new("状态") << str2qstr_new("发单时间") << str2qstr_new("撤单时间")<< str2qstr_new("接口");
		//更新数据，如果已经存在就更新，如果没有就插入
	for (int i = 0; i < m_OrderSubmitTableModel->rowCount(); i++)
	{
		if (m_OrderSubmitTableModel->item(i, 0)->text().toStdString() == data.symbol)//判断是否是一个合约，是就更新数据
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

	//没有找到就插入一个
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
	//ordersubmitheader << str2qstr_new("成交号") << str2qstr_new("委托号") << str2qstr_new("合约代码") << str2qstr_new("交易所")<< str2qstr_new("方向") << str2qstr_new("开平") << str2qstr_new("价格") << str2qstr_new("数量") << str2qstr_new("时间")<< str2qstr_new("接口");
	//更新数据，如果已经存在就更新，如果没有就插入
	for (int i = 0; i < m_TradeSubmitTableModel->rowCount(); i++)
	{
		if (m_TradeSubmitTableModel->item(i, 0)->text().toStdString() == data.symbol)//判断是否是一个合约，是就更新数据
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
	//没有找到就插入一个
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
// //更新价格显示以及合约订阅表
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
		ui.label_7->setText(QString::fromStdString((std::to_string(data.lastprice))));//设置最新价
		ui.label_2->setText(QString::fromStdString((std::to_string(data.bidprice1))));//设置卖一价
		ui.label_3->setText(QString::fromStdString((std::to_string(data.askprice1))));//设置卖一价
		ui.label_5->setText(QString::fromStdString((std::to_string(data.upperLimit))));//设置涨停价
		ui.label_9->setText(QString::fromStdString((std::to_string(data.lowerLimit))));//设置跌停价
		if (ui.checkBox->isChecked() == true) //如果价格的复选框勾选，也要更新这个价格
		{
			ui.lineEdit_2->setText(QString::fromStdString((std::to_string(data.lastprice))));
		}
	}
	UpdateSymbolBox(data);

}
//更新行情订阅表
void MainWindow::UpdateSymbolBox(UpdatePriceTableData data)
{

		//	表列表 << str2qstr_new("合约代码") << str2qstr_new("交易所") << str2qstr_new("最新价") << str2qstr_new("持仓量") << str2qstr_new("涨停") << str2qstr_new("跌停") << str2qstr_new("买一价") << str2qstr_new("卖一价")<< str2qstr_new("时间") << str2qstr_new("接口");


	//更新数据，如果已经存在就更新，如果没有就插入
	for (int i = 0; i < m_SymbolSubscribedTableModel->rowCount(); i++)
	{
		if (m_SymbolSubscribedTableModel->item(i, 0)->text().toStdString() == data.symbol)//判断是否是一个合约，是就更新数据
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


	//没有找到就插入一个
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

//更新账户资金表
void MainWindow::UpdateAccountBox(AccountData data)
{

//	accountheader << str2qstr_new("接口名") << str2qstr_new("账户ID") << str2qstr_new("昨结") << str2qstr_new("净值") << str2qstr_new("可用") << str2qstr_new("手续费") << str2qstr_new("保证金") << str2qstr_new("平仓盈亏") << str2qstr_new("持仓盈亏");

	//更新数据，如果已经存在就更新，如果没有就插入
	for (int i = 0; i < m_AccountModel->rowCount(); i++)
	{
		if (m_AccountModel->item(i, 0)->text().toStdString() == data.gatewayname)//判断表格的接口和这个接口是否一样
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

	//最后一个了还没找到就插入一个
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

//更新持仓表
void MainWindow::UpdatePositionBox(PositionData data)
{
	std::string direction = "";
	if (data.direction == DIRECTION_SHORT)
	{
		direction = "空";
	}
	else if (data.direction == DIRECTION_LONG)
	{
		direction = "多";
	}
	for (int i = 0; i < m_PositionModel->rowCount(); i++)
	{
		if (m_PositionModel->item(i, 1)->text().toLocal8Bit().toStdString() == data.symbol && m_PositionModel->item(i, 2)->text().toLocal8Bit().toStdString() == direction)//判断表格的接口和这个接口是否一样
		{
		//	positionheader << str2qstr_new("接口名") << str2qstr_new("合约") << str2qstr_new("方向") << str2qstr_new("仓位") << str2qstr_new("昨仓") << str2qstr_new("冻结资金") << str2qstr_new("持仓价");

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
	//找不到就插入一行
	int i = m_PositionModel->rowCount();
	m_PositionModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
	m_PositionModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.symbol)));
	m_PositionModel->setItem(i, 2, new QStandardItem(str2qstr_new(direction)));
	m_PositionModel->setItem(i, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
	m_PositionModel->setItem(i, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
	m_PositionModel->setItem(i, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
	m_PositionModel->setItem(i, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));

	
}

//更新日志窗口
void MainWindow::UpdateLogTable(LogData data)
{
	//收取到EVENT_LOG事件后，由MainWindow统一分发日志到不同的窗口，有三个日志窗口mainwindow,ctastrategymanager,backtestermanager
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
	else//发送到MainWindow
	{
		int rowCount = ui.tableWidget->rowCount();
		ui.tableWidget->insertRow(rowCount);
		ui.tableWidget->setItem(rowCount, 0, new QTableWidgetItem(str2qstr_new(data.logTime)));
		ui.tableWidget->setItem(rowCount, 1, new QTableWidgetItem(str2qstr_new(data.msg)));
		ui.tableWidget->setItem(rowCount, 2, new QTableWidgetItem(str2qstr_new(data.gatewayname)));


	}

}





//mainwindow类的记录日志
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