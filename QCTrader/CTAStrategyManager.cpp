#include "CTAStrategyManager.h"

#include <string>  
#include <json.h>
#include <iostream>  
#include <fstream>  

CTAStrategyManager::CTAStrategyManager(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_mainwindow = (MainWindow*)parent;
	setUI();

}

CTAStrategyManager::~CTAStrategyManager()
{
}

void CTAStrategyManager::ReadFileJson()
{
	Json::Reader reader;
	Json::Value root;

	//从文件中读取，保证当前文件有demo.json文件  
	std::ifstream in("./Strategy/cta_strategy_setting.json",std::ios::binary);

	if (!in.is_open())
	{
		std::cout << "Error opening file\n";
		return;
	}

	if (reader.parse(in, root))
	{
		//读取根节点信息  
		std::string name = root["strategy_name"].asString();
		std::string vt_symbol = root["vt_symbol"].asString();
		//std::string name = root["strategy_name"].asString();
		//int age = root["age"].asInt();
		//std::string sex = root["sex"].asString();


		//读取子节点信息  
		std::string friend_name = root["setting"]["friend_name"].asString();
		int friend_age = root["friends"]["friend_age"].asInt();
		std::string friend_sex = root["friends"]["friend_sex"].asString();

		std::cout << "My friend's name is " << friend_name << std::endl;
		std::cout << "My friend's sex is " << friend_sex << std::endl;
		std::cout << "My friend is " << friend_age << " years old" << std::endl;

		//读取数组信息  
		std::cout << "Here's my hobby:" << std::endl;
		for (unsigned int i = 0; i < root["hobby"].size(); i++)
		{
			std::string ach = root["hobby"][i].asString();
			std::cout << ach << '\t';
		}
		std::cout << std::endl;

		std::cout << "Reading Complete!" << std::endl;
	}
	else
	{
		std::cout << "parse error\n" << std::endl;
	}

	in.close();
}

void CTAStrategyManager::setUI()
{
	//ui.comboBox->addItem();

}
void CTAStrategyManager::addStrategy_clicked()
{


}

void CTAStrategyManager::initStrategy_clicked()
{

}
void CTAStrategyManager::startStrategy_cliced()
{


}
void CTAStrategyManager::stopStragegy_clicked()
{

}
void CTAStrategyManager::startAllStrategy_clicked()
{

}
void CTAStrategyManager::stopAllStrategy_clicked() {

}
void CTAStrategyManager::clearLog() {

}