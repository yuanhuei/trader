#include "CTPConnectWidgets.h"
#include"utility.h"
#include <qvalidator.h>
#include<fstream>
#include<string>
#include<qstring.h>
#include"gateway/ctp_gateway/ctpgateway.h"
#include"gateway/gatewaymanager.h"

CTPConnectWidgets::CTPConnectWidgets(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_mainwindow = (MainWindow*)parent;
	//m_ctpgateway = (CTPGateway*) ((m_mainwindow->m_gatewaymanager)->m_gatewaymap["ctp"]);
	//setFixedSize();
	// 
	// ����б༭ֻ���ܴ�0��100000������
	QIntValidator v(0, 100000, this);	
	ui.lineEdit_3->setValidator(&v);
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

void CTPConnectWidgets::buttonOk_clicked()
{
	std::string strName = (ui.lineEdit->text()).toStdString();
	std::string strPassword = (ui.lineEdit_2->text()).toStdString();
	std::string strBroker = (ui.lineEdit_3->text()).toStdString();
	std::string strTDipaddress = (ui.lineEdit_4->text()).toStdString();
	std::string strMDipaddress = (ui.lineEdit_5->text()).toStdString();
	std::string strProductName = (ui.lineEdit_6->text()).toStdString();
	std::string strAuthCode = (ui.lineEdit_7->text()).toStdString();
	std::string strProductInfo = (ui.lineEdit_8->text()).toStdString();
		
	std::fstream file;
	file.open("./CTPGateway/CTP_connect");
	if (!file)
	{
		m_mainwindow->write_log("���ļ�ʧ��", "MainWindow:CTPConnectWidgets");
	}
	else
	{
		//QString qstr = ui.lineEdit->text();
		//std::string* str =qstr.toStdString;
		file << "username=" << strName.c_str()<<"\n";
		file << "password=" << strPassword.c_str() << "\n";
		file << "brokerid=" << strBroker.c_str() << "\n";
		file << "tdipaddress=" << strTDipaddress.c_str() << "\n";
		file << "mdipaddress=" << strMDipaddress.c_str() << "\n";
		file << "productname=" << strProductName.c_str() << "\n";
		file << "authcode=" << strAuthCode.c_str() << "\n";
		file << "productinfo=" << strProductInfo.c_str() << "\n";
	}
	file.close();
	m_mainwindow->m_gatewaymanager->connect("CTP");
	this->close();
}

void CTPConnectWidgets::buttonCancel_clicked()
{
	this->close();
}

