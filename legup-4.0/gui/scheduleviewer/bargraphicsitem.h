/*
 * bargraphcsitem.h
 *
 * Declaration and definition of BarGraphicsItem
 *
 *  Created on: Dec 7, 2012
 *      Author: johnqin
 */

#ifndef BARGRAPHICSITEM_H_
#define BARGRAPHICSITEM_H_

#include <QtGui>
#include "common.h"
#include "task.h"

using namespace std;
/********************************************************************
 * BarGraphicsItem is used for the Schedule View,
 * Supports mouse over effects based on dependency
 *******************************************************************/
class BarGraphicsItem: public QGraphicsRectItem {
public:
  BarGraphicsItem(Task *task):m_task(task){
    setAcceptHoverEvents(true);

    m_normalPen.setStyle(Qt::DotLine);
    m_normalPen.setWidth(0);
    m_normalPen.setBrush(Qt::gray);
    m_normalPen.setCapStyle(Qt::RoundCap);
    m_normalPen.setJoinStyle(Qt::RoundJoin);

    m_normalBrush.setStyle(Qt::SolidPattern);
    if(m_task->m_type == CALL)
        m_normalBrush.setColor(QColor(Qt::green));
    if(m_task->m_type == BR)
        m_normalBrush.setColor(QColor(Qt::black));
    else
        m_normalBrush.setColor(QColor(QColor(0, 255, 255)));

    m_hlPen.setWidth(3);
    m_hlPen.setStyle(Qt::SolidLine);
    m_hlPen.setBrush(Qt::yellow);
    m_hlPen.setCapStyle(Qt::RoundCap);
    m_hlPen.setJoinStyle(Qt::RoundJoin);

    m_dependingPen = m_hlPen;
    m_dependingPen.setBrush(Qt::red);
    m_dependentPen = m_hlPen;
    m_dependentPen.setBrush(QColor(255, 169, 0));

    m_hlBrush.setColor(Qt::yellow);
    m_hlBrush.setStyle(Qt::SolidPattern);


    setPen(m_normalPen);
    setBrush(m_normalBrush);
  }
  ;

  ~BarGraphicsItem(){
  }
  ;
  void hoverEnterEvent(QGraphicsSceneHoverEvent * event) {
      UNUSED(event);
      for(int i = 0; i< (int)m_depending.size(); i++){
          m_depending[i]->setPen(m_dependingPen);
      }
      for(int i = 0; i< (int)m_dependent.size(); i++){
          m_dependent[i]->setPen(m_dependentPen);
      }
      setBrush(m_hlBrush);
  }
  ;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) {
      UNUSED(event);
      for(int i = 0; i< (int)m_depending.size(); i++){
          m_depending[i]->setPen(m_normalPen);
      }
      for(int i = 0; i< (int)m_dependent.size(); i++){
          m_dependent[i]->setPen(m_normalPen);
      }
      setBrush(m_normalBrush);
  };
public:
  QPen m_normalPen;
  QPen m_hlPen;
  QPen m_dependingPen;
  QPen m_dependentPen;
  QBrush m_normalBrush;
  QBrush m_hlBrush;
  Task * m_task;

  vector <BarGraphicsItem *> m_depending;
  vector <BarGraphicsItem *> m_dependent;

};

#endif /* BARGRAPHICSITEM_H_ */
