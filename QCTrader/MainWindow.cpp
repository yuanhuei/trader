#include "MainWindow.h"
#include"CTPConnectWidgets.h"
#include"event_engine/eventengine.h"
#include"gateway/gatewaymanager.h"
#include"utility.h"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);//设置窗口最大化

	LoadEngine();
	ConnectSignalAndSlot();
}

MainWindow::~MainWindow()
{
}

void MainWindow::menu_exit()
{
	this->close();
}

void MainWindow::menu_ctp_connect()
{
	CTPConnectWidgets* e= new CTPConnectWidgets(this);
	e->show();

}


void MainWindow::LoadEngine()
{
	m_eventengine = new EventEngine;//事件驱动引擎

	m_gatewaymanager = new Gatewaymanager(m_eventengine);//接口管理器

	//m_riskmanager = new riskmanager(m_eventengine); //风险管理器

	//m_ctamanager = new CTAmanager(m_gatewaymanager, m_eventengine, m_riskmanager);//cta管理器

	RegEvent();
	m_eventengine->StartEngine();
}

void MainWindow::RegEvent()
{
	m_eventengine->RegEvent(EVENT_LOG, std::bind(&MainWindow::OnLogUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_ACCOUNT, std::bind(&MainWindow::onAccountUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_POSITION, std::bind(&MainWindow::onPositionUpdate, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_LOADSTRATEGY, std::bind(&MainWindow::onStrategyLoaded, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_UPDATESTRATEGY, std::bind(&MainWindow::onStrategyUpdate, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_UPDATEPORTFOLIO, std::bind(&MainWindow::onPortfolioUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TICK, std::bind(&MainWindow::onPriceTableUpdate, this, std::placeholders::_1));
}

void MainWindow::OnLogUpdate(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Log> elog = std::static_pointer_cast<Event_Log>(e);
	std::string msg = "接口名:" + elog->gatewayname + "时间:" + elog->logTime + "信息:" + elog->msg;
	emit WriteLog(str2qstr_new(msg));
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
	std::shared_ptr<Event_Tick> eTick = std::static_pointer_cast<Event_Tick>(e);
	UpdatePriceTableData data;
	data.askprice1 = eTick->askprice1;
	data.bidprice1 = eTick->bidprice1;
	data.date = eTick->date;
	data.lastprice = eTick->lastprice;
	data.lowerLimit = eTick->lowerLimit;
	data.openInterest = eTick->openInterest;
	data.symbol = eTick->symbol;
	data.time = eTick->time;
	data.upperLimit = eTick->upperLimit;
	emit UpdatePriceTableSignal(data);
}

void MainWindow::ConnectSignalAndSlot()
{
	//信号和槽函数
	//qRegisterMetaType<LoadStrategyData>("LoadStrategyData");//注册到元系统中
	//qRegisterMetaType<UpdateStrategyData>("UpdateStrategyData");//注册到元系统中
	qRegisterMetaType<PositionData>("PositionData");//注册到元系统中
	qRegisterMetaType<AccountData>("AccountData");//注册到元系统中
	//qRegisterMetaType<PortfolioData>("PortfolioData");//注册到元系统中
	qRegisterMetaType<std::string>("std::string");//注册到元系统中

	connect(this, SIGNAL(WriteLog(QString)), this, SLOT(UpdateLogTable(QString)));
	connect(this, SIGNAL(UpdatePositionSignal(PositionData)), this, SLOT(UpdatePositionBox(PositionData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdateAccountSignal(AccountData)), this, SLOT(UpdateAccountBox(AccountData)), Qt::QueuedConnection);

	//connect(this, SIGNAL(LoadStrategySignal(LoadStrategyData)), this, SLOT(CreateStrategyBox(LoadStrategyData)), Qt::QueuedConnection);
    //connect(this, SIGNAL(UpdateStrategySignal(UpdateStrategyData)), this, SLOT(UpdateStrategyBox(UpdateStrategyData)), Qt::QueuedConnection); 
	//connect(this, SIGNAL(UpdatePortfolioSignal(PortfolioData)), this, SLOT(UpdatePortfolioBox(PortfolioData)), Qt::QueuedConnection);
}


void MainWindow::UpdateAccountBox(AccountData data)
{
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
			break;
		}
		if (i == m_AccountModel->rowCount() - 1)
		{
			//最后一个了还没找到
			m_AccountModel->setItem(i + 1, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
			m_AccountModel->setItem(i + 1, 1, new QStandardItem(str2qstr_new(data.accountid)));
			m_AccountModel->setItem(i + 1, 2, new QStandardItem(QString("%1").arg(data.preBalance, 0, 'f', 3)));
			m_AccountModel->setItem(i + 1, 3, new QStandardItem(QString("%1").arg(data.balance, 0, 'f', 3)));
			m_AccountModel->setItem(i + 1, 4, new QStandardItem(QString("%1").arg(data.available, 0, 'f', 3)));
			m_AccountModel->setItem(i + 1, 5, new QStandardItem(QString("%1").arg(data.commission, 0, 'f', 3)));
			m_AccountModel->setItem(i + 1, 6, new QStandardItem(QString("%1").arg(data.margin, 0, 'f', 3)));
			m_AccountModel->setItem(i + 1, 7, new QStandardItem(QString("%1").arg(data.closeProfit, 0, 'f', 3)));
			m_AccountModel->setItem(i + 1, 8, new QStandardItem(QString("%1").arg(data.positionProfit, 0, 'f', 3)));
			break;
		}
	}
	if (m_AccountModel->rowCount() == 0)
	{
		m_AccountModel->setItem(0, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
		m_AccountModel->setItem(0, 1, new QStandardItem(str2qstr_new(data.accountid)));
		m_AccountModel->setItem(0, 2, new QStandardItem(QString("%1").arg(data.preBalance, 0, 'f', 3)));
		m_AccountModel->setItem(0, 3, new QStandardItem(QString("%1").arg(data.balance, 0, 'f', 3)));
		m_AccountModel->setItem(0, 4, new QStandardItem(QString("%1").arg(data.available, 0, 'f', 3)));
		m_AccountModel->setItem(0, 5, new QStandardItem(QString("%1").arg(data.commission, 0, 'f', 3)));
		m_AccountModel->setItem(0, 6, new QStandardItem(QString("%1").arg(data.margin, 0, 'f', 3)));
		m_AccountModel->setItem(0, 7, new QStandardItem(QString("%1").arg(data.closeProfit, 0, 'f', 3)));
		m_AccountModel->setItem(0, 8, new QStandardItem(QString("%1").arg(data.positionProfit, 0, 'f', 3)));
	}
}

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
			m_PositionModel->setItem(i, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
			m_PositionModel->setItem(i, 1, new QStandardItem(str2qstr_new(data.symbol)));
			m_PositionModel->setItem(i, 2, new QStandardItem(str2qstr_new(direction)));
			m_PositionModel->setItem(i, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
			m_PositionModel->setItem(i, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
			m_PositionModel->setItem(i, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
			m_PositionModel->setItem(i, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));
			break;
		}
		if (i == m_PositionModel->rowCount() - 1)
		{
			//最后一个了还没找到
			m_PositionModel->setItem(i + 1, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
			m_PositionModel->setItem(i + 1, 1, new QStandardItem(str2qstr_new(data.symbol)));
			if (data.direction == DIRECTION_SHORT)
			{
				m_PositionModel->setItem(i + 1, 2, new QStandardItem(str2qstr_new("空")));
			}
			else if (data.direction == DIRECTION_LONG)
			{
				m_PositionModel->setItem(i + 1, 2, new QStandardItem(str2qstr_new("多")));
			};
			m_PositionModel->setItem(i + 1, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
			m_PositionModel->setItem(i + 1, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
			m_PositionModel->setItem(i + 1, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
			m_PositionModel->setItem(i + 1, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));
			break;
		}
	}
	if (m_PositionModel->rowCount() == 0)
	{
		m_PositionModel->setItem(0, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
		m_PositionModel->setItem(0, 1, new QStandardItem(str2qstr_new(data.symbol)));
		if (data.direction == DIRECTION_SHORT)
		{
			m_PositionModel->setItem(0, 2, new QStandardItem(str2qstr_new("空")));
		}
		else if (data.direction == DIRECTION_LONG)
		{
			m_PositionModel->setItem(0, 2, new QStandardItem(str2qstr_new("多")));
		}
		m_PositionModel->setItem(0, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
		m_PositionModel->setItem(0, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
		m_PositionModel->setItem(0, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
		m_PositionModel->setItem(0, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));

	}
}


void MainWindow::UpdateLogTable(QString str)
{


}
