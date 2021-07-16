#pragma once
#include "../StrategyTemplate.h"
class BarGenerator;
class ArrayManager;

class BollChannelStrategy :
    public StrategyTemplate
{
public:
    BollChannelStrategy(BaseEngine* ctaEngine, std::string strategyName, std::string symbol);
    ~BollChannelStrategy();

    void updateSetting();
    //void onBar(BarData data);
    //��ʼ��
    virtual void onInit();
    //��ʼ 
    virtual void onStart();
    //ֹͣ
    virtual void onStop();


    //TICK
    virtual void onTick(TickData Tick);
    //BAR
    void onBar(BarData Bar);

    void on_5min_bar(BarData data);

    //�����ص�
    virtual void onOrder(std::shared_ptr<Event_Order>e);
    //�ɽ��ص�
    virtual void onTrade(std::shared_ptr<Event_Trade>e);


public:
    BarGenerator *m_BarGenerate;
    ArrayManager *m_ArrayManager;

    int     boll_window = 18;
    float   boll_dev = 3.4;
    int     cci_window = 10;
    int     atr_window = 30;
    float   sl_multiplier = 5.2;
    int     fixed_size = 1;

    float    boll_up = 0;
    float    boll_down = 0;
    float    cci_value = 0;
    float    atr_value = 0;

    float    intra_trade_high = 0;
    float    intra_trade_low = 0;
    float    long_stop = 0;
    float    short_stop = 0;

};

