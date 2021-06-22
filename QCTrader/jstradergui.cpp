#include "jstradergui.h"
//#include"vld.h"

JSTraderGUI::JSTraderGUI(QWidget *parent) : QWidget(parent)
{
	LoadUI();
	LoadCore();
	CreateRiskWindow();
}

void JSTraderGUI::CreateRiskWindow()
{
	//风险管理页
	riskControlWidget = new QWidget;
	QGridLayout *grid5 = new QGridLayout;
	grid5->addWidget(createRiskControl());
	riskControlWidget->setLayout(grid5);
}

JSTraderGUI::~JSTraderGUI()
{
	delete portfoliowidget;
	delete riskControlWidget;
	delete m_portfolioModel;
	delete m_AccountModel;
	delete m_PositionModel;

	for (std::map<std::string, ClickModel*>::iterator it = m_strategyvarmodelmap.begin(); it != m_strategyvarmodelmap.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}

	for (std::map<std::string, QStandardItemModel*>::iterator it = m_strategyparammodelmap.begin(); it != m_strategyparammodelmap.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}

	for (std::map<std::string, SpinBoxDelegate*>::iterator it = m_delegatespinbox.begin(); it != m_delegatespinbox.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}

	for (std::vector<ReadOnlyDelegate*>::iterator it = m_delegatereadonly.begin(); it != m_delegatereadonly.end(); it++)
	{
		delete *it;
		*it = nullptr;
	}

	m_gatewaymanager->exit();
	delete m_riskmanager;
	delete m_ctamanager;
	delete m_gatewaymanager;
	delete m_eventengine;
}

void JSTraderGUI::closeEvent(QCloseEvent   *  event)
{
	QMessageBox::StandardButton button;
	button = QMessageBox::question(this, str2qstr("退出程序"),
		QString(str2qstr("警告：程序有一个任务正在运行中，是否结束操作退出?")),
		QMessageBox::Yes | QMessageBox::No);

	if (button == QMessageBox::No) {
		event->ignore();  //忽略退出信号，程序继续运行
	}
	else if (button == QMessageBox::Yes) {
		event->accept();  //接受退出信号，程序退出
	}
}

QString JSTraderGUI::str2qstr(std::string str)
{
	return QString::fromLocal8Bit(str.c_str());
}

