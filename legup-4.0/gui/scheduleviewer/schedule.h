/*
 * schedule.h
 *
 *  Created on: Sep 30, 2012
 *      Author: johnqin
 */

#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include <QtCore>
#include "task.h"
#include "block.h"
#include "function.h"


using namespace std;

/********************************************************************
* Data model for the instruction scheduling mode
* contains a list of functions
********************************************************************/
class Schedule : public QStandardItemModel{
public:
	Schedule();
  int insertFunction(int basicBlock);
  Function* findFunction(QString &name) const;
	~Schedule();
	int init(QFile *file);
public:
  QList <Function*> m_functions;

};

#endif /* SCHEDULE_H_ */
