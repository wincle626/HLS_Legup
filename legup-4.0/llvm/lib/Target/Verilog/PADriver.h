//===-- PADriver.h - Points-To Analysis -------------------*- C++ -*-===//
//
// This code was taken from:
//      https://code.google.com/p/addr-leaks/wiki/HowToUseThePointerAnalysis
//
//===----------------------------------------------------------------------===//
//
// Determines points-to analysis. Using the algorithm from the paper:
//      Ben Hardekopf and Calvin Lin. 2007. The ant and the grasshopper: fast
//      and accurate pointer analysis for millions of lines of code. In
//      Proceedings of the 2007 ACM SIGPLAN conference on Programming language
//      design and implementation (PLDI '07). ACM, New York, NY, USA, 290-299.
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_PADRIVER_H
#define LEGUP_PADRIVER_H

#include "llvm/IR/Value.h"
#include "llvm/IR/Use.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

class PointerAnalysis;

// class PADriver : public ModulePass {
class PADriver {
  public:
    // +++++ FIELDS +++++ //
    // Used to assign a int ID to Values and store names
    int currInd;
    int nextMemoryBlock;
    std::map<Value *, int> value2int;
    std::map<int, Value *> int2value;

    std::map<Value *, int> valMap;
    std::map<Value *, std::vector<int>> valMem;
    std::map<int, std::string> nameMap;

    std::map<Value *, std::vector<int>> memoryBlock;
    std::map<int, Value *> int2mem;
    std::map<int, std::vector<int>> memoryBlock2;
    std::map<Value *, std::vector<Value *>> phiValues;
    std::map<Value *, std::vector<std::vector<int>>> memoryBlocks;
    unsigned int numInst;

    static char ID;
    PointerAnalysis *pointerAnalysis;

    PADriver();

    // +++++ METHODS +++++ //

    bool runOnModule(Module &M);
    int Value2Int(Value *v);
    void findAllPointerOperands(User *U, std::set<Value *> &ptrs);
    void handleGetElementPtrConst(Value *op);
    int getNewMem(std::string name);
    int getNewInt();
    int getNewMemoryBlock();
    void handleNestedStructs(const Type *Ty, int parent);
    void handleAlloca(Instruction *I);
    void handleGlobalVariable(GlobalVariable *G);
    void handleGetElementPtr(Instruction *I);
    // Value* Int2Value(int);
    virtual void print(raw_ostream &O, const Module *M) const;
    std::string intToStr(int v);
    void process_mem_usage(double &vm_usage, double &resident_set);
    void addConstraints(Function &F);
    void matchFormalWithActualParameters(Function &F);
    void matchReturnValueWithReturnVariable(Function &F);
};
#endif
