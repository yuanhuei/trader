#include "CTPConnectWidgets.h"
#include"utility.h"
#include <qvalidator.h>

CTPConnectWidgets::CTPConnectWidgets(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	//setFixedSize();
	// 
	// 这个行编辑只接受从0到100000的整数
	QIntValidator v(0, 100000, this);	
	ui.lineEdit_3->setValidator(&v);
	/*
	QStringList strList;
	strList << "CFFEX" << "SHFE" << "CZCE" << "DCE" << "INE";
	ui.comboBox_exchange->addItems(strList);

	QStringList strList2;
	strList2 << str2qstr_new("多") << str2qstr_new("空") ;
	ui.comboBox_direction->addItems(strList2);

	QStringList strList3;
	strList3 << str2qstr_new("开") << str2qstr_new("平")<< str2qstr_new("平今") << str2qstr_new( "平昨");
	ui.comboBox_offset->addItems(strList3);

	QStringList strList4;
	strList4 << str2qstr_new("限价") << str2qstr_new ("市价") << "STOP" << "FAK"<<"FOK" << str2qstr_new( "询价");
	ui.comboBox_ordertypy->addItems(strList4);


	QStringList strList5;
	strList5 << "CTP";
	ui.comboBox_ordertypy->addItems(strList5);
	*/
}

CTPConnectWidgets::~CTPConnectWidgets()
{
}

void CTPConnectWidgets::buttonOk_clicked()
{

}

void CTPConnectWidgets::buttonCancel_clicked()
{

}

