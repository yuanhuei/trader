/********************************************************************************
** Form generated from reading UI file 'QCTrader.ui'
**
** Created by: Qt User Interface Compiler version 5.12.11
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QCTRADER_H
#define UI_QCTRADER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include<qstandarditemmodel.h>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE




class Ui_QCTraderClass
{
public:
    QAction *actionCTP;
    QAction *actionQuit;
    QAction *actionabout;
    QAction *actionCTACeLue;
    QAction *actionCTAHuiCe;
    QAction *actiondatamanager;
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QWidget *tab;
    QWidget *widget;
    QGridLayout *gridLayout;
    QGroupBox *groupBox;
    QTableView *tableView;
    QGroupBox *groupBox_2;
    QTableView *tableView_2;
    QGroupBox *groupBox_3;
    QTableView *tableView_3;
    QWidget *tab_2;
    QWidget *layoutWidget;
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox_4;
    QTableView *tableView_4;
    QGroupBox *groupBox_5;
    QTableView *tableView_5;
    QGroupBox *groupBox_6;
    QTableView *tableView_6;
    QMenuBar *menuBar;
    QMenu *menu;
    QMenu *menu_2;
    QMenu *menu_3;
    QMenu *menu_4;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    QString str2qstr_new(std::string str)
    {
        return QString::fromLocal8Bit(str.c_str());
    }

    void setupUi(QMainWindow *QCTraderClass)
    {
        if (QCTraderClass->objectName().isEmpty())
            QCTraderClass->setObjectName(QString::fromUtf8("QCTraderClass"));
        QCTraderClass->resize(951, 534);
        actionCTP = new QAction(QCTraderClass);
        actionCTP->setObjectName(QString::fromUtf8("actionCTP"));
        actionQuit = new QAction(QCTraderClass);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionabout = new QAction(QCTraderClass);
        actionabout->setObjectName(QString::fromUtf8("actionabout"));
        actionCTACeLue = new QAction(QCTraderClass);
        actionCTACeLue->setObjectName(QString::fromUtf8("actionCTACeLue"));
        actionCTAHuiCe = new QAction(QCTraderClass);
        actionCTAHuiCe->setObjectName(QString::fromUtf8("actionCTAHuiCe"));
        actiondatamanager = new QAction(QCTraderClass);
        actiondatamanager->setObjectName(QString::fromUtf8("actiondatamanager"));
        centralWidget = new QWidget(QCTraderClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(10, 10, 921, 471));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("行情和交易"));
        widget = new QWidget(tab);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(0, 20, 901, 411));
        gridLayout = new QGridLayout(widget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        groupBox = new QGroupBox(widget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));

        tableView = new QTableView(groupBox);
        tableView->setObjectName(QString::fromUtf8("tableView"));
        tableView->setGeometry(QRect(10, 20, 871, 101));
//////////////////////表头名//////////////////////////////////////////////
        /*
        行情：代码 交易所 名称 最新价 成交量 开盘价 最高价 最低价 买一价 买一量 卖一价 卖一量 时间 接口

            委托：委托号 来源 代码 交易所 类型 方向 开平 价格 总数量 已成交 状态 时间 接口

            成交：成交号 委托号 代码 交易所 方向 开平  价格 数量 时间 接口

            资金：账户 余额 冻结 可用 接口

            持仓：代码 交易所 方向 数量 昨仓 冻结 均价 盈亏 接口

            日志：时间信息 接口
        */

        QStandardItemModel* m_AccountModel = new QStandardItemModel(3,14,tableView);
        QStringList accountheader;
        accountheader << str2qstr_new("代码") << str2qstr_new("交易所") << str2qstr_new("名称") << str2qstr_new("最新价") << str2qstr_new("成交量") << str2qstr_new("开盘价") << str2qstr_new("最高价") << str2qstr_new("最低价") << str2qstr_new("买一价") << str2qstr_new("买一量") << str2qstr_new("卖一价") << str2qstr_new("卖一量") << str2qstr_new("时间") << str2qstr_new("接口");
        m_AccountModel->setHorizontalHeaderLabels(accountheader);
        tableView->setModel(m_AccountModel);

        gridLayout->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_2 = new QGroupBox(widget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        tableView_2 = new QTableView(groupBox_2);
        tableView_2->setObjectName(QString::fromUtf8("tableView_2"));
        tableView_2->setGeometry(QRect(10, 20, 871, 101));
        QStandardItemModel* m_AccountModel2 = new QStandardItemModel(3, 13, tableView);
        QStringList accountheader2;
        accountheader2 << str2qstr_new("委托号") << str2qstr_new("来源") << str2qstr_new("代码") << str2qstr_new("交易所") << str2qstr_new("类型") << str2qstr_new("方向") << str2qstr_new("开平") << str2qstr_new("价格 ") << str2qstr_new("总数量") << str2qstr_new("已成交") << str2qstr_new("状态") << str2qstr_new("时间") << str2qstr_new("接口");
        m_AccountModel2->setHorizontalHeaderLabels(accountheader2);
        tableView_2->setModel(m_AccountModel2);
        gridLayout->addWidget(groupBox_2, 1, 0, 1, 1);

        groupBox_3 = new QGroupBox(widget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        tableView_3 = new QTableView(groupBox_3);
        tableView_3->setObjectName(QString::fromUtf8("tableView_3"));
        tableView_3->setGeometry(QRect(10, 20, 871, 101));
        QStandardItemModel* m_AccountModel3 = new QStandardItemModel(3, 10, tableView);
        QStringList accountheader3;
        accountheader3 << str2qstr_new("成交号") << str2qstr_new("委托号") << str2qstr_new("代码") << str2qstr_new("交易所") << str2qstr_new("方向") << str2qstr_new("开平") << str2qstr_new("价格 ") << str2qstr_new("数量") << str2qstr_new("时间") << str2qstr_new("接口");
        m_AccountModel3->setHorizontalHeaderLabels(accountheader3);
        tableView_3->setModel(m_AccountModel3);
        gridLayout->addWidget(groupBox_3, 2, 0, 1, 1);

        tabWidget->addTab(tab, "dddd");// QString());
        //tabWidget->setTabText(1, "行情和交易");
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2d"));
        layoutWidget = new QWidget(tab_2);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 20, 901, 411));
        gridLayout_2 = new QGridLayout(layoutWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        groupBox_4 = new QGroupBox(layoutWidget);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        tableView_4 = new QTableView(groupBox_4);
        tableView_4->setObjectName(QString::fromUtf8("tableView_4"));
        tableView_4->setGeometry(QRect(10, 20, 871, 101));
        QStandardItemModel* m_AccountModel4 = new QStandardItemModel(3, 5, tableView);
        QStringList accountheader4;
        accountheader4 << str2qstr_new("账户") << str2qstr_new("余额") << str2qstr_new("冻结") << str2qstr_new("可用") << str2qstr_new("接口");// << str2qstr_new("手续费") << str2qstr_new("保证金") << str2qstr_new("平仓盈亏") << str2qstr_new("持仓盈亏");
        m_AccountModel4->setHorizontalHeaderLabels(accountheader4);
        tableView_4->setModel(m_AccountModel4);
        gridLayout_2->addWidget(groupBox_4, 0, 0, 1, 1);

        groupBox_5 = new QGroupBox(layoutWidget);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        tableView_5 = new QTableView(groupBox_5);
        tableView_5->setObjectName(QString::fromUtf8("tableView_5"));
        tableView_5->setGeometry(QRect(10, 20, 871, 101));
        QStandardItemModel* m_AccountModel5 = new QStandardItemModel(3, 9, tableView);
        QStringList accountheader5;
        accountheader5 << str2qstr_new("代码") << str2qstr_new("交易所") << str2qstr_new("方向") << str2qstr_new("数量") << str2qstr_new("昨仓") << str2qstr_new("冻结") << str2qstr_new("均价") << str2qstr_new("盈亏") << str2qstr_new("接口");
        m_AccountModel5->setHorizontalHeaderLabels(accountheader5);
        tableView_5->setModel(m_AccountModel5);
        gridLayout_2->addWidget(groupBox_5, 1, 0, 1, 1);

        groupBox_6 = new QGroupBox(layoutWidget);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        tableView_6 = new QTableView(groupBox_6);
        tableView_6->setObjectName(QString::fromUtf8("tableView_6"));
        tableView_6->setGeometry(QRect(10, 20, 871, 101));
        QStandardItemModel* m_AccountModel6 = new QStandardItemModel(3, 3, tableView);
        QStringList accountheader6;
        accountheader6 << str2qstr_new("时间") << str2qstr_new("信息") << str2qstr_new("接口");// << str2qstr_new("净值") << str2qstr_new("可用") << str2qstr_new("手续费") << str2qstr_new("保证金") << str2qstr_new("平仓盈亏") << str2qstr_new("持仓盈亏");
        m_AccountModel6->setHorizontalHeaderLabels(accountheader6);
        tableView_6->setModel(m_AccountModel6);
        gridLayout_2->addWidget(groupBox_6, 2, 0, 1, 1);

        tabWidget->addTab(tab_2, QString());
        QCTraderClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QCTraderClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 951, 23));
        menu = new QMenu(menuBar);
        menu->setObjectName(QString::fromUtf8("menu"));
        menu_2 = new QMenu(menuBar);
        menu_2->setObjectName(QString::fromUtf8("menu_2"));
        menu_3 = new QMenu(menuBar);
        menu_3->setObjectName(QString::fromUtf8("menu_3"));
        menu_4 = new QMenu(menuBar);
        menu_4->setObjectName(QString::fromUtf8("menu_4"));
        QCTraderClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QCTraderClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        QCTraderClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QCTraderClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        QCTraderClass->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menuBar->addAction(menu_2->menuAction());
        menuBar->addAction(menu_3->menuAction());
        menuBar->addAction(menu_4->menuAction());
        menu->addAction(actionCTP);
        menu->addAction(actionQuit);
        menu_2->addAction(actionCTACeLue);
        menu_2->addAction(actionCTAHuiCe);
        menu_2->addAction(actiondatamanager);
        menu_4->addAction(actionabout);

        retranslateUi(QCTraderClass);
        QObject::connect(actionCTP, SIGNAL(triggered()), QCTraderClass, SLOT(menu_CTP_clicked()));

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(QCTraderClass);
    } // setupUi

    void retranslateUi(QMainWindow *QCTraderClass)
    {
        QCTraderClass->setWindowTitle(QApplication::translate("QCTraderClass", "QCTrader", nullptr));
        actionCTP->setText(QApplication::translate("QCTraderClass", "CTP\350\277\236\346\216\245", nullptr));
        actionQuit->setText(QApplication::translate("QCTraderClass", "\351\200\200\345\207\272", nullptr));
        actionabout->setText(QApplication::translate("QCTraderClass", "\345\205\263\344\272\216", nullptr));
        actionCTACeLue->setText(QApplication::translate("QCTraderClass", "CTA\347\255\226\347\225\245", nullptr));
        actionCTAHuiCe->setText(QApplication::translate("QCTraderClass", "CTA\345\233\236\346\265\213", nullptr));
        actiondatamanager->setText(QApplication::translate("QCTraderClass", "\346\225\260\346\215\256\347\256\241\347\220\206", nullptr));
        groupBox->setTitle(QApplication::translate("QCTraderClass", "\350\241\214\346\203\205", nullptr));
        groupBox_2->setTitle(QApplication::translate("QCTraderClass", "\345\247\224\346\211\230", nullptr));
        groupBox_3->setTitle(QApplication::translate("QCTraderClass", "\346\210\220\344\272\244", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("QCTraderClass", "Tab 1", nullptr));
        groupBox_4->setTitle(QApplication::translate("QCTraderClass", "\350\265\204\351\207\221", nullptr));
        groupBox_5->setTitle(QApplication::translate("QCTraderClass", "\346\214\201\344\273\223", nullptr));
        groupBox_6->setTitle(QApplication::translate("QCTraderClass", "\346\227\245\345\277\227", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("QCTraderClass", "Tab 2", nullptr));
        menu->setTitle(QApplication::translate("QCTraderClass", "\347\263\273\347\273\237", nullptr));
        menu_2->setTitle(QApplication::translate("QCTraderClass", "\345\212\237\350\203\275", nullptr));
        menu_3->setTitle(QApplication::translate("QCTraderClass", "\351\243\216\346\216\247", nullptr));
        menu_4->setTitle(QApplication::translate("QCTraderClass", "\345\270\256\345\212\251", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QCTraderClass: public Ui_QCTraderClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QCTRADER_H
