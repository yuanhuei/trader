#include "ctp.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QTimer>

#include <time.h>
#include <QScrollBar>

#include "MdSpi.h"
#include "TdSpi.h"

#include "MemLog.h"
MemLog       simulator_log;

MdSpi* gMd;
TdSpi* gTd;


ctp::ctp(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	//设置窗口比例
	//ui.splitter_4->setStretchFactor(0, 1);
	//ui.splitter_4->setStretchFactor(1, 1);
	ui.splitter_3->setStretchFactor(0, 2);
	ui.splitter_3->setStretchFactor(1, 3);

	//信号连接
	connect(gMd,SIGNAL(sendData(QString)),this,SLOT(ReceiveHQ(QString)));
	connect(gTd,SIGNAL(sendCJ(QString)),this,SLOT(ReceiveCJ(QString)));
	connect(gTd,SIGNAL(sendWT(QString)),this,SLOT(ReceiveWT(QString)));
	connect(gTd,SIGNAL(sendCC(QString)),this,SLOT(ReceiveCC(QString)));
	connect(gTd,SIGNAL(sendHY(QString)),this,SLOT(ReceiveHY(QString)));
	connect(gTd,SIGNAL(sendZJ(QString)),this,SLOT(ReceiveZJ(QString)));
	connect(gTd,SIGNAL(sendDELCC(QString)),this,SLOT(RecieveDELCC(QString)));
	connect(ui.BtnXd,SIGNAL(clicked()),this,SLOT(xd()));
	connect(ui.HQTable,SIGNAL(cellClicked(int,int)),this,SLOT(xddm()));

	//*****************行情表********************//
	ui.HQTable->setColumnCount(12);

	QStringList headerHQ;
	headerHQ.append(QString::fromLocal8Bit("合约代码"));
	headerHQ.append(QString::fromLocal8Bit("更新时间"));
	headerHQ.append(QString::fromLocal8Bit("最新价"));
	headerHQ.append(QString::fromLocal8Bit("买一价"));
	headerHQ.append(QString::fromLocal8Bit("买一量"));
	headerHQ.append(QString::fromLocal8Bit("卖一价"));
	headerHQ.append(QString::fromLocal8Bit("卖一量"));
	headerHQ.append(QString::fromLocal8Bit("涨幅"));
	headerHQ.append(QString::fromLocal8Bit("成交量"));
	headerHQ.append(QString::fromLocal8Bit("涨停价"));
	headerHQ.append(QString::fromLocal8Bit("跌停价"));
	headerHQ.append(QString::fromLocal8Bit("交易所"));

	ui.HQTable->setHorizontalHeaderLabels(headerHQ);
	//ui.HQTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.HQTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.HQTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************委托表********************//
	ui.WTTable->setColumnCount(9);

	QStringList headerWT;
	headerWT.append(QString::fromLocal8Bit("委托时间"));
	headerWT.append(QString::fromLocal8Bit("合约代码"));
	headerWT.append(QString::fromLocal8Bit("买卖"));
	headerWT.append(QString::fromLocal8Bit("开平"));
	headerWT.append(QString::fromLocal8Bit("数量"));
	headerWT.append(QString::fromLocal8Bit("价格"));
	headerWT.append(QString::fromLocal8Bit("状态"));
	headerWT.append(QString::fromLocal8Bit("委托号"));
	headerWT.append(QString::fromLocal8Bit("交易所"));

	ui.WTTable->setHorizontalHeaderLabels(headerWT);
	//ui.WTTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.WTTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.WTTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************成交表********************//
	ui.CJTable->setColumnCount(8);
	QStringList headerCJ;
	headerCJ.append(QString::fromLocal8Bit("成交时间"));
	headerCJ.append(QString::fromLocal8Bit("合约代码"));
	headerCJ.append(QString::fromLocal8Bit("买卖"));
	headerCJ.append(QString::fromLocal8Bit("开平"));
	headerCJ.append(QString::fromLocal8Bit("数量"));
	headerCJ.append(QString::fromLocal8Bit("价格"));
	headerCJ.append(QString::fromLocal8Bit("委托号"));
	headerCJ.append(QString::fromLocal8Bit("交易所"));

	ui.CJTable->setHorizontalHeaderLabels(headerCJ);
	ui.CJTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.CJTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.CJTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************持仓表********************//
	ui.CCTable->setColumnCount(4);
	QStringList headerCC;
	headerCC.append(QString::fromLocal8Bit("合约代码"));
	headerCC.append(QString::fromLocal8Bit("持仓类型"));
	headerCC.append(QString::fromLocal8Bit("持仓数量"));
	headerCC.append(QString::fromLocal8Bit("持仓成本"));

	ui.CCTable->setHorizontalHeaderLabels(headerCC);
	//ui.CCTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.CCTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.CCTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************资金表********************//
	ui.ZJTable->setColumnCount(5);

	QStringList headerZJ;
	headerZJ.append(QString::fromLocal8Bit("账号"));
	headerZJ.append(QString::fromLocal8Bit("总权益"));
	headerZJ.append(QString::fromLocal8Bit("占用保证金"));
	headerZJ.append(QString::fromLocal8Bit("可用资金"));
	headerZJ.append(QString::fromLocal8Bit("风险度"));

	ui.ZJTable->setHorizontalHeaderLabels(headerZJ);
	ui.ZJTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.ZJTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.ZJTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************合约表********************//
	ui.HYTable->setColumnCount(5);

	QStringList headerHY;
	headerHY.append(QString::fromLocal8Bit("合约代码"));
	headerHY.append(QString::fromLocal8Bit("交易所"));
	headerHY.append(QString::fromLocal8Bit("合约名称"));
	headerHY.append(QString::fromLocal8Bit("合约乘数"));
	headerHY.append(QString::fromLocal8Bit("合约点数"));

	ui.HYTable->setHorizontalHeaderLabels(headerHY);
	//ui.HYTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.HYTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.HYTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//默认打开行情页
	//ui.tabWidget->setCurrentIndex(0);

	//下单部分初始化
	ui.Editdm->setText("");
	ui.EditLots->setText("1");
	ui.radioSJ->setChecked(true);
	ui.radioPJ->setChecked(true);

	//创建日志显示框
	QString data_style;
	data_style.sprintf("gridline-color: rgb(210,232,247);background-color:rgb(210,232,247)");
	ui.tw_Log->setStyleSheet(data_style);
	ui.tw_Log->horizontalHeader()->setStretchLastSection(true);
	ui.tw_Log->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tw_Log->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui.tw_Log->setColumnCount(1);
	ui.tw_Log->verticalHeader()->setDefaultSectionSize(16);
	//禁止修改
	ui.tw_Log->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//隐藏列表头
	ui.tw_Log->verticalHeader()->setVisible(false);

	//右键菜单
	ui.WTTable->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.WTTable,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(OnWTMenu(const QPoint&)));
	connect(ui.actioncd,SIGNAL(triggered()),this,SLOT(cd()));

	QTimer *testTimer1 = new QTimer(this);
	QTimer *testTimer2 = new QTimer(this);
	connect( testTimer1,SIGNAL(timeout()), this, SLOT(testFunction1()) );
	connect( testTimer2,SIGNAL(timeout()), this, SLOT(testFunction2()) );

	//testTimer1->start(1000);
	//testTimer2->start(1000);

	QTimer *mainTimer = new QTimer(this);
	connect(mainTimer, SIGNAL(timeout()), this, SLOT(OnMainTimer()));
	mainTimer->start(500);
}

