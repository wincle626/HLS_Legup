/*
 * function.h
 *
 *  Created on: Nov 7, 2012
 *      Author: johnqin
 */

#ifndef FUNCTION_H_
#define FUNCTION_H_

#include "QtCore"
#include "QtGui"
#include <QStandardItemModel>
#include "block.h"

#ifdef DISPLAY_GRAPHS 
#include "gvgraph.h"
#endif

class Function : public ScheduleItem{
public:
	Function();
	Function(QString name);
  ~Function();

  int getSize(){
    return m_blocks.size();
  }
	QString getName() const{
		return m_name;
	}
  void insertBlock(Block *block);
  Block *findBlock(QString &basicBlock);

  vector <Block*> m_blocks;
  vector <QString> m_callee;
  QString m_name;
	QGraphicsScene *m_BBScene;

#ifdef DISPLAY_GRAPHS 
	GVGraph *m_BBGraph;
#endif
};

#endif /* FUNCTION_H_ */
