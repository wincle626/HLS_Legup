/*
 * block.h
 *
 *  Created on: Oct 29, 2012
 *      Author: johnqin
 */

#ifndef BLOCK_H_
#define BLOCK_H_
#include "task.h"
#include "common.h"
#include <vector>

using namespace std;

enum BLOCK_TYPE {BLOCK_OTHER, BLOCK_BB, BLOCK_LOOP_START, BLOCK_LOOP_WAIT};

class Block : public ScheduleItem {
public:
	//Block();
  Block(QString &name);
	~Block();

  int getSize(){
    return m_tasks.size();
  }
  int getNumTasks(){
    return getSize();
  }
  Task *getTask(int index) const; 
  void insertTask(Task *task);

public:
	vector <Task*> m_tasks;
	//TODO support for multiple destinations
	//vector <Block *> m_destinations;
	vector <Block*> m_targetBlocks;
	vector <QString> m_targetBlockNames;
	//vector <int> m_branches;
	int m_branch;//TODO get rid of this after target block available
  QString m_name;
  BLOCK_TYPE m_type;
};

#endif /* BLOCK_H_ */
