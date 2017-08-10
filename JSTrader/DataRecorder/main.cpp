/*数据记录程序，确保程序调试过程中继续记录数据*/
#include"eventengine.h"
#include"RecorderManager.h"
#include"Recorder.h"
//#include"vld.h"
//主程序
int main()
{
	EventEngine eventengine;
	Recordermanager recordmanager(&eventengine);
	Recorder recorder(&eventengine, &recordmanager);
	eventengine.RegEvent(EVENT_LOG, std::bind(&Recorder::showlog, &recorder, std::placeholders::_1));
	eventengine.RegEvent(EVENT_TICK, std::bind(&Recorder::OnTick, &recorder, std::placeholders::_1));
	eventengine.RegEvent(EVENT_TIMER, std::bind(&Recorder::OnDailyBar, &recorder));
	eventengine.RegEvent(EVENT_TIMER, std::bind(&Recorder::autoconnect, &recorder));
	eventengine.StartEngine();
	std::string line;
	while (std::getline(std::cin, line))
	{
		if (line == "exit")
		{
			break;
		}
	}
	recorder.exit();
	recordmanager.exit();
	return 0;
}