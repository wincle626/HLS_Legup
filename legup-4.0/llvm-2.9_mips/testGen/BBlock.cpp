/*
 * BBlock.cpp
 *
 *  Created on: 2012-06-23
 *      Author: fire
 */

#include "BBlock.h"

BBlock::BBlock(int type){
	hasGraph=0;
	LLVM_created = 0;
	TerminatorCreated = 0;
	BBType = type;
	inputNodeNum = -1;
	outputNodeNum = -1;
	visited = 0;
	use_CFG = 0;
	//DFG_Ntk = new DAGDFGNtk();
}
BBlock::BBlock(int type, int useCFG){
	hasGraph=0;
	LLVM_created = 0;
	TerminatorCreated = 0;
	BBType = type;
	inputNodeNum = -1;
	outputNodeNum = -1;
	visited = 0;
	use_CFG = useCFG;
}
int BBlock::allPredecessorCreated(){
	BBlock *BBFanin;
	int i;
	int allCreated = 1;
	BBForEachFanin( this, BBFanin, i ){
		if(BBFanin->LLVM_created == 0){
			//printf("Block %d has not been created\n", BBFanin->BBidx);
			allCreated = 0;
			break;
		}
	}


	return allCreated;
}
int BBlock:: assign_level(){
	int max_level = 0;
	int i;
	BBlock *pFanout;
	BBForEachFanout( this, pFanout, i ){
		if(pFanout->level>max_level)
			max_level = pFanout->level;
	}
	return max_level+1;
}