ctp::~ctp()
{
}

void ctp::MDLogin()
{
}

void ctp::OnHQCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
	QTableWidgetItem *item;
	item = ui.HQTable->item(currentRow, 0);
	ui.Editdm->setText(item->text());
	item = ui.HQTable->item(currentRow, 5);
	ui.labelAsk->setText(item->text());
	item = ui.HQTable->item(currentRow, 2);
	ui.labelLast->setText(item->text());
	item = ui.HQTable->item(currentRow, 3);
	ui.labelBid->setText(item->text());
	item = ui.HQTable->item(currentRow, 9);
	ui.labelUP->setText(item->text());
	item = ui.HQTable->item(currentRow, 10);
	ui.labelDOWN->setText(item->text());
	item = ui.HQTable->item(currentRow, 11);
	ui.comboJYS->setCurrentText(item->text());
	/*for (int i = 0; i < ui.comboXd->count(); i++)
	{
		if (ui.comboXd->itemText(i)== item->text())
			ui.comboXd->setCurrentText
	}*/
}

//接收到行情数据
void ctp::ReceiveHQ(QString TICK)
{
	QStringList strlist=TICK.split(",");

	//修改下单界面
	if(strlist.at(0)==ui.Editdm->text())
	{
		ui.labelAsk->setText(strlist.at(5));
		ui.labelLast->setText(strlist.at(2));
		ui.labelBid->setText(strlist.at(3));
		ui.labelUP->setText(strlist.at(9));
		ui.labelDOWN->setText(strlist.at(10));
	}
	
	//在行情列表中查找代码
	for (int i=0;i<ui.HQTable->rowCount();i++)
	{
		if (ui.HQTable->item(i,0)->text()==strlist.at(0))
		{
			ui.HQTable->setItem(i,0,new QTableWidgetItem(strlist.at(0)));
			ui.HQTable->setItem(i,1,new QTableWidgetItem(strlist.at(1)));
			ui.HQTable->setItem(i,2,new QTableWidgetItem(strlist.at(2)));
			ui.HQTable->setItem(i,3,new QTableWidgetItem(strlist.at(3)));
			ui.HQTable->setItem(i,4,new QTableWidgetItem(strlist.at(4)));
			ui.HQTable->setItem(i,5,new QTableWidgetItem(strlist.at(5)));
			ui.HQTable->setItem(i,6,new QTableWidgetItem(strlist.at(6)));
			ui.HQTable->setItem(i,7,new QTableWidgetItem(strlist.at(7)));
			ui.HQTable->setItem(i,8,new QTableWidgetItem(strlist.at(8)));
			ui.HQTable->setItem(i,9,new QTableWidgetItem(strlist.at(9)));
			ui.HQTable->setItem(i, 10, new QTableWidgetItem(strlist.at(10)));
			ui.HQTable->setItem(i, 11, new QTableWidgetItem(strlist.at(11)));
			return;
		}
	}

	//如果在行情列表中没找到，增加一行
	int row=ui.HQTable->rowCount();
	ui.HQTable->insertRow(row);
	ui.HQTable->setItem(row,0,new QTableWidgetItem(strlist.at(0)));
	ui.HQTable->setItem(row,1,new QTableWidgetItem(strlist.at(1)));
	ui.HQTable->setItem(row,2,new QTableWidgetItem(strlist.at(2)));
	ui.HQTable->setItem(row,3,new QTableWidgetItem(strlist.at(3)));
	ui.HQTable->setItem(row,4,new QTableWidgetItem(strlist.at(4)));
	ui.HQTable->setItem(row,5,new QTableWidgetItem(strlist.at(5)));
	ui.HQTable->setItem(row,6,new QTableWidgetItem(strlist.at(6)));
	ui.HQTable->setItem(row,7,new QTableWidgetItem(strlist.at(7)));
	ui.HQTable->setItem(row,8,new QTableWidgetItem(strlist.at(8)));
	ui.HQTable->setItem(row,9,new QTableWidgetItem(strlist.at(9)));
	ui.HQTable->setItem(row, 10, new QTableWidgetItem(strlist.at(10)));
	ui.HQTable->setItem(row, 11, new QTableWidgetItem(strlist.at(11)));
}

