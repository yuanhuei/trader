//用来管理多个接口的数据回报通用类
#include"eventengine.h"
#include"JSRecordClass.h"
#include"structs.h"
class Recordermanager
{
public:
	//通用接口连接
	Recordermanager(EventEngine *eventengine);
	~Recordermanager();
	void Init();
	void connect(std::string gatewayname);
	void subscribe(SubscribeReq req, std::string gatewayname);
	void close(std::string gatewayname);
	void exit();
private:
	EventEngine *m_ptr_eventengine;
	std::map<std::string, std::shared_ptr<JSRecordClass>>m_gatewaymap;//保存gateway名称与gateway对象的对应关系
};