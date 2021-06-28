#include "QCTrader.h"

QCTrader::QCTrader(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    m_CTPDialog = new CTPConnectWidgets();
}

void QCTrader::menu_CTP_clicked()
{
    m_CTPDialog->show();
}

QString QCTrader::str2qstr_new(std::string str)
{
    return QString::fromLocal8Bit(str.c_str());
}