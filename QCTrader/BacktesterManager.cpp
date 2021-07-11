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
	
	m_backtesterEngine->writeCtaLog((str2qstr_new("��ʼ��CTA�ز�����")).toStdString());

}

BacktesterManager::~BacktesterManager()
{

}

void BacktesterManager::InitUI()
{
	//����QCustomplot���
	QVector<double>x(101), y(101);
	for (int i = 0; i < 101; ++i)
	{
		x[i] = i / 50.0 - 1;
		y[i] = x[i] * x[i];
	}
	ui.widget->setWindowTitle("acout");
	ui.widget->windowTitle();
	ui.widget->addGraph();
	ui.widget->graph(0)->setData(x, y);
	ui.widget->xAxis->setLabel("x");
	ui.widget->yAxis->setLabel("y");
	ui.widget->xAxis->setRange(-1, 1);
	ui.widget->yAxis->setRange(0, 1);
	ui.widget->replot();
	ui.widget->show();


	ui.widget->plotLayout()->insertRow(0);
	ui.widget->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget, str2qstr_new("�˻���ֵ")));

	ui.widget_2->plotLayout()->insertRow(0);
	ui.widget_2->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_2, str2qstr_new("��ֵ�س�")));

	ui.widget_3->plotLayout()->insertRow(0);
	ui.widget_3->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_3, str2qstr_new("ÿ��ӯ��")));

	ui.widget_4->plotLayout()->insertRow(0);
	ui.widget_4->plotLayout()->addElement(0, 0, new QCPTextElement(ui.widget_4, str2qstr_new("ӯ���ֲ�")));

	//��ȡ������������
	std::string strName = "./Strategy/cta_strategy_setting_backtester.json";
	m_ctaStrategyMap=ReadStrategyConfFileJson(strName, m_backtesterEngine);

	std::map<std::string, std::map<std::string, float>>::iterator iter;
	for (iter = m_ctaStrategyMap.begin(); iter != m_ctaStrategyMap.end(); iter++)
	{
		QString strStrategyName = QString::fromStdString(iter->first).section("_", 0, 0);
		ui.comboBox->addItem(strStrategyName);

	}



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


	ui.pushButton->setEnabled(true);//��ʼ�زⰴť���´�
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
	//���ݲ������ƻ�ȡ����������
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
	QDateTime starDate, endDate;
	starDate=ui.dateEdit->dateTime();
	endDate = ui.dateEdit_2->dateTime();

	float rate = ui.lineEdit_10->text().toFloat();//����
	float slippage = ui.lineEdit_10->text().toFloat();//���׻���
	float contractsize = ui.lineEdit_10->text().toFloat();//��Լ����
	float pricetick = ui.lineEdit_10->text().toFloat();//�۸�����
	float capital = ui.lineEdit_10->text().toFloat();//�ʽ�

	m_backtesterEngine->StartBacktesting(strStrategyName, strStategyClassName.toStdString(), strSymbol, iInterval,
		starDate, endDate, rate, slippage, contractsize, pricetick, capital, settingMap);
	//���߳���ִ�лز��ʱ�򣬻ز������Ҫ����ϴε��������ݣ�������ťҲ��Ҫdisable
	ui.pushButton->setEnabled(false);
	ui.pushButton_2->setEnabled(false);

}



