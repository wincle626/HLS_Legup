/*
 * pipelineschedule.cpp
 *
 *  Created on: March, 2013
 *      Author: johnqin
 */


#include "pipelineschedule.h"
PipelineSchedule::PipelineSchedule(){
}

int PipelineSchedule::init(QFile *file) {
    QTextStream in(file);
    QString line;
    QRegExp rxFoundLoop("^Found Loop:");
    QRegExp rxII("^II = (\\d+)$");
    QRegExp rxInsn("Time: (\\d+) Stage: (\\d+) instr:\\s+(\\S+.*)$");
    QRegExp rxLabel("^Label: (\\w+)$");
    QRegExp rxBr("^br");
    qDebug() << "reading file" <<endl;
    int currentLoopIndex = -1;
    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (rxFoundLoop.indexIn(line, 0) != -1) {
          qDebug() << line << endl;
          Loop loop;
          loop.m_II = -1;
          loop.m_loopScene = NULL;
          m_loops.push_back(loop);
          if(currentLoopIndex < 0)
            currentLoopIndex = 0;
          else
            currentLoopIndex++;
        }
        else if (rxLabel.indexIn(line, 0) != -1) {
          qDebug() << "Label: " << rxLabel.cap(1) << endl;
          assert(currentLoopIndex>=0);
          m_loops[currentLoopIndex].m_label = rxLabel.cap(1);
        }
        else if (rxII.indexIn(line, 0) != -1) {
          qDebug() << "II: " << rxII.cap(1) << endl;
          //find the last loop element
          assert(currentLoopIndex >= 0);
          m_loops[currentLoopIndex].m_II = rxII.cap(1).toInt();
        }
        else if(rxInsn.indexIn(line, 0) != -1){
          Loop * loop = &m_loops[currentLoopIndex];
          PipelineInsn * insn = new PipelineInsn();
          QString insnName = rxInsn.cap(3);
          qDebug() << currentLoopIndex << insnName << endl;
          //get rid of the metadata
          int seperator = insnName.indexOf(", !");
          if(seperator >= 0)
            insnName = insnName.left(seperator);
          insn->m_insn = insnName;
          insn->m_time = rxInsn.cap(1).toInt();
          insn->m_stage = rxInsn.cap(2).toInt();
	  qDebug() << insn->m_time << " " << insn->m_stage << endl;
          insn->m_type = OTHER;
          if(rxBr.indexIn(insn->m_insn, 0) != -1) {
            insn->m_type = BR;
	    qDebug() << "BRANCH" << endl;
          }
          else{
            insn->m_type = OTHER;
          }
          loop->m_tasks.push_back(insn);

          //expending the size of the matrix if we need to
          if(insn->m_stage >= (int)loop->m_taskMatrix.size()){
            loop->m_taskMatrix.resize(insn->m_stage+1);
          }
          if(insn->m_time >= (int)loop->m_taskMatrix[insn->m_stage].size()){
            loop->m_taskMatrix[insn->m_stage].resize(insn->m_time+1);
          }

          loop->m_taskMatrix[insn->m_stage][insn->m_time].push_back(insn);

        }
    }

    for(int i = 0; i < m_loops.size(); i++)
      buildScene(&m_loops[i]);


    return 0;
}

QGraphicsScene *
PipelineSchedule::getScene(QString prefix, QString label, QString postfix)
{
  for(int i = 0; i < m_loops.size(); i++){
    if(prefix+m_loops[i].m_label+postfix == label)
      return m_loops[i].m_loopScene;
  }
  return NULL;

}

