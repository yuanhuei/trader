#include "CTPConnectWidgets.h"
#include"utility.h"

CTPConnectWidgets::CTPConnectWidgets(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	//setFixedSize();
	/*
	QStringList strList;
	strList << "CFFEX" << "SHFE" << "CZCE" << "DCE" << "INE";
	ui.comboBox_exchange->addItems(strList);

	QStringList strList2;
	strList2 << str2qstr_new("��") << str2qstr_new("��") ;
	ui.comboBox_direction->addItems(strList2);

	QStringList strList3;
	strList3 << str2qstr_new("��") << str2qstr_new("ƽ")<< str2qstr_new("ƽ��") << str2qstr_new( "ƽ��");
	ui.comboBox_offset->addItems(strList3);

	QStringList strList4;
	strList4 << str2qstr_new("�޼�") << str2qstr_new ("�м�") << "STOP" << "FAK"<<"FOK" << str2qstr_new( "ѯ��");
	ui.comboBox_ordertypy->addItems(strList4);


	QStringList strList5;
	strList5 << "CTP";
	ui.comboBox_ordertypy->addItems(strList5);
	*/
}

CTPConnectWidgets::~CTPConnectWidgets()
{
}

void CTPConnectWidgets::SendOrderbuttonclicked()
{

}

void CTPConnectWidgets::CancelOrderbutttonclicded()
{

}

