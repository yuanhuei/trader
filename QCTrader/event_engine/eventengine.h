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
/*********************************ͬ������****************************************/
//ͬ������
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

/************************************�¼���������************************************************/
typedef std::function<void(std::shared_ptr<Event>)> TASK;
//taskָ���Ǵ�����
//eventָ�����¼������Լ�Ҫ���ݵĲ�������
class  EventEngine
{
public:
	EventEngine();
	~EventEngine();
	void StartEngine();
	void StopEngine();
	void RegEvent(std::string eventtype, TASK task);														//�¼����ͺʹ�����
	void UnregEvent(std::string eventtype);
	void DoTask();																							//������
	void Put(std::shared_ptr<Event>e);																		//��������
	void Timer();																							//ÿ���Ӳ�ѯ�ֲ�
private:
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::vector<std::thread*>* m_task_pool = nullptr;														//�̳߳�
	SynQueue<Event>* m_event_queue = nullptr;																//�¼������̰߳�ȫ����
	std::multimap<std::string, TASK>* m_task_map = nullptr;													//�¼��Ա�ӳ��
	std::thread* m_timer_thread = nullptr;;																	//ʱ���߳�
	std::atomic<bool>m_active = false;																		//���濪��
};
#endif
