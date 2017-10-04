/*
 * tasklist.h
 *
 *  Created on: Oct 7, 2012
 *      Author: johnqin
 */

#ifndef TASKLIST_H_
#define TASKLIST_H_

#include "QtGui"
#include <qtreewidget.h>
#include "block.h"

/********************************************************************
 * This is the class for instruction scheudling chart. It's a subview
 * of the Schedule View
 * It lists the all the instructions in a basic block
********************************************************************/

class TaskView: public QTreeWidget {
	Q_OBJECT
public:
	TaskView();
	~TaskView();
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
  void setBlock(Block * block);
  void updateView();

public slots:
	void itemChangedSlot(QTreeWidgetItem *);
signals:
	void itemChanged(int newValue);

private:
  Block *m_block;

};

#endif /* TASKLIST_H_ */
