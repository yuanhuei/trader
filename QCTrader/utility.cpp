#include"utility.h"

//#include<qstring.h>
#include<qdatetime.h>
#include"talib.h"

QString str2qstr_new(std::string str)
{
	return QString::fromLocal8Bit(str.c_str());
}

ArrayManager::ArrayManager(int iSize=100)
{
    m_iSize = iSize;
    m_iInit = false;
    m_iCount = 0;

    openprice_array.resize(m_iSize);
    closeprice_array.resize(m_iSize);
    highprice_array.resize(m_iSize);
    lowprice_array.resize(m_iSize);
    volume_array.resize(m_iSize);
    openinterest_array.resize(m_iSize);

}

ArrayManager::~ArrayManager()
{

}

void ArrayManager::update_bar(BarData barData)
{
    //û����������ֱ������һ��
    if (!m_iInit)
    {
        openprice_array[m_iCount] = barData.open;
        closeprice_array[m_iCount] = barData.close;
        highprice_array[m_iCount] = barData.high;
        lowprice_array[m_iCount] = barData.low;
        volume_array[m_iCount] = barData.volume;
        openinterest_array[m_iCount] = barData.openInterest;
    }
    else//�Ѿ������ˣ���ʼ������ˣ�Ҳ����m_iCount����������m_iSize,�¼���bar��Ҫɾ����0��Ԫ�أ�������ǰ��λһ��
    {
        for (int i = 0; i < m_iSize-1; i++)
        {
            openprice_array[i] = openprice_array[i + 1];
            closeprice_array[i] = closeprice_array[i + 1];
            highprice_array[i] = highprice_array[i + 1];
            lowprice_array[i] = lowprice_array[i + 1];
            volume_array[i] = volume_array[i + 1];
            openinterest_array[i] = openinterest_array[i + 1];
        }

        openprice_array[m_iSize - 1] = barData.open;
        closeprice_array[m_iSize - 1] = barData.close;
        highprice_array[m_iSize - 1] = barData.high;
        lowprice_array[m_iSize - 1] = barData.low;
        volume_array[m_iSize - 1] = barData.volume;
        openinterest_array[m_iSize - 1] = barData.openInterest;
    }
    m_iCount++;
    //û����ɳ�ʼ����ֻ��Ҫ����Ϳ�����
    if (m_iCount >=m_iSize)
        m_iInit = true;
   

}

std::vector<float>* ArrayManager::Get_openprice_array()
{
    return &openprice_array;
}
std::vector<float>* ArrayManager::Get_closeprice_array()
{
    return &closeprice_array;
}
std::vector<float>* ArrayManager::Get_highprice_array()
{
    return &highprice_array;
}
std::vector<float>* ArrayManager::Get_lowprice_array()
{
    return &lowprice_array;
}
std::vector<float>* ArrayManager::Get_volume_array()
{
    return &volume_array;
}
std::vector<float>* ArrayManager::Get_openinterest_array()
{
    return &openinterest_array;
}

float ArrayManager::ema(int n)
{
    float fEma;

    int    startIdx=0;
    int    endIdx=m_iSize-1;
    double inReal[100];
    int           optInTimePeriod; /* From 2 to 100000 */
    int outBegIdx;
    int outNBElement;
    double        outReal[100];
     
    for (int i = 0; i < m_iSize; i++)
    {
        inReal[i] = openprice_array[i];
    }

    int iReturn=TA_EMA(startIdx, endIdx, inReal, n, &outBegIdx, &outNBElement, outReal);
    //if(iReturn!=0)
    //    print("ʧ��");
    fEma = outReal[m_iSize-n]; //����
    return fEma;
}


BarGenerator::BarGenerator(ON_FUNC onBar_Func, int iWindow, ON_FUNC onWindowBar_FUNC, Interval iInterval)
{
    m_onBar_Func = onBar_Func;
    m_onWindowBar_FUNC = onWindowBar_FUNC;
    m_iWindow = iWindow;
    m_interval = iInterval;
    m_Bar = NULL;
    m_lastTick = NULL;
    m_lastBar = NULL;
    m_windowBar = NULL;


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
        m_onBar_Func(*m_Bar);
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
        m_Bar->high =std::max(m_Bar->high, tickData->highPrice);
       // if(tickData->highPrice>m_lastTick->highPrice)
       //     m_Bar->high=std::max(m_Bar->high, tickData->highPrice);

        m_Bar->low = std::min(m_Bar->low, tickData->lowPrice);
        //if (tickData->lowPrice < m_lastTick->lowPrice)
        //    m_Bar->low = std::min(m_Bar->low, tickData->lowPrice);

        m_Bar->close = tickData->lastprice;
        m_Bar->openInterest = tickData->openInterest;

    }

    if (m_lastTick != NULL)
    {
        //double volume_change = tickData->volume - m_lastTick->volume;
        m_Bar->volume = m_Bar->volume + tickData->volume;// std::max(volume_change, 0.0);

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
    //��һ�ο�ʼ�ϳ�windowbar���߸պϳ���һ��,������µ�windowbar
    if (m_windowBar == NULL )
    {
        m_windowBar = new BarData();
        *m_windowBar = *barData;
    }
    if(m_bWindowFinished )//�µ�һ��windowbar��������ȫ��ֵbarData,
    {
        *m_windowBar = *barData;
    }
    else//����һ���µ�windowbar����Ҫ���������ͼۣ����̼ۣ����̼۲�����
    {
        m_windowBar->low = std::max(m_windowBar->low, barData->low);
        m_windowBar->high = std::max(m_windowBar->high, barData->high);
        m_windowBar->close = barData->close;
        m_windowBar->volume += barData->volume;
        m_windowBar->openInterest = barData->openInterest;


    }

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
        else if((m_lastBar!=NULL && barData->time=="10:14:00")&&m_iWindow==30)
        {
            m_bWindowFinished = true;
        }

    }
    else if (m_interval == HOUR)
    {//��д
    }
    else if(m_interval == DAILY)
    {//��д}

    if (m_bWindowFinished)
    {
        (*m_onWindowBar_FUNC)(*m_windowBar);
        //m_windowBar = NULL;

    }
    if (m_lastBar == NULL)
        m_lastBar = new BarData();
    *m_lastBar = *barData;
    
}

