#include "MainWindow.h"
#include"CTPConnectWidgets.h"
#include"event_engine/eventengine.h"
#include"gateway/gatewaymanager.h"
#include"utility.h"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setUI();
	LoadEngine();
	
}

MainWindow::~MainWindow()
{
	//delete portfoliowidget;
	//delete riskControlWidget;
	//delete m_portfolioModel;
	delete m_AccountModel;
	delete m_PositionModel;
	delete m_SymbolSubscribedTableModel;

	m_gatewaymanager->exit();
	//delete m_riskmanager;
	//delete m_ctamanager;
	delete m_gatewaymanager;
	delete m_eventengine;
}

//��UI���ɵĽ�����һЩ����
void  MainWindow::setUI()
{

	setWindowState(Qt::WindowMaximized);//���ô������
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
	QStringList positionheader;
	positionheader << str2qstr_new("��Լ����") << str2qstr_new("������") << str2qstr_new("���¼�") << str2qstr_new("�ֲ���") << str2qstr_new("��ͣ") << str2qstr_new("��ͣ") << str2qstr_new("��һ��") << str2qstr_new("��һ��")<< str2qstr_new("ʱ��") << str2qstr_new("�ӿ�");
	m_SymbolSubscribedTableModel->setHorizontalHeaderLabels(positionheader);
	//QTableView* PositionView = new QTableView;
	ui.tableView_4->setModel(m_SymbolSubscribedTableModel);
	ui.tableView_4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableView_4->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView_4->setSelectionBehavior(QAbstractItemView::SelectRows);  //����ѡ��һ��  
	ui.tableView_4->setSelectionMode(QAbstractItemView::SingleSelection); //����ֻ��ѡ��һ�У����ܶ���ѡ��  
	ui.tableView_4->setAlternatingRowColors(true);

}


void MainWindow::LoadEngine()
{
	m_eventengine = new EventEngine;//�¼���������

	m_gatewaymanager = new Gatewaymanager(m_eventengine);//�ӿڹ�����

	//m_riskmanager = new riskmanager(m_eventengine); //���չ�����

	//m_ctamanager = new CTAmanager(m_gatewaymanager, m_eventengine, m_riskmanager);//cta������

	RegEvent();
	ConnectSignalAndSlot();
	m_eventengine->StartEngine();
}

