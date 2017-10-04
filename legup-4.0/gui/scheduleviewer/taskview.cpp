/*
 * taskview.cpp
 *
 *  Created on: Oct 7, 2012
 *      Author: johnqin
 */

#include "taskview.h"

TaskView::TaskView() {
}

TaskView::~TaskView() {
}
QSize TaskView::minimumSizeHint() const {
	return QSize(100, 100);
}

QSize TaskView::sizeHint() const {
	return QSize(500, 500);
}

void TaskView::itemChangedSlot(QTreeWidgetItem *item) {
  item = item; //suspress the warning
	/*
	emit itemChanged(height);
	*/
}

void TaskView::setBlock(Block * block)
{
  m_block = block;
}

void TaskView::updateView()
{
  clear();

  for(int i = 0; i < (int)m_block->m_tasks.size(); i++){
    Task * task = m_block->m_tasks[i];
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, task->m_insn);
    //item->setText(1, QString::number(task->m_beginState));
    //item->setText(2, QString::number(task->m_endState));
    addTopLevelItem(item);
  }

}