//接收到成交数据
void ctp::ReceiveCJ(QString CJData)
{
	QStringList strlist = CJData.split(",");
	QString buysell = "";
	QString openclose = "";

	if (strlist.at(2) == "0")
	{
		buysell = QString::fromLocal8Bit("买入");
	}
	else
	{
		buysell = QString::fromLocal8Bit("卖出");
	}
	if (strlist.at(3) == "0")
	{
		openclose = QString::fromLocal8Bit("开仓");
	}
	else if (strlist.at(3) == "4")
	{
		openclose = QString::fromLocal8Bit("平昨");
	}
	else
	{
		openclose = QString::fromLocal8Bit("平今");
	}
	int row = ui.CJTable->rowCount();
	ui.CJTable->insertRow(row);
	ui.CJTable->setItem(row, 0, new QTableWidgetItem(strlist.at(0)));
	ui.CJTable->setItem(row, 1, new QTableWidgetItem(strlist.at(1)));
	ui.CJTable->setItem(row, 2, new QTableWidgetItem(buysell));
	ui.CJTable->setItem(row, 3, new QTableWidgetItem(openclose));
	ui.CJTable->setItem(row, 4, new QTableWidgetItem(strlist.at(4)));
	ui.CJTable->setItem(row, 5, new QTableWidgetItem(strlist.at(5)));
	ui.CJTable->setItem(row, 6, new QTableWidgetItem(strlist.at(6)));
	ui.CJTable->setItem(row, 7, new QTableWidgetItem(strlist.at(7)));
}

