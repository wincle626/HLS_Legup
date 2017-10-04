//===- ParallelAPI.cpp - LegUp pre-LTO pass ------------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// ParallelAPI - Replace Pthread/OMP API calls to LegUp functions
//
//===----------------------------------------------------------------------===//

#include "ParallelAPI.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/CallSite.h"
#include "utils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Constants.h"
#include "../lib/IR/ConstantsContext.h"
#include <unordered_map>

using namespace llvm;

namespace legup {

char ParallelAPI::ID = 0;
static RegisterPass<ParallelAPI>
    K("legup-parallel-api", "Replace Pthread/OMP calls to LegUp functions");

bool ParallelAPI::runOnModule(Module &M) {

    bool modified = false;

    modified = getAndReplaceCalls(M);

    modified |= generateWrappersAndReplaceFunctions(M);

    // replace any instrinsic functions for
    // Pthreads/OpenMP
    modified |= replaceParallelIntrinsicFunctions(M);

    // find all internal accels
    // and mark with metadata
    findInternalAccels(M);

    return modified;
}

// get calls instructions and replace any if necessary
bool ParallelAPI::getAndReplaceCalls(Module &M) {

    bool modified = false;
    // region between lock and unlock
    bool atomicRegion = false;
    // region between barrier and end of basic block
    bool barrierRegion = false;

    // look for call instructions and
    // make necessary replacements
    for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
        for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE;
             ++BB) {
            for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;) {

                Instruction *Inst = I++;
                // If it's a call instruction
                if (CallInst *CI = dyn_cast<CallInst>(Inst)) {

                    Function *calledFunction = CI->getCalledFunction();

                    // ignore indirect function calls
                    if (!calledFunction)
                        continue;

                    // get and replace call instructions
                    if (getAndReplaceCallInsts(M, CI, modified, atomicRegion,
                                               barrierRegion))
                        continue;

                    // if it's accelerated function, don't inline it
                    if (LEGUP_CONFIG->isAccelerated(calledFunction->getName()))
                        calledFunction->addFnAttr(
                            Attribute::NoInline); // adding noinline attribute
                                                  // to function

                    // if it's a load
                } else if (LoadInst *LI = dyn_cast<LoadInst>(Inst)) {

                    setLoadtoVolatile(LI, atomicRegion || barrierRegion,
                                      modified);

                } else {

                    replaceUnreachableInst(Inst);
                }
            }

            barrierRegion = false;
        }
    }

    return modified;
}

bool ParallelAPI::getAndReplaceCallInsts(Module &M, CallInst *CI,
                                         bool &modified, bool &atomicRegion,
                                         bool &barrierRegion) {

    // get and replace functions for
    // OpenMP, Pthreads, lock, barriers, etc
    if (getAndReplaceOMPCalls(M, CI, modified) ||
        getAndReplacePthreadCalls(M, CI, modified) ||
        replaceSyncronizationFunctions(M, CI, modified, atomicRegion,
                                       barrierRegion) ||
        replaceAtomicFunctionCalls(M, CI, modified) ||
        replaceHybridOnlyFunctionCalls(CI, modified))
        return true;

    return false;
}

void ParallelAPI::setLoadtoVolatile(LoadInst *LI, bool setVolatile,
                                    bool &modified) {

    // if the load is in a atomic region (between lock and
    // unlock) or in a barrier region,
    // set all loads to volatile
    // since the compiler may optimize the load away
    // in which case the lock wouldn't work properly
    if (setVolatile) {
        LI->setVolatile(true);
        modified = true;
    }
}

bool ParallelAPI::generateWrappersAndReplaceFunctions(Module &M) {

    bool modified = false;

    // vector to store threadID for each pthread function
    std::vector<std::pair<Function *, unsigned>> pthreadFunctionThreadID;

    for (std::vector<std::pair<CallInst *, wrapperType>>::iterator it =
             parallelFunctions.begin();
         it != parallelFunctions.end(); ++it) {

        CallInst *CI = (*it).first;
        wrapperType type = (*it).second;

        if (type == ompcall) {

            modified = replaceOMPFunction(CI);
        } else if (type == pthreadcall) {

            modified = replacePthreadCreateCalls(CI, pthreadFunctionThreadID);
        } else if (type == pthreadpoll) {

            modified = replacePthreadJoinCalls(CI);
        } else
            assert(0 && "No matching wrapper types!\n");
    }

    return modified;
}

// generates a wrapper function in LLVM IR for an OMP function F
Function *ParallelAPI::generateOMPWrapper(Function *F, int numThreads) {

    // create the wrapper function (which will call the Pthread routine)
    std::string wrapperName = getWrapperName(F, ompcall);

    std::vector<Type *> argTypes;
    // only one argument exists for OMP functions
    argTypes.push_back(*(F->getFunctionType()->param_begin()));

    // create wrapper function
    Constant *c = F->getParent()->getOrInsertFunction(
        wrapperName, FunctionType::get(Type::getVoidTy(getGlobalContext()),
                                       argTypes, false));
    Function *wrapperF = cast<Function>(c);
    wrapperF->setCallingConv(CallingConv::C);

    //copy the argument names from the newly created omp function to the wrapper function
	for (Function::arg_iterator I = F->arg_begin(), E = --F->arg_end(),
        I2 = wrapperF->arg_begin(); I != E; ++I, ++I2) {
        I2->setName(I->getName());
    }

    wrapperF->addFnAttr(
        Attribute::NoInline); // adding noinline attribute to function

    std::vector<Value *> Args;
    // only one argument exists for OMP functions
    Args.push_back(wrapperF->arg_begin());

    // now start to create instructions to call each accelerator:
    //
    // create the basic BB
    BasicBlock *entry =
        BasicBlock::Create(getGlobalContext(), "entry", wrapperF);
    IRBuilder<> builder(entry);

    // create a call instruction for each instance of the accelerator
    // each instance is assigned to a unique threadID
    for (int i = 0; i < numThreads; i++) {

        // push the threadID as an extra argument, which is just the index
        // number for the accelerator
        Args.push_back(ConstantInt::get(
            IntegerType::get(getGlobalContext(), 32), i, false));

        // create the call instruction to the accelerated function
        CallInst *call = builder.CreateCall(F, Args, "");

        // set metadata used for later stages in LegUp
        setMetadataStr(call, "TYPE", "omp_function");
        setMetadataStr(call, "NUMTHREADS", utostr(numThreads));
        setMetadataInt(call, "THREADID", i);

        Args.pop_back();
    }

    // create a return void instruction
    builder.CreateRetVoid();

    // return wrapper function pointer
    return wrapperF;
}