/*build scene. technically this should not be done
* with in a data model, but at the moment the entire
* loop pipeline code+data+graphics is self-contained;
* the only interface is through the getScene function
*/
void
PipelineSchedule::buildScene(Loop * loop){

  QGraphicsScene * scene = new QGraphicsScene();

  int columnWidth = 60;
  int rowHeight = 30;
  int initialHeight;
  int currentHeight = 0;
  int maxHeight = 0;
  int leftOffset = 100;

  int numStages = loop->m_taskMatrix.size();
  int stageWidth = loop->m_II * columnWidth;

  QPen dottedPen;
  dottedPen.setStyle(Qt::DotLine);
  dottedPen.setWidth(1);
  dottedPen.setBrush(QColor("#01010101"));
  dottedPen.setCapStyle(Qt::RoundCap);
  dottedPen.setJoinStyle(Qt::RoundJoin);

  scene->addLine(leftOffset, rowHeight, numStages * stageWidth + leftOffset, rowHeight, dottedPen);
  scene->addLine(0, 2*rowHeight, numStages * stageWidth + leftOffset, 2*rowHeight);
  initialHeight = 2*rowHeight;
  currentHeight = initialHeight;
  
  //basic info
    QGraphicsTextItem *info = 
      new QGraphicsTextItem(QString("%1\nII: %2").arg(loop->m_label).arg(loop->m_II));
    info->setPos(0, 0);
    scene->addItem(info);

  //build header
  for(int i = 0; i < loop->m_II * numStages; i++){
    QGraphicsTextItem *headerItem = 
      new QGraphicsTextItem(QString("%1").arg(i));
    headerItem->setPos(i * columnWidth + leftOffset + 20, 0);
    scene->addItem(headerItem);
  }

  for(int i = 0; i < loop->m_II * numStages; i++){
    QGraphicsTextItem *headerItem = 
      new QGraphicsTextItem(QString("%1").arg(i % loop->m_II));
    headerItem->setPos(i * columnWidth + leftOffset + 20, rowHeight);
    headerItem->setDefaultTextColor(QColor("#888"));
    scene->addItem(headerItem);
  }

  //loop through the iterations
  for(int i_iter = 0; i_iter < (int)loop->m_taskMatrix.size(); i_iter++){
    QGraphicsTextItem *stageHeaderItem = 
      new QGraphicsTextItem(QString("Iteration: %1").arg(i_iter));
    stageHeaderItem->setPos(5, currentHeight + 5);
    scene->addItem(stageHeaderItem);

   //currentHeight = rowHeight;
    //loop through the stages
    for(int i_stage=0; i_stage < (int)loop->m_taskMatrix.size()-i_iter; i_stage++){
      //loop through the times
      for(int j_time=0; j_time < (int)loop->m_taskMatrix[i_stage].size(); j_time++){
        //loop through the insns
        for(int k_insn=0; k_insn < (int)loop->m_taskMatrix[i_stage][j_time].size(); k_insn++){
          PipelineInsn * insn = loop->m_taskMatrix[i_stage][j_time][k_insn];
          if(insn->m_type == BR)
            continue;
          QGraphicsRectItem *insnBlockItem = new QGraphicsRectItem();
          QBrush brush(QColor(255, 255, 255, 168));

          int x = stageWidth * i_iter + stageWidth * i_stage + 
            (j_time % loop->m_II) * columnWidth + leftOffset;
          int y = currentHeight + k_insn * rowHeight;

          insnBlockItem->setRect(x, y, columnWidth, rowHeight);
          insnBlockItem->setPen(dottedPen);
          insnBlockItem->setZValue(-1);
          insnBlockItem->setBrush(brush);
          insnBlockItem->setToolTip(insn->m_insn);

          QGraphicsTextItem *insnTextItem = 
            new QGraphicsTextItem(insn->m_insn.left(6));
          insnTextItem->setPos(x, y);
          scene->addItem(insnBlockItem);
          scene->addItem(insnTextItem);

          if(y+rowHeight > maxHeight)
            maxHeight = y+rowHeight;
        } //i_insn
      } //i_time

      //add a wrapper box for the each iteration_stage
      //if(i_iter == 0){
        QGraphicsRectItem *stageWrapperItem = new QGraphicsRectItem();
        int x = stageWidth *(i_iter + i_stage) + leftOffset;
        int y = currentHeight;
        QLinearGradient gradient(QPointF(leftOffset + stageWidth*i_iter, y), 
            QPointF(leftOffset + stageWidth*(i_iter+numStages), y));
        gradient.setColorAt(0, QColor(255, 0, 0));
        gradient.setColorAt(0.2, QColor(255, 255, 0));
        gradient.setColorAt(0.4, QColor(0, 255, 0));
        gradient.setColorAt(0.6, QColor(0, 255, 255));
        gradient.setColorAt(0.8, QColor(0, 0, 255));
        gradient.setColorAt(1, QColor(255, 0, 255));
        QBrush brush(gradient);

        stageWrapperItem->setRect(x, y, stageWidth, maxHeight-currentHeight);;
        stageWrapperItem->setBrush(brush);
        stageWrapperItem->setZValue(-10);
        scene->addItem(stageWrapperItem);
      //}

      //add a horizontal line
      scene->addLine(0, maxHeight, numStages * stageWidth + leftOffset, maxHeight);


    } //i_stage
    currentHeight = maxHeight;

  } //i_iter

  //vertical line on time boundary
  for(int i = 0; i <= loop->m_II * numStages; i++){
    scene->addLine(i * columnWidth + leftOffset, 0, 
        i * columnWidth + leftOffset, currentHeight, dottedPen);
  }

  //vertical line on iteration boundary
  for(int i = 0; i <= numStages; i++){
    scene->addLine(i * stageWidth + leftOffset, 0, 
        i * stageWidth + leftOffset, currentHeight);
  }

  //steady state box
  QGraphicsRectItem *SSBoxItem = new QGraphicsRectItem();
  SSBoxItem->setRect(leftOffset + (numStages-1) * stageWidth, initialHeight,
      stageWidth, maxHeight - 2 * rowHeight);
  QPen hlpen;
  hlpen.setWidth(5);
  hlpen.setBrush(Qt::black);
  hlpen.setCapStyle(Qt::RoundCap);
  hlpen.setJoinStyle(Qt::RoundJoin);

  SSBoxItem->setPen(hlpen);
  scene->addItem(SSBoxItem);


  scene->setSceneRect(0, 0, numStages * stageWidth + leftOffset + 5, currentHeight + 5);
  loop->m_loopScene = scene;

}



