/********************************************************************************
** Form generated from reading UI file 'FormSelectSignals.ui'
**
** Created: Tue Oct 29 11:52:42 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORMSELECTSIGNALS_H
#define UI_FORMSELECTSIGNALS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FormSelectSignals
{
public:
    QAction *actionLoad_All_OnChip_Signals;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QTableWidget *tableWidgetFunctions;
    QSpacerItem *verticalSpacer_3;
    QTableWidget *tableWidgetIRs;
    QSpacerItem *verticalSpacer_2;
    QTableWidget *tableWidgetSignals;
    QSpacerItem *verticalSpacer;
    QTableWidget *tableWidgetExtra;
    QSpacerItem *horizontalSpacer;
    QTableWidget *tableWidgetSelectedSignals;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButtonOK;
    QPushButton *pushButtonCancel;
    QMenuBar *menubar;
    QMenu *menuEdit;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *FormSelectSignals)
    {
        if (FormSelectSignals->objectName().isEmpty())
            FormSelectSignals->setObjectName(QString::fromUtf8("FormSelectSignals"));
        FormSelectSignals->resize(737, 597);
        actionLoad_All_OnChip_Signals = new QAction(FormSelectSignals);
        actionLoad_All_OnChip_Signals->setObjectName(QString::fromUtf8("actionLoad_All_OnChip_Signals"));
        centralwidget = new QWidget(FormSelectSignals);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout_4 = new QHBoxLayout(centralwidget);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        tableWidgetFunctions = new QTableWidget(centralwidget);
        tableWidgetFunctions->setObjectName(QString::fromUtf8("tableWidgetFunctions"));
        QFont font;
        font.setPointSize(9);
        font.setBold(true);
        font.setItalic(true);
        font.setWeight(75);
        tableWidgetFunctions->setFont(font);
        tableWidgetFunctions->setEditTriggers(QAbstractItemView::NoEditTriggers);

        horizontalLayout->addWidget(tableWidgetFunctions);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout->addItem(verticalSpacer_3);

        tableWidgetIRs = new QTableWidget(centralwidget);
        tableWidgetIRs->setObjectName(QString::fromUtf8("tableWidgetIRs"));
        tableWidgetIRs->setFont(font);
        tableWidgetIRs->setEditTriggers(QAbstractItemView::NoEditTriggers);

        horizontalLayout->addWidget(tableWidgetIRs);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout->addItem(verticalSpacer_2);

        tableWidgetSignals = new QTableWidget(centralwidget);
        tableWidgetSignals->setObjectName(QString::fromUtf8("tableWidgetSignals"));
        tableWidgetSignals->setFont(font);
        tableWidgetSignals->setEditTriggers(QAbstractItemView::NoEditTriggers);

        horizontalLayout->addWidget(tableWidgetSignals);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout->addItem(verticalSpacer);

        tableWidgetExtra = new QTableWidget(centralwidget);
        tableWidgetExtra->setObjectName(QString::fromUtf8("tableWidgetExtra"));
        tableWidgetExtra->setFont(font);
        tableWidgetExtra->setEditTriggers(QAbstractItemView::NoEditTriggers);

        horizontalLayout->addWidget(tableWidgetExtra);


        verticalLayout->addLayout(horizontalLayout);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout->addItem(horizontalSpacer);

        tableWidgetSelectedSignals = new QTableWidget(centralwidget);
        tableWidgetSelectedSignals->setObjectName(QString::fromUtf8("tableWidgetSelectedSignals"));
        tableWidgetSelectedSignals->setMaximumSize(QSize(16777215, 150));
        tableWidgetSelectedSignals->setFont(font);
        tableWidgetSelectedSignals->setEditTriggers(QAbstractItemView::NoEditTriggers);

        verticalLayout->addWidget(tableWidgetSelectedSignals);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));

        verticalLayout->addLayout(horizontalLayout_3);


        verticalLayout_2->addLayout(verticalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pushButtonOK = new QPushButton(centralwidget);
        pushButtonOK->setObjectName(QString::fromUtf8("pushButtonOK"));
        pushButtonOK->setMaximumSize(QSize(100, 16777215));

        horizontalLayout_2->addWidget(pushButtonOK);

        pushButtonCancel = new QPushButton(centralwidget);
        pushButtonCancel->setObjectName(QString::fromUtf8("pushButtonCancel"));
        pushButtonCancel->setMaximumSize(QSize(100, 16777215));

        horizontalLayout_2->addWidget(pushButtonCancel);


        verticalLayout_2->addLayout(horizontalLayout_2);


        horizontalLayout_4->addLayout(verticalLayout_2);

        FormSelectSignals->setCentralWidget(centralwidget);
        menubar = new QMenuBar(FormSelectSignals);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 737, 25));
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        FormSelectSignals->setMenuBar(menubar);
        statusbar = new QStatusBar(FormSelectSignals);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        FormSelectSignals->setStatusBar(statusbar);

        menubar->addAction(menuEdit->menuAction());
        menuEdit->addAction(actionLoad_All_OnChip_Signals);

        retranslateUi(FormSelectSignals);
        QObject::connect(tableWidgetFunctions, SIGNAL(itemClicked(QTableWidgetItem*)), FormSelectSignals, SLOT(tableWidgetFunctions_ItemClicked(QTableWidgetItem*)));
        QObject::connect(tableWidgetIRs, SIGNAL(itemClicked(QTableWidgetItem*)), FormSelectSignals, SLOT(tableWidgetIRs_ItemClicked(QTableWidgetItem*)));
        QObject::connect(tableWidgetSignals, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), FormSelectSignals, SLOT(tableWidgetSignals_ItemDoubleClicked(QTableWidgetItem*)));
        QObject::connect(tableWidgetSelectedSignals, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), FormSelectSignals, SLOT(tableWidgetSelectedSignals_ItemDoubleClicked(QTableWidgetItem*)));
        QObject::connect(actionLoad_All_OnChip_Signals, SIGNAL(triggered()), FormSelectSignals, SLOT(actionLoadAllOnChipSignals()));
        QObject::connect(tableWidgetExtra, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), FormSelectSignals, SLOT(tableWidgetExtra_ItemDoubleClicked(QTableWidgetItem*)));
        QObject::connect(pushButtonOK, SIGNAL(clicked()), FormSelectSignals, SLOT(pushButtonOK_clicked()));
        QObject::connect(pushButtonCancel, SIGNAL(clicked()), FormSelectSignals, SLOT(pushButtonCancel_clicked()));

        QMetaObject::connectSlotsByName(FormSelectSignals);
    } // setupUi

    void retranslateUi(QMainWindow *FormSelectSignals)
    {
        FormSelectSignals->setWindowTitle(QApplication::translate("FormSelectSignals", "FormSelectSignals", 0, QApplication::UnicodeUTF8));
        actionLoad_All_OnChip_Signals->setText(QApplication::translate("FormSelectSignals", "Load All OnChip Signals", 0, QApplication::UnicodeUTF8));
        pushButtonOK->setText(QApplication::translate("FormSelectSignals", "OK", 0, QApplication::UnicodeUTF8));
        pushButtonCancel->setText(QApplication::translate("FormSelectSignals", "Cancel", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("FormSelectSignals", "Edit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FormSelectSignals: public Ui_FormSelectSignals {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORMSELECTSIGNALS_H