void MainWindow::RegEvent()
{
	m_eventengine->RegEvent(EVENT_LOG,std::bind(&MainWindow::OnLogUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_ACCOUNT, std::bind(&MainWindow::onAccountUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_POSITION, std::bind(&MainWindow::onPositionUpdate, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_LOADSTRATEGY, std::bind(&MainWindow::onStrategyLoaded, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_UPDATESTRATEGY, std::bind(&MainWindow::onStrategyUpdate, this, std::placeholders::_1));
	//m_eventengine->RegEvent(EVENT_UPDATEPORTFOLIO, std::bind(&MainWindow::onPortfolioUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TICK, std::bind(&MainWindow::onPriceTableUpdate, this, std::placeholders::_1));
}

void MainWindow::ConnectSignalAndSlot()
{
	//�źźͲۺ���
	//qRegisterMetaType<LoadStrategyData>("LoadStrategyData");//ע�ᵽԪϵͳ��
	//qRegisterMetaType<UpdateStrategyData>("UpdateStrategyData");//ע�ᵽԪϵͳ��
	qRegisterMetaType<PositionData>("PositionData");//ע�ᵽԪϵͳ��
	qRegisterMetaType<AccountData>("AccountData");//ע�ᵽԪϵͳ��
	//qRegisterMetaType<PortfolioData>("PortfolioData");//ע�ᵽԪϵͳ��
	qRegisterMetaType<std::string>("std::string");//ע�ᵽԪϵͳ��

	connect(this, SIGNAL(UpdateLog(LogData)), this, SLOT(UpdateLogTable(LogData)));
	connect(this, SIGNAL(UpdatePositionSignal(PositionData)), this, SLOT(UpdatePositionBox(PositionData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdateAccountSignal(AccountData)), this, SLOT(UpdateAccountBox(AccountData)), Qt::QueuedConnection);

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
	emit UpdateLog(logdata);
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
	data.exchange = eTick->exchange;
	data.gatewayname = eTick->gatewayname;

	emit UpdatePriceTableSignal(data);
}

//////////////////////////////////////�ۺ������� //////////////////////////////
// 
//�������鶩�ı�
void MainWindow::UpdateSymbolBox(UpdatePriceTableData data)
{
	//��һ�β�������
	if (m_SymbolSubscribedTableModel->rowCount() == 0)
	{
		m_SymbolSubscribedTableModel->setItem(0, 0, new QStandardItem(str2qstr_new(data.symbol)));
		m_SymbolSubscribedTableModel->setItem(0, 1, new QStandardItem(str2qstr_new(data.exchange)));
		m_SymbolSubscribedTableModel->setItem(0, 2, new QStandardItem(QString::fromStdString((std::to_string(data.lastprice)))));
		m_SymbolSubscribedTableModel->setItem(0, 3, new QStandardItem(QString::fromStdString((std::to_string(data.openInterest)))));
		m_SymbolSubscribedTableModel->setItem(0, 4, new QStandardItem(QString::fromStdString((std::to_string(data.upperLimit)))));
		m_SymbolSubscribedTableModel->setItem(0, 5, new QStandardItem(QString::fromStdString((std::to_string(data.lowerLimit)))));
		m_SymbolSubscribedTableModel->setItem(0, 6, new QStandardItem(QString::fromStdString((std::to_string(data.askprice1)))));
		m_SymbolSubscribedTableModel->setItem(0, 7, new QStandardItem(QString::fromStdString((std::to_string(data.bidprice1)))));
		std::string DateTime = data.date + " " + data.time;
		m_SymbolSubscribedTableModel->setItem(0, 8, new QStandardItem(QString::fromStdString(DateTime)));
		m_SymbolSubscribedTableModel->setItem(0, 9, new QStandardItem(QString::fromStdString(data.gatewayname)));
	
		return;
	}
	//�������ݣ�����Ѿ����ھ͸��£����û�оͲ���
	for (int i = 0; i < m_AccountModel->rowCount(); i++)
	{
		if (m_AccountModel->item(i, 0)->text().toStdString() == data.gatewayname)//�жϱ��Ľӿں�����ӿ��Ƿ�һ��
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
			break;
		}
		if (i == m_AccountModel->rowCount() - 1)
		{
			//���һ���˻�û�ҵ�
			m_SymbolSubscribedTableModel->setItem(i+1, 0, new QStandardItem(str2qstr_new(data.symbol)));
			m_SymbolSubscribedTableModel->setItem(i + 1, 1, new QStandardItem(str2qstr_new(data.exchange)));
			m_SymbolSubscribedTableModel->setItem(i + 1, 2, new QStandardItem(QString::fromStdString((std::to_string(data.lastprice)))));
			m_SymbolSubscribedTableModel->setItem(i + 1, 3, new QStandardItem(QString::fromStdString((std::to_string(data.openInterest)))));
			m_SymbolSubscribedTableModel->setItem(i + 1, 4, new QStandardItem(QString::fromStdString((std::to_string(data.upperLimit)))));
			m_SymbolSubscribedTableModel->setItem(i + 1, 5, new QStandardItem(QString::fromStdString((std::to_string(data.lowerLimit)))));
			m_SymbolSubscribedTableModel->setItem(i + 1, 6, new QStandardItem(QString::fromStdString((std::to_string(data.askprice1)))));
			m_SymbolSubscribedTableModel->setItem(i + 1, 7, new QStandardItem(QString::fromStdString((std::to_string(data.bidprice1)))));
			std::string DateTime = data.date + " " + data.time;
			m_SymbolSubscribedTableModel->setItem(i + 1, 8, new QStandardItem(QString::fromStdString(DateTime)));
			m_SymbolSubscribedTableModel->setItem(i + 1, 9, new QStandardItem(QString::fromStdString(data.gatewayname)));
			break;
		}
	}

}

//�����˻��ʽ��
void MainWindow::UpdateAccountBox(AccountData data)
{
	//��һ�β�������
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
		return;
	}
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
			break;
		}
		if (i == m_AccountModel->rowCount() - 1)
		{
			//���һ���˻�û�ҵ�
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
			//���һ���˻�û�ҵ�
			m_PositionModel->setItem(i + 1, 0, new QStandardItem(str2qstr_new(data.gatewayname)));
			m_PositionModel->setItem(i + 1, 1, new QStandardItem(str2qstr_new(data.symbol)));
			if (data.direction == DIRECTION_SHORT)
			{
				m_PositionModel->setItem(i + 1, 2, new QStandardItem(str2qstr_new("��")));
			}
			else if (data.direction == DIRECTION_LONG)
			{
				m_PositionModel->setItem(i + 1, 2, new QStandardItem(str2qstr_new("��")));
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
			m_PositionModel->setItem(0, 2, new QStandardItem(str2qstr_new("��")));
		}
		else if (data.direction == DIRECTION_LONG)
		{
			m_PositionModel->setItem(0, 2, new QStandardItem(str2qstr_new("��")));
		}
		m_PositionModel->setItem(0, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
		m_PositionModel->setItem(0, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
		m_PositionModel->setItem(0, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
		m_PositionModel->setItem(0, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));

	}
}

//������־����
void MainWindow::UpdateLogTable(LogData data)
{
	int rowCount = ui.tableWidget->rowCount();
	ui.tableWidget->insertRow(rowCount);
	ui.tableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::fromStdString(data.logTime)));
	ui.tableWidget->setItem(rowCount, 1, new QTableWidgetItem(QString::fromStdString(data.msg)));
	ui.tableWidget->setItem(rowCount, 3, new QTableWidgetItem(QString::fromStdString(data.gatewayname)));
}

//���¼۸���ʾ�Լ���Լ���ı�
void MainWindow::UpdateTickTable(UpdatePriceTableData data)
{
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




//�����Լ����س���Ĳۺ���
void MainWindow::symbol_ReturnPressed()
{

	SubscribeReq req;
	req.symbol = ui.lineEdit->text().toStdString();
	if(req.symbol.length()>0)
		m_gatewaymanager->subscribe(req,"CTP");


}

//����˵��˳���Ĳۺ���
void MainWindow::menu_exit()
{
	this->close();
}

//����˵�CTP���� �Ĳۺ��� �����Ի���
void MainWindow::menu_ctp_connect()
{
	CTPConnectWidgets* e = new CTPConnectWidgets(this);
	e->show();

}

//mainwindow��ļ�¼��־
void MainWindow::write_log(std::string msg, std::string gateway_name)
{
	std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
	e->gatewayname = gateway_name;
	e->msg = msg;

	m_eventengine->Put(e);
}