// This function replaces call to pthread_create
// to a call to the actual pthread function
// First insert instructions for loading/incrementing/storing threadID
// then replace call to pthread_create with regular call to the pthread function
bool ParallelAPI::replacePthreadCreateCalls(
    CallInst *CI,
    std::vector<std::pair<Function *, unsigned>> &pthreadFunctionThreadID) {

    // get the pthread function
    Module &M = *CI->getParent()->getParent()->getParent();
    std::string funcName =
        (CI->getArgOperand(2))->stripPointerCasts()->getName().str();
    Function *F = findFuncPtr(M, funcName.c_str());

    // get the argument into pthread function
    CallInst *Call = CI;
    Value *argOp = Call->getArgOperand(3);

    std::vector<Value *> Args;
    // check if the argument is null
    if (!dyn_cast<ConstantPointerNull>(argOp)) {
        Args.push_back(argOp);
    }

    // get the number of threads by analyzing loop trip count
    LoopInfo *LI;
    LI = &getAnalysis<LoopInfo>(*(CI->getParent()->getParent()));
    unsigned numThreads = getLoopTripCount(CI->getParent(), LI);

    // get the threadID and functionID
    // for this pthread function
    unsigned threadID = 0, functionID = 0;
    bool found = false;
    for (vector<pair<Function *, unsigned>>::iterator it =
             pthreadFunctionThreadID.begin();
         it != pthreadFunctionThreadID.end(); ++it) {
        if (it->first == F) {
            threadID = it->second;
            it->second = threadID + 1;
            found = true;
            break;
        }
        functionID++;
    }

    if (!found) {
        pthreadFunctionThreadID.push_back(make_pair(F, threadID + 1));
    }

    // Before replacing the call instruction, we need to insert instructions for
    // getting the threadID.
    // This has to be done outside the wrapper function since to prevent
    // a stale threadID value from being loaded
    IRBuilder<> builder(Call);

    generatePthreadThreadIDInstructions(builder, F, Call, functionID);

    // now replace call
    CallInst *New = replaceCallInstFromOldtoNewFunction(Call, F, Args, false);

    // add noinline attribute to function
    New->setIsNoInline();

    // set metadata for the new call instruction
    setMetadataStr(New, "PTHREADNAME", F->getName());
    setMetadataInt(New, "NUMTHREADS", numThreads);
    setMetadataInt(New, "THREADID", threadID);
    setMetadataInt(New, "FUNCTIONID", functionID);
    setMetadataStr(New, "TYPE", "legup_wrapper_pthreadcall");

    // check if we need to preserve the pthread value
    // this was set before when replacing
    // pthread_exit with a ret instruction
    if (!F->hasFnAttribute(Attribute::NoReturn)) {
        preserveLLVMInstruction(New, New->getNextNode());
    }

    // increment threadID for this pthread function
    //    pthreadFunctionThreadID[F] = ++ID;

    // this is used to get the maximum number of parallel threads
    // that will be forked for a function
    // at the same time
    unsigned numThreadsMax = numThreads;

    // threadID may be bigger, if pthread_create is not used
    // in a loop, in which case numThreads == 1
    // then assign threadID to numThreadsMax
    // + 1 is there since threadID starts from 0
    if (threadID + 1 > numThreadsMax)
        numThreadsMax = threadID + 1;

    // set the total number of threads
    // for this function
    F->addFnAttr("totalNumThreads", utostr(numThreadsMax));

    // remove noreturn attribute now
    // (llvm removes all proceeding instructions otherwise)
    F->removeFnAttr(Attribute::NoReturn);

    return true;
}

// generate instructions for
// loading/increment/storing threadID
// for a pthread function
void
ParallelAPI::generatePthreadThreadIDInstructions(IRBuilder<> &builder,
                                                 Function *F, CallInst *Call,
                                                 const unsigned functionID) {

    // get the thread variable pointer
    // instanceID value will be stored here
    Value *threadVarptr = Call->getArgOperand(0);

    assert(threadVarptr);

    // get the threadID global variable
    // used for this function
    Value *V0 = getPthreadIDGlobalVariable(F);

    // load from globalvar
    // this is the threadID value for this instance of the pthread function
    Instruction *threadIDValue = builder.CreateLoad(
        V0, false, "legup_count_" + F->getName().str() + "_loadedValue");
    assert(threadIDValue);

    // increment the loaded value for next thread
    Value *add = builder.CreateAdd(
        threadIDValue,
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 1, false));
    // store back into global variable
    builder.CreateStore(add, V0, false);

    // create the pthead function ID in LLVM IR
    Value *functionIDVal = ConstantInt::get(
        IntegerType::get(getGlobalContext(), 32), functionID, "functionID");

    // now using the threadID and functionID
    // we create instanceID (= functionID << 16 | threadID)
    // this value is used the polling wrapper to determine
    // which function and which instance of that function to poll and retrieve
    // the return value
    //
    // shift functionID << 16 and OR threadID
    Value *shiftedFunctionID = builder.CreateShl(functionIDVal, 16);
    Value *functionThreadID =
        builder.CreateOr(shiftedFunctionID, threadIDValue);

    // store the instance ID into threadVarPtr
    // make it volatile to make sure it's not optimized away
    Instruction *st = builder.CreateStore(functionThreadID, threadVarptr, true);
    setMetadataInt(st, "legup_pthread_functionthreadID", 1);
    setMetadataInt(st, "legup_pthread_wrapper_inst", 1);
}

