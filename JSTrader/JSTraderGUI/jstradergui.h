#include<qwidget.h>
#include<qtabwidget.h>
#include<qgridlayout.h>
#include<qgroupbox.h>
#include<qstandarditemmodel.h>
#include<qtableview.h>
#include<qtextbrowser.h>
#include<qscrollarea.h>
#include<qlabel.h>
#include<qevent.h>
#include<qmenubar.h>
#include<qheaderview.h>
#include<qpushbutton.h>
#include<qcheckbox.h>
#include<qlineedit.h>
#include<qcombobox.h>
#include<qspinbox.h>
#include<qmessagebox.h>
#include<qmutex.h>
#include<QTextStream>
#include"ReadOnlyDelegate.h"
#include"SpinBoxDelegate.h"
#include"structs.h"
#include"eventengine.h"
#include"gatewaymanager.h"
#include"ctamanager.h"
#include"ClickTableView.h"

struct UpdatePriceTableData
{
	std::string symbol;
	//成交数据
	double lastprice;//最新成交价
	double openInterest;//持仓量
	std::string date;//日期
	std::string time;//时间
	double upperLimit;//涨停
	double lowerLimit;//跌停
	double bidprice1;
	double askprice1;
};

struct UpdateStrategyData
{
	std::string strategyname;
	std::map<std::string, std::string>parammap;
	std::map<std::string, std::string>varmap;
};

struct LoadStrategyData
{
	std::string strategyname;
	std::map<std::string, std::string>parammap;
	std::map<std::string, std::string>varmap;
};

struct PositionData
{
	std::string symbol;
	std::string direction;
	std::string gatewayname;
	double position;
	double todayPosition;
	double ydPosition;
	double todayPositionCost;
	double ydPositionCost;
	double price;
	double frozen;
};

struct AccountData
{
	std::string gatewayname;
	std::string accountid;
	double preBalance;//昨日账户结算净值
	double balance;//账户净值
	double available;//可用资金
	double commission;//今日手续费
	double margin;//保证金占用
	double closeProfit;//平仓盈亏
	double positionProfit;//持仓盈亏
};

struct PortfolioData
{
	std::string dllname;
	std::string strategyname;
	std::string symbol;
	Portfolio_Result_Data Portfoliodata;
	std::vector<int>strategyrows;
};

class JSTraderGUI : public QWidget
{
	Q_OBJECT
signals :
	void WriteLog(QString msg);
	void LoadStrategySignal(LoadStrategyData data);
	void UpdateStrategySignal(UpdateStrategyData data);
	void UpdateAccountSignal(AccountData data);
	void UpdatePositionSignal(PositionData  data);
	void UpdatePortfolioSignal(PortfolioData data);
	void UpdatePriceTableSignal(UpdatePriceTableData data);
	void UpdatePortfolioWinning(QString winning);
public:
	explicit JSTraderGUI(QWidget *parent = 0);
	~JSTraderGUI();

	void closeEvent(QCloseEvent   *  event);

	//转换文字
	QString str2qstr(const std::string str);
	void LoadCore();
	void RegEvent();
	void LoadUI();

	QGroupBox *createAccountGroup();
	QGroupBox *createPositionGroup();
	void CreateRiskWindow();
	QGroupBox *createLogGroup();
	QGroupBox *createPortfolio_1();
	QGroupBox *createPortfolio_2();
	QGroupBox *createsendorder();
	QGroupBox *createControlGroup();
	QScrollArea *createStrategyGroup();
	QGroupBox *createRiskControl();
	//model
	QStandardItemModel* m_AccountModel;			
	QStandardItemModel* m_PositionModel;		
	QStandardItemModel *m_portfolioModel;				
	//动态添加策略框
	QGridLayout *StrategyLayout;
	//组合管理页
	QWidget *portfoliowidget;
	//风险管理页
	QWidget *riskControlWidget;
	QPushButton *buttonSwitchEngineStatus;
	QSpinBox *spinOrderFlowLimit;
	QSpinBox *spinOrderFlowClear;
	QSpinBox *spinOrderSizeLimit;
	QSpinBox *spinTradeLimit;
	QSpinBox *spinWorkingOrderLimit;
	QSpinBox *spinOrderCancelLimit;

	//缓存
	std::map<std::string, ClickModel*>m_strategyvarmodelmap;				QMutex m_strategyvarparammtx;
	std::map<std::string, QStandardItemModel*>m_strategyparammodelmap;
	std::map<std::string, SpinBoxDelegate*>m_delegatespinbox;		
	std::vector<ReadOnlyDelegate*>m_delegatereadonly;
	std::map<QCheckBox*, std::string>m_Strategycheckbox;

	//响应函数
	void OnLog(std::shared_ptr<Event>e);

	void onAccount(std::shared_ptr<Event>e);

	void onPosition(std::shared_ptr<Event>e);

	void onStrategyLoaded(std::shared_ptr<Event>e);

	void onStrategyUpdate(std::shared_ptr<Event>e);

	void onPortfolioUpdate(std::shared_ptr<Event>e);

	void onPriceTableUpdate(std::shared_ptr<Event>e);
	//各种引擎管理器指针
	EventEngine *m_eventengine;//事件驱动引擎
	Gatewaymanager *m_gatewaymanager;//接口管理器
	riskmanager *m_riskmanager;//风险管理器
	CTAmanager *m_ctamanager;//cta管理器
	private slots:

	void switchEngineStatus();

	void OnPressConnectCTP();
	void OnPressConnectIB();
	void OnPressLoadStrategy();
	void OnPressInitStrategy();
	void OnPressStartStrategy();
	void OnPressStopStrategy();
	void OnPressPortfolioShow();
	void onPressRiskControlShow();
	void onPressbuttonClearOrderFlowCount();
	void onPressbuttonclearTradeCount();
	void onPressbuttonSaveSetting();
	void Onchanedpos(std::string name,std::string symbol,double pos);

	void CreateStrategyBox(LoadStrategyData data);
	void UpdateStrategyBox(UpdateStrategyData data);
	void UpdateAccountBox(AccountData data);
	void UpdatePositionBox(PositionData data);
	void UpdatePortfolioBox(PortfolioData data);
};