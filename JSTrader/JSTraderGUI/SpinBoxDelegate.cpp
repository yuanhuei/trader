#include <qspinbox.h>
#include"SpinBoxDelegate.h"
#include"ClickTableView.h"
SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
{

}
QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
	const QStyleOptionViewItem &,
	const QModelIndex &) const
{
	QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
	editor->setFrame(false);
	editor->setMinimum(-999999);
	editor->setMaximum(999999);
	return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
	const QModelIndex &index) const
{
	QString Vstr = index.model()->data(index, Qt::EditRole).toString();
	double value = Vstr.toDouble();
	QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
	spinBox->setValue(value);
	emit changepos(m_strategyname, m_symbol, value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
	const QModelIndex &index) const
{
	QDoubleSpinBox *spinBox = static_cast <QDoubleSpinBox*>(editor);
	spinBox->interpretText();
	QString value = QString::number(spinBox->value());
	qobject_cast<ClickModel*>(model)->setData(index, value, Qt::EditRole);
	emit setupdatetrue();
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
	const QStyleOptionViewItem &option, const QModelIndex &) const
{
	editor->setGeometry(option.rect);
}

void SpinBoxDelegate::savestrategyname(std::string name, std::string symbol)
{
	m_strategyname = name;
	m_symbol = symbol;
}