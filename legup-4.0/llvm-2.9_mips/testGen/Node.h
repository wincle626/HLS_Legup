/*
 * Node.h
 *
 *  Created on: 2012-05-21
 *      Author: fire
 */

#ifndef NODE_H_
#define NODE_H_
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

using namespace llvm;
/*NodeOperation can be following
 *
 */

using namespace std;
enum OperationType{
	ADD, SUB, MULT, DIV, LADD, LSUB, LMULT, LDIV, SHL, LSHR, ASHR, LSHL, LLSHR, LASHR, FADD, FSUB, FMULT, FDIV, DADD, DSUB, DMULT, DDIV
};
class Node {
public:
	enum {
		Constant, Operation, Pi, Po, Pattern
	}Type;
	int nodeSize;
	int NodeIdx;
	OperationType NodeOperation;
	int NodeValue;	//if the node is constant, this should have a value
	int NodeType;
	int NumInput;
	int NumOutput;
	int visited;
	int isPi;
	int isPo;
	int isBrControl;
	int isFake;
	vector<Node*> InputNode;
	vector<Node*> OutputNode;
	int level;
	int level_diff;
	Value* NodeLLVMValue;
	int NodeLLVMValueCreated;
	Node();
	Node(int Type);
	int getExpectedNumInput();
	int isConst();
	int assign_level();
	int assign_level_reverse();
	int isPattern;
	//virtual ~Node();
};
#define NodeForEachFanin( pNode, pFanin, i )                                                           \
    for ( i = 0; (i < pNode->InputNode.size()) && (((pFanin) = pNode->InputNode.at(i)), 1); i++ )
#define NodeForEachFanout( pNode, pFanout, i )                                                           \
    for ( i = 0; (i < pNode->OutputNode.size()) && (((pFanout) = pNode->OutputNode.at(i)), 1); i++ )
#endif /* NODE_H_ */
