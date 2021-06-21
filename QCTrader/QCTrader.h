#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QCTrader.h"

class QCTrader : public QMainWindow
{
    Q_OBJECT

public:
    QCTrader(QWidget *parent = Q_NULLPTR);

private:
    Ui::QCTraderClass ui;
};
