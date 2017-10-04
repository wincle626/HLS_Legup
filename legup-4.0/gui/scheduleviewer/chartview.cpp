/*
 * ChartView.cpp
 *
 *  Created on: Sep 30, 2012
 *      Author: johnqin
 */

#include "chartview.h"
#include "bargraphicsitem.h"

ChartView::ChartView(QWidget *parent) :
    QGraphicsView(parent)
{
    ChartView();
}

ChartView::ChartView(QTreeWidget *taskView) :
    m_taskView(taskView)
{
    m_block = NULL;
    m_oldWidth = -1;
    ChartView();
}

ChartView::ChartView() {
    //setMouseTracking(true);
}

ChartView::~ChartView() {

}

QSize ChartView::minimumSizeHint() const {
    return QSize(100, 100);
}

QSize ChartView::sizeHint() const {
    return QSize(100, 100);
}

void ChartView::setBlock(Block * block)
{
    m_block = block;
}

    void 
ChartView::updateView(Block *block)
{
    if(block == m_block)
        return;
    setBlock(block);
    updateView();
}

    void 
ChartView::updateView()
{
    if(!m_block)
        return;
    QGraphicsScene * ChartViewScene = scene();

    if(!ChartViewScene)
        return;

    ChartViewScene->clear();


    int max_width = 0;
    int state_start = -1;
    int state_end = -1;
    int height = 0;
    int increment = 0;

    /* this scale factor decides the width each state on the ChartView*/
    double scale_factor;

    QTreeWidgetItemIterator iter(m_taskView);
    while(*iter){
        increment = m_taskView->visualItemRect(*iter).height();
        if(increment > 0) break;
    }

    m_rowHeight = increment;

    /* this is the initial value of the height, which is lined up
     * with the tasklist on the left-side. it will be incremented
     * as we traverse through the tasks*/
    height = 25;


    /*figure out the beginning and end of the Basic Block first*/
    for(unsigned i= 0; i < m_block->m_tasks.size(); i++){
        Task * task = m_block->m_tasks[i];
        if (state_end < task->m_endState)
            state_end= task->m_endState;
        if(state_start == -1 || state_start > task->m_beginState)
            state_start = task->m_beginState;
    }

    scale_factor = this->width() / (state_end + 1 - state_start);
    if(scale_factor > 50)
        scale_factor = 50; //upper bound
    else if(scale_factor < 25)
        scale_factor = 25; //lower bound
    max_width = scale_factor * (state_end + 1 - state_start);


    QPen pen;
    pen.setStyle(Qt::DotLine);
    pen.setWidth(1);
    pen.setBrush(Qt::gray);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    QList <BarGraphicsItem *> bars;

    for(unsigned i= 0; i < m_block->m_tasks.size(); i++){
        Task * task = m_block->m_tasks[i];
        ChartViewScene->addLine(0, height, max_width, height, pen);
        int x = (task->m_beginState - state_start) * scale_factor;
        int y = height;
        int x_offset = (task->m_endState + 1 - state_start) * scale_factor - x;
        int y_offset = m_rowHeight;
        height = height + y_offset;

        BarGraphicsItem *bar = new BarGraphicsItem(task);
        bar->setRect(x, y+1, x_offset, y_offset);
        ChartViewScene->addItem(bar);
        bars.push_back(bar);
    }

    //depending copy from task to corresponding bars
    for(int i = 0; i < (int)bars.size(); i++)
    {
        for(int j = 0; j < (int)bars.size(); j++)
        {
            if(i == j) continue;
            if(bars[i]->m_task->m_depending.contains(bars[j]->m_task))
                bars[i]->m_depending.push_back(bars[j]);
            if(bars[i]->m_task->m_dependent.contains(bars[j]->m_task))
                bars[i]->m_dependent.push_back(bars[j]);
        }
    }

    //add a line at the bottom
    ChartViewScene->addLine(0, height, max_width, height, pen);

    //add the column label
    int j = 0;
    for (int i = 0; i < max_width; i += scale_factor) {
        QGraphicsTextItem *textItem = new QGraphicsTextItem(
                QString::number(j+state_start));
        textItem->setPos(i, 0);
        ChartViewScene->addItem(textItem);
        ChartViewScene->addLine(i + scale_factor, 7, i + scale_factor, height, pen);
        j++;
    }

    setSceneRect(0, 0, max_width, height + m_rowHeight);


}

void ChartView::resizeEvent(QResizeEvent *event)
{
    UNUSED(event);
    updateView();
}

void ChartView::setScrollBarValue(int value)
{
    verticalScrollBar()->setValue(m_rowHeight * value);
}
