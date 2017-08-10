#include"ClickTableView.h"
Clicktableview::Clicktableview()
{
	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(mycellEntered(const QModelIndex &)));
}

void Clicktableview::setposcolumn(int column)
{
	m_poscolumn.insert(column);
}

void Clicktableview::mycellEntered(const QModelIndex &index)
{
	if (m_poscolumn.find(index.column()) != m_poscolumn.end())
	{
		if (m_model != nullptr)
		{
			m_model->setupdate(false);
		}
	}
}

void Clicktableview::setModel(QStandardItemModel *model)
{
	m_model = qobject_cast<ClickModel*>(model);
	QTableView::setModel(model);
}

void Clicktableview::setmodelupdatetrue()
{
	m_model->setupdate(true);
}