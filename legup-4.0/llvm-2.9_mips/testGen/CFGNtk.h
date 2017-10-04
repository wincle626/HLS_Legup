/*
 * CFGNtk.h
 *
 *  Created on: 2012-06-23
 *      Author: fire
 */

#ifndef CFGNTK_H_
#define CFGNTK_H_

#include "BBlock.h"
#include "CFGLoop.h"


#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Constants.h>
#include <llvm/GlobalVariable.h>
#include <llvm/Function.h>
#include <llvm/CallingConv.h>
#include <llvm/BasicBlock.h>
#include <llvm/Instructions.h>
#include <llvm/InlineAsm.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include "AutoConfig.h"

#include <vector>
using namespace llvm;

class CFGNtk {
public:
	int cfgIdx;
	int BB_cap;
	int numBB;
	int numLoop;
	int numBranchInput;
	int numBranchCreated;
	int numInput;
	int numOutput;
	int ArrayIn;
	int ArrayOut;
	BBlock *Entry;
	BBlock *Exit;
	vector<Node*> BranchControlInput;	//differ from the inputs for computation(which can be found in Entry block)
	vector<BBlock*> vBBlock;
	vector<CFGLoop*> vCFGLoop;
	ConstantInt* const_one;
	ConstantInt* const_one_float;
	ConstantInt* const_zero;
	ConstantInt* const_one_64;
	ConstantInt* const_one_float_64;
	ConstantInt* const_zero_64;
	AutoConfig *cfgAC;
	CFGNtk(int max_BBNum, int numIn, int numOut, int numBranchIn, AutoConfig * ac);
	CFGNtk(int max_BBNum, int numIn, int numOut, int numBranchIn, int ArrayInput, int ArrayOutput, AutoConfig * ac);
	void addNewBB(BBlock *BBin);
	void print_cfg_to_dot(char* fileName);
	Module* makeLLVMModulefromCFGNtk();
	Function* makeLLVMModulefromSUBCFGNtk(Module* mod, CFGNtk* cfgGraph, int uniqueFuncID);
	void makeLLVMModulefromBasicBlock(Module* Mod, Function* func_test, BBlock *BB);
	void makeLLVMModulefromPatern(Module* mod, Node* PatternNode, BBlock *BB);
	void makeLLVMTerminatorBasicBlock(Module* Mod, Function* func_test, BBlock *BB);
	void createReturnInstrForArrayBasicBlock(Module* mod, BBlock *BB);
	void makeLLVMForLoop(Module* mod, CFGLoop * cfgLoop);
	BinaryOperator* createInstforNode(Module* mod, Node* pNode, Value* op_0, Value* op_1, BasicBlock* BB);
	Value* create_convert_instr(Module* mod, Value* val, Type *TarType, BasicBlock* BB);
	void update_level();
	void reset_visit();
	int CFG_NtkVerification();
};

#define NtkForEachBBlock(CFG, BB, i )                                                           \
    for ( i = 0; (i < CFG->vBBlock.size()) && (((BB) = CFG->vBBlock.at(i)), 1); i++ )
#define NtkForEachLoop(CFG, Loop, i )                                                           \
    for ( i = 0; (i < CFG->vCFGLoop.size()) && (((Loop) = CFG->vCFGLoop.at(i)), 1); i++ )
#endif /* CFGNTK_H_ */

