#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QCTrader.h"

class QCTrader : public QMainWindow
{
    Q_OBJECT

public:
    QCTrader(QWidget *parent = Q_NULLPTR);
    void menu_CTP_clicked();

private:
    Ui::QCTraderClass ui;
};
