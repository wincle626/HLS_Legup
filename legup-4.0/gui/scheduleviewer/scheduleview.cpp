/*
 * scheduleview.cpp
 *
 *  Created on: Sep 30, 2012
 *      Author: johnqin
 */
#include <QtGui>
#include <QtCore>
#include "assert.h"
#ifdef DISPLAY_GRAPHS 
#include "gvc.h"
#endif
#include "scheduleview.h"
#include "chartview.h"
#include "common.h"
#include "bargraphicsitem.h"

using namespace std;

ScheduleView::ScheduleView(QWidget *parent) :
    QWidget(parent)
{

    //constructor constructs the the geometr, but not data

    m_schedule = NULL;
    m_block = NULL;
    m_splitter = new QSplitter();
    m_taskView = new TaskView();
    m_chartView = new ChartView(m_taskView);
    m_chartScene = new QGraphicsScene();

    m_chartView->setScene(m_chartScene);
    m_chartView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_chartView->show();

    m_splitter->addWidget(m_taskView);
    m_splitter->addWidget(m_chartView);

    //syncronize the scrollbar, need to adjust the scroll value
    m_chartView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_taskView->verticalScrollBar(), SIGNAL(valueChanged(int)), m_chartView, SLOT(setScrollBarValue(int)));
    /*connect(m_taskView, SIGNAL(itemExpanded(QTreeWidgetItem *)),m_taskView, SLOT(itemChangedSlot(QTreeWidgetItem *)));
      connect(m_taskView, SIGNAL(itemCollapsed(QTreeWidgetItem *)),m_taskView, SLOT(itemChangedSlot(QTreeWidgetItem *)));
      connect(m_taskView, SIGNAL(itemExpanded(QTreeWidgetItem *)),this, SLOT(itemChangedSlot(QTreeWidgetItem *)));
      connect(m_taskView, SIGNAL(itemCollapsed(QTreeWidgetItem *)),this, SLOT(itemChangedSlot(QTreeWidgetItem *)));
      */
    //connect(m_taskView, SIGNAL(itemChanged(int)),m_chartView, SLOT(redraw(int)));
    //set size for the taskView
    //set a non-zero for the size of the chart, then
    //it will be automatically resized based on window size
    QList<int> sizes;
    sizes.append(200);
    sizes.append(650);
    m_splitter->setSizes(sizes);
    QStringList labels;
    labels << tr("Instruction");// << tr("Start") << tr("End");

    m_taskView->setHeaderLabels(labels);
    //m_taskView->header()->resizeSection(0, 200);
    //m_taskView->header()->resizeSection(1, 50);
    //m_taskView->header()->resizeSection(2, 50);

    //construct vertical layout
    //horizontal layout is done by the splitter
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addWidget(m_splitter);

    setLayout(vlayout);
}

ScheduleView::~ScheduleView()
{
}

void
ScheduleView::setData(QStandardItem *item) {
    assert(item);
    //m_schedule = schedule;
    m_block = (Block*)item;
    m_chartView->setBlock(m_block);
    m_taskView->setBlock(m_block);
    m_taskView->updateView();
    m_chartView->updateView();
}


