/*
 * Viewer.h
 *
 *  Created on: Sep 23, 2012
 *      Author: johnqin
 */

#ifndef VIEWER_H_
#define VIEWER_H_

#include <QtGui>

#include "scheduleview.h"
#include "schedule.h"
#include "explorer.h"
#include "function.h"
#include "pipelineschedule.h"

#ifdef DISPLAY_GRAPHS 
#include "gvgraph.h"
#include "modelgraphicsitem.h"
#endif

/********************************************************************
 * This class provides the top level view which contains the menu bar
 * a split of explorer and the tabs. individual views of the tabs are
 * also controlled here. All scenes except for that of the pipeline
 * view are drawn in this class, in order to acheive decoupling of view
 * and data. transitions between the views are managed here by signals
 * and slots.
 * In summary, this is a top level view, as well as a top level manager
********************************************************************/

class Viewer: public QMainWindow {
	Q_OBJECT

public:
	Viewer(char *schedule_path, char* pipeline_path);
	~Viewer(){}

private:
#ifdef DISPLAY_GRAPHS
  void buildScene(GVGraph *graph, QGraphicsScene *scene);
  ModelGraphicsItem * buildNode(QGraphicsScene *scene, GVNode node, QStandardItem *);
	void buildCallGraph();
  void buildCFGraph(Function * function);
#endif

private slots:
	void open();
	void open(QString filename);
	void openPipeline();
	void openPipeline(QString filename);
	void quit();


	void currentClicked(QModelIndex modelIndex);
	void currentChanged(QModelIndex modelIndex);
  void updateView(QModelIndex modelIndex);

private:

	QAction *m_openAction;
	QAction *m_openPipelineAction;
	QAction *m_exitAction;

	QMenu *m_fileMenu;
	QStatusBar *m_statusBar;
	QSplitter *m_splitter;

	int m_BBTab;
	int m_scheduleTab;
  int m_functionTab;
  int m_reportTab;
  int m_pipelineTab;

	QTabWidget *m_tabBar;

	QTextEdit *m_textEdit;
	ScheduleView *m_scheduleView;

	Schedule *m_schedule;
	Explorer *m_explorer;

  PipelineSchedule *m_pipelineSchedule;

  /*Call graph and control graph*/
#ifdef	DISPLAY_GRAPHS 
	QGraphicsScene *m_functionScene;
	QGraphicsView *m_functionView;
	GVGraph *m_functionGraph;
	QGraphicsView *m_BBView;
#endif

	QGraphicsView *m_pipelineView;

	friend class Schedule;
};

#endif /* VIEWER_H_       m_explorer->setModel(m_schedule);
      m_explorer->update();
      m_explorer->expandAll();
      QFileInfo pathInfo(fullName);
      QString name(pathInfo.fileName());
      setWindowTitle((tr("Schedule Viewer").append("\t-  ").append(name)));
#ifdef DISPLAY_GRAPHS
      buildCallGraph();
      for(int i=0; i < (int)m_schedule->m_functions.size(); i++){
        buildCFGraph(m_schedule->m_functions[i]);
      }
      m_tabBar->setCurrentIndex(m_functionTab);
#endif
*/
