#include"ReadOnlyDelegate.h"
ReadOnlyDelegate::ReadOnlyDelegate(QObject *parent)
{

}
QWidget *ReadOnlyDelegate::createEditor(QWidget *parent,
	const QStyleOptionViewItem &,
	const QModelIndex &) const
{
	return nullptr;
}