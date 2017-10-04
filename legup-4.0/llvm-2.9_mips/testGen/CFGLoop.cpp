/*
 * LoopCFG.cpp
 *
 *  Created on: 2012-06-26
 *      Author: fire
 */

#include "CFGLoop.h"

CFGLoop::CFGLoop(BBlock *inBackEdgeHead, BBlock *inBackEdgeTail, int iterNum)
{
	this->BackEdgeHead = inBackEdgeHead;
	this->BackEdgeTail = inBackEdgeTail;
	this->loop_iteration_num = iterNum;
}

