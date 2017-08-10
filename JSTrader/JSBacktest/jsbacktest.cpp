#include "jsbacktest.h"
#include "backtestengine.h"
#include "vld.h"
//设置传输对象

void TransferObject::setSendCapital(const QString &text)
{
	m_capital = text;
	if (m_success)
	{
		emit sendCapital(m_capital);
	}
}

void TransferObject::setSendVar(const QString &text)
{
	m_var = text;
	if (m_success)
	{
		emit sendVar(m_var);
	}
}

void TransferObject::finished(bool success)
{
	m_success = success;
	if (m_var != "")
	{
		emit sendVar(m_var);
	}
	if (m_capital != "")
	{
		emit sendCapital(m_capital);
	}

}

//
JSBacktest::JSBacktest(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//创建GUI
	CreateMainGUI();
	//创建一个回测引擎
	CreateBacktestEngine();
	//创建画图
	CreatePlotWidget();
}

JSBacktest::~JSBacktest()
{
	for (std::map<std::string, QWebEngineView*>::iterator it = m_vardata_view.begin(); it != m_vardata_view.end(); it++)
	{
		it->second->deleteLater();
	}
	for(std::vector<TransferObject*>::iterator it = m_vardata_transferpointer.begin(); it != m_vardata_transferpointer.end(); it++)
	{
		delete (*it);
	}
	m_vardata_transferpointer.clear();
	m_vardata_view.clear();
	delete CapitalCurveWidget;
	delete m_backtestengine;
	m_thread->terminate();
	m_thread->wait();
	delete m_thread;
	delete m_Model;
}

QString JSBacktest::str2qstr(const std::string str)
{
	return QString::fromLocal8Bit(str.data());
}

void JSBacktest::CreateMainGUI()
{
	QGridLayout *mainlayout = new QGridLayout;
	mainlayout->addWidget(createcontrolGroup(), 0, 0);
	mainlayout->addWidget(createtableviewGroup(), 1, 0);
	mainlayout->addWidget(createlogGroup(), 2, 0);
	setWindowTitle(tr("JSBACKTEST"));
	resize(800, 600);
	setLayout(mainlayout);
}

QGroupBox *JSBacktest::createcontrolGroup()
{
	//四个按钮 两个combo 一个进度条
	strategybox = new QComboBox;
	QPushButton *Add = new QPushButton(str2qstr("添加"));
	QPushButton *Del = new QPushButton(str2qstr("删除"));
	QPushButton *Start = new QPushButton(str2qstr("开始"));
	QPushButton *Stop = new QPushButton(str2qstr("停止"));
	progressbar = new QProgressBar();
	QGroupBox *groupbox = new QGroupBox(str2qstr("控制"));
	TickButton = new QRadioButton(str2qstr("Tick模式"));
	BarButton = new QRadioButton(str2qstr("Bar模式"));

	//回测神器
	VarPlotCheckBox = new QCheckBox(str2qstr("回测神器"));

	BarButton->setChecked(true);
	start = new QDateTimeEdit;
	start->setDateTime(QDateTime(QDate(2016, 12, 01), QTime(9, 00, 00)));
	end = new QDateTimeEdit;
	end->setDateTime(QDateTime(QDate(2017, 07, 01), QTime(0, 00, 00)));
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(strategybox, 0, 0, 1, 2);
	layout->addWidget(start, 0, 2, 1, 1);
	layout->addWidget(end, 0, 3, 1, 1);
	layout->addWidget(Add, 1, 0, 1, 1);
	layout->addWidget(Del, 1, 1, 1, 1);
	layout->addWidget(Start, 1, 2, 1, 1);
	layout->addWidget(Stop, 1, 3, 1, 1);
	layout->addWidget(TickButton, 2, 0, 1, 1);
	layout->addWidget(BarButton, 2, 1, 1, 1);
	layout->addWidget(VarPlotCheckBox, 2, 2, 1, 1);
	layout->addWidget(progressbar, 2, 3, 1, 1);
	groupbox->setLayout(layout);
	connect(Add, SIGNAL(clicked()), this, SLOT(add_symbol_strategy()));
	connect(Del, SIGNAL(clicked()), this, SLOT(del_symbol_strategy()));
	connect(Start, SIGNAL(clicked()), this, SLOT(start_backtest()));
	connect(Stop, SIGNAL(clicked()), this, SLOT(stop_backtest()));
	return groupbox;
}

