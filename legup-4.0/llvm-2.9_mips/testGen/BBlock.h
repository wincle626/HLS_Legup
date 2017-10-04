/*
 * BBlock.h
 *
 *  Created on: 2012-06-23
 *      Author: fire
 */

#ifndef BBLOCK_H_
#define BBLOCK_H_
#include "DFGGenerator.h"
//#include "CFGGenerator.h"
#include "Bitvector.h"

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define BBForEachFanin( BB, BBFanin, i )                                                           \
    for ( i = 0; (i < BB->InputBB.size()) && (((BBFanin) = BB->InputBB.at(i)), 1); i++ )

#define BBForEachFanout( BB, BBFanout, i )                                                           \
    for ( i = 0; (i < BB->OutputBB.size()) && (((BBFanout) = BB->OutputBB.at(i)), 1); i++ )
class BBlock {
public:
	int BBidx;
	int visited;
	int hasGraph;
	int TerminatorCreated;
	int BBType;	//0 for Entry, 1 for Exit, 2 for others
	int level;
	int inputNodeNum;
	int outputNodeNum;
	BasicBlock* LLVM_BB;
	int LLVM_created;

	int use_CFG;
	DFGGenerator *DFG_NtkGenerator;
	void *CFG_NtkGenerator;
	void *CFG_Ntk;
	Function* func_ptr;


	vector<BBlock*> InputBB;
	vector<BBlock*> OutputBB;
	vector<BBlock*> DominatorsBB;
	Bitvector *bvDominatorsBB;
	int expectedFanoutNumber;
	vector<ConstantInt*> vConstantInt_in_BB;
	BBlock(int type);
	BBlock(int type, int useCFG);
	int assign_level();
	int allPredecessorCreated();

};

#endif /* BBLOCK_H_ */
