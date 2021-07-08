#pragma once

#ifndef  UTILITY_H
#define UTILITY_H
#include<String>
#include<QString>
#include"qcstructs.h"
#include <functional> 
class BollChannelStrategy;

QString str2qstr_new(std::string str);
//typedef void(StrategyTemplate::*ON_FUNC)(BarData);
typedef std::function<void(BarData)> ON_Functional;


class ArrayManager
{
    /*
    For:
    1. time series container of bar data
    2. calculating technical indicator value
    */
public:
    ArrayManager(int iSize=100);
    ~ArrayManager();
    void update_bar(BarData barData);

    std::vector<float>* Get_openprice_array();
    std::vector<float>* Get_closeprice_array();
    std::vector<float>* Get_highprice_array();
    std::vector<float>* Get_lowprice_array();
    std::vector<float>* Get_volume_array();
    std::vector<float>* Get_openinterest_array();

    float ema(int n);


public:
    int m_iSize;
    int m_iCount;
    bool m_iInit;

    std::vector<float> openprice_array;
    std::vector<float> closeprice_array;
    std::vector<float> highprice_array;
    std::vector<float> lowprice_array;
    std::vector<float> volume_array;
    std::vector<float> openinterest_array;


    std::map<std::string, double> boll(int iWindow, int iDev);
    double sma(int n);
    double cci(int iWindow);
    double atr(int iWindow);
};

class BarGenerator
{
    /*
    For:
    1. generating 1 minute bar data from tick data
        2. generateing x minute bar / x hour bar data from 1 minute data

        Notice :
    1. for x minute bar, x must be able to divide 60 : 2, 3, 5, 6, 10, 15, 20, 30
        2. for x hour bar, x can be any number

     */
public:
     BarGenerator(ON_Functional onBar_Func,int iWindow, ON_Functional onWindowBar_FUNC,Interval iInterval);
     ~BarGenerator();

private:

    BarData*  m_Bar; //当前的一分钟bar
    BarData* m_windowBar;//合成的x分钟的bar
    ON_Functional m_onBar_Func;
    ON_Functional m_onWindowBar_FUNC;

    bool m_bWindowFinished = false;//barwindow 是否合成完成
    Interval m_interval;
    int interval_count = 0;

    int m_iWindow;
    

    TickData* m_lastTick;
    BarData* m_lastBar=NULL;
public:
    void updateTick(TickData* tickData);
    void updateBar(BarData* barData);

};

#endif // ! UTILITY_H
