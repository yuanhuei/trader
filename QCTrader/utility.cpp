#include<utility.h>

//#include<qstring.h>
#include<qdatetime.h>

QString str2qstr_new(std::string str)
{
	return QString::fromLocal8Bit(str.c_str());
}


BarGenerator::BarGenerator(ON_FUNC onBar_Func, int iWindow, ON_FUNC onWindowBar_FUNC, Interval iInterval)
{



}
BarGenerator::~BarGenerator()
{


}

void BarGenerator::updateTick(TickData* tickData)
{
	bool bNewMinute = false; //是否是新的一分钟
    QDateTime tickDateTime; //转换为QDataTime主要是为了比较大小，早晚
    QDateTime lasttickDateTime;
    QDateTime barDateTime;
    if(m_Bar!=NULL)
        //将std::string的时间转换为QDataTime
        barDateTime = QDateTime::fromString(QString::fromStdString(m_Bar->date + " " + m_Bar->time), "yyyy--mm--dd hh:mm:ss");

   // Filter tick data with 0 last price过滤垃圾tick
    if (tickData->lastprice==0)
        return;
    
    // Filter tick data with older timestamp过滤垃圾tick
    if (m_lastTick != NULL)
    {
        tickDateTime = QDateTime::fromString(QString::fromStdString(tickData->date + " " + tickData->time), "yyyy--mm--dd hh:mm:ss");
        lasttickDateTime = QDateTime::fromString(QString::fromStdString(m_lastTick->date + " " + m_lastTick->time), "yyyy--mm--dd hh:mm:ss");
        if(tickDateTime< lasttickDateTime) //新来的tick的时间要比上一个tick时间早，过滤掉
            return;
    }
    //给bNewMinute赋值 true，分两种情况，一种是新的tick和当前的一分种bar时间不在一个分钟里面，还有就是当前一分钟bar还没有生成
    //这两种情况都需要新new 一个分钟bar
    if (m_Bar == NULL)
    {
        bNewMinute = true;

    }
    else if(barDateTime.date()!= tickDateTime.date()|| barDateTime.time().hour()!=tickDateTime.time().hour()||barDateTime.time().minute()!=tickDateTime.time().minute() )

    {
        //日期，小时，分钟只要有一个不同，tick就属于不同的minute了，需要合成一个bar了
        this->m_onBar_Func(m_Bar);
        bNewMinute = true;

    }
    if (bNewMinute == true)
    {

        m_Bar = new BarData();
        m_Bar->symbol = tickData->symbol;
        m_Bar->exchange = tickData->exchange;
        m_Bar->interval = MINUTE;
        m_Bar->open = tickData->lastprice;
        m_Bar->close = tickData->lastprice;
        m_Bar->high = tickData->lastprice;
        m_Bar->low = tickData->lastprice;
        m_Bar->date = tickData->date;
        m_Bar->time = tickData->time;
        m_Bar->openInterest = tickData->openInterest;
     
    }
    else//不是新的bar，只需要更新高低点，持仓量
    {
        m_Bar->high =std::max(m_Bar->high, tickData->lastprice);
        if(tickData->highPrice>m_lastTick->highPrice)
            m_Bar->high=std::max(m_Bar->high, tickData->highPrice);

        m_Bar->low = std::min(m_Bar->low, tickData->lastprice);
        if (tickData->lowPrice < m_lastTick->lowerLimit)
            m_Bar->low = std::min(m_Bar->low, tickData->lowerLimit);

        m_Bar->close = tickData->lastprice;
        m_Bar->openInterest = tickData->openInterest;

    }

    if (m_lastTick != NULL)
    {
        double volume_change = tickData->volume - m_lastTick->volume;
        m_Bar->volume= m_Bar->volume+std::max(volume_change, 0.0);

    }
    else//第一次来tick，需要new一个lastTick
        m_lastTick = new TickData();
    //把推送的tick复制为lastTick，下次会用
    *m_lastTick = *tickData;

}
void BarGenerator::updateBar(BarData* barData)
{


}