QGroupBox *JSBacktest::createtableviewGroup()
{
	tableview = new QTableView;
	m_Model = new QStandardItemModel;
	QStringList header;
	header << str2qstr("合约") << str2qstr("策略");
	m_Model->setHorizontalHeaderLabels(header);
	tableview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableview->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableview->setModel(m_Model);
	QGroupBox *groupbox = new QGroupBox(str2qstr("对应关系"));
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(tableview);
	groupbox->setLayout(layout);
	return groupbox;
}

QGroupBox *JSBacktest::createlogGroup()
{
	textbrowser = new QTextBrowser;
	QGroupBox *groupbox = new QGroupBox(str2qstr("日志"));
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(textbrowser);
	groupbox->setLayout(layout);
	return groupbox;
}

void JSBacktest::CreateBacktestEngine()
{
	m_thread = new QThread;
	m_backtestengine = new BacktestEngine();
	ConnectSignalsSlots();						//转移线程前先连接信号和槽函数
	m_backtestengine->moveToThread(m_thread);
	m_thread->start();
	m_backtestengine->loadstrategy();			//读取策略
}

void JSBacktest::ConnectSignalsSlots()
{
	qRegisterMetaType<PLOTDATA>("PLOTDATA");//注册到元系统中
	qRegisterMetaType<VarData>("VarData");//注册到元系统中
	connect(m_backtestengine, SIGNAL(PlotVars(VarData)), this, SLOT(plotVarSlots(VarData)), Qt::QueuedConnection);
	connect(m_backtestengine, SIGNAL(PlotCurve(PLOTDATA)), this, SLOT(plotcurve(PLOTDATA)), Qt::QueuedConnection);
	connect(m_backtestengine, SIGNAL(addItem(QString)), this, SLOT(add_item_combobox(QString)), Qt::QueuedConnection);
	connect(this, SIGNAL(runbacktest()), m_backtestengine, SLOT(Runbacktest()), Qt::QueuedConnection);
	connect(this, SIGNAL(stopbacktest()), m_backtestengine, SLOT(Stopbacktest()), Qt::QueuedConnection);
	connect(m_backtestengine, SIGNAL(WriteLog(QString)), textbrowser, SLOT(append(QString), Qt::QueuedConnection));
	connect(m_backtestengine, SIGNAL(setrangesignal(int)), this, SLOT(setrange(int)), Qt::QueuedConnection);
	connect(m_backtestengine, SIGNAL(setvaluesignal(int)), this, SLOT(setvalueSLOT(int)), Qt::QueuedConnection);
}

void JSBacktest::setrange(int max)
{
	progressbar->setRange(0, max);
}

void JSBacktest::setvalueSLOT(int value)
{
	progressbar->setValue(value);
}

void JSBacktest::CreatePlotWidget()
{
	//创建净值图像需要的控件
	CapitalCurveWidget = new QWidget;
	//创建两个groupbox，左侧用来装策略的checkbox，右侧装curve
	QGridLayout *mainlayout = new QGridLayout;
	QGroupBox *strategygroupbox = new QGroupBox(str2qstr("策略"));
	strategylayout = new QVBoxLayout;
	strategygroupbox->setLayout(strategylayout);
	QWebEngineView *view = new QWebEngineView;
	view->setUrl(QUrl("qrc:///capital.html"));
	QWebChannel*channel = new QWebChannel(this);
	channel->registerObject(QStringLiteral("transferobject"), &m_transerobject);
	view->page()->setWebChannel(channel);
	mainlayout->addWidget(strategygroupbox, 0, 0);
	mainlayout->addWidget(view, 0, 1);
	CapitalCurveWidget->setLayout(mainlayout);
	CapitalCurveWidget->show();
}

void JSBacktest::add_symbol_strategy()
{
	QString strategy = strategybox->currentText();
	QString symbol = str2qstr(m_backtestengine->getparam(strategy.toStdString(), "symbol"));
	m_Model->insertRow(0);
	m_Model->setItem(0, 0, new QStandardItem(symbol));
	m_Model->setItem(0, 1, new QStandardItem(strategy));
}

void JSBacktest::del_symbol_strategy()
{
	m_Model->removeRow(tableview->currentIndex().row());
}

void JSBacktest::add_item_combobox(QString strategyname)
{
	strategybox->addItem(strategyname);
}

void JSBacktest::start_backtest()
{
	ClearMemorys();
	//获取开始日期和结束日期
	m_backtestengine->setStartDate(start->dateTime().toTime_t());
	m_backtestengine->setStopDate(end->dateTime().toTime_t());
	//设置回测模式
	if (TickButton->isChecked())
	{
		m_backtestengine->setMode(TICK_MODE);
	}
	else if (BarButton->isChecked())
	{
		m_backtestengine->setMode(BAR_MODE);
	}
	//获取表格数据
	m_backtestengine->GetTableViewData(*m_Model);
	//调用回测函数
	emit runbacktest();
}

