/********************************************************************************
** Form generated from reading UI file 'mainForm.ui'
**
** Created: Tue Oct 29 10:45:48 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINFORM_H
#define UI_MAINFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDockWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_mainForm
{
public:
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_7;
    QLabel *labelHighLevel;
    QLabel *labelIR;
    QLabel *labelHardware;
    QLabel *labelWatch;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QTextEdit *textEditHighLevel;
    QTreeWidget *treeWidgetIR;
    QListWidget *listWidgetHardware;
    QTreeWidget *treeWidgetWatch;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QPushButton *pushButtonOpenConnection;
    QPushButton *pushButtonLoadDesign;
    QPushButton *pushButtonSingleStepping;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLabel *labelCurrentState;
    QVBoxLayout *verticalLayout_3;
    QPushButton *pushButtonExamine;
    QPushButton *pushButtonRun;
    QLabel *labelStepIn;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QPushButton *pushButtonExit;

    void setupUi(QDialog *mainForm)
    {
        if (mainForm->objectName().isEmpty())
            mainForm->setObjectName(QString::fromUtf8("mainForm"));
        mainForm->setWindowModality(Qt::WindowModal);
        mainForm->resize(778, 552);
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(mainForm->sizePolicy().hasHeightForWidth());
        mainForm->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8("Debugging.png"), QSize(), QIcon::Normal, QIcon::Off);
        mainForm->setWindowIcon(icon);
        mainForm->setModal(false);
        verticalLayout_5 = new QVBoxLayout(mainForm);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        labelHighLevel = new QLabel(mainForm);
        labelHighLevel->setObjectName(QString::fromUtf8("labelHighLevel"));
        labelHighLevel->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(labelHighLevel);

        labelIR = new QLabel(mainForm);
        labelIR->setObjectName(QString::fromUtf8("labelIR"));
        labelIR->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(labelIR);

        labelHardware = new QLabel(mainForm);
        labelHardware->setObjectName(QString::fromUtf8("labelHardware"));
        labelHardware->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(labelHardware);

        labelWatch = new QLabel(mainForm);
        labelWatch->setObjectName(QString::fromUtf8("labelWatch"));
        labelWatch->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(labelWatch);


        verticalLayout->addLayout(horizontalLayout_7);

        horizontalSpacer = new QSpacerItem(13, 13, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout->addItem(horizontalSpacer);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setSizeConstraint(QLayout::SetDefaultConstraint);
        textEditHighLevel = new QTextEdit(mainForm);
        textEditHighLevel->setObjectName(QString::fromUtf8("textEditHighLevel"));

        horizontalLayout_2->addWidget(textEditHighLevel);

        treeWidgetIR = new QTreeWidget(mainForm);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidgetIR->setHeaderItem(__qtreewidgetitem);
        treeWidgetIR->setObjectName(QString::fromUtf8("treeWidgetIR"));
        treeWidgetIR->header()->setVisible(false);

        horizontalLayout_2->addWidget(treeWidgetIR);

        listWidgetHardware = new QListWidget(mainForm);
        listWidgetHardware->setObjectName(QString::fromUtf8("listWidgetHardware"));

        horizontalLayout_2->addWidget(listWidgetHardware);

        treeWidgetWatch = new QTreeWidget(mainForm);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem();
        __qtreewidgetitem1->setText(0, QString::fromUtf8("1"));
        treeWidgetWatch->setHeaderItem(__qtreewidgetitem1);
        treeWidgetWatch->setObjectName(QString::fromUtf8("treeWidgetWatch"));
        treeWidgetWatch->setFrameShape(QFrame::StyledPanel);
        treeWidgetWatch->setFrameShadow(QFrame::Sunken);
        treeWidgetWatch->setLineWidth(1);
        treeWidgetWatch->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        treeWidgetWatch->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        treeWidgetWatch->setRootIsDecorated(true);
        treeWidgetWatch->header()->setVisible(false);

        horizontalLayout_2->addWidget(treeWidgetWatch);


        verticalLayout->addLayout(horizontalLayout_2);


        verticalLayout_4->addLayout(verticalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        pushButtonOpenConnection = new QPushButton(mainForm);
        pushButtonOpenConnection->setObjectName(QString::fromUtf8("pushButtonOpenConnection"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButtonOpenConnection->sizePolicy().hasHeightForWidth());
        pushButtonOpenConnection->setSizePolicy(sizePolicy1);

        verticalLayout_2->addWidget(pushButtonOpenConnection);

        pushButtonLoadDesign = new QPushButton(mainForm);
        pushButtonLoadDesign->setObjectName(QString::fromUtf8("pushButtonLoadDesign"));
        sizePolicy1.setHeightForWidth(pushButtonLoadDesign->sizePolicy().hasHeightForWidth());
        pushButtonLoadDesign->setSizePolicy(sizePolicy1);

        verticalLayout_2->addWidget(pushButtonLoadDesign);

        pushButtonSingleStepping = new QPushButton(mainForm);
        pushButtonSingleStepping->setObjectName(QString::fromUtf8("pushButtonSingleStepping"));
        sizePolicy1.setHeightForWidth(pushButtonSingleStepping->sizePolicy().hasHeightForWidth());
        pushButtonSingleStepping->setSizePolicy(sizePolicy1);

        verticalLayout_2->addWidget(pushButtonSingleStepping);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(mainForm);
        label->setObjectName(QString::fromUtf8("label"));
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(label);

        labelCurrentState = new QLabel(mainForm);
        labelCurrentState->setObjectName(QString::fromUtf8("labelCurrentState"));

        horizontalLayout->addWidget(labelCurrentState);


        verticalLayout_2->addLayout(horizontalLayout);


        horizontalLayout_3->addLayout(verticalLayout_2);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        pushButtonExamine = new QPushButton(mainForm);
        pushButtonExamine->setObjectName(QString::fromUtf8("pushButtonExamine"));
        sizePolicy1.setHeightForWidth(pushButtonExamine->sizePolicy().hasHeightForWidth());
        pushButtonExamine->setSizePolicy(sizePolicy1);
        pushButtonExamine->setLayoutDirection(Qt::LeftToRight);

        verticalLayout_3->addWidget(pushButtonExamine);

        pushButtonRun = new QPushButton(mainForm);
        pushButtonRun->setObjectName(QString::fromUtf8("pushButtonRun"));
        sizePolicy1.setHeightForWidth(pushButtonRun->sizePolicy().hasHeightForWidth());
        pushButtonRun->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(pushButtonRun);

        labelStepIn = new QLabel(mainForm);
        labelStepIn->setObjectName(QString::fromUtf8("labelStepIn"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(labelStepIn->sizePolicy().hasHeightForWidth());
        labelStepIn->setSizePolicy(sizePolicy2);
        labelStepIn->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(labelStepIn);


        horizontalLayout_3->addLayout(verticalLayout_3);

        dockWidget = new QDockWidget(mainForm);
        dockWidget->setObjectName(QString::fromUtf8("dockWidget"));
        sizePolicy1.setHeightForWidth(dockWidget->sizePolicy().hasHeightForWidth());
        dockWidget->setSizePolicy(sizePolicy1);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        pushButtonExit = new QPushButton(dockWidgetContents);
        pushButtonExit->setObjectName(QString::fromUtf8("pushButtonExit"));
        pushButtonExit->setGeometry(QRect(70, 10, 85, 27));
        sizePolicy1.setHeightForWidth(pushButtonExit->sizePolicy().hasHeightForWidth());
        pushButtonExit->setSizePolicy(sizePolicy1);
        dockWidget->setWidget(dockWidgetContents);

        horizontalLayout_3->addWidget(dockWidget);


        verticalLayout_4->addLayout(horizontalLayout_3);


        verticalLayout_5->addLayout(verticalLayout_4);


        retranslateUi(mainForm);
        QObject::connect(pushButtonOpenConnection, SIGNAL(clicked()), mainForm, SLOT(pushButtonOpenConnection_clicked()));
        QObject::connect(pushButtonRun, SIGNAL(clicked()), mainForm, SLOT(pushButtonRun_clicked()));
        QObject::connect(pushButtonExamine, SIGNAL(clicked()), mainForm, SLOT(pushButtonExamine_clicked()));
        QObject::connect(pushButtonSingleStepping, SIGNAL(clicked()), mainForm, SLOT(pushButtonSingleStepping_clicked()));
        QObject::connect(pushButtonLoadDesign, SIGNAL(clicked()), mainForm, SLOT(pushButtonLoadDesign_clicked()));
        QObject::connect(treeWidgetIR, SIGNAL(itemClicked(QTreeWidgetItem*,int)), mainForm, SLOT(treeWidgetIR_itemClicked(QTreeWidgetItem*)));
        QObject::connect(pushButtonExit, SIGNAL(clicked()), mainForm, SLOT(reject()));

        QMetaObject::connectSlotsByName(mainForm);
    } // setupUi

    void retranslateUi(QDialog *mainForm)
    {
        mainForm->setWindowTitle(QApplication::translate("mainForm", "HLS Debugger 1.0", 0, QApplication::UnicodeUTF8));
        labelHighLevel->setText(QApplication::translate("mainForm", "C Code", 0, QApplication::UnicodeUTF8));
        labelIR->setText(QApplication::translate("mainForm", "IR Instruction", 0, QApplication::UnicodeUTF8));
        labelHardware->setText(QApplication::translate("mainForm", "Verilog Code", 0, QApplication::UnicodeUTF8));
        labelWatch->setText(QApplication::translate("mainForm", "Watch", 0, QApplication::UnicodeUTF8));
        pushButtonOpenConnection->setText(QApplication::translate("mainForm", "Open Connection", 0, QApplication::UnicodeUTF8));
        pushButtonLoadDesign->setText(QApplication::translate("mainForm", "Load Design", 0, QApplication::UnicodeUTF8));
        pushButtonSingleStepping->setText(QApplication::translate("mainForm", "Single Stepping", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("mainForm", "Current State:", 0, QApplication::UnicodeUTF8));
        labelCurrentState->setText(QApplication::translate("mainForm", "...", 0, QApplication::UnicodeUTF8));
        pushButtonExamine->setText(QApplication::translate("mainForm", "Examine", 0, QApplication::UnicodeUTF8));
        pushButtonRun->setText(QApplication::translate("mainForm", "Run", 0, QApplication::UnicodeUTF8));
        labelStepIn->setText(QApplication::translate("mainForm", "*** Step-In mode ***", 0, QApplication::UnicodeUTF8));
        pushButtonExit->setText(QApplication::translate("mainForm", "Exit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class mainForm: public Ui_mainForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINFORM_H
