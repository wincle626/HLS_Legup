/*
 * chart.h
 *
 *  Created on: Sep 30, 2012
 *      Author: johnqin
 */

#ifndef CHART_H_
#define CHART_H_

#include <QtGui>
#include "block.h"

/********************************************************************
 * This is the class for instruction scheudling chart. It's a subview
 * of the Schedule View
 * It receives data (Block) from higher level view, and draws the scene
 * Note that everytime the view is activated, the scene is redrawn
********************************************************************/

class ChartView: public QGraphicsView {
	Q_OBJECT
public:
	ChartView(QWidget *parent);
	ChartView(QTreeWidget *taskView);
	ChartView();
	~ChartView();
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	//void paintEvent(QPaintEvent *);
  void setBlock(Block * block);
  void updateView(Block * block);
  void updateView();
protected:
	//void mouseMoveEvent (QMouseEvent * event );
	//void mousePressEvent (QMouseEvent * event );
  void resizeEvent(QResizeEvent *event);

public slots:
  void setScrollBarValue(int value);
	//void redraw(int height);

private:
	QTreeWidget *m_taskView;
  Block * m_block;
  Block *m_previousBlock;
  int m_oldWidth;
  int m_rowHeight;
};

#endif /* CHART_H_ */
