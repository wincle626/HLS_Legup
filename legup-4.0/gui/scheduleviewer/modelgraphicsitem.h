/*
 * modelgraphcsitem.h
 * Declaration and definition of ModelGraphicsItem
 *
 *  Created on: Feb 23, 2013
 *      Author: johnqin
 */

#ifndef MODELGRAPHICSITEM_H_
#define MODELGRAPHICSITEM_H_

#include <QtGui>
#include "common.h"

using namespace std;
/********************************************************************
 * Graphics Item for function node in Call Graph and BB node in 
 * conrol flow graph.
 * Provides support for mouse over highlighing, and double clicking
 * *****************************************************************/

class ModelGraphicsItem: public QObject, public QGraphicsEllipseItem{
	Q_OBJECT

public:
  ModelGraphicsItem(const QModelIndex& modelIndex) :
    QObject(), QGraphicsEllipseItem(), m_modelIndex(modelIndex){
    setAcceptHoverEvents(true);
    m_normalPen = pen();
    m_hlPen = m_normalPen;
    m_hlPen.setWidth(3);
    setBrush(QColor(50,50,255,30));//default value
    //m_hlPen.setBrush(Qt::blue);
  };

signals:
  void doubleClicked(QModelIndex modelIndex); 

private:
  void hoverEnterEvent(QGraphicsSceneHoverEvent * event) {
    UNUSED(event);
    setPen(m_hlPen);
  };
  void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) {
    UNUSED(event);
    setPen(m_normalPen);
  };

  void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {
    UNUSED(event);
    emit doubleClicked(m_modelIndex);
  };

private:
  QPen m_normalPen;
  QPen m_hlPen;
  QModelIndex m_modelIndex;
};

#endif /* MODELGRAPHICSITEM_H_ */

