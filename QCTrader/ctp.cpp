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

	//���ô��ڱ���
	//ui.splitter_4->setStretchFactor(0, 1);
	//ui.splitter_4->setStretchFactor(1, 1);
	ui.splitter_3->setStretchFactor(0, 2);
	ui.splitter_3->setStretchFactor(1, 3);

	//�ź�����
	connect(gMd,SIGNAL(sendData(QString)),this,SLOT(ReceiveHQ(QString)));
	connect(gTd,SIGNAL(sendCJ(QString)),this,SLOT(ReceiveCJ(QString)));
	connect(gTd,SIGNAL(sendWT(QString)),this,SLOT(ReceiveWT(QString)));
	connect(gTd,SIGNAL(sendCC(QString)),this,SLOT(ReceiveCC(QString)));
	connect(gTd,SIGNAL(sendHY(QString)),this,SLOT(ReceiveHY(QString)));
	connect(gTd,SIGNAL(sendZJ(QString)),this,SLOT(ReceiveZJ(QString)));
	connect(gTd,SIGNAL(sendDELCC(QString)),this,SLOT(RecieveDELCC(QString)));
	connect(ui.BtnXd,SIGNAL(clicked()),this,SLOT(xd()));
	connect(ui.HQTable,SIGNAL(cellClicked(int,int)),this,SLOT(xddm()));

	//*****************�����********************//
	ui.HQTable->setColumnCount(12);

	QStringList headerHQ;
	headerHQ.append(QString::fromLocal8Bit("��Լ����"));
	headerHQ.append(QString::fromLocal8Bit("����ʱ��"));
	headerHQ.append(QString::fromLocal8Bit("���¼�"));
	headerHQ.append(QString::fromLocal8Bit("��һ��"));
	headerHQ.append(QString::fromLocal8Bit("��һ��"));
	headerHQ.append(QString::fromLocal8Bit("��һ��"));
	headerHQ.append(QString::fromLocal8Bit("��һ��"));
	headerHQ.append(QString::fromLocal8Bit("�Ƿ�"));
	headerHQ.append(QString::fromLocal8Bit("�ɽ���"));
	headerHQ.append(QString::fromLocal8Bit("��ͣ��"));
	headerHQ.append(QString::fromLocal8Bit("��ͣ��"));
	headerHQ.append(QString::fromLocal8Bit("������"));

	ui.HQTable->setHorizontalHeaderLabels(headerHQ);
	//ui.HQTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.HQTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.HQTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************ί�б�********************//
	ui.WTTable->setColumnCount(9);

	QStringList headerWT;
	headerWT.append(QString::fromLocal8Bit("ί��ʱ��"));
	headerWT.append(QString::fromLocal8Bit("��Լ����"));
	headerWT.append(QString::fromLocal8Bit("����"));
	headerWT.append(QString::fromLocal8Bit("��ƽ"));
	headerWT.append(QString::fromLocal8Bit("����"));
	headerWT.append(QString::fromLocal8Bit("�۸�"));
	headerWT.append(QString::fromLocal8Bit("״̬"));
	headerWT.append(QString::fromLocal8Bit("ί�к�"));
	headerWT.append(QString::fromLocal8Bit("������"));

	ui.WTTable->setHorizontalHeaderLabels(headerWT);
	//ui.WTTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.WTTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.WTTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************�ɽ���********************//
	ui.CJTable->setColumnCount(8);
	QStringList headerCJ;
	headerCJ.append(QString::fromLocal8Bit("�ɽ�ʱ��"));
	headerCJ.append(QString::fromLocal8Bit("��Լ����"));
	headerCJ.append(QString::fromLocal8Bit("����"));
	headerCJ.append(QString::fromLocal8Bit("��ƽ"));
	headerCJ.append(QString::fromLocal8Bit("����"));
	headerCJ.append(QString::fromLocal8Bit("�۸�"));
	headerCJ.append(QString::fromLocal8Bit("ί�к�"));
	headerCJ.append(QString::fromLocal8Bit("������"));

	ui.CJTable->setHorizontalHeaderLabels(headerCJ);
	ui.CJTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.CJTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.CJTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************�ֱֲ�********************//
	ui.CCTable->setColumnCount(4);
	QStringList headerCC;
	headerCC.append(QString::fromLocal8Bit("��Լ����"));
	headerCC.append(QString::fromLocal8Bit("�ֲ�����"));
	headerCC.append(QString::fromLocal8Bit("�ֲ�����"));
	headerCC.append(QString::fromLocal8Bit("�ֲֳɱ�"));

	ui.CCTable->setHorizontalHeaderLabels(headerCC);
	//ui.CCTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.CCTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.CCTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************�ʽ��********************//
	ui.ZJTable->setColumnCount(5);

	QStringList headerZJ;
	headerZJ.append(QString::fromLocal8Bit("�˺�"));
	headerZJ.append(QString::fromLocal8Bit("��Ȩ��"));
	headerZJ.append(QString::fromLocal8Bit("ռ�ñ�֤��"));
	headerZJ.append(QString::fromLocal8Bit("�����ʽ�"));
	headerZJ.append(QString::fromLocal8Bit("���ն�"));

	ui.ZJTable->setHorizontalHeaderLabels(headerZJ);
	ui.ZJTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.ZJTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.ZJTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//*****************��Լ��********************//
	ui.HYTable->setColumnCount(5);

	QStringList headerHY;
	headerHY.append(QString::fromLocal8Bit("��Լ����"));
	headerHY.append(QString::fromLocal8Bit("������"));
	headerHY.append(QString::fromLocal8Bit("��Լ����"));
	headerHY.append(QString::fromLocal8Bit("��Լ����"));
	headerHY.append(QString::fromLocal8Bit("��Լ����"));

	ui.HYTable->setHorizontalHeaderLabels(headerHY);
	//ui.HYTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.HYTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.HYTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//Ĭ�ϴ�����ҳ
	//ui.tabWidget->setCurrentIndex(0);

	//�µ����ֳ�ʼ��
	ui.Editdm->setText("");
	ui.EditLots->setText("1");
	ui.radioSJ->setChecked(true);
	ui.radioPJ->setChecked(true);

	//������־��ʾ��
	QString data_style;
	data_style.sprintf("gridline-color: rgb(210,232,247);background-color:rgb(210,232,247)");
	ui.tw_Log->setStyleSheet(data_style);
	ui.tw_Log->horizontalHeader()->setStretchLastSection(true);
	ui.tw_Log->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tw_Log->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui.tw_Log->setColumnCount(1);
	ui.tw_Log->verticalHeader()->setDefaultSectionSize(16);
	//��ֹ�޸�
	ui.tw_Log->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//�����б�ͷ
	ui.tw_Log->verticalHeader()->setVisible(false);

	//�Ҽ��˵�
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

