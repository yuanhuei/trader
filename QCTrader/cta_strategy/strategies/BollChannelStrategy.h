#pragma once
#include "../StrategyTemplate.h"
class BarGenerator;
class ArrayManager;
class BollChannelStrategy :
    public StrategyTemplate
{
public:
    BollChannelStrategy(CtaEngine* ctaEngine, std::string strategyName, std::string symbol);
    ~BollChannelStrategy();

    void updateSetting();
    //void onBar(BarData data);
    //初始化
    virtual void onInit();
    //开始 
    virtual void onStart();
    //停止
    virtual void onStop();


    //TICK
    virtual void onTick(TickData Tick);
    //BAR
    void onBar(BarData Bar);

    void on_5min_bar(BarData data);

    //报单回调
    virtual void onOrder(std::shared_ptr<Event_Order>e);
    //成交回调
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

