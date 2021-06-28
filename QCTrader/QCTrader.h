#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QCTrader.h"
#include"CTPConnectWidgets.h"

class QCTrader : public QMainWindow
{
    Q_OBJECT

public:
    QCTrader(QWidget *parent = Q_NULLPTR);
    CTPConnectWidgets* m_CTPDialog;
    QString str2qstr_new(std::string str);
    
protected slots:
    void menu_CTP_clicked();
private:
    Ui::QCTraderClass ui;
};
