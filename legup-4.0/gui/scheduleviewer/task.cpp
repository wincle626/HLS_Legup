/*
 * task.cpp
 *
 *  Created on: Sep 30, 2012
 *      Author: johnqin
 */

#include "task.h"

Task::Task(QString insn, int BB, int beginState, int endState)
	:m_BB(BB), m_beginState(beginState), m_endState(endState)
{
  //get rid of the metadata
  int seperator = insn.indexOf(", !");
  if(seperator >= 0)
    insn = insn.left(seperator);

  setText(insn);
  m_insn = insn;
  m_destination = -2;
  m_type = OTHER;
}

Task::~Task() {
	// TODO Auto-generated destructor stub
}

