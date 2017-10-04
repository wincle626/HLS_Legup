/*
 * ArrayVar.h
 *
 *  Created on: 2012-07-31
 *      Author: fire
 */

#ifndef ARRAYVAR_H_
#define ARRAYVAR_H_
#include <Node.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string.h>
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


class ArrayVar {
public:
	ArrayVar(int inType, int EleNum);
	vector <Node*> ArrayNodes;
	int numNodes;
	Node* ptrNode;
	int type;	//0 for input, 1 for ouput

};

#endif /* ARRAYVAR_H_ */
