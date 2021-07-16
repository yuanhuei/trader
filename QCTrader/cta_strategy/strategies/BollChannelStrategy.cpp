#include "BollChannelStrategy.h"
#include"utility.h"
#include <algorithm>

BollChannelStrategy::BollChannelStrategy(BaseEngine* ctaEngine, std::string strategyName, std::string symbol):
	StrategyTemplate(ctaEngine, strategyName, symbol)
{
	Interval iInterval = MINUTE;
	ON_Functional on_func1, on_fun2;
	on_func1 = std::bind(&BollChannelStrategy::onBar, this, std::placeholders::_1);
	on_fun2 = std::bind(&BollChannelStrategy::on_5min_bar, this, std::placeholders::_1);

	m_BarGenerate = new BarGenerator(on_func1, 5, on_fun2, iInterval);
	m_ArrayManager = new ArrayManager();

	//m_Pos = 0;
}

BollChannelStrategy::~BollChannelStrategy()
{

	delete m_BarGenerate;
	delete m_ArrayManager;
}
void BollChannelStrategy::updateSetting()
{
	int     boll_window = 18;
	float   boll_dev = 3.4;
	int     cci_window = 10;
	int     atr_window = 30;
	float   sl_multiplier = 5.2;
	int     fixed_size = 1;
	//���ñ�����ֵ
	std::map<std::string, std::string> settingMap = m_strategydata->getallparam();
	std::map<std::string, std::string>::iterator iter;
	if ((iter=settingMap.find("boll_window")) != settingMap.end())
		boll_window = std::stoi(iter->second);
	if ((iter = settingMap.find("boll_dev")) != settingMap.end())
		boll_dev = std::stof(iter->second);
	if ((iter = settingMap.find("cci_window")) != settingMap.end())
		cci_window = std::stoi(iter->second);
	if ((iter = settingMap.find("atr_window")) != settingMap.end())
		atr_window = std::stoi(iter->second);
	if ((iter = settingMap.find("sl_multiplier")) != settingMap.end())
		sl_multiplier = std::stof(iter->second);
	if ((iter = settingMap.find("fixed_size")) != settingMap.end())
		fixed_size = std::stoi(iter->second);
	//���б���ֻ��һ�������ǲ�λ
	settingMap = m_strategydata->getallvar();
	if(settingMap.size()>0)
	{
		if ((iter = settingMap.find("pos")) != settingMap.end())
			m_Pos = std::stoi(iter->second);
	}
}

void BollChannelStrategy::onTick(TickData Tick)
{
	m_BarGenerate->updateTick(&Tick);

}
void BollChannelStrategy::onBar(BarData data)
{
	m_BarGenerate->updateBar(&data);

}
void BollChannelStrategy::on_5min_bar(BarData data)
{

	m_ArrayManager->update_bar(data);
	if (m_ArrayManager->m_iInit != true)
		return;
	inited = true;//��Ҫ�������inited��ֵ
	cancelAllOrder();

	std::map<std::string, double>mapBoll;
	mapBoll= m_ArrayManager->boll(boll_window, boll_dev);
	boll_up = mapBoll["boll_up"];
	boll_down = mapBoll["boll_down"];
	cci_value = m_ArrayManager->cci(cci_window);
	atr_value = m_ArrayManager->atr(atr_window);
	
	if (m_Pos == 0)
	{

		intra_trade_high = data.high;
		intra_trade_low = data.low;

		if (cci_value > 0)
			buy(boll_up, fixed_size,true);
		else if (cci_value < 0)
			sellshort(boll_down, fixed_size, true);
	}
	else if (m_Pos > 0)
	{
		intra_trade_high = std::max(intra_trade_high, float(data.high));
		intra_trade_low = data.low;

		long_stop = intra_trade_high - atr_value * sl_multiplier;
		sell(long_stop, std::abs(m_Pos), true);


	}
	else
	{
		intra_trade_high = data.high;
		intra_trade_low = std::min(intra_trade_low, float(data.low));
		short_stop = intra_trade_low + atr_value * sl_multiplier;
		buycover(short_stop,std::abs(m_Pos), true);
	}

				
	putEvent();

}

 
void BollChannelStrategy::onOrder(std::shared_ptr<Event_Order>e)
{
	m_ctaEngine->writeCtaLog("ί�е��ύ�ɹ�");
}


void BollChannelStrategy::onInit()
{
	//������ʷ���ݳ�ʼ��
	m_ctaEngine->writeCtaLog("���Գ�ʼ��:" + m_strategyName);
	std::vector<BarData>datalist = loadBar(m_symbol, initDays=10);
	for (std::vector<BarData>::iterator it = datalist.begin(); it != datalist.end(); it++)
	{
		onBar(*it);
	}
	//inited = true; ͨ��on_30min_bar���ж�if (m_ArrayManager->m_iInit != true)���inited��ֵtrue��������������bar��������û����ɳ�ʼ������Ӧ�ø�ֵ
}

void BollChannelStrategy::onStart()
{
	StrategyTemplate::onStart();
}
void BollChannelStrategy::onStop()
{
	StrategyTemplate::onStop();
}

void BollChannelStrategy::onTrade(std::shared_ptr<Event_Trade>e)
{
	/*
	if (e->direction == DIRECTION_LONG && e->offset == OFFSET_OPEN)
		m_Pos = e->volume;
	else if (e->direction == DIRECTION_LONG && e->offset == OFFSET_CLOSE)
		m_Pos = m_Pos- e->volume;
	else if (e->direction == DIRECTION_SHORT && e->offset == OFFSET_OPEN)
		m_Pos = -e->volume;
	else if (e->direction == DIRECTION_SHORT && e->offset == OFFSET_CLOSE)
		m_Pos= m_Pos + e->volume;
		*/
	m_ctaEngine->writeCtaLog("�����ɽ�");

}