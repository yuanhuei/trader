#pragma once
#ifndef EVENTENGINE_H
#define EVENTENGINE_H


#include<functional>
#include<string>
#include<queue>
#include<mutex>
#include<memory>
#include<condition_variable>
#include<map>
#include<thread>
#include<atomic>
#include<chrono>
#include<time.h>
#include"../qcstructs.h"
/*********************************同步队列****************************************/
//同步队列
template<typename EVENT>
class  SynQueue
{
public:
	void Push(std::shared_ptr<EVENT>Event)
	{
		std::unique_lock<std::recursive_mutex>lck(m_mutex);
		m_queue.push(Event);
		m_cv.notify_all();
	}
	std::shared_ptr<Event> Take()
	{
		std::unique_lock<std::recursive_mutex>lck(m_mutex);
		while (m_queue.empty())
		{
			m_cv.wait(lck);
		}
		std::shared_ptr<Event>e = m_queue.front();
		m_queue.pop();
		return e;
	}

private:
	std::recursive_mutex m_mutex;
	std::queue<std::shared_ptr<EVENT>> m_queue;
	std::condition_variable_any m_cv;
};

/************************************事件驱动引擎************************************************/
typedef std::function<void(std::shared_ptr<Event>)> TASK;
//task指的是处理函数
//event指的是事件类型以及要传递的参数数据
class  EventEngine
{
public:
	EventEngine();
	~EventEngine();
	void StartEngine();
	void StopEngine();
	void RegEvent(std::string eventtype, TASK task);														//事件类型和处理函数
	void UnregEvent(std::string eventtype);
	void DoTask();																							//任务处理
	void Put(std::shared_ptr<Event>e);																		//放入任务
	void Timer();																							//每秒钟查询持仓
private:
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::vector<std::thread*>* m_task_pool = nullptr;														//线程池
	SynQueue<Event>* m_event_queue = nullptr;																//事件驱动线程安全队列
	std::multimap<std::string, TASK>* m_task_map = nullptr;													//事件对比映射
	std::thread* m_timer_thread = nullptr;;																	//时间线程
	std::atomic<bool>m_active = false;																		//引擎开关
};
#endif
