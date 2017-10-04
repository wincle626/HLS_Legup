/*
 * Viewer.cpp
 *
 *  Created on: Sep 23, 2012
 *      Author: johnqin
 */

#include "viewer.h"
#include "math.h"

Viewer::Viewer(char *schedule_path, char* pipeline_path) {
#ifdef DISPLAY_GRAPHS 
	m_functionGraph = NULL;
#endif
	//Set size
	resize(1000, 600);

	//Set Menu and connect the slots
	m_openAction = new QAction(tr("&Open"), this);
	m_openPipelineAction = new QAction(tr("&Open Pipeline Schedule"), this);
	m_exitAction = new QAction(tr("E&xit"), this);

	connect(m_openAction, SIGNAL(triggered()), this, SLOT(open()));
	connect(m_openPipelineAction, SIGNAL(triggered()), this, SLOT(openPipeline()));
	connect(m_exitAction, SIGNAL(triggered()), this, SLOT(quit()));

	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addAction(m_openPipelineAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exitAction);

	//Set Status Bar
	m_statusBar = statusBar();
	m_statusBar->showMessage("LegUp Schedule Viewer");

  //construct the visual blocks in the view
	m_splitter = new QSplitter();

	m_explorer = new Explorer();
	m_tabBar = new QTabWidget();

  //In the future we may want to view some syntax-highlighted version of the text
	//m_textEdit = new QTextEdit();
	//m_textEdit->setReadOnly(true);

  //build the serveral graphics views
	m_scheduleView = new ScheduleView();
#ifdef DISPLAY_GRAPHS
	m_functionScene = new QGraphicsScene();
	m_functionView = new QGraphicsView(m_functionScene);
	m_BBView = new QGraphicsView();
#endif
	m_pipelineView = new QGraphicsView();

  //build and add the tabs
	//m_reportTab = m_tabBar->addTab(m_textEdit, "Schedule Report");
	m_scheduleTab = m_tabBar->addTab(m_scheduleView, "Schedule Chart");
  m_pipelineTab = -1;//pipelineTab is not enabled by default
#ifdef DISPLAY_GRAPHS
	m_functionTab = m_tabBar->addTab(m_functionView, "Call Graph");
	m_BBTab = m_tabBar->addTab(m_BBView, "Control Flow Graph");
#endif

  //set the blocks
	setCentralWidget(m_splitter);
	m_splitter->addWidget(m_explorer);
	m_splitter->addWidget(m_tabBar);
	setWindowTitle(tr("Hardware Visualizer"));

  //construct the schedules (the data model)
  m_schedule = new Schedule();
  m_pipelineSchedule = new PipelineSchedule();

	if (schedule_path!=NULL) {
		open(QString(schedule_path));
	}
	if (pipeline_path!=NULL) {
		openPipeline(QString(pipeline_path));
	}

  //set up the link between the items in the explorer window
  //and the views (for click actions)
	connect(m_explorer, SIGNAL(currentSelected(QModelIndex)), this,
      SLOT(currentChanged(QModelIndex)));
	connect(m_explorer, SIGNAL(clicked(QModelIndex)), this,
      SLOT(currentClicked(QModelIndex)));

  return;
}

void Viewer::open() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
            tr("Scheduling Reports (*.rpt);;Text Files (*.txt)"));
    open(fileName);
}

void Viewer::openPipeline() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
            tr("Pipleline Scheduling Reports (*.rpt);;Text Files (*.txt)"));
    openPipeline(fileName);
}

static int graphics_initialized = 0;
static QModelIndex previousIndex;

void Viewer::open(QString fullName) {

  if (fullName != "") {
    QFile file(fullName);
    if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
      return;
    }
    if (m_schedule->init(&file) == 0) {
      m_explorer->setModel(m_schedule);
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

    }
  }
}

void Viewer::openPipeline(QString fullName) {

  if (fullName != "") {
    QFile file(fullName);
    if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
      return;
    }
    if (m_pipelineSchedule->init(&file) == 0) {
      if(m_pipelineTab < 0){
	       m_pipelineTab = m_tabBar->addTab(m_pipelineView, "Loop Pipeline Viewer");
      }
      if(m_pipelineSchedule->m_loops.size()>0){
        m_pipelineView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
      }

    }
  }
}

void Viewer::quit() {
    qApp->quit();
}


