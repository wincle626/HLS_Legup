/*
 * schedule.cpp
 *
 *  Created on: Sep 30, 2012
 *      Author: johnqin
 */

#include "schedule.h"
#include "iostream"
#include "scheduleitem.h"

Schedule::Schedule() {
    //constructor do not construct a data model because at this point there's
    //no data available yet.
    //data initilization is done in init();
}

Schedule::~Schedule() {
    // TODO Auto-generated destructor stub
}

int Schedule::init(QFile *file) {
    QString line;
    QString basicBlock;
    int beginState = 0;
    int endState = 0;
    int insnCount = 0;
    QString insn;
    if(!file)
        return 0;
    QTextStream in(file);
    Function * function = NULL;

    clear();
    m_functions.clear();

    Block * block = NULL;
    Task * task = NULL;
    while (!in.atEnd()) {
        int pos = 0;
        int posEqual;
        line = in.readLine().trimmed();
        QRegExp rxStartFunc("^Start Function: (\\w+)");
        QRegExp rxEndFunc("^End Function: (\\w+)");
        QRegExp rxFunctionCall("call.*@(\\w+)\\(");
        QRegExp rxLoad("load");
        QRegExp rxState("^state:");
        QRegExp rxStartState("^state: LEGUP_0");
        QRegExp rxStateBegin("^state.*(BB\\w+)_(\\d+)$");//matching BB names
	QRegExp rxFunctionState("^state: LEGUP_function_call_(\\d+)$"); // states for function calls
        QRegExp rxStateLoopWait("^state.*LEGUP_loop_pipeline_(wait_\\w+)_\\d+(?!\\S+)");
        QRegExp rxTransition("^Transition:");
        QRegExp rxTransitionIf("^Transition: if");
        QRegExp rxTransitionCall("call");
        QRegExp rxTransBB("(BB\\w+)_\\d+(?!\\S+)");//matching BB names
        QRegExp rxTransLoopWait("LEGUP_loop_pipeline_(wait_\\w+)_\\d+(?!\\S+)");
        QRegExp rxStateEnd("^(.*)\\(endState.*_(\\d+)\\)");
        QRegExp rxRegisters("%([0-9a-zA-Z_]+)");
        QRegExp rxEqual("=");
        QRegExp rxBr("^br");
	QRegExp rxSwitch("^switch");
        QRegExp rxRet("^ret");


        if (rxStartFunc.indexIn(line, 0) != -1) {
            QString functionName = rxStartFunc.cap(1);
            function = new Function(functionName);
            m_functions.push_back(function);
        }
	else if (rxStartState.indexIn(line, 0) != -1) {
	  if(function == NULL){
	    qCritical() << "Function cannot be null: " << line;
	    continue;
	  }
	  basicBlock = "LEGUP_0";
	  beginState = 1;
	  block = function->findBlock(basicBlock);
	  if (block == NULL) {
	    block = new Block(basicBlock);
	    block->m_type = BLOCK_BB;
	    function->insertBlock(block);
	  }
	}
        else if (rxStateBegin.indexIn(line, 0) != -1) {
            if(function == NULL){
                qCritical() << "Function cannot be null: " << line;
                continue;
            }
            basicBlock = rxStateBegin.cap(1);
            beginState = rxStateBegin.cap(2).toInt();
            block = function->findBlock(basicBlock);
            if (block == NULL) {
                block = new Block(basicBlock);
                block->m_type = BLOCK_BB;
                function->insertBlock(block);
            }
        }
	else if (rxFunctionState.indexIn(line, 0) != -1) {
	  if(function == NULL){
	    qCritical() << "Function cannot be null: " << line;
	    continue;
	  }
	  // we must already be in a BB ??? janders 
	  beginState = rxFunctionState.cap(1).toInt();
	  //	  qCritical() << "Function start state" << beginState <<  endl;
	}
        else if (rxStateLoopWait.indexIn(line, 0) != -1){
          if(function == NULL){
            qCritical() << "Function cannot be null: " << line;
            continue;
          }
          basicBlock = rxStateLoopWait.cap(1);
          block = function->findBlock(basicBlock);
          if (block == NULL) {
            block = new Block(basicBlock);
            block->m_type = BLOCK_LOOP_WAIT;
            function->insertBlock(block);
          }
        }
        else if (rxStateEnd.indexIn(line, 0) != -1) {
          if(block == NULL){
            qCritical() << "A: Block cannot be null: " << line;
            continue;
          }
          insn = rxStateEnd.cap(1).trimmed();
          //get rid of the metadata
          int seperator = insn.indexOf(", !");
          if(seperator >= 0)
            insn = insn.left(seperator);

          endState = rxStateEnd.cap(2).toInt();
            if (rxLoad.indexIn(line, 0) != -1) {
                //special case for load instruction
                //endState -- ;
            }
            if (!insn.isEmpty()) {
                insnCount ++;
                task = new Task(insn, 0, beginState, endState);
                block->insertTask(task);

                if (rxFunctionCall.indexIn(line, 0) != -1) {
                    /*function calls*/
		  qDebug() << "function " << function->m_name<< " is calling "
                     << rxFunctionCall.cap(1)<< endl;
                    task->m_type = CALL;
                    function->m_callee.push_back(rxFunctionCall.cap(1));
                }
                else if (rxBr.indexIn(line, 0) != -1) {
                  /*branch instructions*/
                  //this is the case if we do have begin/end state in branch instruction
                  task->m_type = BR;
                }
		else if (rxSwitch.indexIn(line, 0) != -1) {
                  /*branch instructions*/
                  //this is the case if we do have begin/end state in branch instruction
                  task->m_type = SWITCH;
                }

                posEqual = rxEqual.indexIn(line, 0);
                pos = 0;
                /*checking dependencies*/
                while ((pos = rxRegisters.indexIn(line, pos)) != -1) {
                    if (pos < posEqual){
                        //qDebug() << "output " << rxRegisters.cap(1) << endl;
                        QString destination = rxRegisters.cap(1);
                        task->m_destination = destination;
                    }
                    else{
                        //case where a register appear on the right side of
                        //an equal size: this is a source operand
                        //qDebug() << "input for instruction" 
                        // << line << " is "
                        //<< rxRegisters.cap(1)<< endl;
                        QString source = rxRegisters.cap(1);
                        task->m_sources.push_back(source);
                    }

                    pos += rxRegisters.matchedLength();

                }
            }
        }
        else if (rxBr.indexIn(line, 0) != -1) {
          /*branch instructions*/
          //note branch instruction may have an begin/end state.
          if(block == NULL){
            qCritical() << "B: Block cannot be null: " << line;
            continue;
          }
          task = new Task(line, 0, beginState, beginState);
          block->insertTask(task);
          task->m_type = BR;
        }
	else if (rxSwitch.indexIn(line, 0) != -1) {
          /*branch instructions*/
          //note branch instruction may have an begin/end state.
          if(block == NULL){
            qCritical() << "B: Block cannot be null: " << line;
            continue;
          }
          task = new Task(line, 0, beginState, beginState);
          block->insertTask(task);
          task->m_type = SWITCH;
        }
        else if (rxRet.indexIn(line, 0) != -1) {
          /*return instructions*/
          //return instruction may have an begin/end state.
          if(block == NULL){
            qCritical() << "C: Block cannot be null: " << line;
            continue;
          }
          task = new Task(line, 0, beginState, beginState);
          block->insertTask(task);
        }
        else if (rxTransition.indexIn(line, 0) != -1) {
          pos = 0;
          /*
          bool conditional = 0;
          //condtional transition, but not a call
          conditional = !(rxTransitionIf.indexIn(line, 0)==-1) 
            && (rxTransitionCall.indexIn(line, 0)==-1);
            */
          while ((pos = rxTransBB.indexIn(line, pos)) != -1) {
            QString targetBlockName =  rxTransBB.cap(1);
            if(block){
              //if(block->m_name!=targetBlockName || conditional)
	      if(task && ((task->m_type==BR) || (task->m_type==SWITCH))) 
		{ 
                block->m_targetBlockNames.push_back(targetBlockName);
                //qDebug() << task->m_insn << task->m_type << "\t\t\t";
                //qDebug() << line << endl;
		} 
            }
            pos += rxTransBB.matchedLength();
          }
          pos = 0;
          while ((pos = rxTransLoopWait.indexIn(line, pos)) != -1) {
            QString targetBlockName =  rxTransLoopWait.cap(1);
            if(block){
              //if(block->m_name!=targetBlockName || conditional)
              //if(task && task->m_type==BR)
              //{
                block->m_targetBlockNames.push_back(targetBlockName);
              //}
            }
            pos += rxTransLoopWait.matchedLength();
          }
        }
        else if (rxEndFunc.indexIn(line, 0) != -1) {
          function = NULL;
          block = NULL;
        }
        else {
          if(rxState.indexIn(line, 0) != -1
	     && rxStartState.indexIn(line,0)==-1) {
            qWarning() << "Skipping a state:"
              "(this may cause incorrect transitioning between Basic blocks)\n"
              << line << endl;
          }
          //qDebug() << line << endl;
        }
    }


    /*construct control flow graph with names*/
    /* we find the actual Block instance from the recorded names. this can not be
     * done while parsing the document because the blocks were not fully
     * constructed yet
     */
    //note we cannot do the same for functions, because some callees, like
    //printf, doesn't have its own Function instance, so all future queries,
    //will be done through the findFunction routine within the Schedule class
    for (int k = 0; k < m_functions.size(); k++) {
      Function * function = m_functions[k];
      for(int j = 0; j < (int)function->m_blocks.size(); j++)
      {
        Block * block = function->m_blocks[j];
        assert(block);
        for(int i = 0; i < (int)block->m_targetBlockNames.size(); i++){
          Block * targetBlock = function->findBlock(block->m_targetBlockNames[i]);
          if(targetBlock){
            block->m_targetBlocks.push_back(targetBlock);
          }
          else
            qDebug() << "Block with name " << block->m_targetBlockNames[i]
              << " for function " << function->getName()
              << "queried by " << block->m_name 
              << " cannot be found" << endl;
        }
      }
    }

    /*construct data model*/
    /* now, ScheduleItem inherts QStandardItem.
     * Function, Block both inherts ScheduleItem,
     * so we do not need to create new object for these items,
     * just use them */
    QStandardItem *rootItem = invisibleRootItem();
    for (int k = 0; k < m_functions.size(); k++) {
        Function * function = m_functions[k];
        //QStandardItem *functionItem = new ScheduleItem(function->getName());
        QStandardItem *functionItem = function;
        functionItem->setFlags(functionItem->flags() & ~Qt::ItemIsEditable);
        rootItem->appendRow(functionItem);
        //QStandardItem *parentItem = item;
        for (int i = 0; i < (int)function->m_blocks.size(); i++) {
            Block *block = function->m_blocks[i];
            QStandardItem *blockItem;
            //blockItem= new ScheduleItem(block->text());
            blockItem= block; 
            blockItem->setFlags(blockItem->flags() & ~Qt::ItemIsEditable);
            functionItem->appendRow(blockItem);
            //qDebug() << "Basic Block: " << block->getIndex() << endl;
            /* we don't need to create child task item anymore since
             * 1. we don't need to display them in the explorer
             * 2. tasks can be accessed through the m_tasks member variable
             */

            /*for (int j = 0; j < block->getNumTasks(); j++) {
              QStandardItem *taskItem_insn = new ScheduleItem(
              block->getTask(j)->m_insn);
              QStandardItem *taskItem_begin = new ScheduleItem(
              QString::number(block->getTask(j)->m_beginState));
              QStandardItem *taskItem_end = new ScheduleItem(
              QString::number(block->getTask(j)->m_endState));
              QList<QStandardItem *> list;
              list << taskItem_insn << taskItem_begin << taskItem_end;
              blockItem->appendRow(list);
            //taskItem->setFlags(taskItem->flags() & ~Qt::ItemIsEditable);
            //qDebug() << "\tInstruction: "
            //	<< block->getTask(j)->m_insn
            //<< "column count" << blockItem->columnCount() << endl;
            }
            */
        }
    }

    //process instruction dependencies
    for (int k = 0; k < m_functions.size(); k++) {
        Function * function = m_functions[k];
        for(int j = 0; j < (int)function->m_blocks.size(); j++)
        {
            Block * block = function->m_blocks[j];
            for(int k = 0; k < (int)block->m_tasks.size(); k++){
                for(int m = 0; m < (int)block->m_tasks.size(); m++){
                    if(k == m) continue;
                    if(block->m_tasks[m]->m_sources.contains(block->m_tasks[k]->m_destination)){
                        block->m_tasks[k]->m_dependent.push_back(block->m_tasks[m]);
                        block->m_tasks[m]->m_depending.push_back(block->m_tasks[k]);
                    }

                }
            }
        }
    }

    setHeaderData(0, Qt::Horizontal, "Explorer");
    return 0;
}


Function *
Schedule::findFunction(QString &name) const
{
    for( int j = 0; j < m_functions.size(); j++){
        if(m_functions[j]->m_name == name)
            return m_functions[j];
    }
    return NULL;
}