//���յ���������
void ctp::ReceiveHQ(QString TICK)
{
	QStringList strlist=TICK.split(",");

	//�޸��µ�����
	if(strlist.at(0)==ui.Editdm->text())
	{
		ui.labelAsk->setText(strlist.at(5));
		ui.labelLast->setText(strlist.at(2));
		ui.labelBid->setText(strlist.at(3));
		ui.labelUP->setText(strlist.at(9));
		ui.labelDOWN->setText(strlist.at(10));
	}
	
	//�������б��в��Ҵ���
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

	//����������б���û�ҵ�������һ��
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

//���յ��ɽ�����
void ctp::ReceiveCJ(QString CJData)
{
	QStringList strlist = CJData.split(",");
	QString buysell = "";
	QString openclose = "";

	if (strlist.at(2) == "0")
	{
		buysell = QString::fromLocal8Bit("����");
	}
	else
	{
		buysell = QString::fromLocal8Bit("����");
	}
	if (strlist.at(3) == "0")
	{
		openclose = QString::fromLocal8Bit("����");
	}
	else if (strlist.at(3) == "4")
	{
		openclose = QString::fromLocal8Bit("ƽ��");
	}
	else
	{
		openclose = QString::fromLocal8Bit("ƽ��");
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

//���յ�ί������
void ctp::ReceiveWT( QString WTData)
{
	QStringList strlist=WTData.split(",");
	//if(strlist.at(7)=="")return;

	QString buysell="";
	QString openclose="";

	if(strlist.at(2)=="0") 
	{
		buysell=QString::fromLocal8Bit("����");
	}
	else
	{
		buysell=QString::fromLocal8Bit("����");
	}
	if(strlist.at(3)=="0") 
	{
		openclose=QString::fromLocal8Bit("����");
	}
	else if (strlist.at(3)=="4") 
	{
		openclose=QString::fromLocal8Bit("ƽ��");
	}
	else
	{
		openclose=QString::fromLocal8Bit("ƽ��");
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

//���յ��ֲ�����
void ctp::ReceiveCC( QString CCData)
{
	QString lx;
	QStringList strlist=CCData.split(",");
	if(strlist.at(1)=="2"){lx=QString::fromLocal8Bit("��");}
	if(strlist.at(1)=="3"){lx=QString::fromLocal8Bit("��");}

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

//���յ��ʽ�����
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

//���յ���Լ����
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

//�µ�����
void ctp::xd()
{
	QString dm=ui.Editdm->text();
	int index = ui.comboJYS->currentIndex();
	QString jys = ui.comboJYS->itemText(index);
	int lots=ui.EditLots->text().toInt();
	QString lx;
	double wtprice;

	//�µ�����
	index=ui.comboXd->currentIndex();
	if (index == 0)
		lx = "kd";	//����
	else if (index == 1)
		lx = "pd";	//ƽ��
	else if (index == 2)
		lx = "kk";	//����
	else if (index == 3)
		lx = "pk";	//ƽ��
	else
		return;

	if (ui.radioSJ->isChecked())	//�м�
	{
		if (index == 0)	//����
			wtprice = ui.labelAsk->text().toDouble();
		else if (index == 1)	//ƽ��
			wtprice = ui.labelBid->text().toDouble();
		else if (index == 2)	//����
			wtprice = ui.labelBid->text().toDouble();
		else if (index == 3)	//ƽ��
			wtprice = ui.labelAsk->text().toDouble();
	}
	else if (ui.radioXJ->isChecked())	//�޼�
	{
		wtprice = ui.EditXj->text().toDouble();
	}
	else
		return;

	gTd->ReqOrderInsert(dm, jys, lx, lots, wtprice, "pz");
//	td1->ReqOrderInsert(dm,lx,lots,wtprice);
}

//ί���б��ϵĵ���ʽ�˵�
void ctp::OnWTMenu( const QPoint& pt )
{
	QMenu menu;
	menu.addAction(ui.actioncd);
	menu.exec(ui.WTTable->mapToGlobal(pt));
}

//��������
void ctp::cd()
{
	int i=ui.WTTable->currentIndex().row();
	if (i < 0)
		return;

	QString wth=ui.WTTable->item(i,7)->text().trimmed();
	QString jys=ui.WTTable->item(i,8)->text();

	gTd->ReqOrderAction(gTd->jy.BROKER_ID,wth,jys);

	QMessageBox::information(this,"",QString::fromLocal8Bit("�ѳ���"));
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
		//���뵽���
		QFont font = QFont("Courier");
		font.setBold(false);
		font.setPixelSize(12);
		QColor bk_color = QColor(210, 232, 247);
		QColor tx_color = QColor(0, 0, 0);
		//��ʾ��ǰ��
		set_item_style(ui.tw_Log, row_count, 0, prt, font, tx_color, bk_color);

		QScrollBar* bar = ui.tw_Log->verticalScrollBar();
		bar->setValue(bar->maximum());
	}
}

void ctp::OnMainTimer()
{
	//��ʾ��־
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
	//�����ѯ��Լ
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