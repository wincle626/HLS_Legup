/*
 * scheduleitem.h
 *
 *  Created on: Jan 10, 2013
 *      Author: johnqin
 */

#ifndef SCHEDULEITEM_H_
#define SCHEDULEITEM_H_

#include <QStandardItemModel>
#include "common.h"

/********************************************************************
 * this is a parent class of the Function Class, BB Class, and Task
 * class
********************************************************************/
class ScheduleItem: public QStandardItem {
public:
	ScheduleItem(const QString & text);
	ScheduleItem(){};
	int type () const;
	~ScheduleItem(){}
};

#endif /* SCHEDULEITEM_H_ */
