/*
 * CFGGenerator.h
 *
 *  Created on: 2012-06-19
 *      Author: fire
 */

#ifndef CFGGENERATOR_H_
#define CFGGENERATOR_H_
#include "CFGNtk.h"
//#include "BBlock.h"
#include "Bitvector.h"
#include "AutoConfig.h"


class CFGGenerator {
public:
	CFGNtk *CFG_Ntk;
	AutoConfig *AC;

	int seed;
	float depthFactor;
	int enableSubFunc;
	int graph_level;
	int replicatedBBs;
	CFGGenerator(int max_BBNum, int inseed, int numIn, int numOut, int branchNum, float DFactor, int ArrayInput, int EnableSubFunc, AutoConfig * ac);
	int addInput(BBlock* tarBB, BBlock* inBB);
	void statsAnalysis();
	void StartGen();
	void CFG_StartGenRand();

private:
	int genFanoutNum(int branchStackCounter);
	void randAssignFanout(BBlock *BB);
	void assignInputNodeNum(BBlock *BBin);
	void CFG_NtkAnalysis();
	void CFG_FindAllDoms();
	//void CFG_InitializeAllDomsBV();
	//void CFG_FindDomsForBlock(BBlock *BB);
	int CFG_GraphVerification();
};

#endif /* CFGGENERATOR_H_ */
