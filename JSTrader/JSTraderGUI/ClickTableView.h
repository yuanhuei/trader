#ifndef CLICKTABLEVIEW_H
#define CLICKTABLEVIEW_H
#include<qtableview.h>
#include<qheaderview.h>
#include<qstandarditemmodel.h>
#include<qevent.h>
#include<set>
class ClickModel :public QStandardItemModel
{
	Q_OBJECT
public:
	void setupdate(bool canornot)
	{
		m_canupdate = canornot;
	}
	bool canupdate()
	{
		return m_canupdate;
	}
private:
	bool m_canupdate=true;
};

class Clicktableview :public QTableView
{
	Q_OBJECT
public:
	Clicktableview();
	void setposcolumn(int column);
	void setModel(QStandardItemModel *model);
private:
	
	std::set<int>m_poscolumn;
	bool is_intableview;
	ClickModel *m_model;
	private slots:
	void mycellEntered(const QModelIndex &index);
	void setmodelupdatetrue();
};
#endif