// this function get the global variable used to count
// the thread ID for the given function F
Value *ParallelAPI::getPthreadIDGlobalVariable(Function *F) {

    Value *V0;
    Value *oldGlobalVar = dyn_cast_or_null<GlobalVariable>(
        F->getParent()->getNamedValue("legup_count_" + F->getName().str()));

    if (oldGlobalVar == NULL) {
        // if it doensn't exist, create one
        ConstantInt *zero = ConstantInt::get(
            IntegerType::get(getGlobalContext(), 32), 0, false);
        // create the global variable and initialize to zero
        V0 = new GlobalVariable(
            *F->getParent(), IntegerType::get(getGlobalContext(), 32), false,
            GlobalValue::ExternalLinkage, zero, "legup_count_" + F->getName());
    } else {
        V0 = oldGlobalVar;
    }

    return V0;
}

bool ParallelAPI::replacePthreadJoinCalls(CallInst *CI) {

    Module *mod = CI->getParent()->getParent()->getParent();

    // The pthread variable will be added as an argument
    // this argument will be used to load the base address of the HW accelerator
    // at run time
    // reading from thread variable will be done inside the wrapper
    // first add the type of the arguments to this vector
    std::vector<Type *> newParamType;
    // get the thread variable
    Value *threadVar = CI->getArgOperand(0);
    // add the type of the thread variable
    newParamType.push_back(threadVar->getType());

    // now add the actual arguments to this vector
    std::vector<Value *> newParam;
    newParam.push_back(threadVar);

    std::string wrapperName = "legup_pthreadpoll";

    // create the function prototype for the wrapper
    FunctionType *FTy = FunctionType::get(
        Type::getInt8PtrTy(getGlobalContext(), 0), newParamType, false);
    Constant *wrapperF = mod->getOrInsertFunction(wrapperName, FTy);
    cast<Function>(wrapperF)->setLinkage(GlobalValue::ExternalLinkage);

    // create the call instruction to pthread join
    CallInst *NewCI = CallInst::Create(wrapperF, newParam, "", CI);

    // if there is a return value, add a store instruction to store the return
    // value to the second parameter in pthread_join
    // store i8* NewCI, i8** retVar
    bool returnNull = false;
    // get the return variable
    Value *retVar = CI->getArgOperand(1);
    if (isa<ConstantPointerNull>(retVar)) {
        returnNull = true;
    }
    if (!returnNull) {
        // StoreInst::StoreInst(NewCI, retVar, ++I);
        new StoreInst(NewCI, retVar, false, CI);
    }

    // add metadata to the new call instruction
    setMetadataStr(NewCI, "TYPE", "legup_wrapper_pthreadpoll");
    setMetadataInt(NewCI, "NUMTHREADS", 1);

    // delete the original call instruction
    deleteInstruction(CI);

    return true;
}

// this function replaces the calls to the synchronization functions (i.e.
// pthread lock, omp lock)
// to call their corresponding legup synchronization functions (i.e.
// legup_lock).
// it also inserts metadata to be used later in other passes
bool ParallelAPI::replaceSyncronizationFunctions(Module &M, CallInst *CI,
                                                 bool &modified,
                                                 bool &atomicRegion,
                                                 bool &barrierRegion) {

    if (replaceLockFunction(M, CI, modified, atomicRegion) ||
        replaceBarrierFunction(M, CI, modified, barrierRegion))
        return true;

    return false;
}

bool ParallelAPI::replaceBarrierFunction(Module &M, CallInst *CI,
                                         bool &modified, bool &barrierRegion) {

    bool replaced = false;

    std::string calledFuncName = CI->getCalledFunction()->getName();

    // if this is a call to pthread_barrier_init
    // find the function pointer to this function and replace the call to legup
    // function
    if (calledFuncName == "pthread_barrier_init") {
        CI =
            ReplaceCallWith("legup_barrier_init", CI,
                            copyArguments((CI->op_end()) - 2, CI->op_end() - 1),
                            Type::getVoidTy(M.getContext()));

        // set no inline attribute to the call instruction
        // this is needed so that we can preserve the metadata added to the call
        // instruction
        cast<CallInst>(CI)
            ->setIsNoInline(); // adding noinline attribute to function

        replaced = modified = true;
    }

    // if this is a call to pthread_barrier_wait
    // find the function pointer to this function and replace the call to legup
    // function
    else if (calledFuncName == "pthread_barrier_wait") {
        CI = ReplaceCallWith("legup_barrier_wait", CI,
                             copyArguments(CI->op_end(), CI->op_end()),
                             Type::getVoidTy(M.getContext()));

        // set no inline attribute to the call instruction
        // this is needed so that we can preserve the metadata added to the call
        // instruction
        cast<CallInst>(CI)
            ->setIsNoInline(); // adding noinline attribute to function

        replaced = modified = true;

        // set barrier region
        barrierRegion = true;
    }

    return replaced;
}

bool ParallelAPI::replaceLockFunction(Module &M, CallInst *CI, bool &modified,
                                      bool &atomicRegion) {

    bool replaced = false;

    std::string calledFuncName = CI->getCalledFunction()->getName();

    // if this is a call to lock/unlock function
    if (isaLockFunction(calledFuncName) || isaUnlockFunction(calledFuncName)) {

        // get the name of the mutex variable used
        // for now use name to differentiate mutexes
        // maybe have to change this later
        std::string mutexName = getMutexName(CI);

        // for lock functions, insert the name of the mutexName
        if (isaLockFunction(calledFuncName)) {
            // insert the mutex into map if it hasn't been already
            if (mutexMap.find(mutexName) == mutexMap.end()) {
                mutexMap.insert(
                    std::pair<std::string, int>(mutexName, mutexMap.size()));
            }
        }

        // if it was an OMP atomic
        // increment the counter for omp atomic locks
        if (calledFuncName == "GOMP_atomic_end")
            numOMPatomic++;

        // if it is a lock function
        if (isaLockFunction(calledFuncName)) {

            // replace all from Pthread/OpenMP lock fuction to legup_lock
            // function
            replaceCalltoLockFunction(M, CI, "legup_lock", mutexName);

            // set atomic region
            atomicRegion = true;

        } else {

            // if it is a unlock function
            // replace call to legup unlock function
            replaceCalltoUnlockFunction(M, CI, "legup_unlock", mutexName);

            // clear atomic region
            atomicRegion = false;
        }

        replaced = modified = true;
    }

    return replaced;
}

