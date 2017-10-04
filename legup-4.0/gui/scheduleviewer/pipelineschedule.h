/*
 * pipelineschedule.h
 *
 *  Created on: March, 2013
 *      Author: johnqin
 */

#ifndef PIPELINESCHEDULE_H_
#define PIPELINESCHEDULE_H_

#include <QtCore>
#include <QtGui>
#include "task.h"

/********************************************************************
 * Class for the PiplelineSchedule
 *
 * This class contains both the data model and the views
 * *****************************************************************/
using namespace std;

class PipelineSchedule : public QStandardItemModel{
  struct PipelineInsn{
    QString m_insn;
    int m_time;
    int m_stage;
    TASK_TYPE m_type;
  };
  struct Loop{
    int m_II;
    QString m_label;
    QList <PipelineInsn *> m_tasks;
	  QGraphicsScene *m_loopScene;
    vector <vector <vector <PipelineInsn *> > >m_taskMatrix;
  };
public:
	PipelineSchedule();
	~PipelineSchedule(){
    for(int i = 0; i < m_loops.size(); i++)
      for(int j = 0; i < m_loops[i].m_tasks.size(); j++)
        delete m_loops[i].m_tasks[j];
  }
  void buildScene(Loop * loop);
	int init(QFile *file);
	QGraphicsScene *getScene(QString prefix, QString label, QString postfix);

public:
  QList <Loop> m_loops;

};

#endif /* PIPELINESCHEDULE_H_ */
