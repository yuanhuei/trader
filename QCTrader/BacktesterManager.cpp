#define WIN32_LEAN_AND_MEAN
#include "BacktesterManager.h"
#include"utility.h"
#include"cta_backtester/BacktesterEngine.h"
#include"event_engine/eventengine.h"


#include <qvalidator.h>
#include"MainWindow.h"

BacktesterManager::BacktesterManager(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::CustomizeWindowHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowMaximizeButtonHint);
	setWindowFlags(Qt::Window);

	ui.setupUi(this);
	m_mainwindow = (MainWindow*)parent;
	m_backtesterEngine = m_mainwindow->m_backtesterEngine;
	
	m_backtesterEngine->writeCtaLog((str2qstr_new("初始化CTA回测引擎")).toStdString());

}

BacktesterManager::~BacktesterManager()
{

}

void BacktesterManager::InitUI()
{
	//设置QCustomplot组件
	ui.widget->addGraph();
	ui.widget->xAxis->setLabel("x");
	ui.widget->yAxis->setLabel("y");
	ui.widget->xAxis->setRange(0, 1000);
	ui.widget->yAxis->setRange(0, 10000);
	ui.widget->replot();
	ui.widget->show();

	ui.widget_2->addGraph();
	ui.widget_2->xAxis->setLabel("x");
	ui.widget_2->yAxis->setLabel("y");
	ui.widget_2->xAxis->setRange(0, 10000);
	ui.widget_2->yAxis->setRange(-10000, 0);
	ui.widget_2->replot();
	ui.widget_2->show();

	ui.widget_3->addGraph();
	ui.widget_3->xAxis->setLabel("x");
	ui.widget_3->yAxis->setLabel("y");
	ui.widget_3->xAxis->setRange(0, 1000);
	ui.widget_3->yAxis->setRange(-500, 10000);
	ui.widget_3->replot();
	ui.widget_3->show();

	ui.widget_4->addGraph();
	ui.widget_4->xAxis->setLabel("x");
	ui.widget_4->yAxis->setLabel("y");
	ui.widget_4->xAxis->setRange(0, 1000);
	ui.widget_4->yAxis->setRange(-500, 10000);
	ui.widget_4->replot();
	ui.widget_4->show();


	ui.widget->plotLayout()->insertRow(0);
	ui.widget->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget, str2qstr_new("账户净值")));

	ui.widget_2->plotLayout()->insertRow(0);
	ui.widget_2->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_2, str2qstr_new("净值回撤")));

	ui.widget_3->plotLayout()->insertRow(0);
	ui.widget_3->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_3, str2qstr_new("每日盈亏")));

	ui.widget_4->plotLayout()->insertRow(0);
	ui.widget_4->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_4, str2qstr_new("盈亏分布")));

	//读取策略名和配置
	std::string strName = "./Strategy/cta_strategy_setting_backtester.json";
	m_ctaStrategyMap=ReadStrategyConfFileJson(strName, m_backtesterEngine);

	std::map<std::string, std::map<std::string, float>>::iterator iter;
	for (iter = m_ctaStrategyMap.begin(); iter != m_ctaStrategyMap.end(); iter++)
	{
		QString strStrategyName = QString::fromStdString(iter->first).section("_", 0, 0);
		ui.comboBox->addItem(strStrategyName);

	}


	//设置只能输入数字
	QValidator* validator = new QIntValidator(0, 9999, this);
	ui.lineEdit_9->setValidator(validator);
	ui.lineEdit_10->setValidator(validator);
	ui.lineEdit_12->setValidator(validator);
	ui.lineEdit_13->setValidator(validator);
	ui.lineEdit_14->setValidator(validator);
}

void BacktesterManager::UpdateLogTable(LogData data)
{
	ui.textEdit->insertPlainText(QString::fromStdString(data.logTime + "    " + data.msg + "\n"));

}
void BacktesterManager::RegisterEvent()
{
	m_mainwindow->m_eventengine->RegEvent(EVENT_CTABACKTESTERFINISHED, std::bind(&BacktesterManager::ProcecssTesterFisnishedEvent, this, std::placeholders::_1));
}