// this function replace a call instruction from a
// pthread/openMP unlock function, to a legup_lock/unlock function
Instruction *
ParallelAPI::replaceCalltoUnlockFunction(Module &M, CallInst *CI,
                                         std::string legupLockFunction,
                                         std::string mutexName) {

    std::vector<Value *> params;
    // create offset integer for mutex which steers memory access
    ConstantInt *offset =
        ConstantInt::get(IntegerType::get(M.getContext(), 32),
                         mutexMap.find(mutexName)->second, false);

    // insert mutex offset
    params.push_back(offset);

    std::vector<Type *> paramType;
    paramType.push_back(IntegerType::get(M.getContext(), 32));

    // create the new function definition
    // get void type
    Type *voidTy = Type::getVoidTy(M.getContext());
    Constant *FCache = M.getOrInsertFunction(
        legupLockFunction, FunctionType::get(voidTy, paramType, false));
    // insert the call to the new function
    CallInst *newCI = CallInst::Create(FCache, params, "", CI);

    // set no inline attribute to the call instruction
    // this is needed so that we can preserve the metadata added to the call
    // instruction
    newCI->setIsNoInline();

    Instruction *ins = CI;
    // delete the original call instruction
    deleteInstruction(ins);

    return newCI;
}

// this function replace a call instruction from a pthread/openMP lock/unlock
// function, to a legup_lock/unlock function
CallInst *ParallelAPI::replaceCalltoLockFunction(Module &M, CallInst *CI,
                                                 std::string legupLockFunction,
                                                 std::string mutexName) {

    std::vector<Value *> params;
    // create offset integer for mutex which steers memory access
    ConstantInt* offset = ConstantInt::get(IntegerType::get(M.getContext(), 32), mutexMap.find(mutexName)->second, false);
    params.push_back(offset);

    std::vector<Type*>paramType;
    paramType.push_back(IntegerType::get(M.getContext(), 32));

    // create the new function definition
    Constant *FCache = M.getOrInsertFunction(
        legupLockFunction,
        FunctionType::get(Type::getVoidTy(M.getContext()), paramType, false));
    // insert the call to the new function
    CallInst *newCI = CallInst::Create(FCache, params, "", CI);

    // set no inline attribute to the call instruction
    // this is needed so that we can preserve the metadata added to the call
    // instruction
    newCI->setIsNoInline(); // adding noinline attribute to function

    // get name of called function
    // to determine type of lock
    std::string calledFuncName = CI->getCalledFunction()->getName();

    // add metadata for what kind of mutex it is
    // this will be used later in SwOnly pass to determine how many
    // mutex cores to instantiate in the SOPC system
    if (calledFuncName == "pthread_mutex_lock") {
        setMetadataStr(newCI, "mutexType", "pthread_mutex_lock");
    } else if (calledFuncName == "GOMP_critical_start") {
        setMetadataStr(newCI, "mutexType", "omp_critical_start");
    } else if (calledFuncName == "GOMP_atomic_start") {
        setMetadataStr(newCI, "mutexType", "omp_atomic_start");
    }
    setMetadataStr(newCI, "mutexName", mutexName);

    Instruction *ins = CI;
    // delete the original call instruction
    deleteInstruction(ins);

    return newCI;
}

bool ParallelAPI::isaLockFunction(std::string funcName) {

    if (funcName == "pthread_mutex_lock" || funcName == "GOMP_critical_start" ||
        funcName == "GOMP_atomic_start")
        return true;
    else 
        return false;
}

bool ParallelAPI::isaUnlockFunction(std::string funcName) {

    if (funcName == "pthread_mutex_unlock" || funcName == "GOMP_critical_end" ||
        funcName == "GOMP_atomic_end")
        return true;
    else
        return false;
}

// this function returns the mutexName depending on the type of the lock
// function that is called
std::string ParallelAPI::getMutexName(CallInst *CI) {

    std::string mutexName; 
    Function *calledFunction = CI->getCalledFunction(); 
    if (calledFunction->getName().str() == "pthread_mutex_lock" 
     || calledFunction->getName().str() == "pthread_mutex_unlock") {
        mutexName = getPthreadMutexVarName(CI);
    } else if (calledFunction->getName().str() == "GOMP_critical_start"
            || calledFunction->getName().str() == "GOMP_critical_end") {
        mutexName = "OMP";
    } else if (calledFunction->getName().str() == "GOMP_atomic_start"
             ||calledFunction->getName().str() == "GOMP_atomic_end") {
        // this is for atomic section, which means it is used for a single
        // memory update right after the pragma
        // and every atomic section uses a different lock
        mutexName = "OMP_atomic_" + utostr(numOMPatomic);
    }

    return mutexName;
}

// this functions replaces calls to instric functions used for Pthreads/OpenMP
// such as omp_get_thread_num() is called to get the thread number
bool ParallelAPI::replaceParallelIntrinsicFunctions(Module &M) {

    std::set<Function *> functions;

    std::string funcName = "omp_get_thread_num";
    std::string argName = "threadID";
    Type *argType = IntegerType::get(M.getContext(), 32);

    getFunctionsToBeModified(M, functions, funcName, argName);

    return insertNewArgumentAndReplaceCalls(M, functions, argName, argType);
}

