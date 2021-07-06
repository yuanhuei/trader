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
	bool bNewMinute = false;
    QDateTime tickDateTime;
    QDateTime lasttickDateTime;
    QDateTime barDateTime;
    if(m_Bar!=NULL)
        barDateTime = QDateTime::fromString(QString::fromStdString(m_Bar->date + " " + m_Bar->time), "yyyy--mm--dd hh:mm:ss");

   // Filter tick data with 0 last price
    if (tickData->lastprice==0)
        return;
    
    // Filter tick data with older timestamp
    if (m_lastTick != NULL)
    {
        tickDateTime = QDateTime::fromString(QString::fromStdString(tickData->date + " " + tickData->time), "yyyy--mm--dd hh:mm:ss");
        lasttickDateTime = QDateTime::fromString(QString::fromStdString(m_lastTick->date + " " + m_lastTick->time), "yyyy--mm--dd hh:mm:ss");
        if(tickDateTime< lasttickDateTime) //������tick��ʱ��Ҫ����һ��tickʱ���磬���˵�
            return;
    }
         
    if (m_Bar == NULL)
    {
        bNewMinute = true;

    }
    else if(barDateTime.date()!= tickDateTime.date()|| barDateTime.time().hour()!=tickDateTime.time().hour()||barDateTime.time().minute()!=tickDateTime.time().minute() )

    {
        //���ڣ�Сʱ������ֻҪ��һ����ͬ��tick�����ڲ�ͬ��minute�ˣ���Ҫ�ϳ�һ��bar��
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
    else
    {
        m_Bar->high =std::max(m_Bar->high, tickData->lastprice);
        if(tickData->highPrice>m_lastTick->highPrice)
            m_Bar->high=std::max(m_Bar->high, tickData->highPrice);


    }

}
void BarGenerator::updateBar(BarData* barData)
{


}
