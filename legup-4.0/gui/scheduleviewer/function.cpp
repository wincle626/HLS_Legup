/*
 * function.cpp
 *
 *  Created on: Nov 7, 2012
 *      Author: johnqin
 */

#include "function.h"

Function::Function() {
	// TODO Auto-generated constructor stub

}
Function::Function(QString name):
  ScheduleItem(name){
	m_name = name;
  m_BBScene = NULL;
#ifdef DISPLAY_GRAPHS 
  m_BBGraph = NULL;
#endif
}

Function::~Function() {
  // TODO Auto-generated destructor stub
}

  void
Function::insertBlock(Block* block)
{
  m_blocks.push_back(block);
}

  Block *
Function::findBlock(QString &basicBlock)
{
  for(unsigned i=0; i< m_blocks.size(); i++){
    if(m_blocks[i]->m_name == basicBlock)
      return (m_blocks[i]);
  }
  return NULL;
}