bool ParallelAPI::insertNewArgumentAndReplaceCalls(
    Module &M, const std::set<Function *> &functions, const std::string argName,
    Type *argType) {

    bool modified = false;

    // go through the callerFunctions
    // add an extra argument (threadID) to the function prototype
    std::vector<pair<Function *, Function *>> newFunctions;

    // first add the extra parameter to all functions
    for (std::set<Function *>::iterator it = functions.begin();
         it != functions.end(); it++) {

        Function *F = (*it);

        // set up the new argument
        std::vector<std::pair<std::string, Type *>> argVector;
        argVector.push_back(std::make_pair(argName, argType));

        // create new function with extra parameter
        Function *NF = createFunctionwithExtraArgument(F, argVector);

        // replace calls in OMP instrinsic functions if there are any
        replaceOMPIntrinsicFunctionCalls(NF);

        // save the new function as a pair with the old function
        newFunctions.push_back(std::make_pair(F, NF));

        modified = true;
    }

    // go through each function
    // and replace call from old to new function
    // with the additional argument
    std::vector<Value *> params;
    for (std::vector<pair<Function *, Function *>>::iterator it =
             newFunctions.begin();
         it != newFunctions.end(); it++) {

        Function *oldF = it->first;
        Function *NF = it->second;
        // add threadID to all call instructions calling the functions which
        // were just added with extra argument
        // if it is a top-level function (avalon accelerator), call instruction
        // doesn't have to be changed since it will be stripped away from HW
        // portion

        // get all call site for function F
        for (User *UI : oldF->users()) {

            CallSite CS(UI);
            Instruction *Call = CS.getInstruction();

            std::vector<Value *> params;
            params.assign(CS.arg_begin(), CS.arg_end());

            Function *parentF = Call->getParent()->getParent();

            Value *newARG;
            // find the presiously added argument from the function prototype
            for (Function::arg_iterator I = parentF->arg_begin(),
                                        E = parentF->arg_end();
                 I != E; ++I) {
                if ((I)->getName() == argName) {
                    newARG = I;
                }
            }
            assert(newARG);

            // add the new argument
            params.push_back(newARG);

            replaceCallInstFromOldtoNewFunction(cast<CallInst>(Call), NF,
                                                params);
        }

        // delete the original function from the module
        M.getFunctionList().remove(oldF);
    }

    return modified;
}

CallInst *ParallelAPI::replaceCallInstFromOldtoNewFunction(
    CallInst *oldCall, Function *newF, std::vector<Value *> newArgs,
    bool copyAttributes) {

    // create new CallInst
    CallInst *newCall = llvm::CallInst::Create(newF, newArgs, "", oldCall);
    // set the calling convention
    llvm::cast<llvm::CallInst>(newCall)
        ->setCallingConv(oldCall->getCallingConv());
    if (llvm::cast<llvm::CallInst>(oldCall)->isTailCall())
        llvm::cast<llvm::CallInst>(newCall)->setTailCall();

    // copy attributes
    if (copyAttributes)
        llvm::cast<llvm::CallInst>(newCall)
            ->setAttributes(oldCall->getAttributes());

    // copy metadata from old to new call instruction
    copyMetaData(oldCall, newCall);

    // replace call uses
    if (!oldCall->use_empty())
        oldCall->replaceAllUsesWith(newCall);

    // copy name
    newCall->takeName(oldCall);
    // remove old call
    oldCall->eraseFromParent();

    return newCall;
}

void ParallelAPI::getFunctionsToBeModified(Module &M,
                                           std::set<Function *> &functions,
                                           const std::string APIFuncName,
                                           const std::string argName) {

    for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
        for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE;
             ++BB) {
            for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;) {

                if (CallInst *CI = dyn_cast<CallInst>(I++)) {

	                Function *calledFunction = CI->getCalledFunction();

	                // ignore indirect function calls
	                if (!calledFunction) continue;

                    if (calledFunction->getName().str() == APIFuncName) {
                        // add this function to the set
                        functions.insert(F);
                        // find all callers of this function
                        getAllCallerFunctionsUntilArgFound(F, argName,
                                                           functions);
                    }
                }
            }
        }
    }
}

// this creates a copy of function oldF, with extra arguments added from
// argVector, and return a pointer to the new function NF
// the function body is also copied from oldF to newF, and the oldF is destroyed
Function *ParallelAPI::createFunctionwithExtraArgument(
    Function *oldF,
    const std::vector<std::pair<std::string, Type *>> &argVector) {

    // start by computing a new prototype for the function, which is the same as
    // the old function, but doesn't have isVarArg set.
    FunctionType *FTy = oldF->getFunctionType();

    std::vector<Type *> Params(FTy->param_begin(), FTy->param_end());

    // insert the new argument types
    for (std::vector<std::pair<std::string, Type *>>::const_iterator it =
             argVector.begin();
         it != argVector.end(); it++) {
        Params.push_back((*it).second);
    }

    FunctionType *NFTy = FunctionType::get(FTy->getReturnType(), Params, false);

    // create the new function body and insert it into the module...
    Function *NF =
        Function::Create(NFTy, oldF->getLinkage(), "", oldF->getParent());

    // copying attributes causes errors for some reason..
    // NF->copyAttributesFrom(oldF);

    NF->takeName(oldF);

    // Loop over the argument list, transferring uses of the old arguments over to
	// the new arguments, also transferring over the names as well.  While we're at
    // it, remove the dead arguments from the DeadArguments list.
    Function::arg_iterator I2 = NF->arg_begin();
    for (Function::arg_iterator I = oldF->arg_begin(), E = oldF->arg_end();
         I != E; ++I, ++I2) {

        // Move the name and users over to the new version.
        I->replaceAllUsesWith(I2);
        I2->takeName(I);
    }

    // Now set the names of the new arguments in the new function
    std::vector<std::pair<std::string, Type *>>::const_iterator it =
        argVector.begin();
    for (Function::arg_iterator E = NF->arg_end(); I2 != E; ++I2) {

        // set the names of the new arguments
        I2->setName((*it).first);
        ++it;
    }

    NF->getBasicBlockList().splice(NF->begin(), oldF->getBasicBlockList());

    return NF;
}

