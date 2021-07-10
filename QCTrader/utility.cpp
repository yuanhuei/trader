#include"utility.h"

//#include<qstring.h>
#include<qdatetime.h>
#include<talib.h>
#include<ta_defs.h>

#include <json/json.h>
#include<qstring.h>

#include <iostream>  
#include <fstream>  
#include"CTAAPI.h"

QString str2qstr_new(std::string str)
{
	return QString::fromLocal8Bit(str.c_str());
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
    //没有填满，就直接填入一个
    if (!m_iInit)
    {
        openprice_array[m_iCount] = barData.open;
        closeprice_array[m_iCount] = barData.close;
        highprice_array[m_iCount] = barData.high;
        lowprice_array[m_iCount] = barData.low;
        volume_array[m_iCount] = barData.volume;
        openinterest_array[m_iCount] = barData.openInterest;
    }
    else//已经填满了，初始化完成了，也就是m_iCount计数等于了m_iSize,新加入bar需要删除第0个元素，整体向前移位一个
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
    //没有完成初始化，只需要填入就可以了
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
    //    print("失败");
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
    int*          outBegIdx;
    int*          outNBElement;
    double        outRealUpperBand[100];
    double        outRealMiddleBand[100];
    double        outRealLowerBand[100];

    for (int i = 0; i < m_iSize; i++)
    {
        inReal[i] = openprice_array[i];
    }

    TA_BBANDS(startIdx, endIdx, inReal, optInTimePeriod, optInNbDevDn, 
        optInNbDevDn, optInMAType, outBegIdx, outNBElement, outRealUpperBand, outRealMiddleBand, outRealLowerBand);
    mapBoll["boll_up"] = outRealUpperBand[m_iSize - iWindow];
    mapBoll["boll_middle"] = outRealMiddleBand[m_iSize - iWindow];
    mapBoll["boll_down"] = outRealLowerBand[m_iSize - iWindow];
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
    int* outBegIdx;
    int* outNBElement;
    double        outReal[100];

    for (int i = 0; i < m_iSize; i++)
    {
        inHigh[i] = highprice_array[i];
        inLow[i] = lowprice_array[i];
        inClose[i] = closeprice_array[i];
    }

    TA_CCI(startIdx, endIdx, inHigh, inLow, inClose, optInTimePeriod, outBegIdx, outNBElement, outReal);

    return outReal[m_iSize - iWindow];
}

double ArrayManager::atr(int iWindow)
{
    int            startIdx = 0;
    int            endIdx = m_iSize - 1;
    double inHigh[100];
    double inLow[100];
    double inClose[100];
    int           optInTimePeriod = iWindow; /* From 2 to 100000 */
    int* outBegIdx;
    int* outNBElement;
    double        outReal[100];

    for (int i = 0; i < m_iSize; i++)
    {
        inHigh[i] = highprice_array[i];
        inLow[i] = lowprice_array[i];
        inClose[i] = closeprice_array[i];
    }

    TA_ATR(startIdx, endIdx, inHigh, inLow, inClose, optInTimePeriod, outBegIdx, outNBElement, outReal);

    return outReal[m_iSize - iWindow];
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
        m_Bar = new BarData();
        bNewMinute = true;

    }
    else if(barDateTime.date()!= tickDateTime.date()|| barDateTime.time().hour()!=tickDateTime.time().hour()||barDateTime.time().minute()!=tickDateTime.time().minute() )

    {
        //日期，小时，分钟只要有一个不同，tick就属于不同的minute了，需要合成一个新bar了
        //bar的时间,例如”13：30：09“，需要把秒写成0
        QString time = QString::fromStdString(m_Bar->time);
        m_Bar->time = (time.section(":", 0, 1)).toStdString() + ":00";
        //走完一个bar,之后调用on_bar回调函数，再新建一个bar
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
    else//不是新的bar，只需要更新高低点，持仓量
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
    else//第一次来tick，需要new一个lastTick
    {
        m_lastTick = new TickData();
    }
    //把推送的tick复制为lastTick，下次会用
    *m_lastTick = *tickData;

}
void BarGenerator::updateBar(BarData* barData)
{
    //第一次开始合成windowbar或者刚合成完一次,这次是新的windowbar
    if (m_windowBar == NULL )
    {
        m_windowBar = new BarData();
        *m_windowBar = *barData;
    }
    if(m_bWindowFinished )//新的一根windowbar，数据完全赋值barData,
    {
        *m_windowBar = *barData;
    }
    else//不是一根新的windowbar，需要更新最高最低价，收盘价，开盘价不更新
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
        //获取分钟
        QString time = QString::fromStdString(barData->time);
        int iMinute=time.section(":", 1, 1).toInt();
        //是否能被iWindow数字整除
        float f = (iMinute+1) / m_iWindow;
        if ((f - int(f)) == 0) 
        {
            m_bWindowFinished = true;
        }//#商品期货的30分钟周期合成需要特别处理一下10：15到10：30的停牌    
        else if((m_lastBar!=NULL && barData->time=="10:14:00")&&m_iWindow==30)
        {
            m_bWindowFinished = true;
        }

    }
    /*
    else if (m_interval == HOUR)
    {//待写
    }
    else if(m_interval == DAILY)
    {//待写}*/

    if (m_bWindowFinished)
    {
        m_onWindowBar_FUNC(*m_windowBar);
        //m_windowBar = NULL;

    }
    if (m_lastBar == NULL)
        m_lastBar = new BarData();
    *m_lastBar = *barData;
    
}

