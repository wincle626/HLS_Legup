/********************************************************************************
** Form generated from reading UI file 'formMain.ui'
**
** Created: Mon Jul 7 10:48:14 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_formMain_H
#define UI_formMain_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_formMain
{
public:
    QAction *actionView_IR_Instructions;
    QAction *actionView_verilog_Code;
    QAction *actionView_Watch_Signals;
    QAction *actionOpen_Connection;
    QAction *actionLoad_Design;
    QAction *actionExit;
    QAction *actionReBuildCode;
    QAction *actionRun_On_Chip;
    QAction *actionSelect_Signals;
    QAction *actionAutomatic_Signal_Selection;
    QAction *actionRun_Reference_Sim;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_7;
    QTabWidget *tabWidget;
    QWidget *tabDebug;
    QVBoxLayout *verticalLayout_5;
    QSplitter *splitter;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButtonSingleStepping;
    QPushButton *pushButtonContinue;
    QVBoxLayout *verticalLayout;
    QRadioButton *radioButtonStepInto;
    QRadioButton *radioButtonStepOver;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLabel *labelCurrentState;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_6;
    QVBoxLayout *verticalLayout_3;
    QLabel *labelHighLevel;
    QHBoxLayout *horizontalLayout_4;
    QTextEdit *textEditLineNumber;
    QTextEdit *textEditHighLevel;
    QVBoxLayout *verticalLayout_6;
    QLabel *labelIR;
    QTreeWidget *treeWidgetIR;
    QVBoxLayout *verticalLayout_7;
    QLabel *labelHardware;
    QListWidget *listWidgetHardware;
    QVBoxLayout *verticalLayout_4;
    QLabel *labelWatch;
    QHBoxLayout *horizontalLayout_5;
    QVBoxLayout *verticalLayout_2;
    QTreeWidget *treeWidgetWatch;
    QLabel *labelWatch_3;
    QTreeWidget *treeWidgetHWValues;
    QLabel *labelWatchSWVariables;
    QTableWidget *tableWidgetSWVariables;
    QWidget *tabDiscrepanyLog;
    QHBoxLayout *horizontalLayout_8;
    QTableWidget *plainTableEditLog;
    QWidget *tabSourceCode;
    QHBoxLayout *horizontalLayout_9;
    QTextEdit *textEditCSource;
    QWidget *tabVerilogCode;
    QHBoxLayout *horizontalLayout_10;
    QTextEdit *textEditVerilogFile;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuView;
    QMenu *menuDebug;
    QMenu *menuEdit;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *formMain)
    {
        if (formMain->objectName().isEmpty())
            formMain->setObjectName(QString::fromUtf8("formMain"));
        formMain->resize(958, 702);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(formMain->sizePolicy().hasHeightForWidth());
        formMain->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8("Debugging.png"), QSize(), QIcon::Normal, QIcon::Off);
        formMain->setWindowIcon(icon);
        actionView_IR_Instructions = new QAction(formMain);
        actionView_IR_Instructions->setObjectName(QString::fromUtf8("actionView_IR_Instructions"));
        actionView_IR_Instructions->setCheckable(true);
        actionView_IR_Instructions->setChecked(true);
        actionView_verilog_Code = new QAction(formMain);
        actionView_verilog_Code->setObjectName(QString::fromUtf8("actionView_verilog_Code"));
        actionView_verilog_Code->setCheckable(true);
        actionView_verilog_Code->setChecked(true);
        actionView_Watch_Signals = new QAction(formMain);
        actionView_Watch_Signals->setObjectName(QString::fromUtf8("actionView_Watch_Signals"));
        actionView_Watch_Signals->setCheckable(true);
        actionView_Watch_Signals->setChecked(true);
        actionOpen_Connection = new QAction(formMain);
        actionOpen_Connection->setObjectName(QString::fromUtf8("actionOpen_Connection"));
        actionLoad_Design = new QAction(formMain);
        actionLoad_Design->setObjectName(QString::fromUtf8("actionLoad_Design"));
        actionExit = new QAction(formMain);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionReBuildCode = new QAction(formMain);
        actionReBuildCode->setObjectName(QString::fromUtf8("actionReBuildCode"));
        actionRun_On_Chip = new QAction(formMain);
        actionRun_On_Chip->setObjectName(QString::fromUtf8("actionRun_On_Chip"));
        actionSelect_Signals = new QAction(formMain);
        actionSelect_Signals->setObjectName(QString::fromUtf8("actionSelect_Signals"));
        actionSelect_Signals->setCheckable(false);
        actionAutomatic_Signal_Selection = new QAction(formMain);
        actionAutomatic_Signal_Selection->setObjectName(QString::fromUtf8("actionAutomatic_Signal_Selection"));
        actionRun_Reference_Sim = new QAction(formMain);
        actionRun_Reference_Sim->setObjectName(QString::fromUtf8("actionRun_Reference_Sim"));
        centralwidget = new QWidget(formMain);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy1);
        horizontalLayout_7 = new QHBoxLayout(centralwidget);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabDebug = new QWidget();
        tabDebug->setObjectName(QString::fromUtf8("tabDebug"));
        verticalLayout_5 = new QVBoxLayout(tabDebug);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        splitter = new QSplitter(tabDebug);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy);
        splitter->setOrientation(Qt::Vertical);
        layoutWidget = new QWidget(splitter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        pushButtonSingleStepping = new QPushButton(layoutWidget);
        pushButtonSingleStepping->setObjectName(QString::fromUtf8("pushButtonSingleStepping"));
        sizePolicy.setHeightForWidth(pushButtonSingleStepping->sizePolicy().hasHeightForWidth());
        pushButtonSingleStepping->setSizePolicy(sizePolicy);
        pushButtonSingleStepping->setMaximumSize(QSize(120, 40));

        horizontalLayout_3->addWidget(pushButtonSingleStepping);

        pushButtonContinue = new QPushButton(layoutWidget);
        pushButtonContinue->setObjectName(QString::fromUtf8("pushButtonContinue"));
        sizePolicy.setHeightForWidth(pushButtonContinue->sizePolicy().hasHeightForWidth());
        pushButtonContinue->setSizePolicy(sizePolicy);
        pushButtonContinue->setMaximumSize(QSize(120, 40));

        horizontalLayout_3->addWidget(pushButtonContinue);


        horizontalLayout_2->addLayout(horizontalLayout_3);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        radioButtonStepInto = new QRadioButton(layoutWidget);
        radioButtonStepInto->setObjectName(QString::fromUtf8("radioButtonStepInto"));
        sizePolicy.setHeightForWidth(radioButtonStepInto->sizePolicy().hasHeightForWidth());
        radioButtonStepInto->setSizePolicy(sizePolicy);
        radioButtonStepInto->setMaximumSize(QSize(16777215, 20));

        verticalLayout->addWidget(radioButtonStepInto);

        radioButtonStepOver = new QRadioButton(layoutWidget);
        radioButtonStepOver->setObjectName(QString::fromUtf8("radioButtonStepOver"));
        sizePolicy.setHeightForWidth(radioButtonStepOver->sizePolicy().hasHeightForWidth());
        radioButtonStepOver->setSizePolicy(sizePolicy);
        radioButtonStepOver->setMaximumSize(QSize(16777215, 20));
        radioButtonStepOver->setChecked(true);

        verticalLayout->addWidget(radioButtonStepOver);


        horizontalLayout_2->addLayout(verticalLayout);

        horizontalSpacer = new QSpacerItem(258, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(label);

        labelCurrentState = new QLabel(layoutWidget);
        labelCurrentState->setObjectName(QString::fromUtf8("labelCurrentState"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(labelCurrentState->sizePolicy().hasHeightForWidth());
        labelCurrentState->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(labelCurrentState);


        horizontalLayout_2->addLayout(horizontalLayout);

        splitter->addWidget(layoutWidget);
        layoutWidget1 = new QWidget(splitter);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        horizontalLayout_6 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalLayout_6->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        labelHighLevel = new QLabel(layoutWidget1);
        labelHighLevel->setObjectName(QString::fromUtf8("labelHighLevel"));
        labelHighLevel->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(labelHighLevel);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        textEditLineNumber = new QTextEdit(layoutWidget1);
        textEditLineNumber->setObjectName(QString::fromUtf8("textEditLineNumber"));
        textEditLineNumber->setMaximumSize(QSize(50, 16777215));
        textEditLineNumber->setLineWrapMode(QTextEdit::NoWrap);
        textEditLineNumber->setReadOnly(true);

        horizontalLayout_4->addWidget(textEditLineNumber);

        textEditHighLevel = new QTextEdit(layoutWidget1);
        textEditHighLevel->setObjectName(QString::fromUtf8("textEditHighLevel"));
        textEditHighLevel->setMinimumSize(QSize(300, 0));
        textEditHighLevel->setFrameShape(QFrame::StyledPanel);
        textEditHighLevel->setLineWrapMode(QTextEdit::NoWrap);

        horizontalLayout_4->addWidget(textEditHighLevel);


        verticalLayout_3->addLayout(horizontalLayout_4);


        horizontalLayout_6->addLayout(verticalLayout_3);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        labelIR = new QLabel(layoutWidget1);
        labelIR->setObjectName(QString::fromUtf8("labelIR"));
        labelIR->setAlignment(Qt::AlignCenter);

        verticalLayout_6->addWidget(labelIR);

        treeWidgetIR = new QTreeWidget(layoutWidget1);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidgetIR->setHeaderItem(__qtreewidgetitem);
        treeWidgetIR->setObjectName(QString::fromUtf8("treeWidgetIR"));
        treeWidgetIR->header()->setVisible(false);

        verticalLayout_6->addWidget(treeWidgetIR);


        horizontalLayout_6->addLayout(verticalLayout_6);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        labelHardware = new QLabel(layoutWidget1);
        labelHardware->setObjectName(QString::fromUtf8("labelHardware"));
        labelHardware->setAlignment(Qt::AlignCenter);

        verticalLayout_7->addWidget(labelHardware);

        listWidgetHardware = new QListWidget(layoutWidget1);
        listWidgetHardware->setObjectName(QString::fromUtf8("listWidgetHardware"));

        verticalLayout_7->addWidget(listWidgetHardware);


        horizontalLayout_6->addLayout(verticalLayout_7);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        labelWatch = new QLabel(layoutWidget1);
        labelWatch->setObjectName(QString::fromUtf8("labelWatch"));
        labelWatch->setAlignment(Qt::AlignCenter);

        verticalLayout_4->addWidget(labelWatch);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        treeWidgetWatch = new QTreeWidget(layoutWidget1);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem();
        __qtreewidgetitem1->setText(0, QString::fromUtf8("1"));
        treeWidgetWatch->setHeaderItem(__qtreewidgetitem1);
        treeWidgetWatch->setObjectName(QString::fromUtf8("treeWidgetWatch"));
        sizePolicy.setHeightForWidth(treeWidgetWatch->sizePolicy().hasHeightForWidth());
        treeWidgetWatch->setSizePolicy(sizePolicy);
        treeWidgetWatch->setMaximumSize(QSize(16777215, 16777215));
        treeWidgetWatch->setFrameShape(QFrame::StyledPanel);
        treeWidgetWatch->setFrameShadow(QFrame::Sunken);
        treeWidgetWatch->setLineWidth(1);
        treeWidgetWatch->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        treeWidgetWatch->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        treeWidgetWatch->setRootIsDecorated(true);
        treeWidgetWatch->header()->setVisible(false);

        verticalLayout_2->addWidget(treeWidgetWatch);

        labelWatch_3 = new QLabel(layoutWidget1);
        labelWatch_3->setObjectName(QString::fromUtf8("labelWatch_3"));
        labelWatch_3->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(labelWatch_3);

        treeWidgetHWValues = new QTreeWidget(layoutWidget1);
        QTreeWidgetItem *__qtreewidgetitem2 = new QTreeWidgetItem();
        __qtreewidgetitem2->setText(0, QString::fromUtf8("1"));
        treeWidgetHWValues->setHeaderItem(__qtreewidgetitem2);
        treeWidgetHWValues->setObjectName(QString::fromUtf8("treeWidgetHWValues"));
        treeWidgetHWValues->setAlternatingRowColors(true);
        treeWidgetHWValues->setHeaderHidden(true);

        verticalLayout_2->addWidget(treeWidgetHWValues);

        labelWatchSWVariables = new QLabel(layoutWidget1);
        labelWatchSWVariables->setObjectName(QString::fromUtf8("labelWatchSWVariables"));
        labelWatchSWVariables->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(labelWatchSWVariables);

        tableWidgetSWVariables = new QTableWidget(layoutWidget1);
        tableWidgetSWVariables->setObjectName(QString::fromUtf8("tableWidgetSWVariables"));
        tableWidgetSWVariables->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        tableWidgetSWVariables->horizontalHeader()->setDefaultSectionSize(140);
        tableWidgetSWVariables->horizontalHeader()->setMinimumSectionSize(80);
        tableWidgetSWVariables->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_2->addWidget(tableWidgetSWVariables);


        horizontalLayout_5->addLayout(verticalLayout_2);


        verticalLayout_4->addLayout(horizontalLayout_5);


        horizontalLayout_6->addLayout(verticalLayout_4);

        splitter->addWidget(layoutWidget1);

        verticalLayout_5->addWidget(splitter);

        tabWidget->addTab(tabDebug, QString());
        tabDiscrepanyLog = new QWidget();
        tabDiscrepanyLog->setObjectName(QString::fromUtf8("tabDiscrepanyLog"));
        horizontalLayout_8 = new QHBoxLayout(tabDiscrepanyLog);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        plainTableEditLog = new QTableWidget(tabDiscrepanyLog);
        plainTableEditLog->setObjectName(QString::fromUtf8("plainTableEditLog"));

        horizontalLayout_8->addWidget(plainTableEditLog);

        tabWidget->addTab(tabDiscrepanyLog, QString());
        tabSourceCode = new QWidget();
        tabSourceCode->setObjectName(QString::fromUtf8("tabSourceCode"));
        horizontalLayout_9 = new QHBoxLayout(tabSourceCode);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        textEditCSource = new QTextEdit(tabSourceCode);
        textEditCSource->setObjectName(QString::fromUtf8("textEditCSource"));
        textEditCSource->setMinimumSize(QSize(300, 0));
        textEditCSource->setFrameShape(QFrame::StyledPanel);
        textEditCSource->setLineWrapMode(QTextEdit::NoWrap);

        horizontalLayout_9->addWidget(textEditCSource);

        tabWidget->addTab(tabSourceCode, QString());
        tabVerilogCode = new QWidget();
        tabVerilogCode->setObjectName(QString::fromUtf8("tabVerilogCode"));
        horizontalLayout_10 = new QHBoxLayout(tabVerilogCode);
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        textEditVerilogFile = new QTextEdit(tabVerilogCode);
        textEditVerilogFile->setObjectName(QString::fromUtf8("textEditVerilogFile"));
        textEditVerilogFile->setMinimumSize(QSize(300, 0));
        textEditVerilogFile->setFrameShape(QFrame::StyledPanel);
        textEditVerilogFile->setLineWrapMode(QTextEdit::NoWrap);

        horizontalLayout_10->addWidget(textEditVerilogFile);

        tabWidget->addTab(tabVerilogCode, QString());

        horizontalLayout_7->addWidget(tabWidget);

        formMain->setCentralWidget(centralwidget);
        menuBar = new QMenuBar(formMain);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 958, 25));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuDebug = new QMenu(menuBar);
        menuDebug->setObjectName(QString::fromUtf8("menuDebug"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        formMain->setMenuBar(menuBar);
        statusbar = new QStatusBar(formMain);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        formMain->setStatusBar(statusbar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuDebug->menuAction());
        menuFile->addAction(actionOpen_Connection);
        menuFile->addAction(actionLoad_Design);
        menuFile->addAction(actionRun_Reference_Sim);
        menuFile->addAction(actionRun_On_Chip);
        menuFile->addAction(actionExit);
        menuView->addAction(actionView_IR_Instructions);
        menuView->addAction(actionView_verilog_Code);
        menuView->addAction(actionView_Watch_Signals);
        menuDebug->addAction(actionReBuildCode);
        menuEdit->addAction(actionSelect_Signals);
        menuEdit->addAction(actionAutomatic_Signal_Selection);

        retranslateUi(formMain);
        QObject::connect(actionView_IR_Instructions, SIGNAL(changed()), formMain, SLOT(actionView_IR_Instructions_changed()));
        QObject::connect(actionView_verilog_Code, SIGNAL(changed()), formMain, SLOT(actionView_HW_Info_changed()));
        QObject::connect(actionView_Watch_Signals, SIGNAL(changed()), formMain, SLOT(actionView_Watch_changed()));
        QObject::connect(actionOpen_Connection, SIGNAL(triggered()), formMain, SLOT(pushButtonOpenConnection_clicked()));
        QObject::connect(actionLoad_Design, SIGNAL(triggered()), formMain, SLOT(pushButtonLoadDesign_clicked()));
        QObject::connect(actionExit, SIGNAL(triggered()), formMain, SLOT(pushButtonExit_clicked()));
        QObject::connect(actionReBuildCode, SIGNAL(triggered()), formMain, SLOT(actionReBuildCode_clicked()));
        QObject::connect(actionSelect_Signals, SIGNAL(triggered()), formMain, SLOT(actionSelectSignals_clicked()));
        QObject::connect(pushButtonSingleStepping, SIGNAL(clicked()), formMain, SLOT(pushButtonSingleStepping_clicked()));
        QObject::connect(pushButtonContinue, SIGNAL(clicked()), formMain, SLOT(pushButtonContinue_clicked()));
        QObject::connect(textEditLineNumber, SIGNAL(cursorPositionChanged()), formMain, SLOT(textEditLineNumbers_CursorPositionChanged()));
        QObject::connect(textEditHighLevel, SIGNAL(cursorPositionChanged()), formMain, SLOT(textEditHighLevel_CursorPositionChanged()));
        QObject::connect(radioButtonStepInto, SIGNAL(toggled(bool)), formMain, SLOT(radioButtonStepInto_Toggled(bool)));
        QObject::connect(treeWidgetIR, SIGNAL(itemClicked(QTreeWidgetItem*,int)), formMain, SLOT(treeWidgetIR_itemClicked(QTreeWidgetItem*,int)));
        QObject::connect(actionRun_On_Chip, SIGNAL(triggered()), formMain, SLOT(actionRunOnChip_clicked()));
        QObject::connect(actionAutomatic_Signal_Selection, SIGNAL(triggered()), formMain, SLOT(actionAutomaticSignalSelection_clicked()));
        QObject::connect(actionRun_Reference_Sim, SIGNAL(triggered()), formMain, SLOT(actionRunReferenceSim_clicked()));
        QObject::connect(plainTableEditLog, SIGNAL(itemClicked(QTableWidgetItem*)), formMain, SLOT(plainTableEditLog_itemClicked(QTableWidgetItem*)));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(formMain);
    } // setupUi

    void retranslateUi(QMainWindow *formMain)
    {
        formMain->setWindowTitle(QApplication::translate("formMain", "Inspect - LegUp HLS Source-Level Debugger 1.0", 0, QApplication::UnicodeUTF8));
        actionView_IR_Instructions->setText(QApplication::translate("formMain", "View IR Instructions", 0, QApplication::UnicodeUTF8));
        actionView_verilog_Code->setText(QApplication::translate("formMain", "View verilog Code", 0, QApplication::UnicodeUTF8));
        actionView_Watch_Signals->setText(QApplication::translate("formMain", "Watch Variables and Signals", 0, QApplication::UnicodeUTF8));
        actionOpen_Connection->setText(QApplication::translate("formMain", "Open Connection", 0, QApplication::UnicodeUTF8));
        actionLoad_Design->setText(QApplication::translate("formMain", "Load Design", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("formMain", "Exit", 0, QApplication::UnicodeUTF8));
        actionReBuildCode->setText(QApplication::translate("formMain", "ReBuild Code", 0, QApplication::UnicodeUTF8));
        actionRun_On_Chip->setText(QApplication::translate("formMain", "Run On-Chip", 0, QApplication::UnicodeUTF8));
        actionSelect_Signals->setText(QApplication::translate("formMain", "Select Signals", 0, QApplication::UnicodeUTF8));
        actionAutomatic_Signal_Selection->setText(QApplication::translate("formMain", "Automatic Signal Selection", 0, QApplication::UnicodeUTF8));
        actionRun_Reference_Sim->setText(QApplication::translate("formMain", "Run Reference Sim", 0, QApplication::UnicodeUTF8));
        pushButtonSingleStepping->setText(QApplication::translate("formMain", "Step", 0, QApplication::UnicodeUTF8));
        pushButtonContinue->setText(QApplication::translate("formMain", "Run/Continue", 0, QApplication::UnicodeUTF8));
        radioButtonStepInto->setText(QApplication::translate("formMain", "Cycle-Step", 0, QApplication::UnicodeUTF8));
        radioButtonStepOver->setText(QApplication::translate("formMain", "Statement-Step", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("formMain", "Current State:", 0, QApplication::UnicodeUTF8));
        labelCurrentState->setText(QApplication::translate("formMain", "...", 0, QApplication::UnicodeUTF8));
        labelHighLevel->setText(QApplication::translate("formMain", "C Code", 0, QApplication::UnicodeUTF8));
        labelIR->setText(QApplication::translate("formMain", "IR Instruction", 0, QApplication::UnicodeUTF8));
        labelHardware->setText(QApplication::translate("formMain", "Verilog Code", 0, QApplication::UnicodeUTF8));
        labelWatch->setText(QApplication::translate("formMain", "HW Signals", 0, QApplication::UnicodeUTF8));
        labelWatch_3->setText(QApplication::translate("formMain", "HW Variables", 0, QApplication::UnicodeUTF8));
        labelWatchSWVariables->setText(QApplication::translate("formMain", "SW Variables", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabDebug), QApplication::translate("formMain", "Debug View", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabDiscrepanyLog), QApplication::translate("formMain", "Discrepancy Log", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabSourceCode), QApplication::translate("formMain", "C Source File", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabVerilogCode), QApplication::translate("formMain", "Verilog File", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("formMain", "File", 0, QApplication::UnicodeUTF8));
        menuView->setTitle(QApplication::translate("formMain", "View", 0, QApplication::UnicodeUTF8));
        menuDebug->setTitle(QApplication::translate("formMain", "Debug", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("formMain", "Edit", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class formMain: public Ui_formMain {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_formMain_H
