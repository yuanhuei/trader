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
		void xd();					//�µ�����
		void ReceiveHQ(QString);	//���յ���������
		void ReceiveCJ(QString);	//���յ��ɽ�����
		void ReceiveWT(QString);	//���յ�ί������
		void ReceiveCC(QString);	//���յ��ֲ�����
		void ReceiveZJ(QString);	//���յ��ʽ�����
		void ReceiveHY(QString);
		void RecieveDELCC(QString);
		void OnWTMenu(const QPoint& pt);	//ί���б��ϵĵ���ʽ�˵�
		void cd();					//��������
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
