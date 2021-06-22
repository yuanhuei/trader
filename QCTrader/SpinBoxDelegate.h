#ifndef SPINEBOXDELEGATE_H
#define SPINEBOXDELEGATE_H
#include <qstyleditemdelegate.h>
#include "ClickTableView.h"
class SpinBoxDelegate : public QStyledItemDelegate
{
	Q_OBJECT
signals:
	void changepos(std::string strategyname,std::string symbol, double pos) const;
	void setupdatetrue() const;
public:
	SpinBoxDelegate(QObject *parent = 0);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void savestrategyname(std::string name,std::string symbol);
private:
	std::string m_strategyname; 
	std::string m_symbol;
};
#endif