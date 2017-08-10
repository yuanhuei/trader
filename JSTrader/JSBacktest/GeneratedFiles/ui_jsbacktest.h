/********************************************************************************
** Form generated from reading UI file 'jsbacktest.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_JSBACKTEST_H
#define UI_JSBACKTEST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_JSBacktestClass
{
public:

    void setupUi(QWidget *JSBacktestClass)
    {
        if (JSBacktestClass->objectName().isEmpty())
            JSBacktestClass->setObjectName(QStringLiteral("JSBacktestClass"));
        JSBacktestClass->resize(600, 400);

        retranslateUi(JSBacktestClass);

        QMetaObject::connectSlotsByName(JSBacktestClass);
    } // setupUi

    void retranslateUi(QWidget *JSBacktestClass)
    {
        JSBacktestClass->setWindowTitle(QApplication::translate("JSBacktestClass", "JSBacktest", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class JSBacktestClass: public Ui_JSBacktestClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JSBACKTEST_H
