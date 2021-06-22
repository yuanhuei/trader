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
#include"qcstructs.h"
#include"event_engine/eventengine.h"
#include"gateway/gatewaymanager.h"
#include"gateway/qcgateway.hpp"
#include"gateway/ctp_gateway/ctpgateway.h"
#include"gateway/qcgateway.hpp"
#include"cta_strategy/ctamanager.h"
#include"risk_manager/riskmanager.h"
#include"ClickTableView.h"

class riskmanager;
class CTAmanager;

struct UpdatePriceTableData
{
	std::string symbol;
	//�ɽ�����
	double lastprice;//���³ɽ���
	double openInterest;//�ֲ���
	std::string date;//����
	std::string time;//ʱ��
	double upperLimit;//��ͣ
	double lowerLimit;//��ͣ
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
	double preBalance;//�����˻����㾻ֵ
	double balance;//�˻���ֵ
	double available;//�����ʽ�
	double commission;//����������
	double margin;//��֤��ռ��
	double closeProfit;//ƽ��ӯ��
	double positionProfit;//�ֲ�ӯ��
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

	//ת������
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
	//��̬��Ӳ��Կ�
	QGridLayout *StrategyLayout;
	//��Ϲ���ҳ
	QWidget *portfoliowidget;
	//���չ���ҳ
	QWidget *riskControlWidget;
	QPushButton *buttonSwitchEngineStatus;
	QSpinBox *spinOrderFlowLimit;
	QSpinBox *spinOrderFlowClear;
	QSpinBox *spinOrderSizeLimit;
	QSpinBox *spinTradeLimit;
	QSpinBox *spinWorkingOrderLimit;
	QSpinBox *spinOrderCancelLimit;

	//����
	std::map<std::string, ClickModel*>m_strategyvarmodelmap;				QMutex m_strategyvarparammtx;
	std::map<std::string, QStandardItemModel*>m_strategyparammodelmap;
	std::map<std::string, SpinBoxDelegate*>m_delegatespinbox;		
	std::vector<ReadOnlyDelegate*>m_delegatereadonly;
	std::map<QCheckBox*, std::string>m_Strategycheckbox;

	//��Ӧ����
	void OnLog(std::shared_ptr<Event>e);

	void onAccount(std::shared_ptr<Event>e);

	void onPosition(std::shared_ptr<Event>e);

	void onStrategyLoaded(std::shared_ptr<Event>e);

	void onStrategyUpdate(std::shared_ptr<Event>e);

	void onPortfolioUpdate(std::shared_ptr<Event>e);

	void onPriceTableUpdate(std::shared_ptr<Event>e);
	//�������������ָ��
	EventEngine *m_eventengine;//�¼���������
	Gatewaymanager *m_gatewaymanager;//�ӿڹ�����
	riskmanager *m_riskmanager;//���չ�����
	CTAmanager *m_ctamanager;//cta������
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