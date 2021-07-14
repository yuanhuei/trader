#include "ContractQueryManager.h"
#include"MainWindow.h"
#include"./gateway/gatewaymanager.h"
#include"utility.h"

ContractQueryManager::ContractQueryManager(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::CustomizeWindowHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowMaximizeButtonHint);
	setWindowFlags(Qt::Window);

	ui.setupUi(this);
	m_mainwindow = (MainWindow*)parent;
}

ContractQueryManager::~ContractQueryManager()
{
}
void ContractQueryManager::Query_clicked()
{
	QString symbol=ui.lineEdit->text();
	std::shared_ptr<Event_Contract> ptr_contract;
	std::map<std::string, std::shared_ptr<Event_Contract>> allContractMap;
	if (symbol.size() == 0)//没有输入查询所有合约
	{
		ui.tableWidget->clearContents();

		allContractMap=m_mainwindow->m_gatewaymanager->getAllContract();
		std::map<std::string, std::shared_ptr<Event_Contract>>::iterator iter;
		int iRowCount = 0;
		for (iter = allContractMap.begin(); iter != allContractMap.end(); iter++)
		{
			ptr_contract =iter->second;
			ui.tableWidget->insertRow(iRowCount);
			ui.tableWidget->setItem(iRowCount, 0, new QTableWidgetItem(str2qstr_new(ptr_contract->symbol)));
			ui.tableWidget->setItem(iRowCount, 1, new QTableWidgetItem(str2qstr_new(ptr_contract->exchange)));
			ui.tableWidget->setItem(iRowCount, 2, new QTableWidgetItem(str2qstr_new(ptr_contract->name)));
			ui.tableWidget->setItem(iRowCount, 3, new QTableWidgetItem(str2qstr_new(ptr_contract->productClass)));
			int iSize = m_mainwindow->m_gatewaymanager->GetSymbolSize(ptr_contract->symbol, ptr_contract->gatewayname);
			ui.tableWidget->setItem(iRowCount, 4, new QTableWidgetItem(str2qstr_new(std::to_string(iSize))));
			ui.tableWidget->setItem(iRowCount, 5, new QTableWidgetItem(str2qstr_new(std::to_string(ptr_contract->priceTick))));
			ui.tableWidget->setItem(iRowCount, 6, new QTableWidgetItem(str2qstr_new(ptr_contract->gatewayname)));
			iRowCount++;
		}

	}
	else
	{
		ptr_contract = m_mainwindow->m_gatewaymanager->getContract(symbol.toStdString());
		if (ptr_contract != nullptr)
		{
			ui.tableWidget->clearContents();
			ui.tableWidget->insertRow(0);
			ui.tableWidget->setItem(0, 0, new QTableWidgetItem(str2qstr_new(ptr_contract->symbol)));
			ui.tableWidget->setItem(0, 1, new QTableWidgetItem(str2qstr_new(ptr_contract->exchange)));
			ui.tableWidget->setItem(0, 2, new QTableWidgetItem(str2qstr_new(ptr_contract->name)));
			ui.tableWidget->setItem(0, 3, new QTableWidgetItem(str2qstr_new(ptr_contract->productClass)));
			int iSize = m_mainwindow->m_gatewaymanager->GetSymbolSize(symbol.toStdString(), ptr_contract->gatewayname);
			ui.tableWidget->setItem(0, 4, new QTableWidgetItem(str2qstr_new(std::to_string(iSize))));
			ui.tableWidget->setItem(0, 5, new QTableWidgetItem(str2qstr_new(std::to_string(ptr_contract->priceTick))));
			ui.tableWidget->setItem(0, 6, new QTableWidgetItem(str2qstr_new(ptr_contract->gatewayname)));
		}
	}


}