//接收到委托数据
void ctp::ReceiveWT( QString WTData)
{
	QStringList strlist=WTData.split(",");
	//if(strlist.at(7)=="")return;

	QString buysell="";
	QString openclose="";

	if(strlist.at(2)=="0") 
	{
		buysell=QString::fromLocal8Bit("买入");
	}
	else
	{
		buysell=QString::fromLocal8Bit("卖出");
	}
	if(strlist.at(3)=="0") 
	{
		openclose=QString::fromLocal8Bit("开仓");
	}
	else if (strlist.at(3)=="4") 
	{
		openclose=QString::fromLocal8Bit("平昨");
	}
	else
	{
		openclose=QString::fromLocal8Bit("平今");
	}

	for (int i=0;i<ui.WTTable->rowCount();i++)
	{
		if (ui.WTTable->item(i,7)->text()==strlist.at(7))
		{
			ui.WTTable->setItem(i,0,new QTableWidgetItem(strlist.at(0)));
			ui.WTTable->setItem(i,1,new QTableWidgetItem(strlist.at(1)));
			ui.WTTable->setItem(i,2,new QTableWidgetItem(buysell));
			ui.WTTable->setItem(i,3,new QTableWidgetItem(openclose));
			ui.WTTable->setItem(i,4,new QTableWidgetItem(strlist.at(4)));
			ui.WTTable->setItem(i,5,new QTableWidgetItem(strlist.at(5)));
			ui.WTTable->setItem(i,6,new QTableWidgetItem(strlist.at(6)));
			ui.WTTable->setItem(i,7,new QTableWidgetItem(strlist.at(7)));
			ui.WTTable->setItem(i,8,new QTableWidgetItem(strlist.at(8)));
			return;
		}
	}

	int row=ui.WTTable->rowCount();
	ui.WTTable->insertRow(row);
	ui.WTTable->setItem(row,0,new QTableWidgetItem(strlist.at(0)));
	ui.WTTable->setItem(row,1,new QTableWidgetItem(strlist.at(1)));
	ui.WTTable->setItem(row,2,new QTableWidgetItem(buysell));
	ui.WTTable->setItem(row,3,new QTableWidgetItem(openclose));
	ui.WTTable->setItem(row,4,new QTableWidgetItem(strlist.at(4)));
	ui.WTTable->setItem(row,5,new QTableWidgetItem(strlist.at(5)));
	ui.WTTable->setItem(row,6,new QTableWidgetItem(strlist.at(6)));
	ui.WTTable->setItem(row,7,new QTableWidgetItem(strlist.at(7)));
	ui.WTTable->setItem(row,8,new QTableWidgetItem(strlist.at(8)));
}