void JSBacktest::stop_backtest()
{
	emit stopbacktest();
}

void JSBacktest::ClearMemorys()
{
	for (std::map<std::string, QWebEngineView*>::iterator it = m_vardata_view.begin(); it != m_vardata_view.end(); it++)
	{
		it->second->deleteLater();
	}
	m_vardata_view.clear();

	for (std::vector<QCheckBox*>::iterator iter = m_checkbox_v.begin(); iter != m_checkbox_v.end(); iter++)
	{
		strategylayout->removeWidget(*iter);
		(*iter)->deleteLater();
	}
	m_checkbox_v.clear();
	m_checkbox_ResultVector.clear();
}

void JSBacktest::plotcurve(PLOTDATA plotdata)
{
	for (std::map<std::string, std::vector<int>>::iterator iter = plotdata.m_capital_datetime.begin(); iter != plotdata.m_capital_datetime.end(); iter++)
	{
		//key是策略名，按照策略名创建checkbox
		QCheckBox *checkbox = new QCheckBox(str2qstr(iter->first));
		connect(checkbox, SIGNAL(clicked()), this, SLOT(checkedcurve()));
		m_checkbox_v.push_back(checkbox);
		strategylayout->addWidget(checkbox);
		std::vector<ResultPoint>v;
		m_checkbox_ResultVector.insert(std::pair<QCheckBox*, std::vector<ResultPoint>>(checkbox, v));
		int size = plotdata.m_capital_datetime[iter->first].size();
		for (int i = 0; i < size; i++)
		{
			ResultPoint point;
			point.X = QDateTime::fromTime_t(iter->second[i]).toString("yyyy/MM/dd hh:mm:ss").toStdString();
			point.Y = Utils::doubletostring(plotdata.m_holding_and_totalwinning[iter->first][i]);
			m_checkbox_ResultVector[checkbox].push_back(point);
		}
	}
	CapitalCurveWidget->resize(1000, 600);
	CapitalCurveWidget->show();
}

void JSBacktest::checkedcurve()
{
	//设置要传输的数据
	std::vector<std::string>X;
	std::vector<std::string>Y;

	m_checkbox_ResultVectormtx.lock();
	for (std::map<QCheckBox*, std::vector<ResultPoint>>::iterator iter = m_checkbox_ResultVector.begin(); iter != m_checkbox_ResultVector.end(); iter++)
	{
		if (iter->first->isChecked() == true)
		{
			//点中
			for (std::vector<ResultPoint>::iterator it = iter->second.begin(); it != iter->second.end(); it++)
			{
				//遍历坐标
				if (X.size() < iter->second.size())
				{
					//XY都是空的
					X.push_back((*it).X);
					Y.push_back((*it).Y);
				}
				else
				{
					//已经有数据了，再原有数据上添加
					Y[it - iter->second.begin()] = Utils::doubletostring(atof(Y[it - iter->second.begin()].c_str()) + atof((*it).Y.c_str()));
				}
			}
		}
	}
	m_checkbox_ResultVectormtx.unlock();

	QJsonArray X_array;
	for (std::vector<std::string>::iterator iter = X.begin(); iter != X.end(); iter++)
	{
		X_array.insert(iter - X.begin(), QString::fromStdString(*iter));
	}

	QJsonArray Y_array;
	for (std::vector<std::string>::iterator iter = Y.begin(); iter != Y.end(); iter++)
	{
		Y_array.insert(iter - Y.begin(), QString::fromStdString(*iter));
	}

	QJsonObject object;
	object.insert("X", X_array);
	object.insert("Y", Y_array);

	QJsonDocument document;
	document.setObject(object);
	QByteArray byte_array = document.toJson(QJsonDocument::Compact);
	QString json_str(byte_array);

	m_transerobject.setSendCapital(json_str);
}