// replace all OMP functions
bool ParallelAPI::replaceOMPFunction(CallInst *CI) {

    // get the OMP function
    std::string HwFuncName = (CI->op_begin())->get()->getName().str();
    Module &M = *CI->getParent()->getParent()->getParent();
    Function *HwFuncPtr = findFuncPtr(M, HwFuncName.c_str());

    // get the number of threads
    ConstantInt *constInt = dyn_cast<ConstantInt>((CI->op_begin() + 2)->get());
    assert(constInt);
    int numThreads = constInt->getValue().getZExtValue();

    // add noinline attribute to the OMP function, since this should never be
    // inlined
    HwFuncPtr->addFnAttr(Attribute::NoInline);

    // delete the call instruction
    deleteInstruction(CI);

    Function &Fn = *HwFuncPtr;

    // after creating the new openmp function
    // recurse down to all functions that it calls
    // and apply the code below

    // set up the attributes
    AttrBuilder B;
    B.addAttribute("totalNumThreads", utostr(numThreads))
        .addAttribute(Attribute::NoInline);
    AttributeSet attrs =
        AttributeSet::get(getGlobalContext(), AttributeSet::FunctionIndex, B);

    // set up the vector to store metadata
    std::vector<std::pair<std::string, std::string>> metadataStrVector;
    metadataStrVector.push_back(
        std::make_pair("NUMTHREADS", utostr(numThreads)));
    metadataStrVector.push_back(std::make_pair("OMPNAME", Fn.getName()));
    metadataStrVector.push_back(std::make_pair("TYPE", "legup_wrapper_omp"));

    createNewFunctionAndReplaceCalls(&Fn, numThreads, attrs, metadataStrVector);

    return true;
}

void ParallelAPI::getCalledFunctions(Function *F,
                                     std::vector<Function *> &functions) {

    for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
        for (BasicBlock::iterator I = BB->begin(), EE = BB->end(); I != EE;
             ++I) {
            if (CallInst *CI = dyn_cast<CallInst>(I)) {

                Function *calledFunction = CI->getCalledFunction();

                // ignore indirect function calls
                if (!calledFunction)
                    continue;

                if (find(functions.begin(), functions.end(), calledFunction) ==
                    functions.end()) {
                    functions.push_back(calledFunction);
                    getCalledFunctions(calledFunction, functions);
                }
            }
        }
    }
}

Function *ParallelAPI::createNewFunctionAndReplaceCalls(
    Function *oldF, const int numThreads, AttributeSet attrs,
    std::vector<std::pair<std::string, std::string>> metadataStrVector) {

    // build the vectors for extra arguments
    std::vector<std::pair<std::string, Type *>> argVector;
    // insert extra argument as "threadID" with a 32-bit integer type
    argVector.push_back(
        std::make_pair("threadID", IntegerType::get(getGlobalContext(), 32)));

    // create new function with the threadID argument
    Function *NF = createFunctionwithExtraArgument(oldF, argVector);

    if (!attrs.isEmpty())
        NF->addAttributes(AttributeSet::FunctionIndex, attrs);

    // generate OMP calling wrapper
    Function *wrapperF = generateOMPWrapper(NF, numThreads);

    replaceCallsWhileAddingMetadata(oldF, wrapperF, metadataStrVector);

    // replace the uses of the values returned from omp functions
    // with the inserted arguments, numThreads and threadID
    replaceOMPIntrinsicFunctionCalls(NF, numThreads);

    // delete the original function from the module
    Module *M = NF->getParent();
    M->getFunctionList().remove(*oldF);

    return NF;
}

// replace calls all calls to oldF
// with calls to newF while adding metadata
void ParallelAPI::replaceCallsWhileAddingMetadata(
    Function *oldF, Function *newF,
    const std::vector<std::pair<std::string, std::string>> &metadataStrVector) {

    // Loop over all of the callers of the function, transforming the call sites
    // to pass in the new arguments into the new function.
    std::vector<Value *> Args;
    Instruction *New;

    while (!oldF->use_empty()) {

        CallSite CS(oldF->user_back());
        Instruction *Call = CS.getInstruction();

        Args.assign(CS.arg_begin(), CS.arg_end());

        // replace calls
        New = replaceCallInstFromOldtoNewFunction(cast<CallInst>(Call), newF,
                                                  Args);

        // set new metadata
        for (std::vector<std::pair<std::string, std::string>>::const_iterator
                 it = metadataStrVector.begin();
             it != metadataStrVector.end(); ++it) {

            std::string kind = (*it).first;
            std::string value = (*it).second;

            setMetadataStr(New, kind, value);
        }
    }
}

// if it's a called to functions which are only for the hybrid flow
// such as legup_start_counter, legup_stop_counter
// and it's not using the hybrid flow
// delete the call instruction
bool ParallelAPI::replaceHybridOnlyFunctionCalls(CallInst *CI, bool &modified) {

    if (isaHybridOnlyFunction(CI->getCalledFunction()->getName().str()) &&
        !LEGUP_CONFIG->isHybridFlow()) {

        deleteInstruction(CI);
        modified = true;

        return true;
    }

    return false;
}

// return true is this a function only used for the hybrid flow
bool ParallelAPI::isaHybridOnlyFunction(const std::string funcName) {

    if (funcName == "legup_start_counter" 
     || funcName == "legup_stop_counter") 
        return true;
    return false;
}

bool ParallelAPI::getAndReplaceOMPCalls(Module &M, CallInst *CI,
                                        bool &modified) {

    bool replaced = false;
    std::string HwFuncName;
    std::string calledFuncName = CI->getCalledFunction()->getName();

    // if this is a call to fork OMP threads
    if (calledFuncName == "GOMP_parallel_start") {

        parallelFunctions.push_back(
            std::pair<CallInst *, wrapperType>(CI, ompcall));

        replaced = true;
    }

    // if this is call to join OMP threads
    else if (calledFuncName == "GOMP_parallel_end") {

        // we just delete it
        // in LegUp only one wrapper is generated for OMP
        // which does both forking and joining
        deleteInstruction(CI);
        replaced = modified = true;
    }

    return replaced;
}