//接收到持仓数据
void ctp::ReceiveCC( QString CCData)
{
	QString lx;
	QStringList strlist=CCData.split(",");
	if(strlist.at(1)=="2"){lx=QString::fromLocal8Bit("买");}
	if(strlist.at(1)=="3"){lx=QString::fromLocal8Bit("卖");}

	for (int i=0;i<ui.CCTable->rowCount();i++)
	{
		if (ui.CCTable->item(i,0)->text()==strlist.at(0)  && ui.CCTable->item(i,1)->text()==lx)
		{
			ui.CCTable->setItem(i,2,new QTableWidgetItem(strlist.at(2)));
			ui.CCTable->setItem(i,3,new QTableWidgetItem(strlist.at(3)));
			return;
		}
	}

	int row=ui.CCTable->rowCount();
	ui.CCTable->insertRow(row);
	ui.CCTable->setItem(row,0,new QTableWidgetItem(strlist.at(0)));
	ui.CCTable->setItem(row,1,new QTableWidgetItem(lx));
	ui.CCTable->setItem(row,2,new QTableWidgetItem(strlist.at(2)));
	ui.CCTable->setItem(row,3,new QTableWidgetItem(strlist.at(3)));
}

//接收到资金数据
void ctp::ReceiveZJ( QString ZJData)
{
	QStringList strlist=ZJData.split(",");

	for (int i=0;i<ui.ZJTable->rowCount();i++)
	{
		if (ui.ZJTable->item(i,0)->text()==strlist.at(0))
		{
			ui.ZJTable->setItem(i,1,new QTableWidgetItem(strlist.at(1)));
			ui.ZJTable->setItem(i,2,new QTableWidgetItem(strlist.at(2)));
			ui.ZJTable->setItem(i,3,new QTableWidgetItem(strlist.at(3)));
			ui.ZJTable->setItem(i,4,new QTableWidgetItem(strlist.at(4)));
			return;
		}
	}
	int row=ui.ZJTable->rowCount();
	ui.ZJTable->insertRow(row);
	ui.ZJTable->setItem(row,0,new QTableWidgetItem(strlist.at(0)));
	ui.ZJTable->setItem(row,1,new QTableWidgetItem(strlist.at(1)));
	ui.ZJTable->setItem(row,2,new QTableWidgetItem(strlist.at(2)));
	ui.ZJTable->setItem(row,3,new QTableWidgetItem(strlist.at(3)));
	ui.ZJTable->setItem(row,4,new QTableWidgetItem(strlist.at(4)));
}

//接收到合约数据
void ctp::ReceiveHY( QString HYData)
{
	QStringList strlist=HYData.split(",");
	int row=ui.HYTable->rowCount();
	ui.HYTable->insertRow(row);
	ui.HYTable->setItem(row, 0, new QTableWidgetItem(strlist.at(0)));
	ui.HYTable->setItem(row, 1, new QTableWidgetItem(strlist.at(1)));
	ui.HYTable->setItem(row, 2, new QTableWidgetItem(strlist.at(2)));
	ui.HYTable->setItem(row, 3, new QTableWidgetItem(strlist.at(12)));
	ui.HYTable->setItem(row, 4, new QTableWidgetItem(strlist.at(13)));

	WriteTxt("hy.txt",HYData);
}

//下单操作
void ctp::xd()
{
	QString dm=ui.Editdm->text();
	int index = ui.comboJYS->currentIndex();
	QString jys = ui.comboJYS->itemText(index);
	int lots=ui.EditLots->text().toInt();
	QString lx;
	double wtprice;

	//下单类型
	index=ui.comboXd->currentIndex();
	if (index == 0)
		lx = "kd";	//开多
	else if (index == 1)
		lx = "pd";	//平多
	else if (index == 2)
		lx = "kk";	//开空
	else if (index == 3)
		lx = "pk";	//平空
	else
		return;

	if (ui.radioSJ->isChecked())	//市价
	{
		if (index == 0)	//开多
			wtprice = ui.labelAsk->text().toDouble();
		else if (index == 1)	//平多
			wtprice = ui.labelBid->text().toDouble();
		else if (index == 2)	//开空
			wtprice = ui.labelBid->text().toDouble();
		else if (index == 3)	//平空
			wtprice = ui.labelAsk->text().toDouble();
	}
	else if (ui.radioXJ->isChecked())	//限价
	{
		wtprice = ui.EditXj->text().toDouble();
	}
	else
		return;

	gTd->ReqOrderInsert(dm, jys, lx, lots, wtprice, "pz");
//	td1->ReqOrderInsert(dm,lx,lots,wtprice);
}

