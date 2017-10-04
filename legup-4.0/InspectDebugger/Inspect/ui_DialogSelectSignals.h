/********************************************************************************
** Form generated from reading UI file 'DialogSelectSignals.ui'
**
** Created: Mon Jul 14 11:10:31 2014
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGSELECTSIGNALS_H
#define UI_DIALOGSELECTSIGNALS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogSelectSignals
{
public:
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QTableWidget *tableWidgetFunctions;
    QSpacerItem *verticalSpacer_3;
    QTableWidget *tableWidgetIRs;
    QSpacerItem *verticalSpacer_2;
    QTableWidget *tableWidgetSignals;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *verticalLayout_3;
    QPushButton *pushButton;
    QTableWidget *tableWidgetExtra;
    QSpacerItem *horizontalSpacer;
    QTableWidget *tableWidgetSelectedSignals;
    QHBoxLayout *horizontalLayout_3;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogSelectSignals)
    {
        if (DialogSelectSignals->objectName().isEmpty())
            DialogSelectSignals->setObjectName(QString::fromUtf8("DialogSelectSignals"));
        DialogSelectSignals->resize(710, 563);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DialogSelectSignals->sizePolicy().hasHeightForWidth());
        DialogSelectSignals->setSizePolicy(sizePolicy);
        horizontalLayout_2 = new QHBoxLayout(DialogSelectSignals);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        tableWidgetFunctions = new QTableWidget(DialogSelectSignals);
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

        tableWidgetIRs = new QTableWidget(DialogSelectSignals);
        tableWidgetIRs->setObjectName(QString::fromUtf8("tableWidgetIRs"));
        tableWidgetIRs->setFont(font);
        tableWidgetIRs->setEditTriggers(QAbstractItemView::NoEditTriggers);

        horizontalLayout->addWidget(tableWidgetIRs);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout->addItem(verticalSpacer_2);

        tableWidgetSignals = new QTableWidget(DialogSelectSignals);
        tableWidgetSignals->setObjectName(QString::fromUtf8("tableWidgetSignals"));
        tableWidgetSignals->setFont(font);
        tableWidgetSignals->setEditTriggers(QAbstractItemView::NoEditTriggers);

        horizontalLayout->addWidget(tableWidgetSignals);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout->addItem(verticalSpacer);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        pushButton = new QPushButton(DialogSelectSignals);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setMaximumSize(QSize(100, 30));

        verticalLayout_3->addWidget(pushButton);

        tableWidgetExtra = new QTableWidget(DialogSelectSignals);
        tableWidgetExtra->setObjectName(QString::fromUtf8("tableWidgetExtra"));
        tableWidgetExtra->setFont(font);
        tableWidgetExtra->setEditTriggers(QAbstractItemView::NoEditTriggers);

        verticalLayout_3->addWidget(tableWidgetExtra);


        horizontalLayout->addLayout(verticalLayout_3);


        verticalLayout->addLayout(horizontalLayout);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout->addItem(horizontalSpacer);

        tableWidgetSelectedSignals = new QTableWidget(DialogSelectSignals);
        tableWidgetSelectedSignals->setObjectName(QString::fromUtf8("tableWidgetSelectedSignals"));
        tableWidgetSelectedSignals->setMaximumSize(QSize(16777215, 150));
        tableWidgetSelectedSignals->setFont(font);
        tableWidgetSelectedSignals->setEditTriggers(QAbstractItemView::NoEditTriggers);

        verticalLayout->addWidget(tableWidgetSelectedSignals);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));

        verticalLayout->addLayout(horizontalLayout_3);


        verticalLayout_4->addLayout(verticalLayout);

        buttonBox = new QDialogButtonBox(DialogSelectSignals);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_4->addWidget(buttonBox);


        horizontalLayout_2->addLayout(verticalLayout_4);


        retranslateUi(DialogSelectSignals);
        QObject::connect(buttonBox, SIGNAL(accepted()), DialogSelectSignals, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DialogSelectSignals, SLOT(reject()));
        QObject::connect(tableWidgetFunctions, SIGNAL(itemClicked(QTableWidgetItem*)), DialogSelectSignals, SLOT(tableWidgetFunctions_ItemClicked(QTableWidgetItem*)));
        QObject::connect(tableWidgetIRs, SIGNAL(itemClicked(QTableWidgetItem*)), DialogSelectSignals, SLOT(tableWidgetIRs_ItemClicked(QTableWidgetItem*)));
        QObject::connect(tableWidgetSignals, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), DialogSelectSignals, SLOT(tableWidgetSignals_ItemDoubleClicked(QTableWidgetItem*)));
        QObject::connect(tableWidgetExtra, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), DialogSelectSignals, SLOT(tableWidgetExtra_ItemDoubleClicked(QTableWidgetItem*)));
        QObject::connect(tableWidgetSelectedSignals, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), DialogSelectSignals, SLOT(tableWidgetSelectedSignals_ItemDoubleClicked(QTableWidgetItem*)));
        QObject::connect(pushButton, SIGNAL(clicked()), DialogSelectSignals, SLOT(pushButtonLoadSignals_clicked()));

        QMetaObject::connectSlotsByName(DialogSelectSignals);
    } // setupUi

    void retranslateUi(QDialog *DialogSelectSignals)
    {
        DialogSelectSignals->setWindowTitle(QApplication::translate("DialogSelectSignals", "DialogSelectSignals", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("DialogSelectSignals", "Load Signals", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DialogSelectSignals: public Ui_DialogSelectSignals {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGSELECTSIGNALS_H
