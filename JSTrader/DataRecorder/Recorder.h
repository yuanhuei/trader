#ifndef __RECORDER_H__
#define __RECORDER_H__
#include"EventEngine.h"
#include"structs.h"
#include<set>
#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <atomic>
#include"utils.hpp"
class Recorder
{
public:
	Recorder(EventEngine *eventengine, Recordermanager *recordermanager);
	~Recorder();
	void showlog(std::shared_ptr<Event>e);
	void readsymbols(std::string gatewaynam);
	void OnTick(std::shared_ptr<Event>e);
	void OnBar(BarData bar);
	void OnDailyBar();
	void autoconnect();
	void exit();



private:
	void dbInsert(const char* dbname, const char* collectionname, std::shared_ptr<JSData>data, mongoc_client_pool_t *mongopool);
	const time_t getsystemunixdatetime(std::string time,std::string type);
	const time_t timetounixtimestamp(int hour, int minute, int seconds);
	EventEngine *m_ptr_eventengine;
	Recordermanager *m_ptr_Recordermanager;
	mongoc_client_pool_t *m_pool;
	mongoc_uri_t         *uri;
	std::set<std::string>m_allsymbols;//所有要订阅的合约
	std::set<std::string>m_ticksymbols;//tick
	std::set<std::string>m_barsymbols;//bar
	std::set<std::string>m_dailybarsymbols;//dailybar
	std::map<std::string, BarData>m_barmap;
	std::map<std::string, TickData>m_dailybarmap;//一次只存一个最新的TICK等到收盘时候合成日线
	std::map<std::string,int>m_barMinute;
	std::map<std::string,int>m_barHour;
	std::map<std::string, long long>m_timestampmap;           std::mutex m_timestamp_mtx; //防重复
	std::atomic<bool>m_connectstatus;
	std::mutex m_mutex;
	std::mutex m_hourminutemtx;
	std::mutex m_logmtx;

	//
	std::set<std::string> m_ninetoeleven;
	std::set<std::string> m_ninetohalfeleven;
	std::set<std::string> m_ninetoone;
	std::set<std::string> m_ninetohalftwo;
};




#endif