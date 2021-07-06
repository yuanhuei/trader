#include<utility.h>

//#include<qstring.h>
#include<qdatetime.h>

QString str2qstr_new(std::string str)
{
	return QString::fromLocal8Bit(str.c_str());
}


BarGenerator::BarGenerator(ON_FUNC onBar_Func, int iWindow, ON_FUNC onWindowBar_FUNC, Interval iInterval)
{
    m_onBar_Func = onBar_Func;
    m_onWindowBar_FUNC = onWindowBar_FUNC;
    m_iWindow = iWindow;
    m_interval = iInterval;


}
BarGenerator::~BarGenerator()
{
    delete m_Bar;
    delete m_lastTick;
    delete m_lastBar;
    delete m_windowBar;

}

void BarGenerator::updateTick(TickData* tickData)
{
	bool bNewMinute = false; //�Ƿ����µ�һ����
    QDateTime tickDateTime; //ת��ΪQDataTime��Ҫ��Ϊ�˱Ƚϴ�С������
    QDateTime lasttickDateTime;
    QDateTime barDateTime;
    if(m_Bar!=NULL)
        //��std::string��ʱ��ת��ΪQDataTime
        barDateTime = QDateTime::fromString(QString::fromStdString(m_Bar->date + " " + m_Bar->time), "yyyy--mm--dd hh:mm:ss");

   // Filter tick data with 0 last price��������tick
    if (tickData->lastprice==0)
        return;
    
    // Filter tick data with older timestamp��������tick
    if (m_lastTick != NULL)
    {
        tickDateTime = QDateTime::fromString(QString::fromStdString(tickData->date + " " + tickData->time), "yyyy--mm--dd hh:mm:ss");
        lasttickDateTime = QDateTime::fromString(QString::fromStdString(m_lastTick->date + " " + m_lastTick->time), "yyyy--mm--dd hh:mm:ss");
        if(tickDateTime< lasttickDateTime) //������tick��ʱ��Ҫ����һ��tickʱ���磬���˵�
            return;
    }
    //��bNewMinute��ֵ true�������������һ�����µ�tick�͵�ǰ��һ����barʱ�䲻��һ���������棬���о��ǵ�ǰһ����bar��û������
    //�������������Ҫ��new һ������bar
    if (m_Bar == NULL)
    {
        m_Bar = new BarData();
        bNewMinute = true;

    }
    else if(barDateTime.date()!= tickDateTime.date()|| barDateTime.time().hour()!=tickDateTime.time().hour()||barDateTime.time().minute()!=tickDateTime.time().minute() )

    {
        //���ڣ�Сʱ������ֻҪ��һ����ͬ��tick�����ڲ�ͬ��minute�ˣ���Ҫ�ϳ�һ����bar��
        //bar��ʱ��,���硱13��30��09������Ҫ����д��0
        QString time = QString::fromStdString(m_Bar->time);
        m_Bar->time = (time.section(":", 0, 1)).toStdString() + ":00";
        //����һ��bar,֮�����on_bar�ص����������½�һ��bar
        m_onBar_Func(m_Bar);
        bNewMinute = true;

    }
    if (bNewMinute == true)
    {
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
    else//�����µ�bar��ֻ��Ҫ���¸ߵ͵㣬�ֲ���
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
    else//��һ����tick����Ҫnewһ��lastTick
    {
        m_lastTick = new TickData();
    }
    //�����͵�tick����ΪlastTick���´λ���
    *m_lastTick = *tickData;

}
void BarGenerator::updateBar(BarData* barData)
{
    //��һ�ο�ʼ�ϳ�windowbar���߸պϳ���һ��
    if (m_windowBar == NULL || m_bWindowFinished==true)
    {
        m_windowBar = new BarData();
        *m_windowBar = *barData;
    }
    else
    {
        m_windowBar->high = std::max(m_windowBar->high, barData->high);
        m_windowBar->low = std::max(m_windowBar->low, barData->low);

    }
    m_windowBar->close =  barData->close;
    m_windowBar->volume += barData->volume;
    m_windowBar->openInterest = barData->openInterest;

    //bool bFinished = false;
    if (m_interval == MINUTE)
    {
        //��ȡ����
        QString time = QString::fromStdString(barData->time);
        int iMinute=time.section(":", 1, 1).toInt();
        //�Ƿ��ܱ�iWindow��������
        float f = (iMinute+1) / m_iWindow;
        if ((f - int(f)) == 0) 
        {
            m_bWindowFinished = true;
        }//#��Ʒ�ڻ���30�������ںϳ���Ҫ�ر���һ��10��15��10��30��ͣ��    
        else if((m_lastBar!=NULL && barData->time=="10:14:00")&&m_iWindow=30)
        {
            m_bWindowFinished = true;
        }

    }
    else if (m_interval == HOUR)
    {}
    else if(m_interval == DAILY)
    { }

    if (m_bWindowFinished)
    {
        (*m_onWindowBar_FUNC)(m_windowBar);
        //m_windowBar = NULL;

    }
    if (m_lastBar == NULL)
        m_lastBar = new BarData();
    *m_lastBar = *barData;
    
}
