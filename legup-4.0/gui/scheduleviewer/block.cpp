/*
 * block.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: johnqin
 */

#include "block.h"

//Block::Block() {
//}

Block::Block(QString &name):
  ScheduleItem(name)
{
  m_branch = -5;
  m_name = name;
  m_type = BLOCK_OTHER;
}

Block::~Block() {
}


 Task* 
Block::getTask(int index) const{
  assert(index < (int)m_tasks.size());
  return m_tasks[index];
}

void 
Block::insertTask(Task *task){
  m_tasks.push_back(task);
}