void Viewer::updateView(QModelIndex index) {
  QStandardItem *item = m_schedule->itemFromIndex(index);
  /* the following is a terrible way of checking 
   * if this node is a Basic Block node. This will be improved
   * when we use a custom model, with custom item type instead
   * of qstandarditem, and emded an item type in it.*/
  if (item->parent()){
    if (item->parent()->parent() == NULL) {
      if(((Block*)item)->m_type == BLOCK_LOOP_WAIT){ // ??? janders
        if(m_pipelineTab>=0){
          assert(m_pipelineSchedule);
          QGraphicsScene * loopScene = 
            m_pipelineSchedule->getScene("wait_", ((Block*)item)->m_name, "_1");
          if(loopScene){
            m_pipelineView->setScene(loopScene);
            m_pipelineView->show();
            m_tabBar->setCurrentIndex(m_pipelineTab);
            //set scrollbar to the top-left
            m_pipelineView->horizontalScrollBar()->setValue(0);
            m_pipelineView->verticalScrollBar()->setValue(0);
          }
          else
            qCritical() << "Cannot find a loop with label: " << ((Block*)item)->m_name << endl;
        }
      }
      //else if(((Block*)item)->m_type == BLOCK_LOOP_WAIT){
      //}
      else{
        m_scheduleView->setData(item);
        m_tabBar->setCurrentIndex(m_scheduleTab);
      }
    }
  }
#ifdef DISPLAY_GRAPHS
  else{
    Function *function = (Function *)item;
    assert(function->m_BBScene);
    if(m_BBView->scene()!=function->m_BBScene)
      m_BBView->setScene(function->m_BBScene);
    m_BBView->show();
    m_BBView->verticalScrollBar()->setValue(0);
    m_tabBar->setCurrentIndex(m_BBTab);
  }
#endif
}
void Viewer::currentClicked(QModelIndex index) {
  //if we're clicking on the already selected item, currentChanged
  //will not be called. so updateView in this case
  //otherwise, let's currentChanged handle it
  if(index == previousIndex){
    updateView(index);
  }
}

void Viewer::currentChanged(QModelIndex index) {
  if(graphics_initialized){
    previousIndex = index;
    updateView(index);
  }
  else{
    previousIndex = m_explorer->currentIndex();
    graphics_initialized = 1;
  }

}

#ifdef DISPLAY_GRAPHS

/*
 * build a textnode together with its box
*/
 ModelGraphicsItem *
Viewer::buildNode(QGraphicsScene *scene, GVNode node, QStandardItem *item)
{
  ModelGraphicsItem *nodeItem = new ModelGraphicsItem(m_schedule->indexFromItem(item));
  if(item){
    connect(nodeItem, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(updateView(QModelIndex)));
  }
  nodeItem->setRect(node.x, node.y, node.width, node.height);

  QGraphicsTextItem *textItem = new QGraphicsTextItem(node.name);
  //textItem->setX(node.labelX - 15);
  //textItem->setY(node.labelY - 8);
  //note, labelX and labelY are not always reliable. so use x, y instead
  textItem->setX(node.x + 15);
  textItem->setY(node.y + 10);
  //textItem->setFont(font);

  scene->addItem(textItem);
  scene->addItem(nodeItem);
  return nodeItem;
}

static const double Pi = atan(1.0)*4;

/*
 * build a arrow polygon: give starting point and end piont,
 * draw a line, and then draw a triangle that points to the
 * end point
 */
QGraphicsPolygonItem * buildArrow(double startX, double startY, double endX, double endY)
{
  //this function builds an arrow with a focus point at startX, startY,
  //pointing towards endX, endY
  double arrowSize = 12;
  double dx = startX - endX;
  if(dx == 0)
    dx = 0.0001;
  double dy = startY - endY;
  double angle = atan(dy/dx);
  if(dx < 0)
    angle = angle + Pi;

  //find the vertices of the triangles
  QPointF focusP(startX, startY);
  QPointF arrowP1 = focusP + QPointF(cos(angle + Pi/9)*arrowSize, sin(angle + Pi/9)*arrowSize);
  QPointF arrowP2 = focusP + QPointF(cos(angle - Pi/9)*arrowSize, sin(angle - Pi/9)*arrowSize);

  QGraphicsPolygonItem *arrowItem = new QGraphicsPolygonItem(QPolygonF()<< focusP << arrowP1 << arrowP2);
  arrowItem->setBrush(QBrush(Qt::SolidPattern));
  return arrowItem;
}

