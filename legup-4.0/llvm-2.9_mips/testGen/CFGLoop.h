/*
 * LoopCFG.h
 *
 *  Created on: 2012-06-26
 *      Author: fire
 */

#ifndef LOOPCFG_H_
#define LOOPCFG_H_
#include "BBlock.h"

class CFGLoop {
public:
	BBlock *BackEdgeHead;
	BBlock *BackEdgeTail;
	int loop_iteration_num;
	CFGLoop(BBlock *inBackEdgeHead, BBlock *inBackEdgeTail, int iterNum);
};

#endif /* LOOPCFG_H_ */