std::map<std::string, std::map<std::string, float>> ReadStrategyConfFileJson(std::string fileName,CTAAPI* ctaEngine)
{
    Json::Reader reader;
    Json::Value root;
    std::map<std::string, std::map<std::string, float>> strategyConfigInfo_map;
    std::string StrategyName, ClassName;

    //从文件中读取，保证当前文件有demo.json文件  
    std::ifstream in(fileName, std::ios::binary);

    if (!in.is_open())
    {
        ctaEngine->writeCtaLog("打开策略配置文件失败");
        return strategyConfigInfo_map;
    }

    if (reader.parse(in, root))
    {
        ctaEngine->writeCtaLog("打开策略配置文件成功");
        for (int i = 0; i < root.size(); i++)
        {
            //读取策略名称和合约名称

            StrategyName = root[i]["strategy_name"].asString();
           // std::string vt_symbol = root[i]["vt_symbol"].asString();
            ClassName = root[i]["class_name"].asString();
            if (StrategyName.length() < 1  || ClassName.length() < 1)
            {
                ctaEngine->writeCtaLog("配置文件策略信息不全");
                return strategyConfigInfo_map;
            }

            //读取策略配置信息 
            std::map<std::string, float> settingMap;
            Json::Value::Members members;
            members = root[i]["setting"].getMemberNames();
            //std::vector<std::string> settingKeys= root["setting"].getMemberNames();
            for (Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // 遍历每个key
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
            //插入到策略配置map中
            strategyConfigInfo_map[StrategyName  + "_" + ClassName] = settingMap;
        }
    }
    else
    {
        ctaEngine->writeCtaLog("解析策略配置文件失败");
    }

    in.close();
    ctaEngine->writeCtaLog("策略配置加载完成");
    return  strategyConfigInfo_map;
}

void WriteStrategyDataJson(std::map<std::string, std::string>dataMap, std::string fileName)
{


    Json::Value root;

    //根节点属性
    std::map<std::string, std::string>::iterator iter;
    for (iter = dataMap.begin(); iter != dataMap.end(); iter++)
    {
        std::string varName = iter->first;
        std::string varValue = iter->second;
        root[varName] = Json::Value(varValue);
    }

    Json::StyledWriter sw;

    //输出到文件
    std::ofstream os;
    std::string file = "./Strategy/cta_strategy_data_" + fileName + ".json";
    os.open(file);
    os << sw.write(root);
    os.close();


}