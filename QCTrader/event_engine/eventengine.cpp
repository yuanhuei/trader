#include "eventengine.h"


EventEngine::EventEngine()
{
	//���캯��
	//�ȴ���һ�����ж����������
	m_event_queue = new SynQueue<Event>;
	//����һ���Ա�ӳ���multimap
	m_task_map = new std::multimap<std::string, TASK>;
}

EventEngine::~EventEngine()
{
	this->StopEngine();
	if (m_event_queue)
	{
		delete m_event_queue;
		m_event_queue = nullptr;
	}

	if (m_task_map)
	{
		delete m_task_map;
		m_task_map = nullptr;
	}
}

void EventEngine::RegEvent(std::string eventtype, TASK task)
{
	//ע���¼��Ͷ�Ӧ��������ӳ��
	m_mutex.lock();
	m_task_map->insert(std::make_pair(eventtype, task));
	m_mutex.unlock();
}

void EventEngine::UnregEvent(std::string eventtype)
{
	//�˶��Ա�ӳ����¼�
	m_mutex.lock();
	m_task_map->erase(eventtype);
	m_mutex.unlock();
}

void EventEngine::Put(std::shared_ptr<Event>e)
{
	m_event_queue->Push(e);
}

void EventEngine::StartEngine()
{
	//��������
	m_active = true;
	m_timer_thread = new std::thread(std::bind(&EventEngine::Timer, this));
	m_task_pool = new std::vector<std::thread*>;
	std::function<void()>f = std::bind(&EventEngine::DoTask, this);

	for (unsigned i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		//�����̳߳�
		std::thread* thread_worker = new std::thread(f);
		m_task_pool->push_back(thread_worker);
	}
}

void EventEngine::StopEngine()
{
	//ֹͣ����ǰ�Ƚ���������������ͨ���ݽ��˳��¼� �˳�������ڴ�й©
	for (unsigned int i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		std::shared_ptr<Event_Exit>e = std::make_shared<Event_Exit>();
		this->Put(e);
	}

	m_active = false;

	if (m_timer_thread)
	{
		m_timer_thread->join();
		delete m_timer_thread;
		m_timer_thread = nullptr;
	}

	if (m_task_pool != nullptr)
	{
		for (std::vector<std::thread*>::iterator it = m_task_pool->begin(); it != m_task_pool->end(); it++)
		{
			(*it)->join();
			delete (*it);
		}
		delete m_task_pool;
		m_task_pool = nullptr;
	}
}

void EventEngine::DoTask()
{
	while (m_active)
	{
		std::shared_ptr<Event>e = m_event_queue->Take();															//���¼�ȡ����
		//����ر�ʱ�˳�
		e->GetEventType();
		if (e->GetEventType() == EVENT_QUIT)
		{
			break;
		}
		std::pair<std::multimap<std::string, TASK>::iterator, std::multimap<std::string, TASK>::iterator> ret;
		m_mutex.lock();
		ret = m_task_map->equal_range(e->GetEventType());															//��ȡ�¼����ͽ��жԱ�
		m_mutex.unlock();
		for (std::multimap<std::string, TASK>::iterator it = ret.first; it != ret.second; ++it)
		{
			TASK t;
			t = it->second;																							//Ҫ���еĺ���
			t(e);																									//�Ѳ���e����������
		}
	}
}

void EventEngine::Timer()
{
	while (m_active)
	{
		std::shared_ptr<Event_Timer>e = std::make_shared<Event_Timer>();
		Put(e);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}