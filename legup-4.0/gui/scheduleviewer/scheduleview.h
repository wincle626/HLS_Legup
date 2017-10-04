/*
 * scheduleview.h
 *
 *  Created on: Sep 30, 2012
 *      Author: johnqin
 */

#ifndef SCHEDULEVIEW_H_
#define SCHEDULEVIEW_H_


#include "vector"
#include "QtGui"
#include "chartview.h"
#include "task.h"
#include "schedule.h"
#include "taskview.h"

/********************************************************************
 * This is the view inside the Schedule View tab. It contains a task
 * tree, and the actual ChartView
 *
 * This class does the management of the lower level views. actual
 * drawing is done inside the ChartView
 *
 * For the moment, we pass the pointer of the m_taskView
 * to m_chart, so that m_chart can figure draw its retangles
 * aligned to Qt's default QTreeView dimensions
 ********************************************************************/

using namespace std;

class ScheduleView: public QWidget {
	Q_OBJECT

public:
	ScheduleView(QWidget *parent = NULL);
	~ScheduleView();
	void setData(QStandardItem *item);

public slots:
	void itemChangedSlot(QTreeWidgetItem *item)
	{
        UNUSED(item);
    //to be implemented;
	}
signals:
	void taskScrollChangedSignal(int value);
	void chartScrollChangedSignal(int value);
private:
	QSplitter *m_splitter;
	TaskView *m_taskView;
	ChartView *m_chartView;
  QGraphicsScene *m_chartScene;
	Schedule *m_schedule;


  Block * m_block;
};

#endif /* SCHEDULEVIEW_H_ */
