/*
 * task.h
 *
 *  Created on: Sep 30, 2012
 *      Author: johnqin
 */

#ifndef TASK_H_
#define TASK_H_

#include <QtCore>
#include <QStandardItemModel>
#include "scheduleitem.h"

using namespace std;

enum TASK_TYPE {OTHER, CALL, BR, SWITCH};

class Task : public ScheduleItem{
public:
	Task(QString insn, int BB, int beginState, int endState);
	~Task();
public:
	QString	m_insn;
	int		m_BB;
	int		m_beginState;
	int		m_endState;
  QString m_destination;
  QList <QString> m_sources;
  QList <Task *> m_depending;
  QList <Task *> m_dependent;
  TASK_TYPE m_type;
  

  //for pipeline use only;
  int m_time;
  int m_stage;

};

#endif /* TASK_H_ */
