#ifndef CTP_H
#define CTP_H

#include <QtWidgets/QMainWindow>
#include "ui_ctp.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include <atlstr.h>

using namespace std;

class ctp : public QMainWindow,public CThostFtdcMdSpi
{
	Q_OBJECT

public:
	ctp(QWidget *parent = 0);
	~ctp();

private slots:
		void MDLogin();
		void OnHQCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
		void xd();					//下单操作
		void ReceiveHQ(QString);	//接收到行情数据
		void ReceiveCJ(QString);	//接收到成交数据
		void ReceiveWT(QString);	//接收到委托数据
		void ReceiveCC(QString);	//接收到持仓数据
		void ReceiveZJ(QString);	//接收到资金数据
		void ReceiveHY(QString);
		void RecieveDELCC(QString);
		void OnWTMenu(const QPoint& pt);	//委托列表上的弹出式菜单
		void cd();					//撤单操作
		void xddm();
        CString GetAppPath();
		void testFunction1();
		void testFunction2();
		void OnMainTimer();
		void on_btnHQ_clicked();
		void on_btnHQQX_clicked();
		void on_btnHY_clicked();
		void on_btnWT_clicked();
		void on_btnCJ_clicked();
		void on_btnZJ_clicked();
		void on_btnCC_clicked();

private:
	Ui::ctpClass ui;
    void WriteTxt(QString path,QString data);

	void set_item_style(QTableWidget *table, int row, int column,
		QString name, QFont font, QColor txc, QColor bkc);
	void display_log();
};

#endif // CTP_H
