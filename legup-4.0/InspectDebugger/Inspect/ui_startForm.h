/********************************************************************************
** Form generated from reading UI file 'startForm.ui'
**
** Created: Mon Jul 14 11:10:31 2014
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STARTFORM_H
#define UI_STARTFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_startForm
{
public:
    QLabel *label;
    QPushButton *pushButtonStart;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QRadioButton *radioButtonSimulationRun;
    QRadioButton *radioButtonOnChipRun;
    QRadioButton *radioButtonGDB;
    QRadioButton *radioButtonGDBSync;
    QRadioButton *radioButtonBugDetection;
    QRadioButton *radioButtonOnChipBugDetection;

    void setupUi(QDialog *startForm)
    {
        if (startForm->objectName().isEmpty())
            startForm->setObjectName(QString::fromUtf8("startForm"));
        startForm->resize(416, 224);
        startForm->setMinimumSize(QSize(416, 224));
        startForm->setMaximumSize(QSize(416, 224));
        QIcon icon;
        icon.addFile(QString::fromUtf8("Debugging.png"), QSize(), QIcon::Normal, QIcon::Off);
        startForm->setWindowIcon(icon);
        label = new QLabel(startForm);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 372, 17));
        pushButtonStart = new QPushButton(startForm);
        pushButtonStart->setObjectName(QString::fromUtf8("pushButtonStart"));
        pushButtonStart->setGeometry(QRect(310, 190, 98, 27));
        layoutWidget = new QWidget(startForm);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(30, 31, 241, 164));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        radioButtonSimulationRun = new QRadioButton(layoutWidget);
        radioButtonSimulationRun->setObjectName(QString::fromUtf8("radioButtonSimulationRun"));

        verticalLayout->addWidget(radioButtonSimulationRun);

        radioButtonOnChipRun = new QRadioButton(layoutWidget);
        radioButtonOnChipRun->setObjectName(QString::fromUtf8("radioButtonOnChipRun"));

        verticalLayout->addWidget(radioButtonOnChipRun);

        radioButtonGDB = new QRadioButton(layoutWidget);
        radioButtonGDB->setObjectName(QString::fromUtf8("radioButtonGDB"));

        verticalLayout->addWidget(radioButtonGDB);

        radioButtonGDBSync = new QRadioButton(layoutWidget);
        radioButtonGDBSync->setObjectName(QString::fromUtf8("radioButtonGDBSync"));

        verticalLayout->addWidget(radioButtonGDBSync);

        radioButtonBugDetection = new QRadioButton(layoutWidget);
        radioButtonBugDetection->setObjectName(QString::fromUtf8("radioButtonBugDetection"));

        verticalLayout->addWidget(radioButtonBugDetection);

        radioButtonOnChipBugDetection = new QRadioButton(layoutWidget);
        radioButtonOnChipBugDetection->setObjectName(QString::fromUtf8("radioButtonOnChipBugDetection"));

        verticalLayout->addWidget(radioButtonOnChipBugDetection);


        retranslateUi(startForm);
        QObject::connect(pushButtonStart, SIGNAL(clicked()), startForm, SLOT(pushButtonStart_clicked()));
        QObject::connect(startForm, SIGNAL(rejected()), startForm, SLOT(startForm_rejected()));

        QMetaObject::connectSlotsByName(startForm);
    } // setupUi

    void retranslateUi(QDialog *startForm)
    {
        startForm->setWindowTitle(QApplication::translate("startForm", "Inspect - LegUp HLS Source-Level Debugger 1.0", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("startForm", "Please select one of the below modes and click on Start:", 0, QApplication::UnicodeUTF8));
        pushButtonStart->setText(QApplication::translate("startForm", "Start", 0, QApplication::UnicodeUTF8));
        radioButtonSimulationRun->setText(QApplication::translate("startForm", "Simulation Run using ModelSim", 0, QApplication::UnicodeUTF8));
        radioButtonOnChipRun->setText(QApplication::translate("startForm", "OnChip Run using Quartus II", 0, QApplication::UnicodeUTF8));
        radioButtonGDB->setText(QApplication::translate("startForm", "GDB Run", 0, QApplication::UnicodeUTF8));
        radioButtonGDBSync->setText(QApplication::translate("startForm", "GDB Sync Mode", 0, QApplication::UnicodeUTF8));
        radioButtonBugDetection->setText(QApplication::translate("startForm", "Bug Detection Mode", 0, QApplication::UnicodeUTF8));
        radioButtonOnChipBugDetection->setText(QApplication::translate("startForm", "OnChip Bug Detection", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class startForm: public Ui_startForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STARTFORM_H