void JSTraderGUI::LoadUI()
{
	//tab控件
	QTabWidget *maintabwidget = new QTabWidget();
	QWidget *tab1widget = new QWidget();//账户信息页
	QWidget *tab2widget = new QWidget();//策略信息页
	maintabwidget->addTab(tab1widget, str2qstr("账户信息"));
	maintabwidget->addTab(tab2widget, str2qstr("策略信息"));

	//第一页
	QGridLayout *grid = new QGridLayout;
	grid->addWidget(createAccountGroup(), 0, 0);
	grid->addWidget(createPositionGroup(), 1, 0);
	grid->addWidget(createLogGroup(), 2, 0);
	tab1widget->setLayout(grid);


	//第二页
	QGridLayout *grid2 = new QGridLayout;
	QWidget* control = createControlGroup();
	control->setFixedHeight(80);
	grid2->addWidget(control, 0, 0);
	grid2->addWidget(createStrategyGroup(), 1, 0);
	grid2->setRowStretch(0, 1);
	grid2->setRowStretch(1, 5);
	tab2widget->setLayout(grid2);

	//主程序
	QGridLayout *mainlayout = new QGridLayout();
	mainlayout->addWidget(maintabwidget);
	setLayout(mainlayout);
	setWindowTitle(str2qstr("JSTraderGUI 5月22日"));
	resize(1200, 800);


	//组合管理页面
	portfoliowidget = new QWidget;
	QGridLayout *grid3 = new QGridLayout;
	grid3->addWidget(createPortfolio_1(), 0, 0);
	grid3->addWidget(createPortfolio_2(), 1, 0);
	grid3->setRowStretch(0, 1);
	grid3->setRowStretch(1, 5);
	portfoliowidget->setLayout(grid3);
	portfoliowidget->resize(800, 600);


	//菜单
	QMenuBar *menubar = new QMenuBar(this);
	QMenu *connectmenu = menubar->addMenu(str2qstr("连接接口"));
	connectmenu->addAction(str2qstr("连接CTP"), this, SLOT(OnPressConnectCTP()));
	connectmenu->addAction(str2qstr("连接IB"), this, SLOT(NULL));
	menubar->addAction(str2qstr("风控"), this, SLOT(onPressRiskControlShow()));
	menubar->addAction(str2qstr("组合管理"), this, SLOT(OnPressPortfolioShow()));
	mainlayout->setMenuBar(menubar);

	//信号和槽函数
	qRegisterMetaType<LoadStrategyData>("LoadStrategyData");//注册到元系统中
	qRegisterMetaType<UpdateStrategyData>("UpdateStrategyData");//注册到元系统中
	qRegisterMetaType<PositionData>("PositionData");//注册到元系统中
	qRegisterMetaType<AccountData>("AccountData");//注册到元系统中
	qRegisterMetaType<PortfolioData>("PortfolioData");//注册到元系统中
	qRegisterMetaType<std::string>("std::string");//注册到元系统中

	connect(this, SIGNAL(LoadStrategySignal(LoadStrategyData)), this, SLOT(CreateStrategyBox(LoadStrategyData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdateStrategySignal(UpdateStrategyData)), this, SLOT(UpdateStrategyBox(UpdateStrategyData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdatePositionSignal(PositionData)), this, SLOT(UpdatePositionBox(PositionData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdateAccountSignal(AccountData)), this, SLOT(UpdateAccountBox(AccountData)), Qt::QueuedConnection);
	connect(this, SIGNAL(UpdatePortfolioSignal(PortfolioData)), this, SLOT(UpdatePortfolioBox(PortfolioData)), Qt::QueuedConnection);
}

void JSTraderGUI::LoadCore()
{
	m_eventengine = new EventEngine;//事件驱动引擎

	m_gatewaymanager = new Gatewaymanager(m_eventengine);//接口管理器

	m_riskmanager = new riskmanager(m_eventengine); //风险管理器

	m_ctamanager = new CTAmanager(m_gatewaymanager, m_eventengine, m_riskmanager);//cta管理器

	RegEvent();
	m_eventengine->StartEngine();
}

void JSTraderGUI::RegEvent()
{
	m_eventengine->RegEvent(EVENT_LOG, std::bind(&JSTraderGUI::OnLog, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_ACCOUNT, std::bind(&JSTraderGUI::onAccount, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_POSITION, std::bind(&JSTraderGUI::onPosition, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_LOADSTRATEGY, std::bind(&JSTraderGUI::onStrategyLoaded, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_UPDATESTRATEGY, std::bind(&JSTraderGUI::onStrategyUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_UPDATEPORTFOLIO, std::bind(&JSTraderGUI::onPortfolioUpdate, this, std::placeholders::_1));
	m_eventengine->RegEvent(EVENT_TICK, std::bind(&JSTraderGUI::onPriceTableUpdate, this, std::placeholders::_1));
}

QGroupBox *JSTraderGUI::createLogGroup()
{
	QGroupBox *groupBox = new QGroupBox(str2qstr("日志"));
	QTextBrowser *Logbrowser = new QTextBrowser;
	QGridLayout *grid = new QGridLayout;
	grid->addWidget(Logbrowser);
	groupBox->setLayout(grid);
	connect(this, SIGNAL(WriteLog(QString)), Logbrowser, SLOT(append(QString)));
	return groupBox;
}
//账户
QGroupBox *JSTraderGUI::createAccountGroup()
{
	QGroupBox *groupBox = new QGroupBox(str2qstr("账户"));
	m_AccountModel = new QStandardItemModel;
	QStringList accountheader;
	accountheader << str2qstr("接口名") << str2qstr("账户ID") << str2qstr("昨结") << str2qstr("净值") << str2qstr("可用") << str2qstr("手续费") << str2qstr("保证金") << str2qstr("平仓盈亏") << str2qstr("持仓盈亏");
	m_AccountModel->setHorizontalHeaderLabels(accountheader);
	QTableView *tableView = new QTableView;
	tableView->setModel(m_AccountModel);
	tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	tableView->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	tableView->setAlternatingRowColors(true);
	QGridLayout *grid = new QGridLayout;
	grid->addWidget(tableView);
	groupBox->setLayout(grid);
	return groupBox;
}

QGroupBox *JSTraderGUI::createPositionGroup()
{
	QGroupBox *groupBox = new QGroupBox(str2qstr("持仓"));
	m_PositionModel = new QStandardItemModel;
	QStringList positionheader;
	positionheader << str2qstr("接口名") << str2qstr("合约") << str2qstr("方向") << str2qstr("仓位") << str2qstr("昨仓") << str2qstr("冻结资金") << str2qstr("持仓价");
	m_PositionModel->setHorizontalHeaderLabels(positionheader);
	QTableView *PositionView = new QTableView;
	PositionView->setModel(m_PositionModel);
	PositionView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	PositionView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	PositionView->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	PositionView->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	PositionView->setAlternatingRowColors(true);
	QGridLayout *grid = new QGridLayout;
	grid->addWidget(PositionView);
	groupBox->setLayout(grid);
	return groupBox;
}

QGroupBox *JSTraderGUI::createControlGroup()
{
	QGroupBox *groupBox = new QGroupBox(str2qstr("控制"));
	QPushButton *load = new QPushButton(str2qstr("加载策略"));
	QPushButton *init = new QPushButton(str2qstr("初始化"));
	QPushButton *start = new QPushButton(str2qstr("启动策略"));
	QPushButton *stop = new QPushButton(str2qstr("停止策略"));

	connect(load, SIGNAL(clicked()), this, SLOT(OnPressLoadStrategy()));
	connect(init, SIGNAL(clicked()), this, SLOT(OnPressInitStrategy()));
	connect(start, SIGNAL(clicked()), this, SLOT(OnPressStartStrategy()));
	connect(stop, SIGNAL(clicked()), this, SLOT(OnPressStopStrategy()));

	QHBoxLayout *Hbox = new QHBoxLayout;
	Hbox->addWidget(load);
	Hbox->addWidget(init);
	Hbox->addWidget(start);
	Hbox->addWidget(stop);
	Hbox->addStretch();
	groupBox->setLayout(Hbox);
	return groupBox;
}

QScrollArea *JSTraderGUI::createStrategyGroup()//创建策略栏
{

	QScrollArea *scrollarea = new QScrollArea();
	QWidget *w = new QWidget;
	StrategyLayout = new QGridLayout;
	w->setLayout(StrategyLayout);
	scrollarea->setWidget(w);
	scrollarea->setWidgetResizable(true);
	return scrollarea;
}

QGroupBox *JSTraderGUI::createRiskControl()
{
	QGroupBox *groupBox = new QGroupBox(str2qstr("风控"));
	if (m_riskmanager->active == true)
	{
		buttonSwitchEngineStatus = new QPushButton(str2qstr("风控模块已启动"));
	}
	else
	{
		buttonSwitchEngineStatus = new QPushButton(str2qstr("风控模块未启动"));
	}
	spinOrderFlowLimit = new QSpinBox();
	spinOrderFlowLimit->setRange(0, 1000);
	spinOrderFlowLimit->setValue(m_riskmanager->orderFlowLimit);
	spinOrderFlowClear = new QSpinBox();
	spinOrderFlowClear->setRange(0, 1000);
	spinOrderFlowClear->setValue(m_riskmanager->orderFlowClear);
	spinOrderSizeLimit = new QSpinBox();
	spinOrderSizeLimit->setRange(0, 1000);
	spinOrderSizeLimit->setValue(m_riskmanager->orderSizeLimit);
	spinTradeLimit = new QSpinBox();
	spinTradeLimit->setRange(0, 1000);
	spinTradeLimit->setValue(m_riskmanager->tradeLimit);
	spinWorkingOrderLimit = new QSpinBox();
	spinWorkingOrderLimit->setRange(0, 1000);
	spinWorkingOrderLimit->setValue(m_riskmanager->workingOrderLimit);
	spinOrderCancelLimit = new QSpinBox();
	spinOrderCancelLimit->setRange(0, 1000);
	spinOrderCancelLimit->setValue(m_riskmanager->orderCancelLimit);

	QPushButton *buttonClearOrderFlowCount = new QPushButton(str2qstr("清空流控计数"));
	QPushButton *buttonclearTradeCount = new QPushButton(str2qstr("清空总成交计数"));
	QPushButton *buttonSaveSetting = new QPushButton(str2qstr("保存设置"));


	connect(buttonSwitchEngineStatus, SIGNAL(clicked()), this, SLOT(switchEngineStatus()), Qt::QueuedConnection);
	connect(buttonClearOrderFlowCount, SIGNAL(clicked()), this, SLOT(onPressbuttonClearOrderFlowCount()), Qt::QueuedConnection);
	connect(buttonclearTradeCount, SIGNAL(clicked()), this, SLOT(onPressbuttonclearTradeCount()), Qt::QueuedConnection);
	connect(buttonSaveSetting, SIGNAL(clicked()), this, SLOT(onPressbuttonSaveSetting()), Qt::QueuedConnection);
	QFrame *line1 = new QFrame();   line1->setFrameShape(QFrame::HLine);    line1->setFrameShadow(QFrame::Sunken);
	QFrame *line2 = new QFrame();	line2->setFrameShape(QFrame::HLine);	line2->setFrameShadow(QFrame::Sunken);
	QFrame *line3 = new QFrame();	line3->setFrameShape(QFrame::HLine);	line3->setFrameShadow(QFrame::Sunken);
	QFrame *line4 = new QFrame();	line4->setFrameShape(QFrame::HLine);	line4->setFrameShadow(QFrame::Sunken);
	QFrame *line5 = new QFrame();	line5->setFrameShape(QFrame::HLine);	line5->setFrameShadow(QFrame::Sunken);

	QGridLayout *grid = new QGridLayout();
	grid->addWidget(new QLabel(str2qstr("工作状态")), 0, 0);
	grid->addWidget(buttonSwitchEngineStatus, 0, 1);
	grid->addWidget(line1, 1, 0, 1, 2);
	grid->addWidget(new QLabel(str2qstr("流控上限")), 2, 0);
	grid->addWidget(spinOrderFlowLimit, 2, 1);
	grid->addWidget(new QLabel(str2qstr("流控清空秒数间隔")), 3, 0);
	grid->addWidget(spinOrderFlowClear, 3, 1);
	grid->addWidget(line2, 4, 0, 1, 2);
	grid->addWidget(new QLabel(str2qstr("单笔委托上限")), 5, 0);
	grid->addWidget(spinOrderSizeLimit, 5, 1);
	grid->addWidget(line3, 6, 0, 1, 2);
	grid->addWidget(new QLabel(str2qstr("总成交上限")), 7, 0);
	grid->addWidget(spinTradeLimit, 7, 1);
	grid->addWidget(line4, 8, 0, 1, 2);
	grid->addWidget(new QLabel(str2qstr("活动订单上限")), 9, 0);
	grid->addWidget(spinWorkingOrderLimit, 9, 1);
	grid->addWidget(line5, 10, 0, 1, 2);
	grid->addWidget(new QLabel(str2qstr("单合约撤单上限")), 11, 0);
	grid->addWidget(spinOrderCancelLimit, 11, 1);

	QHBoxLayout *hbox = new QHBoxLayout();
	hbox->addWidget(buttonClearOrderFlowCount);
	hbox->addWidget(buttonclearTradeCount);
	hbox->addStretch();
	hbox->addWidget(buttonSaveSetting);

	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->addLayout(grid);
	vbox->addLayout(hbox);
	groupBox->setLayout(vbox);
	return groupBox;
}

QGroupBox *JSTraderGUI::createPortfolio_1()
{
	QGroupBox *groupBox = new QGroupBox(str2qstr("组合"));
	QHBoxLayout *Hbox = new QHBoxLayout;
	QLabel *label_portfolio_winning = new QLabel(str2qstr("组合盈利"));
	QLabel *label_portfolio_test = new QLabel(str2qstr("╮(╯▽╰)╭"));
	Hbox->addWidget(label_portfolio_winning);
	Hbox->addWidget(label_portfolio_test);
	groupBox->setLayout(Hbox);
	connect(this, SIGNAL(UpdatePortfolioWinning(QString)), label_portfolio_winning, SLOT(setText(QString)));
	return groupBox;
}

QGroupBox *JSTraderGUI::createPortfolio_2()
{
	QGroupBox *groupBox = new QGroupBox(str2qstr("策略"));
	QGridLayout *layout = new QGridLayout;
	QTableView *tableview = new QTableView;
	tableview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableview->setSelectionBehavior(QAbstractItemView::SelectRows);  //单击选择一行  
	tableview->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中  
	tableview->setAlternatingRowColors(true);
	m_portfolioModel = new QStandardItemModel;
	QStringList portfolioheader;
	portfolioheader << str2qstr("Dll名字") << str2qstr("策略") << str2qstr("合约") << str2qstr("盈利") << str2qstr("亏损") << str2qstr("净盈利") << str2qstr("持仓盈亏") << str2qstr("总盈亏") << str2qstr("持仓") << str2qstr("均价") << str2qstr("回撤") << str2qstr("今日盈亏");
	m_portfolioModel->setHorizontalHeaderLabels(portfolioheader);
	tableview->setModel(m_portfolioModel);
	layout->addWidget(tableview);
	groupBox->setLayout(layout);
	return groupBox;
}

void JSTraderGUI::CreateStrategyBox(LoadStrategyData data)//再GUI线程创建
{
	//接收CTAMANAGER处理的策略putgui的事件
	QGroupBox *strategybox = new QGroupBox(str2qstr(data.strategyname));
	StrategyLayout->addWidget(strategybox);

	QStandardItemModel *param_model = new QStandardItemModel;
	ClickModel *var_model = new ClickModel;

	QCheckBox *checkbox = new QCheckBox(str2qstr("选择此策略"));

	QTableView *param_tableview = new QTableView;
	param_tableview->setModel(param_model);
	param_tableview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	param_tableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	param_tableview->verticalHeader()->hide();
	Clicktableview *var_tableview = new Clicktableview;

	var_tableview->setModel(var_model);
	var_tableview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	var_tableview->verticalHeader()->hide();
	
	m_strategyvarmodelmap.insert(std::pair<std::string, ClickModel*>(data.strategyname, var_model));
	m_strategyparammodelmap.insert(std::pair<std::string, QStandardItemModel*>(data.strategyname, param_model));
	m_Strategycheckbox.insert(std::pair<QCheckBox*, std::string>(checkbox, data.strategyname));

	//参数设置
	QStringList paramhead;
	int i = 0;
	for (std::map<std::string, std::string>::iterator it = data.parammap.begin(); it != data.parammap.end(); it++)
	{
		paramhead << str2qstr(it->first);
		param_model->setItem(0, i, new QStandardItem(str2qstr(it->second)));
		i++;
	}
	param_model->setHorizontalHeaderLabels(paramhead);
	param_tableview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


	//变量设置
	QStringList varhead;
	i = 0;
	for (std::map<std::string, std::string>::iterator it = data.varmap.begin(); it != data.varmap.end(); it++)
	{
		varhead << str2qstr(it->first);
		std::string::size_type idpos = it->first.find("pos_");
		if ( idpos!= std::string::npos)
		{
			//代理 delegate
			SpinBoxDelegate *spinboxdelegate = new SpinBoxDelegate;
			var_tableview->setItemDelegateForColumn(i, spinboxdelegate);
			var_tableview->setposcolumn(i);
			std::string symbol=it->first.substr(idpos+4);
			spinboxdelegate->savestrategyname(data.strategyname, symbol);
			m_delegatespinbox.insert(std::pair<std::string, SpinBoxDelegate*>(data.strategyname, spinboxdelegate));
			connect(spinboxdelegate, SIGNAL(changepos(std::string, std::string, double)), this, SLOT(Onchanedpos(std::string, std::string, double)), Qt::QueuedConnection);
			connect(spinboxdelegate, SIGNAL(setupdatetrue()), var_tableview, SLOT(setmodelupdatetrue()));
		}
		else
		{
			//设置只读代理
			ReadOnlyDelegate *readonly = new ReadOnlyDelegate;
			var_tableview->setItemDelegateForColumn(i, readonly);
			m_delegatereadonly.push_back(readonly);
		}
		var_model->setItem(0, i, new QStandardItem(str2qstr(it->second)));

		i++;
	}
	
	var_model->setHorizontalHeaderLabels(varhead);
	var_tableview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QVBoxLayout *vboxlayout = new QVBoxLayout;
	vboxlayout->addWidget(checkbox);
	vboxlayout->addWidget(param_tableview);
	vboxlayout->addWidget(var_tableview);
	strategybox->setLayout(vboxlayout);
}

void JSTraderGUI::OnLog(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_Log> elog = std::static_pointer_cast<Event_Log>(e);
	std::string msg = "接口名:" + elog->gatewayname + "时间:" + elog->logTime + "信息:" + elog->msg;
	emit WriteLog(str2qstr(msg));
}

void JSTraderGUI::onAccount(std::shared_ptr<Event>e)
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

void JSTraderGUI::onPosition(std::shared_ptr<Event>e)
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

void JSTraderGUI::onStrategyLoaded(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_LoadStrategy> eLoadStrategy = std::static_pointer_cast<Event_LoadStrategy>(e);
	LoadStrategyData data;
	data.parammap = eLoadStrategy->parammap;
	data.varmap = eLoadStrategy->varmap;
	data.strategyname = eLoadStrategy->strategyname;
	emit LoadStrategySignal(data);
}

void JSTraderGUI::onStrategyUpdate(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_UpdateStrategy> eUpdateStrategy = std::static_pointer_cast<Event_UpdateStrategy>(e);
	UpdateStrategyData data;
	data.parammap = eUpdateStrategy->parammap;
	data.varmap = eUpdateStrategy->varmap;
	data.strategyname = eUpdateStrategy->strategyname;
	emit UpdateStrategySignal(data);
}

void JSTraderGUI::onPortfolioUpdate(std::shared_ptr<Event>e)
{
	std::shared_ptr<Event_UpdatePortfolio> eUpdatePortfolio = std::static_pointer_cast<Event_UpdatePortfolio>(e);
	PortfolioData data;
	data.Portfoliodata = eUpdatePortfolio->Portfoliodata;
	data.strategyname = eUpdatePortfolio->strategyname;
	data.dllname = Utils::split(eUpdatePortfolio->strategyname, "_")[0];
	data.symbol = eUpdatePortfolio->symbol;
	data.strategyrows = eUpdatePortfolio->strategyrows;
	emit UpdatePortfolioSignal(data);
}

void JSTraderGUI::onPriceTableUpdate(std::shared_ptr<Event>e)
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

void JSTraderGUI::OnPressConnectCTP()
{
	m_gatewaymanager->connect("CTP");
}

void JSTraderGUI::OnPressConnectIB()
{
	m_gatewaymanager->connect("IB");
}

void JSTraderGUI::OnPressLoadStrategy()
{
	m_ctamanager->loadStrategy();
}

void JSTraderGUI::OnPressInitStrategy()
{
	for (std::map<QCheckBox*, std::string>::iterator it = m_Strategycheckbox.begin(); it != m_Strategycheckbox.end(); it++)
	{
		//遍历一次整个框框 
		if (it->first->isChecked() == true)
		{
			m_ctamanager->initStrategy(it->second);
		}
	}
}

void JSTraderGUI::Onchanedpos(std::string name, std::string symbol, double pos)
{
	m_ctamanager->changesupposedpos(name, symbol, pos);
}

void JSTraderGUI::OnPressStartStrategy()
{
	for (std::map<QCheckBox*, std::string>::iterator it = m_Strategycheckbox.begin(); it != m_Strategycheckbox.end(); it++)
	{
		//遍历一次整个框框 
		if (it->first->isChecked() == true)
		{
			m_ctamanager->startStrategy(it->second);
		}
	}
}

void JSTraderGUI::OnPressStopStrategy()
{
	for (std::map<QCheckBox*, std::string>::iterator it = m_Strategycheckbox.begin(); it != m_Strategycheckbox.end(); it++)
	{
		//遍历一次整个框框 
		if (it->first->isChecked() == true)
		{
			m_ctamanager->stopStrategy(it->second);
		}
	}
}

void JSTraderGUI::OnPressPortfolioShow()
{
	portfoliowidget->show();
}

void JSTraderGUI::onPressRiskControlShow()
{
	riskControlWidget->show();
}
 
void JSTraderGUI::switchEngineStatus()
{
	if (m_riskmanager->switchEngineStatus() == true)
	{
		buttonSwitchEngineStatus->setText(str2qstr("风控模块已启动"));
	}
	else
	{
		buttonSwitchEngineStatus->setText(str2qstr("风控模块未启动"));
	}

}

void JSTraderGUI::onPressbuttonSaveSetting()
{
	m_riskmanager->setOrderFlowClearTime(spinOrderFlowClear->value());
	m_riskmanager->setOrderFlowLimit(spinOrderFlowLimit->value());
	m_riskmanager->setOrderSizeLimit(spinOrderSizeLimit->value());
	m_riskmanager->setTradeLimit(spinTradeLimit->value());
	m_riskmanager->setWorkingOrderLimit(spinWorkingOrderLimit->value());
	m_riskmanager->setOrderCancelLimit(spinOrderCancelLimit->value());
	m_riskmanager->saveSetting();
}

void JSTraderGUI::onPressbuttonClearOrderFlowCount()
{
	m_riskmanager->clearOrderFlowCount();
}
void JSTraderGUI::onPressbuttonclearTradeCount()
{
	m_riskmanager->clearTradeCount();
}

void JSTraderGUI::UpdateStrategyBox(UpdateStrategyData data)
{
	m_strategyvarparammtx.lock();
	std::string strategyname = data.strategyname;
	if (m_strategyvarmodelmap.find(strategyname) == m_strategyvarmodelmap.end())
	{
		m_strategyvarparammtx.unlock();
		return;
	}
	ClickModel *var_model = m_strategyvarmodelmap[strategyname];//这个策略对应的列表
	int i = 0;
	for (std::map<std::string, std::string>::iterator it = data.varmap.begin(); it != data.varmap.end(); it++)
	{
		if (var_model->canupdate() == true)
		{
			var_model->setItem(0, i, new QStandardItem(str2qstr(it->second)));
		}
		i++;
	}
	m_strategyvarparammtx.unlock();
}

void JSTraderGUI::UpdateAccountBox(AccountData data)
{
	for (int i = 0; i < m_AccountModel->rowCount(); i++)
	{
		if (m_AccountModel->item(i, 0)->text().toStdString() == data.gatewayname)//判断表格的接口和这个接口是否一样
		{
			m_AccountModel->setItem(i, 0, new QStandardItem(str2qstr(data.gatewayname)));
			m_AccountModel->setItem(i, 1, new QStandardItem(str2qstr(data.accountid)));
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
			m_AccountModel->setItem(i + 1, 0, new QStandardItem(str2qstr(data.gatewayname)));
			m_AccountModel->setItem(i + 1, 1, new QStandardItem(str2qstr(data.accountid)));
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
		m_AccountModel->setItem(0, 0, new QStandardItem(str2qstr(data.gatewayname)));
		m_AccountModel->setItem(0, 1, new QStandardItem(str2qstr(data.accountid)));
		m_AccountModel->setItem(0, 2, new QStandardItem(QString("%1").arg(data.preBalance, 0, 'f', 3)));
		m_AccountModel->setItem(0, 3, new QStandardItem(QString("%1").arg(data.balance, 0, 'f', 3)));
		m_AccountModel->setItem(0, 4, new QStandardItem(QString("%1").arg(data.available, 0, 'f', 3)));
		m_AccountModel->setItem(0, 5, new QStandardItem(QString("%1").arg(data.commission, 0, 'f', 3)));
		m_AccountModel->setItem(0, 6, new QStandardItem(QString("%1").arg(data.margin, 0, 'f', 3)));
		m_AccountModel->setItem(0, 7, new QStandardItem(QString("%1").arg(data.closeProfit, 0, 'f', 3)));
		m_AccountModel->setItem(0, 8, new QStandardItem(QString("%1").arg(data.positionProfit, 0, 'f', 3)));
	}
}

void JSTraderGUI::UpdatePositionBox(PositionData data)
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
		if (m_PositionModel->item(i, 1)->text().toLocal8Bit().toStdString() == data.symbol&&m_PositionModel->item(i, 2)->text().toLocal8Bit().toStdString() == direction)//判断表格的接口和这个接口是否一样
		{
			m_PositionModel->setItem(i, 0, new QStandardItem(str2qstr(data.gatewayname)));
			m_PositionModel->setItem(i, 1, new QStandardItem(str2qstr(data.symbol)));
			m_PositionModel->setItem(i, 2, new QStandardItem(str2qstr(direction)));
			m_PositionModel->setItem(i, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
			m_PositionModel->setItem(i, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
			m_PositionModel->setItem(i, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
			m_PositionModel->setItem(i, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));
			break;
		}
		if (i == m_PositionModel->rowCount() - 1)
		{
			//最后一个了还没找到
			m_PositionModel->setItem(i + 1, 0, new QStandardItem(str2qstr(data.gatewayname)));
			m_PositionModel->setItem(i + 1, 1, new QStandardItem(str2qstr(data.symbol)));
			if (data.direction == DIRECTION_SHORT)
			{
				m_PositionModel->setItem(i + 1, 2, new QStandardItem(str2qstr("空")));
			}
			else if (data.direction == DIRECTION_LONG)
			{
				m_PositionModel->setItem(i + 1, 2, new QStandardItem(str2qstr("多")));
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
		m_PositionModel->setItem(0, 0, new QStandardItem(str2qstr(data.gatewayname)));
		m_PositionModel->setItem(0, 1, new QStandardItem(str2qstr(data.symbol)));
		if (data.direction == DIRECTION_SHORT)
		{
			m_PositionModel->setItem(0, 2, new QStandardItem(str2qstr("空")));
		}
		else if (data.direction == DIRECTION_LONG)
		{
			m_PositionModel->setItem(0, 2, new QStandardItem(str2qstr("多")));
		}
		m_PositionModel->setItem(0, 3, new QStandardItem(QString("%1").arg(data.position, 0, 'f', 3)));
		m_PositionModel->setItem(0, 4, new QStandardItem(QString("%1").arg(data.ydPosition, 0, 'f', 3)));
		m_PositionModel->setItem(0, 5, new QStandardItem(QString("%1").arg(data.frozen, 0, 'f', 3)));
		m_PositionModel->setItem(0, 6, new QStandardItem(QString("%1").arg(data.price, 0, 'f', 3)));

	}
}

void JSTraderGUI::UpdatePortfolioBox(PortfolioData data)
{
	emit UpdatePortfolioWinning(str2qstr("组合盈亏:   ") + str2qstr(Utils::doubletostring(data.Portfoliodata.portfolio_winning)));
	int row = m_portfolioModel->rowCount();
	for (int i = 0; i <row; i++)
	{
		std::string dllname = m_portfolioModel->item(i, 0)->text().toStdString();
		std::string strategyname = m_portfolioModel->item(i, 1)->text().toStdString();
		std::string symbol = m_portfolioModel->item(i, 2)->text().toStdString();
		if (dllname == data.dllname && strategyname == data.strategyname && symbol == data.symbol)//判断表格的接口和这个接口是否一样
		{
			m_portfolioModel->setItem(i, 0, new QStandardItem(str2qstr(data.dllname)));
			m_portfolioModel->setItem(i, 1, new QStandardItem(str2qstr(data.strategyname)));
			m_portfolioModel->setItem(i, 2, new QStandardItem(str2qstr(data.symbol)));
			m_portfolioModel->setItem(i, 3, new QStandardItem(QString("%1").arg(data.Portfoliodata.Winning, 0, 'f', 3)));
			m_portfolioModel->setItem(i, 4, new QStandardItem(QString("%1").arg(data.Portfoliodata.Losing, 0, 'f', 3)));
			m_portfolioModel->setItem(i, 5, new QStandardItem(QString("%1").arg(data.Portfoliodata.totalwinning, 0, 'f', 3)));
			m_portfolioModel->setItem(i, 6, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingwinning, 0, 'f', 3)));
			m_portfolioModel->setItem(i, 7, new QStandardItem(QString("%1").arg(data.Portfoliodata.holding_and_totalwinning, 0, 'f', 3)));
			m_portfolioModel->setItem(i, 8, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingposition, 0, 'f', 3)));
			m_portfolioModel->setItem(i, 9, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingprice, 0, 'f', 3)));
			m_portfolioModel->setItem(i, 10, new QStandardItem(QString("%1").arg(data.Portfoliodata.drawdown, 0, 'f', 3)));
			m_portfolioModel->setItem(i, 11, new QStandardItem(QString("%1").arg(data.Portfoliodata.delta, 0, 'f', 3)));
			break;
		}
		if (i == m_portfolioModel->rowCount() - 1)
		{
			//最后一个了还没找到
			m_portfolioModel->setItem(i + 1, 0, new QStandardItem(str2qstr(data.dllname)));
			m_portfolioModel->setItem(i + 1, 1, new QStandardItem(str2qstr(data.strategyname)));
			m_portfolioModel->setItem(i + 1, 2, new QStandardItem(str2qstr(data.symbol)));
			m_portfolioModel->setItem(i + 1, 3, new QStandardItem(QString("%1").arg(data.Portfoliodata.Winning, 0, 'f', 3)));
			m_portfolioModel->setItem(i + 1, 4, new QStandardItem(QString("%1").arg(data.Portfoliodata.Losing, 0, 'f', 3)));
			m_portfolioModel->setItem(i + 1, 5, new QStandardItem(QString("%1").arg(data.Portfoliodata.totalwinning, 0, 'f', 3)));
			m_portfolioModel->setItem(i + 1, 6, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingwinning, 0, 'f', 3)));
			m_portfolioModel->setItem(i + 1, 7, new QStandardItem(QString("%1").arg(data.Portfoliodata.holding_and_totalwinning, 0, 'f', 3)));
			m_portfolioModel->setItem(i + 1, 8, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingposition, 0, 'f', 3)));
			m_portfolioModel->setItem(i + 1, 9, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingprice, 0, 'f', 3)));
			m_portfolioModel->setItem(i + 1, 10, new QStandardItem(QString("%1").arg(data.Portfoliodata.drawdown, 0, 'f', 3)));
			m_portfolioModel->setItem(i + 1, 11, new QStandardItem(QString("%1").arg(data.Portfoliodata.delta, 0, 'f', 3)));
			break;
		}
	}

	if (m_portfolioModel->rowCount() == 0)
	{
		m_portfolioModel->setItem(0, 0, new QStandardItem(str2qstr(data.dllname)));
		m_portfolioModel->setItem(0, 1, new QStandardItem(str2qstr(data.strategyname)));
		m_portfolioModel->setItem(0, 2, new QStandardItem(str2qstr(data.symbol)));
		m_portfolioModel->setItem(0, 3, new QStandardItem(QString("%1").arg(data.Portfoliodata.Winning, 0, 'f', 3)));
		m_portfolioModel->setItem(0, 4, new QStandardItem(QString("%1").arg(data.Portfoliodata.Losing, 0, 'f', 3)));
		m_portfolioModel->setItem(0, 5, new QStandardItem(QString("%1").arg(data.Portfoliodata.totalwinning, 0, 'f', 3)));
		m_portfolioModel->setItem(0, 6, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingwinning, 0, 'f', 3)));
		m_portfolioModel->setItem(0, 7, new QStandardItem(QString("%1").arg(data.Portfoliodata.holding_and_totalwinning, 0, 'f', 3)));
		m_portfolioModel->setItem(0, 8, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingposition, 0, 'f', 3)));
		m_portfolioModel->setItem(0, 9, new QStandardItem(QString("%1").arg(data.Portfoliodata.holdingprice, 0, 'f', 3)));
		m_portfolioModel->setItem(0, 10, new QStandardItem(QString("%1").arg(data.Portfoliodata.drawdown, 0, 'f', 3)));
		m_portfolioModel->setItem(0, 11, new QStandardItem(QString("%1").arg(data.Portfoliodata.delta, 0, 'f', 3)));
	}
}