void JSBacktest::plotVarSlots(VarData vardata)
{
	if (VarPlotCheckBox->isChecked() == false)
	{
		return;
	}
	for (std::map<std::string, std::vector<BarData>>::iterator iter = vardata.m_strategy_bardata.begin(); iter != vardata.m_strategy_bardata.end(); iter++)
	{
		QWebEngineView *view = new QWebEngineView;
		view->setUrl(QUrl("qrc:///var.html"));
		QWebChannel*channel = new QWebChannel(this);
		TransferObject *transobj = new TransferObject;		
		channel->registerObject(QStringLiteral("transferobject"), transobj);
		view->page()->setWebChannel(channel);
		view->show();
		m_vardata_transferpointer.push_back(transobj);

		m_vardata_view.insert(std::pair<std::string, QWebEngineView*>(iter->first, view));
		QJsonArray X_array;
		QJsonArray OHLC_OBJ;
		for (int i = 0; i < vardata.m_strategy_bardata[iter->first].size(); i++)
		{
			QJsonArray OHLC_array;
			X_array.insert(i, QDateTime::fromTime_t(vardata.m_strategy_bardata[iter->first][i].unixdatetime).toString("yyyy-MM-dd hh:mm:ss"));
			OHLC_array.append(QString::fromStdString(Utils::doubletostring(vardata.m_strategy_bardata[iter->first][i].open)));
			OHLC_array.append(QString::fromStdString(Utils::doubletostring(vardata.m_strategy_bardata[iter->first][i].close)));
			OHLC_array.append(QString::fromStdString(Utils::doubletostring(vardata.m_strategy_bardata[iter->first][i].low)));
			OHLC_array.append(QString::fromStdString(Utils::doubletostring(vardata.m_strategy_bardata[iter->first][i].high)));
			OHLC_OBJ.append(OHLC_array);
		}
		QJsonObject object;
		object.insert("X", X_array);
		object.insert("Bar", OHLC_OBJ);

		QJsonObject mainchartobj;
		//遍历所有的变量序列
		for (std::map<std::string, std::vector<double>>::iterator it = vardata.m_strategy_varplotrecord_mainchart[iter->first].begin(); it != vardata.m_strategy_varplotrecord_mainchart[iter->first].end(); it++)
		{

			QJsonArray vararray;
			for (int i = 0; i < it->second.size(); i++)
			{
				vararray.insert(i, QString::fromStdString(Utils::doubletostring(it->second[i])));
			}
			mainchartobj.insert(QString::fromStdString(it->first), vararray);
		}
		object.insert("MainChart", mainchartobj);

		QJsonObject indicatorobj;
		for (std::map<std::string, std::vector<double>>::iterator it = vardata.m_strategy_varplotrecord_indicator[iter->first].begin(); it != vardata.m_strategy_varplotrecord_indicator[iter->first].end(); it++)
		{

			QJsonArray vararray;
			for (int i = 0; i < it->second.size(); i++)
			{
				vararray.insert(i, QString::fromStdString(Utils::doubletostring(it->second[i])));
			}
			indicatorobj.insert(QString::fromStdString(it->first), vararray);
		}
		object.insert("Indicator", indicatorobj);

		QJsonObject boolobj;
		for (std::map<std::string, std::vector<bool>>::iterator it = vardata.m_strategy_varplotrecord_bool[iter->first].begin(); it != vardata.m_strategy_varplotrecord_bool[iter->first].end(); it++)
		{

			QJsonArray vararray;
			for (int i = 0; i < it->second.size(); i++)
			{
				if (it->second[i] == true)
				{
					vararray.insert(i, 1);
				}
				else
				{
					vararray.insert(i, -1);
				}
			}
			boolobj.insert(QString::fromStdString(it->first), vararray);
		}
		object.insert("BoolVar", boolobj);


		QJsonArray pnl;
		for (std::vector<double>::iterator it = vardata.m_strategy_varplotrecord_pnl[iter->first].begin(); it != vardata.m_strategy_varplotrecord_pnl[iter->first].end(); it++)
		{
			pnl.insert(it - vardata.m_strategy_varplotrecord_pnl[iter->first].begin(), QString::fromStdString(Utils::doubletostring( *it)));
		}
		object.insert("pnl", pnl);


		/*for (std::map<std::string, std::vector<std::string>>::iterator it = vardata.m_strategy_varplotrecord_string[iter->first].begin(); it != vardata.m_strategy_varplotrecord_string[iter->first].end(); it++)
		{

			QJsonArray vararray;
			for (int i = 0; i < it->second.size(); i++)
			{
				vararray.insert(i, QString::fromStdString(Utils::doubletostring(it->second[i])));
			}
			Varobject.insert(QString::fromStdString(it->first), vararray);
		}*/

		QJsonDocument document;
		document.setObject(object);
		QByteArray byte_array = document.toJson(QJsonDocument::Compact);
		QString json_str(byte_array);

		transobj->setSendVar(json_str);
	}
	//绘制图像
	if (VarPlotCheckBox->isChecked() == true)
	{
		for (std::map<std::string, QWebEngineView*>::iterator it = m_vardata_view.begin(); it != m_vardata_view.end(); it++)
		{
			it->second->show();
			it->second->resize(1000, 800);
		}
	}
}