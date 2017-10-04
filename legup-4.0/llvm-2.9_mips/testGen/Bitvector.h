/*
 * Bitvector.h
 *
 *  Created on: 2012-08-09
 *      Author: fire
 */

#ifndef BITVECTOR_H_
#define BITVECTOR_H_
//#include "BBlock.h"
#include <vector>
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



//using namespace llvm;

class Bitvector {
public:

	Bitvector(int size);
	void setAllOnes();
	void setBit(int pos, char val);
	char getBit(int pos);
	int getSize();
	Bitvector* BVor(Bitvector* bv_0, Bitvector* bv_1);
	Bitvector* BVand(Bitvector* bv_0, Bitvector* bv_1);
	void BVcopy(Bitvector* tar_bv);
	int BVsame(Bitvector* tar_bv);
	void printBV();
private:
	vector<char> bv;
};
//static Bitvector* BVand(Bitvector* bv_0, Bitvector* bv_1);
//static Bitvector* BVor(Bitvector* bv_0, Bitvector* bv_1);


#endif /* BITVECTOR_H_ */