/*
 * build and draw the call graph, give all the functions and their callees
 */
void Viewer::buildCallGraph()
{
  //clear old stuff first
    assert(m_functionScene != NULL);
    QList<QGraphicsItem *> list = m_functionScene->items();
    QList<QGraphicsItem *>::Iterator it = list.begin();
    for (; it != list.end(); ++it) {
        if (*it) {
            m_functionScene->removeItem(*it);
            delete *it;
        }
    }
    if (m_functionGraph)
        delete m_functionGraph;

    //construct gvgraph
    QFont font("Helvetica", 12);
        m_functionGraph = new GVGraph("Function Graph", font);
    for (int i = 0; i < m_schedule->m_functions.size(); i++) {
        QString sourceName = m_schedule->m_functions[i]->getName();
        m_functionGraph->addNode(sourceName);
        for (int j = 0; 
                j < (int)m_schedule->m_functions[i]->m_callee.size();
                j++) {
            QString targetName = m_schedule->m_functions[i]->m_callee[j];
            m_functionGraph->addNode(targetName);
            m_functionGraph->addEdge(sourceName, targetName);
        }
    }

    //lay it out inside gv
    m_functionGraph->applyLayout();

    QList<GVNode> nodes = m_functionGraph->getNodes();
    QList<GVEdge> edges = m_functionGraph->getEdges();

    //construct items on the canvas
    // - build the nodes
    for (int i = 0; i < nodes.size(); i++) {
        Function * function = m_schedule->findFunction(nodes[i].name);
        buildNode(m_functionScene, nodes[i], (QStandardItem *)function);
    }
    // - build the edges
    for (int i = 0; i < edges.size(); i++) {
        QPainterPath *pathItem = new QPainterPath(edges[i].path);
        m_functionScene->addPath(*pathItem);
        QGraphicsPolygonItem * arrowItem = buildArrow(edges[i].tailX, edges[i].tailY, 
            edges[i].tailNodeCenterX, edges[i].tailNodeCenterY);
        m_functionScene->addItem(arrowItem);
    }

    //finish up the scene and set the view
    m_functionScene->setSceneRect(m_functionGraph->boundingRect());
    m_functionView->verticalScrollBar()->setValue(0);
    m_functionView->show();

}

/* build and draw a control flow graph, give the function,
 * which has all the basic blocks and their flow destinations
 */
void
Viewer::buildCFGraph(Function * function)
{
  assert(function);
  if(function->m_BBScene==NULL){
    assert(function->m_BBGraph == NULL);

    QFont font("Helvetica", 12);
    function->m_BBScene = new QGraphicsScene();
    function->m_BBGraph = new GVGraph("Control Flow Graph", font);

    //construct gvgraph
    for (int i = 0; (int)i < function->m_blocks.size(); i++) {
      Block *block = function->m_blocks[i];
      QString sourceName = block->m_name;
      function->m_BBGraph->addNode(sourceName);
      for (int j = 0; 
          j < (int)block->m_targetBlocks.size();
          j++) {
        QString targetName = block->m_targetBlocks[j]->m_name;
        function->m_BBGraph->addNode(targetName);
        function->m_BBGraph->addEdge(sourceName, targetName);
      }
    }

    //lay it out inside gv
    function->m_BBGraph->applyLayout();

    //retrieve info from gv
    QList<GVNode> nodes = function->m_BBGraph->getNodes();
    QList<GVEdge> edges = function->m_BBGraph->getEdges();
    
    //construct items on the canvas
    for (int i = 0; i < nodes.size(); i++) {
      Block *block = function->findBlock(nodes[i].name);
      ModelGraphicsItem *nodeItem = buildNode(function->m_BBScene, nodes[i], (QStandardItem *)block);

      if(block->m_type == BLOCK_LOOP_START)
        nodeItem->setBrush(QColor(0, 255, 128, 30));
    }

    for (int i = 0; i < edges.size(); i++) {
      QPainterPath *pathItem = new QPainterPath(edges[i].path);
      function->m_BBScene->addPath(*pathItem);
      QGraphicsPolygonItem * arrowItem = buildArrow(edges[i].tailX, edges[i].tailY,
          edges[i].tailNodeCenterX, edges[i].tailNodeCenterY);
      function->m_BBScene->addItem(arrowItem);
    }

        //finish up the scene
    function->m_BBScene->setSceneRect(function->m_BBGraph->boundingRect());

  }
}

#endif