bool ParallelAPI::getAndReplacePthreadCalls(Module &M, CallInst *CI,
                                            bool &modified) {

    bool replaced = false;
    std::string HwFuncName;
    std::string calledFuncName = CI->getCalledFunction()->getName().str();

    // if this is a call to pthread create
    if (calledFuncName == "pthread_create" && !LEGUP_CONFIG->isPCIeFlow()) {

        parallelFunctions.push_back(
            std::pair<CallInst *, wrapperType>(CI, pthreadcall));

        // check if the pthread function returns void
        // if not add function attribute

        replaced = true;
    }

    // if this is a call to pthread join
    else if (calledFuncName == "pthread_join" && !LEGUP_CONFIG->isPCIeFlow()) {

        parallelFunctions.push_back(
            std::pair<CallInst *, wrapperType>(CI, pthreadpoll));

        replaced = true;
    }

    // if this is a call to pthread_exit
    else if (calledFuncName == "pthread_exit") {
        Value *ret = CI->getArgOperand(0);

        // check the return type
        // if it returns anything, add a ret instruction
        // else return a null pointer
        if (!isa<ConstantPointerNull>(ret)) {

            // create the return instruction
            ReturnInst::Create(M.getContext(), ret, CI);

            // this will be used later to check if the returned value
            // from the pthread function needs to be preserved
            CI->getParent()->getParent()->removeFnAttr(Attribute::NoReturn);
        } else {

            // create a null pointer
            ConstantPointerNull *nullPtr = ConstantPointerNull::get(
                PointerType::get(IntegerType::get(M.getContext(), 8), 0));

            // create the return instruction with the null pointer
            // there has to be a return instruction (even for void) since it's a
            // terminator for the basic block
            ReturnInst::Create(M.getContext(), nullPtr, CI);

            // set NoReturn function attribute
            // this attribute will later be removed, but before then
            // this will be checked to see if the returned value
            // from the pthread function needs to be preserved
            CI->getParent()->getParent()->addFnAttr(Attribute::NoReturn);
        }

        // delete the original call instruction
        deleteInstruction(CI);
        replaced = modified = true;
    }

    return replaced;
}

bool ParallelAPI::replaceAtomicFunctionCalls(Module &M, CallInst *CI,
                                             bool &modified) {

    bool replaced = false;
    std::string calledFuncName = CI->getCalledFunction()->getName().str();

    // if this is a call to atomic_fetch_add (OpenMP)
    // replace with legup lock,
    // then load/increment/store
    // then legup unlock
    if (calledFuncName == "__atomic_fetch_add_4") {

        // add the load/add/store instructions
        // load from first operand, add the second operand, and store back
        IRBuilder<> builder(CI);
        Value *address = CI->getArgOperand(0);
        Value *increment = CI->getArgOperand(1);

        // create bitcast to *i32
        Value *bitcast = builder.CreateBitCast(
            address, PointerType::get(IntegerType::get(M.getContext(), 32), 0),
            "");

        // create call to GOMP_atomic_start which will be replaced later
        Type *voidTy = Type::getVoidTy(M.getContext());
        std::vector<Type *> paramType;
        Value *FCache = M.getOrInsertFunction(
            "GOMP_atomic_start", FunctionType::get(voidTy, paramType, false));
        CallInst *lock = builder.CreateCall(FCache, "");

        bool atomicRegion = false;
        replaceLockFunction(M, lock, modified, atomicRegion);

        // load from pointer, add, and store back
        LoadInst *ld = builder.CreateLoad(bitcast, "loaded_value");
        Value *add = builder.CreateAdd(ld, increment, "incremented_value");
        builder.CreateStore(add, bitcast);

        // create call to GOMP_atomic_end which will be replace later
        FCache = M.getOrInsertFunction(
            "GOMP_atomic_end", FunctionType::get(voidTy, paramType, false));
        CallInst *unlock = builder.CreateCall(FCache, "");

        replaceLockFunction(M, unlock, modified, atomicRegion);

        // delete the original call instruction
        deleteInstruction(CI);

        replaced = modified = true;
    }

    return replaced;
}

bool ParallelAPI::replaceBarrierCalls(Module &M, CallInst *CI, bool &modified) {

    bool replaced = false;
    CallInst *newCI;
    std::string calledFuncName = CI->getCalledFunction()->getName().str();

    // if this is a call to pthread_barrier_init
    // find the function pointer to this function and replace the call to legup
    // function
    if (calledFuncName == "pthread_barrier_init") {
        newCI =
            ReplaceCallWith("legup_barrier_init", CI,
                            copyArguments((CI->op_end()) - 2, CI->op_end() - 1),
                            Type::getVoidTy(M.getContext()));

        // set no inline attribute to the call instruction
        // this is needed so that we can preserve the metadata added to the call
        // instruction
        cast<CallInst>(newCI)
            ->setIsNoInline(); // adding noinline attribute to function

        replaced = modified = true;
    }

    // if this is a call to pthread_barrier_wait
    // find the function pointer to this function and replace the call to legup
    // function
    else if (calledFuncName == "pthread_barrier_wait") {
        newCI = ReplaceCallWith("legup_barrier_wait", CI,
                                copyArguments(CI->op_end(), CI->op_end()),
                                Type::getVoidTy(M.getContext()));

        // set no inline attribute to the call instruction
        // this is needed so that we can preserve the metadata added to the call
        // instruction
        cast<CallInst>(newCI)
            ->setIsNoInline(); // adding noinline attribute to function

        replaced = modified = true;
    }

    return replaced;
}

// if it's an unreachable instruction,
// check if the previous instruction is a return (changed from pthread_exit)
// it it is then just delete it, if not leave it
void ParallelAPI::replaceUnreachableInst(Instruction *Inst) {

    // check if it's an unreachable instruction
    if (llvm::UnreachableInst::classof(Inst)) {

        BasicBlock::iterator BBiter = Inst;
        Instruction *prevInst = (--BBiter);

        // if the previous instruction is a return
        if (llvm::ReturnInst::classof(prevInst)) {

            // delete the unreachable instruction
            deleteInstruction(Inst);
        }
    }
}

