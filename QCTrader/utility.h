#pragma once

#ifndef  UTILITY_H
#define UTILITY_H
#include<String>
#include<QString>
#include"qcstructs.h"

QString str2qstr_new(std::string str);
typedef void(*ON_FUNC)(BarData*);

typedef enum 
{
    MINUTE=0,
    HOUR=1,
    DAILY=2,
    WEEKLY=3 ,
    TICK=4 
}Interval;

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
     BarGenerator(ON_FUNC onBar_Func,int iWindow, ON_FUNC onWindowBar_FUNC,Interval iInterval);
     ~BarGenerator();

private:

    BarData*  m_Bar; //当前的一分钟bar
    BarData* m_windowBar;//合成的x分钟的bar
    ON_FUNC m_onBar_Func;
    ON_FUNC m_onWindowBar_FUNC;

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
