#include"utility.h"

//#include<qstring.h>
#include<qdatetime.h>
#include<talib.h>
#include<ta_defs.h>
#include<math.h>
#include <json/json.h>
#include<qstring.h>

#include <iostream>  
#include <fstream>  
#include"BaseEngine.h"
#include"event_engine/eventengine.h"

    QString str2qstr_new(std::string str)
    {
        return QString::fromLocal8Bit(str.c_str());
    }

    std::string Global_FUC::time_t2str(time_t datetime)
    {
        auto tt = datetime;
        struct tm* ptm = localtime(&tt);
        char date[60] = { 0 };
        sprintf(date, "%d-%02d-%02d      %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
        return std::string(date);
    }

    int  Global_FUC::round_double(double number)
    {
        return (number > 0.0) ? (number + 0.5) : (number - 0.5);
    }

    void Global_FUC::savetraderecord(std::string strategyname, std::shared_ptr<Event_Trade>etrade, EventEngine* eventEngine)
    {
        //���׼�¼
        if (_access("./traderecord", 0) != -1)
        {
            std::fstream f;
            f.open("./traderecord/" + strategyname + ".csv", std::ios::app | std::ios::out);
            if (!f.is_open())
            {
                //����򲻿��ļ�
                std::shared_ptr<Event_Log>e = std::make_shared<Event_Log>();
                e->msg = "�޷����潻�׼�¼";
                e->gatewayname = "CTP";
                eventEngine->Put(e);
                return;
            }
            std::string symbol = etrade->symbol;
            std::string direction = etrade->direction;
            std::string offset = etrade->offset;
            std::string tradetime = etrade->tradeTime;
            std::string volume = Utils::doubletostring(etrade->volume);
            std::string price = Utils::doubletostring(etrade->price);

            f << strategyname << "," << tradetime << "," << symbol << "," << direction << "," << offset << "," << price << "," << volume << "\n";

            f.close();
        }
    }

    std::map<std::string, std::map<std::string, float>> Global_FUC::ReadStrategyConfFileJson(std::string fileName, BaseEngine* ctaEngine)
    {
        Json::Reader reader;
        Json::Value root;
        std::map<std::string, std::map<std::string, float>> strategyConfigInfo_map;
        std::string StrategyName, ClassName;

        //���ļ��ж�ȡ����֤��ǰ�ļ���demo.json�ļ�  
        std::ifstream in(fileName, std::ios::binary);

        if (!in.is_open())
        {
            ctaEngine->writeCtaLog("�򿪲��������ļ�ʧ��");
            return strategyConfigInfo_map;
        }

        if (reader.parse(in, root))
        {
            ctaEngine->writeCtaLog("�򿪲��������ļ��ɹ�");
            for (int i = 0; i < root.size(); i++)
            {
                //��ȡ�������ƺͺ�Լ����

                StrategyName = root[i]["strategy_name"].asString();
                std::string vt_symbol = root[i]["vt_symbol"].asString();
                ClassName = root[i]["class_name"].asString();
                if (StrategyName.length() < 1 || ClassName.length() < 1)
                {
                    ctaEngine->writeCtaLog("�����ļ�������Ϣ��ȫ");
                    return strategyConfigInfo_map;
                }

                //��ȡ����������Ϣ 
                std::map<std::string, float> settingMap;
                Json::Value::Members members;
                members = root[i]["setting"].getMemberNames();
                //std::vector<std::string> settingKeys= root["setting"].getMemberNames();
                for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // ����ÿ��key
                {
                    std::string strKey = *iterMember;
                    float fValue = root[i]["setting"][strKey.c_str()].asFloat();
                    /*
                    if (root[i]["setting"][strKey.c_str()].isString())
                    {
                        fValue = root[i]["setting"][strKey.c_str()].asString();
                    }
                    else
                        fValue = root[i]["setting"][strKey.c_str()].asFloat();
                    */
                    //if(fValue.ist)
                    settingMap.insert({ strKey,  fValue });

                }
                //���뵽��������map��
                strategyConfigInfo_map[StrategyName + "_" + vt_symbol +"_" + ClassName] = settingMap;
            }
        }
        else
        {
            ctaEngine->writeCtaLog("�������������ļ�ʧ��");
        }

        in.close();
        ctaEngine->writeCtaLog("�������ü������");
        return  strategyConfigInfo_map;
    }

    void Global_FUC::WriteStrategyDataJson(std::map<std::string, std::string>dataMap, std::string fileName)
    {


        Json::Value root;

        //���ڵ�����
        std::map<std::string, std::string>::iterator iter;
        for (iter = dataMap.begin(); iter != dataMap.end(); iter++)
        {
            std::string varName = iter->first;
            std::string varValue = iter->second;
            root[varName] = Json::Value(varValue);
        }

        Json::StyledWriter sw;

        //������ļ�
        std::ofstream os;
        std::string file = "./Strategy/cta_strategy_data_" + fileName + ".json";
        os.open(file);
        os << sw.write(root);
        os.close();

    }

ArrayManager::ArrayManager(int iSize)
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

double ArrayManager::sma(int n)
{
    int    startIdx=0;
    int    endIdx=m_iSize-1;
    double inReal[100];
    int           optInTimePeriod=n; /* From 2 to 100000 */
    int outBegIdx;
    int outNBElement;
    double        outReal[100];
     
    for (int i = 0; i < m_iSize; i++)
    {
        inReal[i] = openprice_array[i];
    }

    int iReturn=TA_SMA(startIdx, endIdx, inReal, optInTimePeriod, &outBegIdx, &outNBElement, outReal);
    //if(iReturn!=0)
    //    print("ʧ��");
    return outReal[m_iSize - n];
}
std::map<std::string, double> ArrayManager::boll(int iWindow,int iDev)
{
    std::map<std::string, double> mapBoll;
    int            startIdx = 0;
    int            endIdx =m_iSize-1;
    double        inReal[100];
    int           optInTimePeriod=iWindow; /* From 2 to 100000 */
    double        optInNbDevUp=iDev; /* From TA_REAL_MIN to TA_REAL_MAX */
    double        optInNbDevDn=iDev; /* From TA_REAL_MIN to TA_REAL_MAX */
    TA_MAType     optInMAType = TA_MAType_SMA;
    int          outBegIdx;
    int          outNBElement;
    double        outRealUpperBand[100];
    double        outRealMiddleBand[100];
    double        outRealLowerBand[100];

    for (int i = 0; i < m_iSize; i++)
    {
        inReal[i] = openprice_array[i];
    }

    TA_BBANDS(startIdx, endIdx, inReal, optInTimePeriod, optInNbDevDn, 
        optInNbDevDn, optInMAType, &outBegIdx, &outNBElement, outRealUpperBand, outRealMiddleBand, outRealLowerBand);
    mapBoll["boll_up"] = outRealUpperBand[outNBElement-1];
    mapBoll["boll_middle"] = outRealMiddleBand[outNBElement - 1];
    mapBoll["boll_down"] = outRealLowerBand[outNBElement - 1];
    return mapBoll;

}

double ArrayManager::cci(int iWindow)
{
    int            startIdx = 0;
    int            endIdx = m_iSize - 1;
    double inHigh[100];
     double inLow[100];
     double inClose[100];
    int           optInTimePeriod=iWindow; /* From 2 to 100000 */
    int outBegIdx;
    int outNBElement;
    double        outReal[100];
    for (int i = 0; i < m_iSize; i++)
    {
        inHigh[i] = highprice_array[i];
        inLow[i] = lowprice_array[i];
        inClose[i] = closeprice_array[i];
    }

    TA_CCI(startIdx, endIdx, inHigh, inLow, inClose, optInTimePeriod, &outBegIdx, &outNBElement, outReal);
    return outReal[outNBElement - 1];
}

double ArrayManager::atr(int iWindow)
{
    int            startIdx = 0;
    int            endIdx = m_iSize - 1;
    double inHigh[100];
    double inLow[100];
    double inClose[100];
    int           optInTimePeriod = iWindow; /* From 2 to 100000 */
    int outBegIdx;
    int outNBElement;
    double        outReal[100];
    for (int i = 0; i < m_iSize; i++)
    {
        inHigh[i] = highprice_array[i];
        inLow[i] = lowprice_array[i];
        inClose[i] = closeprice_array[i];
    }

    TA_ATR(startIdx, endIdx, inHigh, inLow, inClose, optInTimePeriod, &outBegIdx, &outNBElement, outReal);
    return outReal[outNBElement - 1];
}

BarGenerator::BarGenerator(ON_Functional onBar_Func, int iWindow, ON_Functional onWindowBar_FUNC, Interval iInterval)
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
    /*
    else if (m_interval == HOUR)
    {//��д
    }
    else if(m_interval == DAILY)
    {//��д}*/

    if (m_bWindowFinished)
    {
        m_onWindowBar_FUNC(*m_windowBar);
        //m_windowBar = NULL;

    }
    if (m_lastBar == NULL)
        m_lastBar = new BarData();
    *m_lastBar = *barData;
    
}