// this function replaces the calls to
// omp_get_num_threads() and omp_get_thread_num()
void ParallelAPI::replaceOMPIntrinsicFunctionCalls(Function *F,
                                                   int numThreads) {

    Value *threadIDArg;

    for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(); I != E;
         ++I) {
        if ((I)->getName() == "threadID") {
            threadIDArg = I;
        }
    }

    ConstantInt *numThreadsVal = ConstantInt::get(
        IntegerType::get(F->getContext(), 32), numThreads, false);

    for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE; ++BB) {
        for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;) {
            Instruction *Inst = I++;

            // If it's a call instruction
            if (CallInst *CI = dyn_cast<CallInst>(Inst)) {
                Function *calledFunc = CI->getCalledFunction();

                // for indirect function calls
                if (!calledFunc) {
                    Value *Callee = CI->getCalledValue();

                    // if it's an constant expression
                    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(Callee)) {
                        if (Function *RF =
                                dyn_cast<Function>(CE->getOperand(0))) {
                            Function *called = RF;

                            replaceOMPThreadFunctionCalls(
                                called, CI, numThreadsVal, threadIDArg);
                        }
                    }
                    continue;
                }

                replaceOMPThreadFunctionCalls(calledFunc, CI, numThreadsVal,
                                              threadIDArg);
            }
        }
    }
}

void ParallelAPI::replaceOMPThreadFunctionCalls(Function *F, CallInst *CI,
                                                Value *numThreads,
                                                Value *threadID) {

    std::string funcName = F->getName();

    if (funcName == "omp_get_num_threads") {
        CI->replaceAllUsesWith(numThreads);
        deleteInstruction(CI);
    }

    else if (funcName == "omp_get_thread_num") {
        CI->replaceAllUsesWith(threadID);
        deleteInstruction(CI);
    }
}

// this function returns the mutex variable name for pthread_mutex_lock and
// pthread_mutex_unlock
std::string ParallelAPI::getPthreadMutexVarName(CallInst *CI) {

    std::string mutexName;
    Value *mutexVar = CI->getArgOperand(0);

    // if the mutex variable is an array type
    // set the mutex name as
    // "arrayname"_"indexDim1_indexDim2_..."
    // first check if it's a getelementptr constantexpr
    if (GetElementPtrConstantExpr *gep =
            dyn_cast<llvm::GetElementPtrConstantExpr>(mutexVar)) {

        // get the name of the first operand, which is name of the array
        Value *array = gep->getOperand(0);
        mutexName = array->getName().str();

        // get the dimensions of the array
        int arrayDim = 0;
        PointerType *PTy = dyn_cast<PointerType>(array->getType());
        assert(PTy);
        Type *ETy = PTy->getElementType();

        if (ArrayType *ATy = dyn_cast<ArrayType>(ETy)) {

            arrayDim++;
            while (ArrayType *ATy2 =
                       dyn_cast<ArrayType>(ATy->getElementType())) {

                arrayDim++;
                ATy = ATy2;
            }
        }

        // iterate through each dimension of the array and get the indices
        // append the indices to the mutex name
        for (int i = 0; i < arrayDim; i++) {
            //add the dimension index
            //the first dimension index starts at getOperand(0)
            if (ConstantInt *index = dyn_cast<ConstantInt>(gep->getOperand(i+2))) {
                mutexName += "_" + utostr(index->getZExtValue());
            } else {
                assert(0 && "Array index is not a constant integer!\n");
            }
        }

    } else {

        mutexName = mutexVar->getName().str();
    }

    return mutexName;
}

void ParallelAPI::findInternalAccels(Module &M) {

    std::string FuncName;

    for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
        for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE;
             ++BB) {
            for (BasicBlock::iterator I = BB->begin(), EE = BB->end(); I != EE;
                 ++I) {
                if (CallInst *CI = dyn_cast<CallInst>(I)) {
                    Function *calledFunction = CI->getCalledFunction();
			        // ignore indirect function calls
			        if (!calledFunction) continue;

                    std::string type = getMetadataStr(CI, "TYPE");
                    std::string Name = calledFunction->getName().str();

					// Don't consider OpenMP or Pthread functions for PCIe flow
                    if (LEGUP_CONFIG->isPCIeFlow()) {
                        // if user-designated function
                        if (LEGUP_CONFIG->isAccelerated(Name)) {
                            setInternalAccels(M, calledFunction);
                        }
                        continue;
                    }

                    // if Pthread function
                    if (type == "legup_wrapper_pthreadcall") {
                        // get the name of the function being forked to
                        setInternalAccels(M, calledFunction);
                    }
                }
            }
        }
    }
}

void ParallelAPI::setInternalAccels(Module &M, Function *F) {

    std::string FuncName;

    for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
        for (BasicBlock::iterator I = BB->begin(), EE = BB->end(); I != EE;
             ++I) {
            if (CallInst *CI = dyn_cast<CallInst>(I)) {
 
                Function *calledFunction = CI->getCalledFunction();
                // ignore indirect function calls
                if (!calledFunction) continue;

			    std::string Type = getMetadataStr(CI, "TYPE");
                // get the name of the function being forked to
                FuncName = calledFunction->getName().str();

                if (Type == "omp_function") {

                    // add metadata
                    setMetadataInt(CI, "isInternalAccel", 1);
                    calledFunction->addFnAttr("isInternalAccel", utostr(1));
                }

                setInternalAccels(M, calledFunction);
            }
        }
    }
}

// this function finds all the caller functions of the current function
// and adds the functions pointers to a set
void ParallelAPI::getAllCallerFunctionsUntilArgFound(
    Function *F, std::string Arg, std::set<Function *> &callerSet) {

    Function *CallerF;

    for (User *UI : F->users()) {

        CallSite CS(UI);
        Instruction *Call = CS.getInstruction();

        CallerF = Call->getParent()->getParent();
        assert(CallerF);

        for (Function::const_arg_iterator I = CallerF->arg_begin(),
                                          E = CallerF->arg_end();
             I != E; ++I) {
            if (I->getName() == Arg)
                return;
        }

        // insert into set
        callerSet.insert(CallerF);
        // recurse
        getAllCallerFunctionsUntilArgFound(CallerF, Arg, callerSet);
    }
}

} // end of legup namespace
