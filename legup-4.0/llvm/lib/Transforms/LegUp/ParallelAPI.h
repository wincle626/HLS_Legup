//===- ParallelAPI.cpp - LegUp pre-LTO pass
//------------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// ParallelAPI - Replace Pthread/OMP API calls to LegUp functions
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_PARALLELAPI_H
#define LEGUP_PARALLELAPI_H

#include "llvm/Pass.h"
#include "utils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include <unordered_map>

using namespace llvm;

namespace legup {

class ParallelAPI : public ModulePass {
  public:
    static char ID; // Pass identification, replacement for typeid
    ParallelAPI() : ModulePass(ID), numOMPatomic(0) {}

    virtual bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<LoopInfo>();
        AU.addRequired<CallGraphWrapperPass>();
    }

  private:
    bool getAndReplaceCalls(Module &M);
    bool getAndReplaceOMPCalls(Module &M, CallInst *CI, bool &modified);
    bool getAndReplacePthreadCalls(Module &M, CallInst *CI, bool &modified);
    void getCalledFunctions(Function *F, std::vector<Function *> &functions);
    std::string getPthreadMutexVarName(CallInst *CI);
    std::string getMutexName(CallInst *CI);
    void getAllCallerFunctionsUntilArgFound(Function *F, std::string Arg,
                                            std::set<Function *> &callerSet);
    void getFunctionsToBeModified(Module &M, std::set<Function *> &functions,
                                  const std::string APIFuncName,
                                  const std::string argName);
    Value *getPthreadIDGlobalVariable(Function *F);

    bool replaceAtomicFunctionCalls(Module &M, CallInst *CI, bool &modified);
    bool replaceBarrierCalls(Module &M, CallInst *CI, bool &modified);
    void replaceUnreachableInst(Instruction *Inst);
    bool replaceParallelIntrinsicFunctions(Module &M);
    void replaceOMPIntrinsicFunctionCalls(Function *F, int numThreads = 0);
    bool replaceSyncronizationFunctions(Module &M, CallInst *CI, bool &modified,
                                        bool &atomicRegion,
                                        bool &barrierRegion);
    bool replaceHybridOnlyFunctionCalls(CallInst *CI, bool &replaced);
    bool replacePthreadJoinCalls(CallInst *CI);
    bool replacePthreadCreateCalls(
        CallInst *CI,
        std::vector<std::pair<Function *, unsigned>> &pthreadFunctionThreadID);
    bool replaceLockFunction(Module &M, CallInst *CI, bool &modified,
                             bool &atomicRegion);
    bool replaceBarrierFunction(Module &M, CallInst *CI, bool &modified,
                                bool &barrierRegion);
    bool replaceOMPFunction(CallInst *CI);
    bool getAndReplaceCallInsts(Module &M, CallInst *CI, bool &modified,
                                bool &atomicRegion, bool &barrierRegion);
    void replaceOMPThreadFunctionCalls(Function *F, CallInst *CI,
                                       Value *numThreads, Value *threadID);
    void replaceCallsWhileAddingMetadata(
        Function *oldF, Function *newF,
        const std::vector<std::pair<std::string, std::string>> &
            metadataStrVector);
    CallInst *replaceCallInstFromOldtoNewFunction(CallInst *oldCall,
                                                  Function *newF,
                                                  std::vector<Value *> newArgs,
                                                  bool copyAttributes = true);

    CallInst *replaceCalltoLockFunction(Module &M, CallInst *CI,
                                        std::string legupLockFunction,
                                        std::string mutexName);
    Instruction *replaceCalltoUnlockFunction(Module &M, CallInst *CI,
                                             std::string legupLockFunction,
                                             std::string mutexName);

    bool isaHybridOnlyFunction(const std::string funcName);
    bool isaLockFunction(std::string funcName);
    bool isaUnlockFunction(std::string funcName);

    Function *generatePthreadCallWrapper(Function *F);
    Function *generateOMPWrapper(Function *F, int numThreads);
    bool generateWrappersAndReplaceFunctions(Module &M);
    void generatePthreadThreadIDInstructions(IRBuilder<> &builder, Function *F,
                                             CallInst *Call,
                                             const unsigned pthreadFunctionID);

    Function *createFunctionwithExtraArgument(
        Function *oldF,
        const std::vector<std::pair<std::string, Type *>> &argVector);
    Function *createNewFunctionAndReplaceCalls(
        Function *oldF, const int numThreads,
        AttributeSet attrs = AttributeSet(),
        std::vector<pair<string, string>> metadataStrVector =
            std::vector<pair<string, string>>());

    void setLoadtoVolatile(LoadInst *LI, bool setVolatile, bool &modified);

    void findInternalAccels(Module &M);
    void setInternalAccels(Module &M, Function *F);

    bool insertNewArgumentAndReplaceCalls(Module &M,
                                          const std::set<Function *> &functions,
                                          const std::string argName,
                                          Type *argType);

    std::vector<std::pair<CallInst *, wrapperType>> parallelFunctions;
    std::map<std::string, int> mutexMap;
    int numOMPatomic;
};

} // end of legup namespace

#endif