//委托列表上的弹出式菜单
void ctp::OnWTMenu( const QPoint& pt )
{
	QMenu menu;
	menu.addAction(ui.actioncd);
	menu.exec(ui.WTTable->mapToGlobal(pt));
}

//撤单操作
void ctp::cd()
{
	int i=ui.WTTable->currentIndex().row();
	if (i < 0)
		return;

	QString wth=ui.WTTable->item(i,7)->text().trimmed();
	QString jys=ui.WTTable->item(i,8)->text();

	gTd->ReqOrderAction(gTd->jy.BROKER_ID,wth,jys);

	QMessageBox::information(this,"",QString::fromLocal8Bit("已撤单"));
}

void ctp::xddm()
{
	int i=ui.HQTable->currentIndex().row();
	int k=ui.HQTable->currentIndex().column();
	QString str = ui.HQTable->item(i,0)->text();
	ui.Editdm->setText(str);
}

CString ctp::GetAppPath()
{
	TCHAR modulePath[MAX_PATH];
	GetModuleFileName(NULL,modulePath,MAX_PATH);
	CString strModulePath(modulePath);
	strModulePath=strModulePath.Left(strModulePath.ReverseFind(_T('\\')));
	return strModulePath;
}

void ctp::WriteTxt(QString path,QString data)
{
	QFile mydata(path);
	if(mydata.open(QFile::WriteOnly | QIODevice::Append))
	{
		QTextStream out(&mydata);
		out<<data+"\r\n";
		mydata.close();
	}
}

void ctp::RecieveDELCC( QString )
{
	ui.CCTable->setRowCount(0);
}

void ctp::testFunction1()
{
	/*if (ui.CJTable->rowCount()>0)
	{
		gTd->ReqQryInvestorPosition();
	}*/
}

void ctp::testFunction2()
{
	/*if (ui.CJTable->rowCount()>0)
	{
		gTd->ReqQryTradingAccount();
	}*/
}

void ctp::set_item_style(QTableWidget *table, int row, int column,
	QString name, QFont font, QColor txc, QColor bkc)
{
	QTableWidgetItem *item = new QTableWidgetItem(name);
	if (item == NULL)
		return;

	item->setFont(font);
	item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	item->setBackgroundColor(bkc);
	item->setTextColor(txc);

	table->setItem(row, column, item);
}

void ctp::display_log()
{
	QString str;

	while (1)
	{
		int ret = simulator_log.display_log(str);
		if (ret == 0)
			break;

		time_t ltime;
		time(&ltime);
		struct tm *ptr = localtime(&ltime);

		QString prt;
		prt.sprintf("%02d:%02d:%02d ", ptr->tm_hour, ptr->tm_min, ptr->tm_sec);
		prt += str;
		int row_count = ui.tw_Log->rowCount();
		if (row_count >= 300)
		{
			ui.tw_Log->removeRow(0);
			row_count = ui.tw_Log->rowCount();
		}
		ui.tw_Log->setRowCount(row_count + 1);
		//插入到最后
		QFont font = QFont("Courier");
		font.setBold(false);
		font.setPixelSize(12);
		QColor bk_color = QColor(210, 232, 247);
		QColor tx_color = QColor(0, 0, 0);
		//显示当前行
		set_item_style(ui.tw_Log, row_count, 0, prt, font, tx_color, bk_color);

		QScrollBar* bar = ui.tw_Log->verticalScrollBar();
		bar->setValue(bar->maximum());
	}
}

void ctp::OnMainTimer()
{
	//显示日志
	display_log();
}

void ctp::on_btnHQ_clicked()
{
	gMd->SubscribeMarketData();
}

void ctp::on_btnHQQX_clicked()
{
	gMd->SubscribeMarketData();
}

void ctp::on_btnHY_clicked()
{
	//请求查询合约
	gTd->ReqQryInstrument();
}

void ctp::on_btnWT_clicked()
{
	gTd->ReqQryInstrument();
}

void ctp::on_btnCJ_clicked()
{
	gTd->ReqQryInstrument();
}

void ctp::on_btnZJ_clicked()
{
	gTd->ReqQryTradingAccount();
}

void ctp::on_btnCC_clicked()
{
	gTd->ReqQryInvestorPosition();
}