void BacktesterManager::ProcecssTesterFisnishedEvent(std::shared_ptr<Event>e)
{
	//输出回测统计信息
	ui.tableWidget->setItem(0, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["start_date"])));
	ui.tableWidget->setItem(1, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["end_date"])));
	ui.tableWidget->setItem(2, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["total_days"])));
	ui.tableWidget->setItem(3, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["profit_days"])));
	ui.tableWidget->setItem(4, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["loss_days"])));
	ui.tableWidget->setItem(5, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["end_balance"])));
	ui.tableWidget->setItem(6, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["max_drawdown"])));
	ui.tableWidget->setItem(7, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["max_ddpercent"])));
	ui.tableWidget->setItem(8, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["total_net_pnl"])));
	ui.tableWidget->setItem(9, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["daily_net_pnl"])));
	ui.tableWidget->setItem(10, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["total_commission"])));
	ui.tableWidget->setItem(11, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["daily_commission"])));

	ui.tableWidget->setItem(11, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["total_slippage"])));
	ui.tableWidget->setItem(12, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["total_turnover"])));
	ui.tableWidget->setItem(13, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["total_trade_count"])));
	ui.tableWidget->setItem(14, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["daily_trade_count"])));
	ui.tableWidget->setItem(15, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["total_return"])));
	ui.tableWidget->setItem(16, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["annual_return"])));
	ui.tableWidget->setItem(17, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["daily_return"])));

	//ui.tableWidget->setItem(14, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["return_std"])));
	//ui.tableWidget->setItem(15, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["sharpe_ratio"])));
	//ui.tableWidget->setItem(16, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["return_drawdown_ratio"])));
	//ui.tableWidget->setItem(17, 1, new QTableWidgetItem(str2qstr_new(m_backtesterEngine->m_result_statistics["return_std"])));

	//画图
	//账户净值
	//净值回撤
	// 每日盈亏
	// 盈亏分布
	int nDay = m_backtesterEngine->m_daily_resultMap.size();
	QVector<double> x(nDay), balance_y(nDay),drawdown_y(nDay),pnl_y(nDay),pnl_distribution_y(nDay);
	int n = 0;
	x[n] = 0;
	for (auto &iter: m_backtesterEngine->m_daily_resultMap)
	{
		balance_y[n] = iter.second->m_balance;
		drawdown_y[n] = iter.second->m_drawdown;
		pnl_y[n] = iter.second->m_net_pnl;
		balance_y[n] = iter.second->m_balance;

		x[n++] = n++ ;
		
	}
	ui.widget->graph(0)->setData(x, balance_y);
	ui.widget->replot();
	ui.widget->show();
	ui.widget_2->graph(0)->setData(x, drawdown_y);
	ui.widget_2->replot();
	ui.widget_2->show();
	ui.widget_3->graph(0)->setData(x, pnl_y);
	ui.widget_3->replot();
	ui.widget_3->show();
	//ui.widget_4->graph(0)->setData(x, pnl_distribution_y);
	ui.widget_4->replot();
	ui.widget_4->show();
	// 
	// 
	//开始回测按钮重新打开
	ui.pushButton->setEnabled(true);
	/*
	statistics = self.backtester_engine.get_result_statistics()
		self.statistics_monitor.set_data(statistics)

		df = self.backtester_engine.get_result_df()
		self.chart.set_data(df)

		self.trade_button.setEnabled(True)
		self.order_button.setEnabled(True)
		self.daily_button.setEnabled(True)
	*/

}
void BacktesterManager::startBacktest_clicked()
{
	std::string strStrategyName = ui.comboBox->currentText().toStdString();
	std::string strSymbol = ui.lineEdit_2->text().toStdString();
	//根据策略名称获取策略类名称
	QString strStategyClassName;
	std::map<std::string, float> settingMap;
	std::map<std::string, std::map<std::string, float>>::iterator iter;
	for (iter = m_ctaStrategyMap.begin(); iter != m_ctaStrategyMap.end(); iter++)
	{
		if (iter->first.find(strStrategyName) != iter->first.npos)
		{
			strStategyClassName = QString::fromStdString(iter->first).section("_", 1, 1);
			settingMap = iter->second;
		}
	}


	Interval iInterval = Interval(ui.comboBox_2->currentIndex());
	QDate starDate, endDate;
	starDate=ui.dateEdit->dateTime().date();
	endDate = ui.dateEdit_2->dateTime().date();

	float rate = ui.lineEdit_10->text().toFloat();//费率
	float slippage = ui.lineEdit_10->text().toFloat();//交易滑点
	float contractsize = ui.lineEdit_10->text().toFloat();//合约乘数
	float pricetick = ui.lineEdit_10->text().toFloat();//价格跳动
	float capital = ui.lineEdit_10->text().toFloat();//资金

	m_backtesterEngine->StartBacktesting(strStrategyName, strStategyClassName.toStdString(), strSymbol, iInterval,
		starDate, endDate, rate, slippage, contractsize, pricetick, capital, settingMap);
	//子线程在执行回测的时候，回测界面需要清除上次的遗留数据，几个按钮也需要disable
	ui.pushButton->setEnabled(false);
	ui.pushButton_2->setEnabled(false);

}



