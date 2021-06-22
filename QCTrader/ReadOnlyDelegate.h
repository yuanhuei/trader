#ifndef READONLYDELEGATE_H
#define READONLYDELEGATE_H
#include <qstyleditemdelegate.h>
class ReadOnlyDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	ReadOnlyDelegate(QObject *parent = 0);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const;
};
#endif