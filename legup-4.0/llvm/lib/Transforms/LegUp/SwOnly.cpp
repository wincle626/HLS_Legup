//===- SwOnly.cpp - LegUp pre-LTO pass ------------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// SwOnly - Replace accelerated functions with wrappers and create
// legup_wrappers.c. Create tcl files (legup_sopc.tcl, _hw.tcl) to
// control SOPC builder to add the accelerator to the system.
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Signals.h"
#include "LegupConfig.h"
#include "SwOnly.h"
#include "utils.h"
#include <math.h>
#include <set>
#include <sstream>
#include "llvm/IR/LLVMContext.h"

#define DEBUG_TYPE "LegUp:SwOnly"

using namespace llvm;

namespace legup {

char SwOnly::ID = 0;
static RegisterPass<SwOnly>
    Z("legup-sw-only",
      "Replace accelerated functions with wrappers and produce legup_wrappers");

void SwOnly::initializePCIeFunctions(Module &M) {
    // assert(PCIeFunctions.size() == 0);
    if (PCIeFunctions.size() != 0) {
        return;
    }

    // pcie_write and pcie_read
    {
        std::vector<Type *> argTypes;
        argTypes.push_back(Type::getInt32PtrTy(getGlobalContext()));
        argTypes.push_back(Type::getInt32Ty(getGlobalContext()));
        argTypes.push_back(Type::getInt32Ty(getGlobalContext()));

        FunctionType *funcType = FunctionType::get(
            Type::getInt32Ty(getGlobalContext()), argTypes, false);

        Function *func =
            cast<Function>(M.getOrInsertFunction("pcie_write", funcType));
        assert(func != NULL);
        PCIeFunctions["pcie_write"] = func;

        func = cast<Function>(M.getOrInsertFunction("pcie_read", funcType));
        PCIeFunctions["pcie_read"] = func;
    }

    // getAccel
    {
        std::vector<Type *> argTypes;
        argTypes.push_back(Type::getInt32PtrTy(getGlobalContext()));

        FunctionType *funcType = FunctionType::get(
            Type::getInt32Ty(getGlobalContext()), argTypes, false);

        Function *func =
            cast<Function>(M.getOrInsertFunction("getAccel", funcType));
        assert(func != NULL);
        PCIeFunctions["getAccel"] = func;
    }

    // freeAccel
    {
        std::vector<Type *> argTypes;
        argTypes.push_back(Type::getInt32PtrTy(getGlobalContext()));
        argTypes.push_back(Type::getInt32Ty(getGlobalContext()));

        FunctionType *funcType = FunctionType::get(
            Type::getInt32Ty(getGlobalContext()), argTypes, false);

        Function *func =
            cast<Function>(M.getOrInsertFunction("freeAccel", funcType));
        assert(func != NULL);
        PCIeFunctions["freeAccel"] = func;
    }

    // pthread_yield
    {
        std::vector<Type *> argTypes;

        FunctionType *funcType = FunctionType::get(
            Type::getInt32Ty(getGlobalContext()), argTypes, false);

        Function *func =
            cast<Function>(M.getOrInsertFunction("pthread_yield", funcType));
        assert(func != NULL);
        PCIeFunctions["pthread_yield"] = func;
    }

    fprintf(stderr, "PCIeFunction Initialized.\n");
}

bool SwOnly::doInitialization(Module &M) {
    if (LEGUP_CONFIG->isPCIeFlow()) {
        initializePCIeFunctions(M);
    }
    return false;
}

bool SwOnly::doFinalization(Module &M) { return false; }

bool SwOnly::runOnModule(Module &M) {

    bool modified = false;
    modified |= doInitialization(M);

    // if accelerated function is main, assert
    if (LEGUP_CONFIG->isAccelerated("main")) {
        errs() << "The main function cannot be accelerated in the hybrid flow\n"
               << "To compile main to hardware, please use the pure hardware "
                  "flow\n";
        assert(0);
    }

    // get all accelerators which are internal to another accelerator
    // internal accelerators are created when an accelerated function contains
    // pthreads or openmp
    // Only the uppermost function (whether it's a user-designated function,
    // pthread function, or openmp function) are created as Avalon accelerators.
    getInternalAccels(M, internalAccels);

    // if (!LEGUP_CONFIG->isPCIeFlow()) {
    modified |= getAndReplaceCalls(M);
    // } else {
    //     modified |= replaceCallsPCIe(M);
    // }

    // add all global variables used in HW functions
    // to llvm.used intrinic global variable, to prevent LTO
    // from optimizing it away
    preserveGlobalVariablesUsedInHW(M);

    generateParallelAccelConfigs();
    setCacheParameters();

    // if (!LEGUP_CONFIG->isPCIeFlow()) {
    modified |= generateWrappersAndPrintSideFiles();
    // } else {
    //     modified |= printSWfilesPCIe();
    // }

    modified |= doFinalization(M);

    return modified;
}

bool SwOnly::getAndReplaceCalls(Module &M) {

    bool modified = false;

    // replace all calls to accelerated functions with calls to the wrapper
    for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
        for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE;
             ++BB) {
            for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;) {
                if (CallInst *CI = dyn_cast<CallInst>(I++)) {

                    Function *calledFunction = CI->getCalledFunction();
                    // ignore indirect function calls
                    if (!calledFunction)
                        continue;

                    // add performance counters if there are any, continue if
                    // replaced
                    if (addPerformanceCounters(CI, calledFunction))
                        continue;

                    // for parallel functions
                    if (isaCalltoParallelFunction(CI)) {

                        if (getAndReplaceParallelFunctionCalls(M, CI, I,
                                                               modified))
                            continue;

                        // if it is a sequential function (a function cannot be
                        // both parallel and sequential)
                    } else {

                        // replace synchronization functions (locks, barriers)
                        if (getSynchronizationFunctions(F, CI, calledFunction))
                            continue;

                        // else replace sequential functions if there are any
                        modified |=
                            replaceSequentialFunction(CI, calledFunction);
                    }
                }
            }
        }
    }

    return modified;
}

// this function replaces gets the calls to Pthread/OpenMP functions, and store
// them in AcceleratedFcts
// For Pthreads, it also needs to replace it with a new call, since additional
// arguments need to be added
// and also some following instructions need to be deleted
bool SwOnly::getAndReplaceParallelFunctionCalls(Module &M, CallInst *CI,
                                                BasicBlock::iterator &I,
                                                bool &modified) {

    bool replaced = false;

    // get OpenMP functions if there are any, continue if replaced
    if (getOMPFunctions(M, CI)) {
        replaced = true;
    }

    // get Pthread functions if there are any, continue if replaced
    if (replacePthreadFunctionsCalls(M, CI)) {

        // delete the call to legup_preserve_value
        // which is used the for pure HW flow to
        // preserve the return value from the pthread function
        if (CallInst *dummyCall = dyn_cast<CallInst>(I)) {
            if (dummyCall->getCalledFunction()->getName() ==
                "__legup_preserve_value") {
                Instruction *pthreadRetVal = I++;
                deleteInstruction(pthreadRetVal);
            }
        }
        modified = true;
        replaced = true;
    }

    return replaced;
}

// this function find all the global variables which are stored
// in the llvm.used variable
void SwOnly::findUsedValues(GlobalVariable *LLVMUsed,
                            SmallPtrSet<GlobalValue *, 8> &UsedValues) {
    if (!LLVMUsed)
        return;

    ConstantArray *Inits = cast<ConstantArray>(LLVMUsed->getInitializer());
    for (unsigned i = 0, e = Inits->getNumOperands(); i != e; ++i)
        if (GlobalValue *GV = dyn_cast<GlobalValue>(
                Inits->getOperand(i)->stripPointerCasts()))
            UsedValues.insert(GV);
}

// return the accelFcts struct, given the function, the type
// and the number of times this function is accelerated
accelFcts SwOnly::getAccelFcts(Function *F, wrapperType functionType,
                               int numAccelerated) {

    accelFcts accel;

    accel.fct = F;
    accel.type = functionType;
    accel.numAccelerated = numAccelerated;

    return accel;
}

// given a function, get the accelFcts struct and add to AcceleratedFcts
// if its a top-level accelerator
void SwOnly::addAcceleratedFcts(Function *calledFunction,
                                wrapperType functionType,
                                unsigned numAccelerated) {

    // if top-level accelerator
    if (internalAccels.find(calledFunction) == internalAccels.end()) {

        accelFcts accel;
        // if it's not a parallel function
        // add to AcceleratedFcts
        if (!numAccelerated) {
            accel = getAccelFcts(calledFunction, functionType);
            addAcceleratedFct(accel);
        }
        // if it is, it may already exist in AcceleratedFcts
        // hence updated the number of acclerators
        else {
            accel = getAccelFcts(calledFunction, functionType, numAccelerated);
            updateAcceleratedFct(accel);
        }
    }
}

// this function adds HW functions to AcceleratedFcts if it doesn't exist
// already
void SwOnly::addAcceleratedFct(accelFcts accel) {

    // add to AcceleratedFcts only if it doesn't exist already
    if ((find(AcceleratedFcts.begin(), AcceleratedFcts.end(), accel) ==
         AcceleratedFcts.end())) {
        AcceleratedFcts.push_back(accel);
    }
}

// this function adds HW functions to AcceleratedFcts it it doesn't exist
// already
// if it does exist, it updates the number of accelerators (parallel case)
void SwOnly::updateAcceleratedFct(accelFcts accel) {

    // add to AcceleratedFcts only if it doesn't exist already
    std::vector<accelFcts>::iterator found =
        find(AcceleratedFcts.begin(), AcceleratedFcts.end(), accel);
    if ((found == AcceleratedFcts.end())) {
        AcceleratedFcts.push_back(accel);
    } else {
        found->numAccelerated += accel.numAccelerated;
    }
}

bool SwOnly::generateWrappersAndPrintSideFiles() {

    bool modified = false;

    unsigned long long StartAddr_Wrapper;
    if (ProcessorArchitecture == "ARMA9") {
        StartAddr_Wrapper = 0xC8000000; // The actual memory address assigned to
                                        // accelerator, will
        // be the one printed out in C wrapper file
    } else {
        StartAddr_Wrapper = 0xf0000000; // The actual memory address assigned to
                                        // accelerator, will
        // be the one printed out in C wrapper file
    }

    if (LEGUP_CONFIG->isPCIeFlow()) {
        StartAddr_Wrapper = 0L;
    }

    unsigned long long CurrAddr_Wrapper = StartAddr_Wrapper;
    std::string AccelName;
    int AccelCount = 0, AddressSize = 0, numAccelerated, addrBusWidth;
    bool pthreadReturn = false;

    // make file stream for Modelsim waveforms
    formatted_raw_ostream *wave_file;
    raw_ostream &wave = initFileStream(wave_file, "wave.do");

    // make file stream for SOPC commands
    formatted_raw_ostream *sopc_file;
    raw_ostream &sopc = initFileStream(sopc_file, "legup_sopc.tcl");

    // make file stream for QSYS commands
    formatted_raw_ostream *qsys_file;
    raw_ostream &qsys = initFileStream(
        qsys_file, LEGUP_CONFIG->getLegupOutputPath() + "/legup_qsys.tcl");

    // initializing modelsim wave file with common processor signals
    initWaveFile(wave);

    // Generate wrappers for all accelerated functions
    // note: we don't need a wrapper for functions _called_ by accelerated
    // functions
    if (!AcceleratedFcts.empty()) {

        wrapperType type;
        Function *F;
        bool pthreadPollUsed = false;

        // BF: Commenting out for now
        // Want to generate both Qsys and SOPC Builder
        // if(LEGUP_CONFIG->getParameterInt("USE_QSYS")) {
        // qsys << "package require -exact qsys 13.0\n";

        if (!LEGUP_CONFIG->isPCIeFlow()) {
            printQSYSFileInitial(qsys);
            //    qsys << "load_system legup_system.qsys\n\n";
        } else {
            qsys << "package require -exact qsys 13.0\n";
            qsys << "load_system legup_riffa/legup_riffa.qsys\n\n";
        }
        // } else {
        printSopcFileInitial(sopc);
        // }

        printCacheParametersFile();
        printCacheHWtcl();

        for (std::vector<accelFcts>::iterator I = AcceleratedFcts.begin(),
                                              E = AcceleratedFcts.end();
             I != E; ++I) {

            type = (*I).type;
            F = (*I).fct;
            // for parallel accelerators, generated wrapper and sopc function
            // for each instance
            numAccelerated = (*I).numAccelerated;
            printf("numAccelerated = %d\n", numAccelerated);

            // caculate the amount of memory mapped address occupied by this
            // accelerator
            CurrAddr_Wrapper = calculateMemorySpace(F, StartAddr_Wrapper, type);

            AddressSize = calculateAddressSize(CurrAddr_Wrapper,
                                               StartAddr_Wrapper, addrBusWidth);

            // generate wrapper in IR instead of printing in C
            generateWrapper(F, type, StartAddr_Wrapper, AddressSize,
                            numAccelerated);
            modified = true;

            // add the HW accelerator to sopc system as many times as the
            // number of threads
            for (int i = 0; i < numAccelerated; i++) {

                // Memory address used in System Builder tool. Pipeline bridge's
                // base address set to f0000000, hence the accelerator only
                // needs the offset from Pipeline bridges base address, so we
                // mask out the top bits
                unsigned long long StartAddr_SystemBuilder;
                if (ProcessorArchitecture == "ARMA9") {
                    StartAddr_SystemBuilder = StartAddr_Wrapper & 0x07FFFFFF;
                } else {
                    StartAddr_SystemBuilder = StartAddr_Wrapper & 0x0FFFFFFF;
                }

                // BF: Commenting out for now
                // Want to generate both Qsys and SOPC Builder
                // if(LEGUP_CONFIG->getParameterInt("USE_QSYS")) {
                // print tcl which controls QSYS to add accelerator to system
                if (!LEGUP_CONFIG->isPCIeFlow()) {
                    printQSYSFile(qsys, F, type, StartAddr_SystemBuilder, i,
                                  AccelCount, addrBusWidth);
                } else {
                    printQSYSFilePCIeShared(qsys, F, StartAddr_SystemBuilder, i,
                                            AddressSize);
                }
                // } else {
                // print tcl which controls SOPC builder to add accelerator to
                // system
                printSopcFile(sopc, F, type, StartAddr_SystemBuilder, i,
                              AccelCount);
                // }

                printHWtcl(F, type, addrBusWidth);
                StartAddr_Wrapper += AddressSize;

                // Adding accelerator top signals to Modelsim wave file
                addAcceltoWaveFile(wave, F, i);
                AccelCount++;
            }

            // if this is a pthread call
            if (type == pthreadcall) {
                // pthread poll wrapper needs to be printed out
                pthreadPollUsed = true;
                // check if this pthread function returns anything
                if (!F->getReturnType()->isVoidTy())
                    pthreadReturn = true;
            }
        }

        // pthread polling wrapper needs to be printed out very last
        // since there is only one wrapper for all pthread functions
        // and you need to know how many types of pthread functions have more
        // than 1 thread
        // to print the case statement inside the wrapper
        if (pthreadPollUsed) {
            generatePthreadPollingWrapper(*F->getParent(), pthreadReturn);
            modified = true;
        }

        // BF: Commenting out for now
        // Want to generate both Qsys and SOPC Builder
        //		if(LEGUP_CONFIG->getParameterInt("USE_QSYS")) {
        printQSYSFileEnd(qsys);
        // } else {
        printSopcFileEnd(sopc);
        // }
        //

    } else {
        errs() << "\nERROR: There are no functions to be accelerated.\n"
               << "Make sure you specified a function in the config.tcl file "
               << "with set_accelerator_function \"function_name\".\n"
               << "If you already have specified a function, "
               << "this usually means that the specified function "
               << "is never called, or optimized away by the "
                  "compiler.\n"
               << "Please check the LLVM IR to confirm.\n\n";
        assert(0);
    }

    finishWaveFile(wave);

    // flush out all files
    delete wave_file;
    // BF: Commenting out for now
    // Want to generate both Qsys and SOPC Builder
    //	if(LEGUP_CONFIG->getParameterInt("USE_QSYS")) {
    delete qsys_file;
    //	} else {
    delete sopc_file;
    //	}
    return modified;
}

int SwOnly::calculateAddressSize(unsigned long long currAddr,
                                 unsigned long long prevAddr,
                                 int &addrBusWidth) {

    // calculate offset between current and prev wrapper
    int addrOffset = currAddr - prevAddr;
    // calculate avalon slave bus width for HW accelrator
    addrBusWidth = ceil(log(addrOffset / 4) / log(2));
    // calculate the address size mapped to current HW accelerator
    int addrSize = 4 * pow(2, addrBusWidth);

    return addrSize;
}

// returns true if replaced
bool SwOnly::addPerformanceCounters(CallInst *CI, Function *calledFunction) {

    bool replaced = false;
    if (calledFunction->getName().str() == "legup_start_counter") {
        replaced = true;
        perfCounterUsed = true;
        Value *argValue = CI->getArgOperand(0);
        if (ConstantInt *constantInt = dyn_cast<ConstantInt>(argValue)) {
            int index = constantInt->getSExtValue();
            if (index > numPerfCounter)
                numPerfCounter = index;
        } else {
            assert(0 && "counter offset has to be a constant integer!\n");
        }
    }

    return replaced;
}

// returns true if replaced
bool SwOnly::replacePthreadFunctionsCalls(Module &M, CallInst *CI) {

    bool found = false;

    // pthread_create
    if (getMetadataStr(CI, "TYPE") == "legup_wrapper_pthreadcall") {

        Function *F = CI->getCalledFunction();
        // if it's not an internal accel, add to AcceleratedFcts
        if (!internalAccels.count(F)) {

            // create a new call to the pthread function
            CallInst *newCI = createNewCalltoPthreadFunction(M, F, CI);

            // if the next instruction is to store legup_thread_functionID
            // this is only for the pure HW flow, so delete it
            if (getMetadataInt(newCI->getPrevNode(),
                               "legup_pthread_functionthreadID")) {
                // Instruction *storeThreadID = I++;
                deleteInstruction(newCI->getPrevNode());
            }

            // add pthread function to AcceleratedFcts
            getPthreadFunction(CI);

            // delete the original call instruction
            deleteInstruction(CI);

            parallelAccelUsed = true;
            found = true;
        }
    }

    return found;
}

// create a new call to pthread function, with additional arguments
// inserted for LegUp
CallInst *SwOnly::createNewCalltoPthreadFunction(Module &M, Function *F,
                                                 CallInst *CI) {

    // here we need to make a call to the pthread create wrapper
    // which will have two an additional argument than the pthread
    // function
    // 1. the pointer to the thread variable
    // 2. the threadID
    // the thread ID value will be used to calculate the memory-mapped
    // address
    // which will be saved in the pointer to the thread variable
    //
    // the actual body of the wrapper function will be generated later

    // create the wrapper function definition (which will call the Pthread
    // accelerator)
    std::string wrapperName = getWrapperName(F, pthreadcall);
    std::vector<Type *> Params;
    if (F->arg_size()) {
        Params.push_back(F->arg_begin()->getType());
    }
    // thread ptr type
    Params.push_back(
        PointerType::get(IntegerType::get(getGlobalContext(), 32), 0));
    // threadID value type
    Params.push_back(IntegerType::get(getGlobalContext(), 32));

    // pthread_create wrappers are of void type
    Constant *c = M.getOrInsertFunction(
        wrapperName,
        FunctionType::get(Type::getVoidTy(M.getContext()), Params, false));
    Function *wrapperF = cast<Function>(c);
    wrapperF->setCallingConv(CallingConv::C);

    Function::arg_iterator argI = --wrapperF->arg_end();
    // set the name last argument of the wrapper function
    argI->setName("threadIDValue");
    argI->getPrevNode()->setName("threadID");

    // get the arguments to pass into the pthread function
    std::vector<Value *> Args;
    // add the original argument if there is any
    if (F->arg_size()) {
        Args.push_back(CI->getArgOperand(0));
        wrapperF->arg_begin()->setName(F->arg_begin()->getName());
    }

    // the next is the thread pointer
    // followed by the threadID value
    Instruction *threadID = CI->getPrevNode();
    Value *threadPtrArg, *threadIDValueArg;
    if (StoreInst *st = dyn_cast<StoreInst>(threadID)) {
        if (getMetadataInt(threadID, "legup_pthread_functionthreadID")) {
            // get the value operand of the store instruction
            threadPtrArg = st->getPointerOperand();
            threadIDValueArg = st->getValueOperand();
        } else {
            assert(0 && "The Pthread function threadID cannot be found\n");
        }
    } else {
        assert(0 && "The store instruction after the call to pthread function "
                    "cannot be found\n");
    }
    assert(threadPtrArg);
    assert(threadIDValueArg);

    Args.push_back(threadPtrArg);
    // need the bottom 16 bits, which is the actual threadID value
    // top 16 bits may be the functionID if there are multiple
    // pthread functions

    // AND with 0x0000FFFF
    // to truncate off the top 16 bits
    BinaryOperator *tr = BinaryOperator::Create(
        Instruction::And, threadIDValueArg,
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 0x0000FFFF,
                         false),
        "", CI);
    Args.push_back(tr);

    // create call the pthread wrapper
    CallInst *newCI = CallInst::Create(wrapperF, Args, "", CI);

    return newCI;
}

// returns true if replaced
bool SwOnly::getSynchronizationFunctions(Function *F, CallInst *CI,
                                         Function *calledFunction) {

    bool replaced = false;
    // if (calledFunction->getName().str() == "legup_lock") {
    if (calledFunction->getName().str() == "legup_lock") {

        std::string mutexName;

        // get the name of mutex depending on what type of mutex it is
        if (getMetadataStr(CI, "mutexType") == "pthread_mutex_lock") {
            mutexName = getMetadataStr(CI, "mutexName");
        } else if (getMetadataStr(CI, "mutexType") == "omp_critical_start") {
            // mutexName = "OMP";
            mutexName = getMetadataStr(CI, "mutexName");
        } else if (getMetadataStr(CI, "mutexType") == "omp_atomic_start") {
            // mutexName = "OMP_atomic_" + utostr(numOMPatomic);
            mutexName = getMetadataStr(CI, "mutexName");
            numOMPatomic++;
        }
        // insert the mutex into map if it hasn't been already
        int mutexNum = mutexMap.size();
        if (mutexMap.find(mutexName) == mutexMap.end()) {
            mutexMap.insert(std::pair<std::string, int>(mutexName, mutexNum));
            std::set<Function *> callerSet;
            callerSet.insert(F);
            // we want to find all the functions that call the current function
            findCallerFunctions(F, callerSet);
            // this map keeps track of what are the caller functions to the
            // current function which uses the lock
            // this is to be used later to determine which accelerator should
            // connect to which mutex
            // this is needed because the function which uses mutex_lock may not
            // be the top most function for that accelerator
            mutexFunctionMap[mutexNum] = callerSet;
        }
        lockUsed = true;
        replaced = true;
    }
    // if barriers are used in the program
    else if (calledFunction->getName().str() == "legup_barrier_init") {
        barrierUsed = true;
        replaced = true;
    }

    return replaced;
}

// replace OpenMP functions, returns true if replaced
bool SwOnly::getOMPFunctions(Module &M, CallInst *CI) {

    bool found = false;
    std::string type = getMetadataStr(CI, "TYPE");

    // if this is a call to fork OMP threads
    if (type == "legup_wrapper_omp") {

        // if it's not an internal Accel
        if (!internalAccels.count(CI->getCalledFunction())) {

            // get call to OMP function and add to AcceleratedFcts
            getOMPParallel(M, CI);
            parallelAccelUsed = true;
        }
        found = true;
    }

    return found;
}

void SwOnly::getOMPParallel(Module &M, CallInst *CI) {

    // get the name of the actual OpenMP function
    std::string ompFuncName = getMetadataStr(CI, "OMPNAME");
    // find the function pointer to the OpenMP function
    Function *ompFuncPtr = findFuncPtr(M, ompFuncName.c_str());
    assert(ompFuncPtr);

    // get the number of threads
    int numThreads;
    ompFuncPtr->getFnAttribute("totalNumThreads")
        .getValueAsString()
        .getAsInteger(0, numThreads);
    assert(numThreads);
    //    int numThreads = getMetadataInt(CI, "NUMTHREADS");

    addAcceleratedFcts(ompFuncPtr, getWrapperType(CI), numThreads);
}

bool SwOnly::replaceSequentialFunction(CallInst *CI, Function *calledFunction) {

    bool modified = false;

    wrapperType wType = LEGUP_CONFIG->isPCIeFlow() ? pcie : seq;
    // if it's a user-designated function
    if (LEGUP_CONFIG->isAccelerated(*calledFunction)) {
        // add to acceleratedFcts
        int numAccelerated =
            LEGUP_CONFIG->getNumOfInstances(calledFunction->getName().str());
        addAcceleratedFcts(calledFunction, wType, numAccelerated);
        modified |= replaceHwCallWithWrapper(CI, calledFunction, wType);
    }
    return modified;
}

/// generateWrapper - generate LLVM IR of the legup wrapper for the accelerated
/// function.
// wrapper was already generate in ParallelAPI pass for the pure HW flow
// for the hybrid flow, you need to strip out the function body of the wrapper
// and replace it with appropriate LLVM IR
void SwOnly::generateWrapper(Function *F, const wrapperType type, int StartAddr,
                             const int AddressSize, const int numAccelerated) {

    // generate LLVM IR for the hybrid flow wrapper
    switch (type) {
        case pthreadcall:
            generatePthreadCallingWrapper(F,
                    type, StartAddr, AddressSize, numAccelerated);
            break;
        case ompcall:
            generateOMPWrapper(F, type, StartAddr, AddressSize, numAccelerated);
            break;
        case seq:
            generateSequentialWrapper(F, type, StartAddr);
            break;
        case pcie:
            generatePCIeWrapper(F, type, StartAddr, numAccelerated);
            break;
        default:
            return;
    }
    return;
}

Function* SwOnly::generatePCIeWrapperPrototype(Function *F,
        const wrapperType type) {

    // get the wrapper name and create the wrapper function
    std::string wrapperName = getWrapperName(F, type);

    Constant *c =
        F->getParent()->getOrInsertFunction(wrapperName, F->getFunctionType());
    Function *wrapperF = cast<Function>(c);
    wrapperF->setCallingConv(CallingConv::C);

    wrapperF->addFnAttr(
        Attribute::NoInline); // adding noinline attribute to function

    // copy the argument names from the acclerated function to the wrapper
    // function
    for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
                                I2 = wrapperF->arg_begin();
         I != E; ++I, ++I2) {
        I2->setName(I->getName());
    }

    return wrapperF;
}

void SwOnly::setUpPCIeAcceleratorArguments(IRBuilder<> &builder, Function * wrapperF,
        Value* paramsAddr)
{
    int bitWidth;
    // int size = 0;

    // Compute the stack size needed to store all arguments
    // for (Function::arg_iterator I = wrapperF->arg_begin();
    //      I != wrapperF->arg_end(); ++I) {

    //     // check the type of the argument
    //     int bitWidth = I->getType()->getScalarSizeInBits();

    //     // bitWidth for wrappers is either 32 or 64 bits
    //     if (bitWidth > 32)
    //         size += 8;
    //     else
    //         size += 4;
    // }

    // Create the stack space
    // Value * vSize = ConstantInt::get(
    //         IntegerType::get(getGlobalContext(), 32), size);
    // AllocaInst * buf = builder.CreateAlloca(
    //         Type::getInt32Ty(getGlobalContext()), vSize);
    // buf->setAlignment(sizeof(int));

    // int nextAddrOnStack = 0;
    for (Function::arg_iterator I = wrapperF->arg_begin();
         I != wrapperF->arg_end(); ++I) {

        // check the type of the argument
        bitWidth = I->getType()->getScalarSizeInBits();

        // bitWidth for wrappers is either 32 or 64 bits
        if (bitWidth > 32)
            bitWidth = 64;
        else
            bitWidth = 32;

        AllocaInst * buf = builder.CreateAlloca(I->getType());
        buf->setAlignment(sizeof(int));

        // assert((nextAddrOnStack + bitWidth / 4) <= size);

        // Value * GEP = generateGEPfromIntAddressandValueOffset(builder,
        //             nextAddrOnStack, buf, bitWidth);

        builder.CreateStore(I, buf, true);

        // increment address for next pointer
        // nextAddrOnStack += bitWidth / sizeof(int);

        generatePCIeWriteCall(builder, buf, bitWidth / 8, paramsAddr);

        // Increment addr for next parameter
        if (I != wrapperF->arg_end())
            paramsAddr = builder.CreateAdd(paramsAddr,
                ConstantInt::get(IntegerType::get(getGlobalContext(), 32),
                    bitWidth / 8));
    }

    // call pcie_write to transfer to the accelerator
    // generatePCIeWriteCall(builder, buf, size, StartAddr);

}

// Generate polling loop until done signal for pcie
void SwOnly::sendStartSignalAndWaitForResult(IRBuilder<> &builder, Function * wrapperF,
        Value* statusAddr)
{
    // send start signal
    AllocaInst * statusReg = builder.CreateAlloca(
            IntegerType::get(getGlobalContext(), 32));
    statusReg->setAlignment(4);
    Value *one = ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 1);
    builder.CreateStore(one, statusReg, true);
    generatePCIeWriteCall(builder, statusReg, 4, statusAddr);

    // poll the status register until "done" is received from the accelerator
    BasicBlock *loop =
        BasicBlock::Create(getGlobalContext(), "poll_loop", wrapperF);
    BasicBlock *done =
        BasicBlock::Create(getGlobalContext(), "done", wrapperF);

    // poll loop body
    builder.CreateBr(loop);
    builder.SetInsertPoint(loop);

    // call pthread_yield for each iteration
    generatePthreadYieldCall(builder);

    // generate pcie_read call to poll the status register in the loop body
    generatePCIeReadCall(builder, statusReg, 4, statusAddr);
    LoadInst *load = builder.CreateLoad(statusReg, true);
    Value *cmp = builder.CreateICmpNE(load, 
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 0, false));
    // if status register is none zero, we are done, else, keep looping.
    builder.CreateCondBr(cmp, done, loop);

    // done
    builder.SetInsertPoint(done);

}

GlobalVariable* SwOnly::createSchedulerStorage(Module &M, int totalAccels, std::string name)
{
    std::string schedulerName = "scheduler";
    std::vector<llvm::Constant *> elements;
    Constant *zero = ConstantInt::get(
            IntegerType::get(getGlobalContext(), 32), 0);

    elements.push_back(zero);
    elements.push_back(zero);
    elements.push_back(ConstantInt::get(
            IntegerType::get(getGlobalContext(), 32), totalAccels));
    elements.push_back(zero);
    for (int i = 0; i < 22; i++)
        elements.push_back(zero);

    ArrayType* ArrayTy =
        ArrayType::get(IntegerType::get(M.getContext(), 32), 26);
    GlobalVariable* schedStorage= new GlobalVariable(/*Module=*/M,
            /*Type=*/ArrayTy,
            /*isConstant=*/false,
            /*Linkage=*/GlobalValue::ExternalLinkage,
            /*Initializer=*/0,
            /*Name=*/name);

    schedStorage->setInitializer(ConstantArray::get(ArrayTy, elements));
    return schedStorage;
}

// Generate the getAccel call in pcie wrapper
Value * SwOnly::getAccel(IRBuilder<> &builder, Value * sched)
{
    assert(PCIeFunctions.find("getAccel") != PCIeFunctions.end());
    Function * func = PCIeFunctions["getAccel"];

    return builder.CreateCall(func, builder.CreatePointerCast(
            sched, Type::getInt32PtrTy(getGlobalContext())), "handle");
}

// Generate the freeAccel call in pcie wrapper
void SwOnly::freeAccel(IRBuilder<> &builder, Value * sched, Value * handle)
{
    assert(PCIeFunctions.find("freeAccel") != PCIeFunctions.end());
    Function * func = PCIeFunctions["freeAccel"];

    builder.CreateCall2(func, builder.CreatePointerCast(
            sched, Type::getInt32PtrTy(getGlobalContext())), handle);
}

// Generate wrapper for pcie accelerated functions
void SwOnly::generatePCIeWrapper(Function *F, const wrapperType type,
                                       int StartAddr, int numAccelerated) {

    std::string wrapperName = getWrapperName(F, type);
    Function* wrapperF = generatePCIeWrapperPrototype(F, type);

    // create a basic block
    BasicBlock *block = BasicBlock::Create(getGlobalContext(), "", wrapperF);
    IRBuilder<> builder(block);

    // Create a global storage for scheduler
    GlobalVariable* schedStorage =
        createSchedulerStorage(*F->getParent(),
                numAccelerated, wrapperName + "_scheduler");

    // first acquire a available accelerator
    Value * handle = getAccel(builder, schedStorage);
    Value * accelAddr = builder.CreateMul(handle, ConstantInt::get(
            IntegerType::get(getGlobalContext(), 32), 32), "accelAddr");
    Value * statusAddr = builder.CreateAdd(accelAddr, ConstantInt::get(
            IntegerType::get(getGlobalContext(), 32), 8), "statusAddr");
    Value * paramsAddr = builder.CreateAdd(accelAddr, ConstantInt::get(
            IntegerType::get(getGlobalContext(), 32), 12), "paramsAddr");
    Value * returnAddr = accelAddr;

    // first send over all arguments

    // pass arguments to accelerator
    setUpPCIeAcceleratorArguments(builder, wrapperF, paramsAddr);

    // send start signal and wait until done is received
    sendStartSignalAndWaitForResult(builder, wrapperF, statusAddr);

    // once "done is received, we need to
    // load return value from accelerator

    // check the return type of the function
    Type *rT = F->getReturnType();
    int size = rT->getScalarSizeInBits() > 32 ? sizeof(long long) : sizeof(int);

    // create a return instruction depending on the function return type
    if (rT->getScalarSizeInBits() == 0) {
        // free the accelerator
        freeAccel(builder, schedStorage, handle);
        // create a return void instruction
        builder.CreateRetVoid();
    } else {

        // load result from accelerator
        AllocaInst *returnReg = builder.CreateAlloca(rT);
        returnReg->setAlignment(4);
        generatePCIeReadCall(builder, returnReg, size, returnAddr);
        Value *loadVal = builder.CreateLoad(returnReg, true);

        // free the accelerator
        freeAccel(builder, schedStorage, handle);
        builder.CreateRet(loadVal);
    }
}

// Generate wrapper for sequential functions
void SwOnly::generateSequentialWrapper(Function *F, const wrapperType type,
                                       int StartAddr) {

    // get the wrapper name and create the wrapper function
    std::string wrapperName = getWrapperName(F, type);

    Constant *c =
        F->getParent()->getOrInsertFunction(wrapperName, F->getFunctionType());
    Function *wrapperF = cast<Function>(c);
    wrapperF->setCallingConv(CallingConv::C);

    wrapperF->addFnAttr(
        Attribute::NoInline); // adding noinline attribute to function

    // copy the argument names from the acclerated function to the wrapper
    // function
    for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
                                I2 = wrapperF->arg_begin();
         I != E; ++I, ++I2) {
        I2->setName(I->getName());
    }

    // create a basic block
    BasicBlock *block = BasicBlock::Create(getGlobalContext(), "", wrapperF);
    IRBuilder<> builder(block);

    StartAddr += 8;
    int StatusAddr = StartAddr;

    // create the volatile store instructions to send data to accelerators
    // first send over all arguments
    StartAddr += 4;
    int bitWidth;
    Value *zero_offset;

    for (Function::arg_iterator I = wrapperF->arg_begin();
         I != wrapperF->arg_end(); ++I) {

        // check the type of the argument
        bitWidth = I->getType()->getScalarSizeInBits();

        // bitWidth for wrappers is either 32 or 64 bits
        if (bitWidth > 32)
            bitWidth = 64;
        else if (bitWidth < 32)
            bitWidth = 32;

        // the offset for GEP used in sequential wrappers is 0
        zero_offset = ConstantInt::get(
            IntegerType::get(getGlobalContext(), bitWidth), 0);

        // generate instructions to send arguments
        generateWrapperStoreArgument(builder, I, StartAddr, zero_offset,
                                     bitWidth);

        // increment address for next pointer
        StartAddr += 4;
        if (bitWidth > 32)
            StartAddr += 4;
    }

    // send start signal
    zero_offset = ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 0);
    generateWrapperGiveStartSignaltoAccel(builder, StatusAddr, zero_offset);

    // the processor automatically stalls
    // after sending the start signal
    // until "done" is received from the accelerator
    //
    // once "done is received, we need to
    // load return value from accelerator
    //
    // check the return type of the function
    Type *rT = F->getReturnType();
    bitWidth = rT->getScalarSizeInBits();

    // create a return instruction depending on the function return type
    if (bitWidth == 0) {
        // generate load instruction to load from accelerator
        // even though accelerator does not return a value
        // this is so that processor will read from accelerator
        // and then will be stalled by the waitrequest signal
        generateWrapperLoadValue(builder, StatusAddr - 8, zero_offset, 32,
                                 true);
        // create a return void instruction
        builder.CreateRetVoid();
    } else {

        int offsetBitWidth = 32;
        if (bitWidth > 32)
            offsetBitWidth = 64;

        // the offset for GEP used in sequential wrappers is 0
        zero_offset = ConstantInt::get(
            IntegerType::get(getGlobalContext(), offsetBitWidth), 0);

        // generate load instruction to load from accelerator
        Value *loadVal = NULL;
        loadVal = generateWrapperLoadValue(builder, StatusAddr - 8,
                                           zero_offset, offsetBitWidth);
        Value *returnVal;
        if (rT->isPointerTy()) {
            returnVal = builder.CreateIntToPtr(
                loadVal,
                PointerType::get(IntegerType::get(getGlobalContext(), bitWidth),
                                 0));
        } else {
            returnVal = builder.CreateTrunc(
                loadVal, IntegerType::get(getGlobalContext(), bitWidth));
        }

        builder.CreateRet(returnVal);
    }
}

// Generate wrapper for OMP accelerators
void SwOnly::generateOMPWrapper(Function *F, const wrapperType type,
                                int StartAddr, const int AddressSize,
                                const int numAccelerated) {

    // get the existing wrapper function
    Function *wrapperF = getWrapperFunctionAndDeleteBody(F, ompcall);

    wrapperF->addFnAttr(
        Attribute::NoInline); // adding noinline attribute to function

    // generate the calling portion of the OMP wrapper
    //
    // create a new basic block
    BasicBlock *entry =
        BasicBlock::Create(getGlobalContext(), "call_preheader", wrapperF);

    // use IR builder to build instructions
    IRBuilder<> builder(entry);

    // increment by 8 to skip the DATA pointer, since it is never used for OMP
    // functions
    // OMP functions don't have a return value
    StartAddr += 8;
    int StatusAddr = StartAddr;

    // create a new BB for the loop body, which iterates to call the acclerators
    BasicBlock *loop =
        generateBBandSetIncomingBr(builder, wrapperF, "call_loop");

    // generate instructions to call the accelerators in a loop
    Value *exitCond_callingLoop = generateOMPWrapperCallingLoop(
        builder, wrapperF, loop, entry, numAccelerated, AddressSize, StartAddr);

    // generate the polling portion of the OMP wrapper
    //
    // create polling loop BB
    BasicBlock *poll_entry = generateBBandSetIncomingBr(
        builder, wrapperF, "poll_outerloop_preheader", true, loop,
        exitCond_callingLoop);

    // generate instructions to poll the accelerators in a loop
    Value *exitCond_pollingLoop = generateOMPWrapperPollingLoop(
        builder, wrapperF, loop, numAccelerated, AddressSize, StatusAddr);

    // create exit BB
    generateBBandSetIncomingBr(builder, wrapperF, "exit", true, poll_entry,
                               exitCond_pollingLoop);

    // create a return void instruction
    builder.CreateRetVoid();
}

Value *SwOnly::generateOMPWrapperPollingLoop(IRBuilder<> &builder, Function *F,
                                             BasicBlock *prevBB,
                                             const int numAccelerated,
                                             const int AddressSize,
                                             const int StatusAddr) {

    // create phi node to get induction variable
    PHINode *phi2 =
        builder.CreatePHI(IntegerType::get(getGlobalContext(), 32), 2, "i2");
    // one incoming value is 0 from entry BB
    phi2->addIncoming(
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 0, false),
        prevBB);

    // create the while loop BB for the polling part
    BasicBlock *poll_while =
        generateBBandSetIncomingBr(builder, F, "poll_whileloop");

    // volatile load from status pointer
    Value *offset =
        generateAccelOffsetfromThreadIDandAddrSize(builder, phi2, AddressSize);
    Value *done = generateWrapperLoadValue(builder, StatusAddr, offset);

    // compare if the loaded value equals 0
    Value *cmp2 = builder.CreateICmpNE(
        done,
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 0, false));

    // create the outer loop body BB
    BasicBlock *poll_outer = generateBBandSetIncomingBr(
        builder, F, "poll_outerloop", true, poll_while, cmp2);

    // increment induction var in outer loop (i2)
    Value *nextvar2 = builder.CreateAdd(
        phi2,
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 1, false),
        "nextvar2");
    // the other incoming value of phi is nextvar in the same BB
    phi2->addIncoming(nextvar2, poll_outer);
    // check if nextvar equals numThreads
    Value *exitCond = builder.CreateICmpEQ(
        nextvar2, ConstantInt::get(IntegerType::get(getGlobalContext(), 32),
                                   numAccelerated, false),
        "exitcond2");

    return exitCond;
}

Value *SwOnly::generateOMPWrapperCallingLoop(
    IRBuilder<> &builder, Function *F, BasicBlock *loopBB, BasicBlock *prevBB,
    const int numAccelerated, const int AddressSize, int &StartAddr) {

    // create phi node to get induction variable
    PHINode *phi =
        builder.CreatePHI(IntegerType::get(getGlobalContext(), 32), 2, "i");
    // one incoming value is 0 from entry BB
    phi->addIncoming(
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 0, false),
        prevBB);

    // calculate how much memory-mapped address offset is needed for each
    // accelerator
    Value *offset =
        generateAccelOffsetfromThreadIDandAddrSize(builder, phi, AddressSize);

    int StatusAddr = StartAddr;
    // send over argument to accelerator
    // CHECK: will there ever be a time where there is more than one argument??
    if (F->arg_size() != 0) {

        StartAddr += 4;
        generateWrapperStoreArgument(builder, F->arg_begin(), StartAddr,
                                     offset);
    }

    StartAddr += 4;

    // generate instructions to send over threadID
    generateWrapperStoreArgument(builder, phi, StartAddr, offset);

    // generate instructions to send start signal
    generateWrapperGiveStartSignaltoAccel(builder, StatusAddr, offset);

    // add instruction to increment induction variable
    Value *nextvar = builder.CreateAdd(
        phi,
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 1, false),
        "nextvar");
    // the other incoming value of phi is nextvar in the same BB
    phi->addIncoming(nextvar, loopBB);

    // check if nextvar equals numThreads
    Value *exitCond = builder.CreateICmpEQ(
        nextvar, ConstantInt::get(IntegerType::get(getGlobalContext(), 32),
                                  numAccelerated, false),
        "exitcond");

    return exitCond;
}

// Generate a new BB and set the branch coming into the new BB
BasicBlock *SwOnly::generateBBandSetIncomingBr(
    IRBuilder<> &builder, Function *F, const std::string BBName,
    bool isConditionalBr, BasicBlock *prevBB, Value *cond) {

    // create a new BB
    BasicBlock *BB = BasicBlock::Create(getGlobalContext(), BBName, F);

    // create a branch into new BB
    if (isConditionalBr) {
        // conditional branch
        builder.CreateCondBr(cond, BB, prevBB);
    } else {
        // non-conditional branch
        builder.CreateBr(BB);
    }

    // set the insert point to loop BB
    builder.SetInsertPoint(BB);

    return BB;
}

// Generate calling wrapper for Pthread accelerators
void SwOnly::generatePthreadCallingWrapper(Function *F, const wrapperType type,
                                           int StartAddr, const int AddressSize,
                                           const int numAccelerated) {

    // get the function
    Function *wrapperF = getWrapperFunctionAndDeleteBody(F, pthreadcall);

    wrapperF->addFnAttr(
        Attribute::NoInline); // adding noinline attribute to function

    // create a basic block
    BasicBlock *block = BasicBlock::Create(getGlobalContext(), "", wrapperF);

    // use IR build to build instructions
    IRBuilder<> builder(block);

    // get the threadVarPtr and the threadIDValue arguments
    Value *threadVarPtr = NULL, *threadIDValue = NULL;
    // get the threadVar argument
    for (Function::arg_iterator I = wrapperF->arg_begin(),
                                E = wrapperF->arg_end();
         I != E; ++I) {
        if (I->getName() == "threadID")
            threadVarPtr = I;
        else if (I->getName() == "threadIDValue")
            threadIDValue = I;
    }
    // this is a thread pointer to store back the memory-mapped address of
    // accelerator
    assert(threadVarPtr->getName() == "threadID");
    // this is the threadID value for this instance of the pthread function
    assert(threadIDValue->getName() == "threadIDValue");

    StartAddr += 8;
    // calculate how much memory-mapped address offset is needed for each
    // accelerator
    Value *accelOffset = generateAccelOffsetfromThreadIDandAddrSize(
        builder, threadIDValue, AddressSize);

    int StatusAddr = StartAddr;

    // send function arguments to accelerator
    for (Function::arg_iterator I = wrapperF->arg_begin();
         I != wrapperF->arg_end(); ++I) {
        assert(I);

        for (Function::arg_iterator I2 = F->arg_begin(); I2 != F->arg_end();
             ++I2) {
            assert(I2);

            if (I->getName() == I2->getName()) {

                StartAddr += 4;

                generateWrapperStoreArgument(builder, I, StartAddr,
                                             accelOffset);

                break;
            }
        }
    }

    // store accel address into thread variable
    generatePthreadCallingWrapperStoreAccelAddr(builder, threadVarPtr,
                                                StatusAddr, accelOffset);

    // now give start signal for the accelerator
    generateWrapperGiveStartSignaltoAccel(builder, StatusAddr, accelOffset);

    // create a return void
    builder.CreateRetVoid();
}

Function *SwOnly::getWrapperFunctionAndDeleteBody(Function *F,
                                                  wrapperType type) {

    // get the function name
    std::string funcName = getWrapperName(F, type);

    // get the function
    Function *wrapperF = findFuncPtr(*F->getParent(), funcName.c_str());
    assert(wrapperF);

    // first delete the existing function body
    wrapperF->deleteBody();

    return wrapperF;
}

void SwOnly::generatePthreadCallingWrapperStoreAccelAddr(IRBuilder<> &builder,
                                                         Value *threadVarPtr,
                                                         const int intBaseAddr,
                                                         Value *offset) {

    // first get the address of this accelerator
    // to do this:
    //
    // generate the GEP of the accel address
    Value *GEP =
        generateGEPfromIntAddressandValueOffset(builder, intBaseAddr, offset);
    // convert to int to match types
    Value *accelAddress =
        builder.CreatePtrToInt(GEP, IntegerType::get(getGlobalContext(), 32));

    // store Accel address to thread variable
    builder.CreateStore(accelAddress, threadVarPtr);
}

// generate instructions to give start signal to accelerator
void SwOnly::generateWrapperGiveStartSignaltoAccel(IRBuilder<> &builder,
                                                   const int intBaseAddr,
                                                   Value *offset) {

    // create value (1) to store to pointer
    Value *one = ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 1);

    // store 1 to accelerator
    generateWrapperStoreArgument(builder, one, intBaseAddr, offset);
}

// generate instructions to store an argument to accelerator
// "value" is the value of the argument
// "baseAddr" and "offset" are used to generate the GEP to
// store the argument to
void SwOnly::generateWrapperStoreArgument(IRBuilder<> &builder, Value *value,
                                          const int baseAddr, Value *offset,
                                          const int bitWidth, bool isVolatile) {

    // get the GEP to store to
    Value *GEP = generateGEPfromIntAddressandValueOffset(builder, baseAddr,
                                                         offset, bitWidth);

    Value *valueIntTy = value;
    // match types if necessary
    if (value->getType()->isPointerTy()) {

        // convert to int type if it is a pointer
        valueIntTy = builder.CreatePtrToInt(
            value, IntegerType::get(getGlobalContext(), bitWidth));
    } else {

        // sign extend to correct bitwidth
        int valueBitWidth = value->getType()->getScalarSizeInBits();
        if (valueBitWidth < 32) {
            valueIntTy = builder.CreateSExt(
                value, IntegerType::get(getGlobalContext(), bitWidth));
        }
    }

    // volatile store of argument into the GEP
    builder.CreateStore(valueIntTy, GEP, isVolatile);
}

// generate instructions to load value from accelerator
// "baseAddr" and "offset" are used to generate the GEP to
// load from accelerator
Value *SwOnly::generateWrapperLoadValue(IRBuilder<> &builder,
                                        const int baseAddr, Value *offset,
                                        const int bitWidth, bool isVolatile) {

    // get the GEP to load from
    Value *GEP = generateGEPfromIntAddressandValueOffset(builder, baseAddr,
                                                         offset, bitWidth);

    // create a load instruction to load from accelerator
    LoadInst *load = builder.CreateLoad(GEP, isVolatile);

    return load;
}

Value *SwOnly::generateGEPfromIntAddressandValueOffset(IRBuilder<> &builder,
                                                       const int addr,
                                                       Value *offset,
                                                       const int bitWidth) {

    // generate a pointer from integer address
    Value *addressPtr = generatePtrfromIntAddress(builder, addr, bitWidth);
    // generate GEP from pointer and offset
    Value *GEP = builder.CreateGEP(addressPtr, offset);

    return GEP;
}

Value *SwOnly::generatePtrfromIntAddress(IRBuilder<> &builder, const int addr,
                                         const int bitWidth) {

    Value *addrValue =
        ConstantInt::get(IntegerType::get(getGlobalContext(), bitWidth), addr);

    Type *pointerType =
        PointerType::get(IntegerType::get(getGlobalContext(), bitWidth), 0);

    Value *addrPtr = builder.CreateIntToPtr(addrValue, pointerType);

    return addrPtr;
}

Value *SwOnly::generateAccelOffsetfromThreadIDandAddrSize(IRBuilder<> &builder,
                                                          Value *threadID,
                                                          int addrSize) {

    // calculate how much memory-mapped address offset is needed for each
    // accelerator
    // basically log(addressSize/4)/log(2)
    int addrBusWidth = ceil(log(addrSize / 4) / log(2));

    // shl with this value (replacing mult operation with shift) to get the
    // address offset for this instance of accelerator
    Value *offset = builder.CreateShl(
        threadID, ConstantInt::get(IntegerType::get(getGlobalContext(), 32),
                                   addrBusWidth, false),
        "offset");

    return offset;
}

// Generate polling wrapper for Pthread accelerators
void SwOnly::generatePthreadPollingWrapper(Module &M, bool pthreadReturn) {

    // first find the existing polling wrapper function
    Function *F = findFuncPtr(M, "legup_pthreadpoll");

    // set the function type/name/attribute
    Constant *FCache =
        M.getOrInsertFunction(F->getName(), F->getFunctionType());
    F = cast<Function>(FCache);
    F->addFnAttr(Attribute::NoInline); // adding noinline attribute to function
    F->setName("legup_pthreadpoll");

    // set the name of argument
    Function::arg_iterator I = --F->arg_end();
    I->setName("threadID");

    // set the param type
    std::vector<Type *> Params;
    IntegerType *IntTy = Type::getInt32Ty(getGlobalContext());
    Params.push_back(IntTy);

    // create a basic block
    BasicBlock *entry = BasicBlock::Create(getGlobalContext(), "entry", F);

    // use IR build to build instructions
    IRBuilder<> builder(entry);

    // get the threadVar argument
    Value *threadVar = NULL;
    for (Function::arg_iterator I = F->arg_begin(); I != F->arg_end(); ++I) {
        if (I->getName() == "threadID") {
            threadVar = I;
            break;
        }
    }
    assert(threadVar);

    // create a new BB for the while loop to check if accelerator is done
    BasicBlock *loop = generateBBandSetIncomingBr(builder, F, "loop");

    // create the status pointer from threadVar argument
    Value *statusAddressPtr = builder.CreateIntToPtr(
        threadVar, Type::getInt32PtrTy(getGlobalContext()));
    // volatile load from status pointer
    LoadInst *done = builder.CreateLoad(statusAddressPtr, true);

    // compare if the loaded value equals 0
    Value *cmp = builder.CreateICmpNE(
        done,
        ConstantInt::get(IntegerType::get(getGlobalContext(), 32), 0, false));

    // create the exit BB and set the branch
    generateBBandSetIncomingBr(builder, F, "exit", true, loop, cmp);

    // if any of the pthread functions are non-void type
    // create DATA pointer to retrieve return value
    // else return void
    if (pthreadReturn) {

        // get a pointer to data address (status address - 2)
        Value *dataAddressPtr = builder.CreateGEP(
            statusAddressPtr,
            ConstantInt::get(IntegerType::get(getGlobalContext(), 32), -2,
                             false));
        // volatile load from data pointer to retrieve return value
        LoadInst *returnVal = builder.CreateLoad(dataAddressPtr, true);
        // convert return value from int to ptr
        Value *returnValPtr = builder.CreateIntToPtr(
            returnVal, Type::getInt8PtrTy(getGlobalContext()));
        // return ptr
        builder.CreateRet(returnValPtr);
    } else {
        builder.CreateRetVoid();
    }
}

/// replaceHwCallWithWrapper - replace the call instruction with a call to the
/// legup wrapper function
bool SwOnly::replaceHwCallWithWrapper(CallInst *CI, Function *calledFunction,
                                      wrapperType type) {
    string wrapperName = getWrapperName(calledFunction, type);

    ReplaceCallWith(wrapperName.c_str(), CI,
                    copyArguments(CI->op_begin(), CI->op_end() - 1),
                    calledFunction->getReturnType());

    return false;
}

// Caculate the amount of memory space occupied this accelerator
unsigned long long SwOnly::calculateMemorySpace(Function *F,
                                                unsigned long long CurAddr,
                                                wrapperType type) {

    unsigned long long oldCurAddr = CurAddr;

    // for now just increment no matter what for the DATA pointer
    // incrememt by 8 since it can be a long long pointer
    CurAddr = CurAddr + 8;

    // incrememt by another 4 for the STATUS pointer
    CurAddr = CurAddr + 4;

    // for every argument into the function, increment address by appropriate
    // amount
    for (Function::arg_iterator it = F->arg_begin(), e = F->arg_end(); it != e;
         ++it) {

        if (type == ompcall && it->getName() == "threadID")
            continue;
        else {

            bool arg64bit = false;

            // if integer type
            if (const IntegerType *ITy = dyn_cast<IntegerType>(it->getType())) {
                // if 64 bits
                if (ITy->getBitWidth() == 64) {
                    arg64bit = true;
                }
            } else if (it->getType()->isDoubleTy()) {
                arg64bit = true;
            }

            // incrememt address for next pointer
            CurAddr = CurAddr + 4;
            if (arg64bit)
                CurAddr = CurAddr + 4;
        }
    }

    // for openMP threads, insert extra arguments to pass in the numThreads and
    // threadID
    if (type == ompcall) {

        CurAddr = CurAddr + 4;

        if (lockUsed) {
            CurAddr = CurAddr + 4;
        }
    }

    return oldCurAddr + (CurAddr - oldCurAddr);
}

// this functions add all global variables used in HW
// to llvm.used intrinsic global variable, which prohibits the compiler
// from optimizing the global variable away
// this is need since the SW IR needs to keep the global variable, so that
// its address can be parsed later in the Verilog backend
void SwOnly::preserveGlobalVariablesUsedInHW(Module &M) {

    std::set<GlobalValue *> HwFcts;
    // get all HW functions (user-designated functions, parallel functions, and
    // all of their descendants)
    for (std::vector<accelFcts>::const_iterator I = AcceleratedFcts.begin(),
                                                E = AcceleratedFcts.end();
         I != E; ++I) {
        Function *accelF = (*I).fct;
        HwFcts.insert(accelF);
        addCalledFunctions(accelF, HwFcts);
    }

    SmallPtrSet<GlobalValue *, 8> GVUsed;
    // get the llvm.used global variable if it exists already
    GlobalVariable *LLVMUsed = M.getGlobalVariable("llvm.used");
    // get all global variables stored in llvm.used
    findUsedValues(LLVMUsed, GVUsed);
    // delete the original llvm.used variable
    if (LLVMUsed)
        LLVMUsed->eraseFromParent();

    // create a new vector
    // to transfer all global variables which was stored in
    // the existing llvm.used variable
    std::vector<Constant *> GVUsedNew;
    llvm::Type *i8PTy = llvm::Type::getInt8PtrTy(getGlobalContext());
    if (!GVUsed.empty()) {
        for (auto *GV : GVUsed) {
            Constant *c = ConstantExpr::getBitCast(GV, i8PTy);
            GVUsedNew.push_back(c);
        }
    }

    // iterate through all global variables
    // store into GVUsedNew vector, if the global variable is used in
    // the HW functions
    for (Module::global_iterator I = M.global_begin(), E = M.global_end();
         I != E; ++I) {
        // bitcast to the format in llvm.used
        Constant *c = ConstantExpr::getBitCast(I, i8PTy);

        // add to the vector if it doesn't exists already
        if (find(GVUsedNew.begin(), GVUsedNew.end(), c) == GVUsedNew.end())
            GVUsedNew.push_back(c);

#if 0 // this doesn't get all global correctly, since isUsedinBasicBlock used in
      // isUsedinFunction doesn't get all the values, if a value is an operand 
      // of another operand (i.e load (bitcast(GetElementPtr(Value))))
      // To make this work, we would have to recurse into all operands
      // OR recurse out of all values until we get the instruction
      // for now we add all globals to llvm.used and rely on -globalopt pass
      // to rip out any unnecessary globals prior to this
            // if it already exists in the vector, continue
            if (find(GVUsedNew.begin(), GVUsedNew.end(), c) !=
                GVUsedNew.end())
                continue;

            // check if it's used in any of the HW functions
            for (std::set<GlobalValue *>::iterator it = HwFcts.begin();
                 it != HwFcts.end(); ++it) {
                Function *F = cast<Function>(*it);
                errs () << "function = " <<  F->getName() << "\n";
                if (isUsedinFunction(I, F)) {
                    GVUsedNew.push_back(c);
                    break;
            }
        }
#endif
    }

    // create the llvm.used global variable
    // which contains the bistcast of all global variables
    // used by the HW functions
    llvm::ArrayType *ATy = llvm::ArrayType::get(i8PTy, GVUsedNew.size());
    LLVMUsed = new llvm::GlobalVariable(
        M, ATy, false, llvm::GlobalValue::AppendingLinkage,
        llvm::ConstantArray::get(ATy, GVUsedNew), "llvm.used");
    // set the correct section, as required by llvm
    LLVMUsed->setSection("llvm.metadata");
}

void SwOnly::printCacheHWtcl() {

    formatted_raw_ostream *HW_tcl;
    std::string fileName = "data_cache_hw.tcl";
    HW_tcl = GetOutputStream(fileName);
    assert(HW_tcl);
    raw_ostream &tcl = *HW_tcl;

    int sdramSize;
    if (FPGABoard == "DE4") {
        sdramSize = 256;
    } else {
        sdramSize = 32;
    }
    int byteenableSize = sdramSize / 8;

    tcl << "package require -exact sopc 11.0\n\n"
        << "set_module_property NAME data_cache\n"
        << "set_module_property VERSION 1.0\n"
        << "set_module_property INTERNAL false\n"
        << "set_module_property OPAQUE_ADDRESS_MAP true\n"
        << "set_module_property DISPLAY_NAME data_cache\n"
        << "set_module_property TOP_LEVEL_HDL_FILE data_cache.v\n"
        << "set_module_property TOP_LEVEL_HDL_MODULE data_cache\n"
        << "set_module_property INSTANTIATE_IN_SYSTEM_MODULE true\n"
        << "set_module_property EDITABLE true\n"
        << "set_module_property ANALYZE_HDL TRUE\n\n"

        << "add_file data_cache.v {SYNTHESIS SIMULATION}\n\n"

        << "add_interface clockreset clock end\n"
        << "set_interface_property clockreset clockRate 0\n"
        << "set_interface_property clockreset ENABLED true\n"
        << "add_interface_port clockreset csi_clockreset_clk clk Input 1\n"
        << "add_interface_port clockreset csi_clockreset_reset_n reset_n Input "
           "1\n\n";

    if (multiportedCache && dcacheType == "MP" && FPGABoard == "DE4") {
        tcl << "add_interface clockreset2X clock end\n"
            << "set_interface_property clockreset2X clockRate 0\n"
            << "set_interface_property clockreset2X ENABLED true\n"
            << "add_interface_port clockreset2X csi_clockreset2X_clk clk Input "
               "1\n\n"

            << "add_interface clockreset2X_reset reset end\n"
            << "set_interface_property clockreset2X_reset associatedClock "
               "clockreset\n"
            << "set_interface_property clockreset2X_reset synchronousEdges "
               "DEASSERT\n"
            << "set_interface_property clockreset2X_reset ENABLED true\n"
            << "add_interface_port clockreset2X_reset csi_clockreset2X_reset_n "
               "reset_n Input 1\n\n";
    }

    tcl << "add_interface PROC avalon_streaming start\n"
        << "set_interface_property PROC associatedClock clockreset\n"
        << "set_interface_property PROC dataBitsPerSymbol 8\n"
        << "set_interface_property PROC errorDescriptor \"\"\n"
        << "set_interface_property PROC maxChannel 0\n"
        << "set_interface_property PROC readyLatency 0\n"
        << "set_interface_property PROC ENABLED true\n"
        << "add_interface_port PROC aso_PROC_data data Output 8\n\n";

    for (int i = 0; i < dcacheports; i++) {
        tcl << "add_interface CACHE" << i << " avalon end\n"
            << "set_interface_property CACHE" << i
            << " addressAlignment DYNAMIC\n"
            << "set_interface_property CACHE" << i << " addressUnits WORDS\n"
            << "set_interface_property CACHE" << i
            << " associatedClock clockreset\n"
            << "set_interface_property CACHE" << i
            << " burstOnBurstBoundariesOnly false\n"
            << "set_interface_property CACHE" << i << " explicitAddressSpan 0\n"
            << "set_interface_property CACHE" << i << " holdTime 0\n"
            << "set_interface_property CACHE" << i << " isMemoryDevice false\n"
            << "set_interface_property CACHE" << i
            << " isNonVolatileStorage false\n"
            << "set_interface_property CACHE" << i << " linewrapBursts false\n"
            << "set_interface_property CACHE" << i
            << " maximumPendingReadTransactions 0\n"
            << "set_interface_property CACHE" << i << " printableDevice false\n"
            << "set_interface_property CACHE" << i << " readLatency 0\n"
            << "set_interface_property CACHE" << i << " readWaitTime 1\n"
            << "set_interface_property CACHE" << i << " setupTime 0\n"
            << "set_interface_property CACHE" << i << " timingUnits Cycles\n"
            << "set_interface_property CACHE" << i << " writeWaitTime 0\n"
            << "set_interface_property CACHE" << i << " ENABLED true\n\n"

            << "add_interface_port CACHE" << i << " avs_CACHE" << i
            << "_begintransfer begintransfer Input 1\n"
            << "add_interface_port CACHE" << i << " avs_CACHE" << i
            << "_read read Input 1\n"
            << "add_interface_port CACHE" << i << " avs_CACHE" << i
            << "_write write Input 1\n"
            << "add_interface_port CACHE" << i << " avs_CACHE" << i
            << "_writedata writedata Input 128\n"
            << "add_interface_port CACHE" << i << " avs_CACHE" << i
            << "_readdata readdata Output 128\n"
            << "add_interface_port CACHE" << i << " avs_CACHE" << i
            << "_waitrequest waitrequest Output 1\n\n"

            << "add_interface dataMaster" << i << " avalon start\n"
            << "set_interface_property dataMaster" << i
            << " addressUnits SYMBOLS\n"
            << "set_interface_property dataMaster" << i
            << " associatedClock clockreset\n"
            << "set_interface_property dataMaster" << i
            << " burstOnBurstBoundariesOnly false\n"
            << "set_interface_property dataMaster" << i
            << " doStreamReads false\n"
            << "set_interface_property dataMaster" << i
            << " doStreamWrites false\n"
            << "set_interface_property dataMaster" << i
            << " linewrapBursts false\n"
            << "set_interface_property dataMaster" << i << " readLatency 0\n"
            << "set_interface_property dataMaster" << i << " ENABLED true\n\n";

        // if (LEGUP_CONFIG->getParameterInt("USE_QSYS")) {
        //    tcl << "add_interface_port dataMaster" << i << " avm_dm" << i
        //        << "_read read Output 1\n"
        //        << "add_interface_port dataMaster" << i << " avm_dm" << i
        //        << "_write write Output 1\n"
        //        << "add_interface_port dataMaster" << i << " avm_dm" << i
        //        << "_address address Output 32\n"
        //
        //        << "add_interface_port dataMaster" << i << " avm_dm" << i
        //        << "_writedata writedata Output " << sdramSize << "\n"
        //        << "add_interface_port dataMaster" << i << " avm_dm" << i
        //        << "_byteenable byteenable Output " << byteenableSize << "\n"
        //        << "add_interface_port dataMaster" << i << " avm_dm" << i
        //        << "_readdata readdata Input " << sdramSize << "\n"
        //
        //        << "add_interface_port dataMaster" << i << " avm_dm" << i
        //        << "_waitrequest waitrequest Input 1\n"
        //        << "add_interface_port dataMaster" << i << " avm_dm" << i
        //        << "_readdatavalid readdatavalid Input 1\n\n";
        //} else {
        tcl << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_read read Output 1\n"
            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_write write Output 1\n"
            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_address address Output 32\n"

            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_writedata writedata Output " << sdramSize << "\n"
            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_byteenable byteenable Output " << byteenableSize << "\n"
            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_readdata readdata Input " << sdramSize << "\n"

            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_beginbursttransfer beginbursttransfer Output 1\n"
            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_burstcount burstcount Output 6\n"
            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_waitrequest waitrequest Input 1\n"
            << "add_interface_port dataMaster" << i << " avm_dataMaster" << i
            << "_readdatavalid readdatavalid Input 1\n\n";
        //}
    }

    delete HW_tcl;
}

// print _hw.tcl file to specify interface specs for the accelerator (needed for
// SOPC builder)
void SwOnly::printHWtcl(Function *F, wrapperType type, int AddressBusWidth) {

    formatted_raw_ostream *HW_tcl;
    string AccelName = F->getName();
    stripInvalidCharacters(AccelName);
    HW_tcl = GetOutputStream(AccelName.append("_hw.tcl"));
    assert(HW_tcl);
    raw_ostream &tcl = *HW_tcl;

    AccelName = F->getName();
    stripInvalidCharacters(AccelName);
    string AccelFilename = std::getenv("LEGUP_ACCELERATOR_FILENAME");

    tcl << "package require -exact sopc 11.0\n\n"

        << "set_module_property NAME " << AccelName << "\n"
        << "set_module_property VERSION 1.0\n"
        << "set_module_property INTERNAL false\n"
        << "set_module_property GROUP \"\"\n"
        << "set_module_property DISPLAY_NAME " << AccelName << "\n"
        << "set_module_property TOP_LEVEL_HDL_FILE " << AccelFilename << ".v\n"
        << "set_module_property TOP_LEVEL_HDL_MODULE " << AccelName << "_top\n"
        << "set_module_property INSTANTIATE_IN_SYSTEM_MODULE true\n"
        << "set_module_property EDITABLE true\n"
        << "set_module_property ANALYZE_HDL TRUE\n\n"
        << "add_file " << AccelFilename << ".v {SYNTHESIS SIMULATION}\n\n"
        << "add_interface clockreset clock end\n"
        << "set_interface_property clockreset ENABLED true\n"
        << "add_interface_port clockreset csi_clockreset_clk clk Input 1\n"
        << "add_interface_port clockreset csi_clockreset_reset reset Input "
           "1\n\n"

        << "add_interface s1 avalon end\n"
        << "set_interface_property s1 addressAlignment DYNAMIC\n"
        << "set_interface_property s1 associatedClock clockreset\n"
        << "set_interface_property s1 burstOnBurstBoundariesOnly false\n"
        << "set_interface_property s1 explicitAddressSpan 0\n"
        << "set_interface_property s1 holdTime 0\n"
        << "set_interface_property s1 isMemoryDevice false\n"
        << "set_interface_property s1 isNonVolatileStorage false\n"
        << "set_interface_property s1 linewrapBursts false\n"
        << "set_interface_property s1 maximumPendingReadTransactions 0\n"
        << "set_interface_property s1 printableDevice false\n"
        << "set_interface_property s1 readLatency 0\n"
        << "set_interface_property s1 readWaitTime 1\n"
        << "set_interface_property s1 setupTime 0\n"
        << "set_interface_property s1 timingUnits Cycles\n"
        << "set_interface_property s1 writeWaitTime 0\n\n"

        << "set_interface_property s1 ASSOCIATED_CLOCK clockreset\n"
        << "set_interface_property s1 ENABLED true\n\n"

        << "add_interface_port s1 avs_s1_address address Input "
        << AddressBusWidth << "\n"
        << "add_interface_port s1 avs_s1_read read Input 1\n"
        << "add_interface_port s1 avs_s1_write write Input 1\n"
        << "add_interface_port s1 avs_s1_writedata writedata Input 32\n" // always
                                                                         // 32
                                                                         // bits
        // since
        // its
        // a 32
        // bit
        // processor
        << "add_interface_port s1 avs_s1_readdata readdata Output 32\n"

        << "add_interface ACCEL avalon start\n"
        << "set_interface_property ACCEL associatedClock clockreset\n"
        << "set_interface_property ACCEL burstOnBurstBoundariesOnly false\n"
        << "set_interface_property ACCEL doStreamReads false\n"
        << "set_interface_property ACCEL doStreamWrites false\n"
        << "set_interface_property ACCEL linewrapBursts false\n\n"

        << "set_interface_property ACCEL ASSOCIATED_CLOCK clockreset\n"
        << "set_interface_property ACCEL ENABLED true\n"

        << "add_interface_port ACCEL avm_ACCEL_readdata readdata Input 128\n"
        << "add_interface_port ACCEL avm_ACCEL_waitrequest waitrequest Input "
           "1\n"
        << "add_interface_port ACCEL avm_ACCEL_address address Output 32\n"
        << "add_interface_port ACCEL avm_ACCEL_writedata writedata Output 128\n"
        << "add_interface_port ACCEL avm_ACCEL_write write Output 1\n"
        << "add_interface_port ACCEL avm_ACCEL_read read Output 1\n\n";

    delete HW_tcl;
}

// last part of sopc file
void SwOnly::printSopcFileEnd(raw_ostream &out) {

    out << "save_system\n\n";
    out << "generate_system\n";
}

// last part of qsys file
void SwOnly::printQSYSFileEnd(raw_ostream &out) { out << "save_system\n\n"; }

// initial part of sopc file, removing the connection only needs to be done once
void SwOnly::printSopcFileInitial(raw_ostream &sopc) {

    sopc << "load_system tiger/tiger.sopc\n";

    // need to first remove data cache and add it again in order to add
    // accelerator port to cache
    sopc << "remove_module data_cache_0\n"
         << "add_module data_cache data_cache_0\n"
         << "set_avalon_base_address data_cache_0.CACHE1 \"0x00000000\"\n";

    // add the clock connection
    if (FPGABoard == "DE4") {
        sopc << "add_connection ddr2.sysclk data_cache_0.clockreset\n";
    } else {
        sopc << "add_connection clk.clk data_cache_0.clockreset\n";
    }

    if (multiportedCache && dcacheType == "MP" && FPGABoard == "DE4") {
        sopc << "add_connection ddr2.auxfull data_cache_0.clockreset2X\n"
             << "set_avalon_base_address data_cache_0.CACHE2 \"0x01000000\"\n"
             << "set_avalon_base_address data_cache_0.CACHE3 \"0x01000000\"\n";
    }

    // connect the other ports from data cache
    sopc << "add_connection data_cache_0.PROC tiger_top_0.PROC\n";

    for (int i = 0; i < dcacheports; i++) {
        sopc << "add_connection data_cache_0.dataMaster" << i
             << "  pipeline_bridge_MEMORY.s1\n";
    }

    // setting this to false for testing
    parallelAccelUsed = false;
    // if parallel accelerators are used, add pipeline bridge to improve Fmax
    if (parallelAccelUsed) {
        for (int i = 0; i < dcacheports - 1; i++) {

            // add pipeline bridge between accelerators and cache to improve
            // Fmax
            sopc << "add_module altera_avalon_pipeline_bridge pipeline_bridge_"
                 << i + 1 << "\n";
            if (FPGABoard == "DE4") {
                sopc << "add_connection ddr2.sysclk pipeline_bridge_" << i + 1
                     << ".clk\n";
            } else {
                sopc << "add_connection clk.clk pipeline_bridge_" << i + 1
                     << ".clk\n";
            }
            // the bridge is pipelined from master-to-slave
            sopc << "set_parameter pipeline_bridge_" << i + 1
                 << " dataWidth 128\n";

            // don't pipeline downstream (from accelerator to cache)
            sopc << "set_parameter pipeline_bridge_" << i + 1
                 << " downstreamPipeline false\n";

            // pipeline upstream (from cache to accelerator)
            sopc << "set_parameter pipeline_bridge_" << i + 1
                 << " upstreamPipeline true\n";

            // pipeline waitrequest
            sopc << "set_parameter pipeline_bridge_" << i + 1
                 << " waitrequestPipeline true\n";

            sopc << "add_connection pipeline_bridge_" << i + 1
                 << ".m1 data_cache_0.CACHE" << i % (dcacheports - 1) + 1
                 << "\n";
            sopc << "set_avalon_base_address pipeline_bridge_" << i + 1
                 << ".s1 0x00\n";
        }
    }
    sopc << "add_connection tiger_top_0.CACHE data_cache_0.CACHE0\n\n";

    // add omp, lock, barrier, perf counter cores if needed
    printSopcFileAPIcores(sopc);
}

void SwOnly::printSopcFileAPIcores(raw_ostream &sopc) {

    // if locks are used, connect the mutex core to the processor
    if (lockUsed) {
        int mutexNum = mutexMap.size();

        unsigned long long mutexAddr = 0xc5000000;
        for (int i = 0; i < mutexNum; i++) {
            sopc << "add_module mutex mutex_" << i << "\n"
                 << "set_avalon_base_address mutex_" << i << ".s1 " << mutexAddr
                 << "\n";
            if (FPGABoard == "DE4") {
                sopc << "add_connection ddr2.sysclk mutex_" << i
                     << ".clockreset\n\n";
            } else {
                sopc << "add_connection clk.clk mutex_" << i
                     << ".clockreset\n\n";
            }
            mutexAddr += 0x20;
        }
    }

    // if barriers are used, connect the mutex core to the processor
    if (barrierUsed) {
        sopc << "add_module legup_barrier legup_barrier_0\n"
             << "add_connection tiger_top_0.procMaster legup_barrier_0.s1\n"
             << "set_avalon_base_address legup_barrier_0.s1 \"0xc5001000\"\n";
        if (FPGABoard == "DE4") {
            sopc << "add_connection ddr2.sysclk legup_barrier_0.clockreset\n\n";
        } else {
            sopc << "add_connection clk.clk legup_barrier_0.clockreset\n\n";
        }
    }

    // if perf counters are used, connect the perf counter core to the processor
    if ((perfCounterUsed) && (ProcessorArchitecture != "ARMA9")) {
        unsigned long long perfCounterAddr = 0x01000000;
        for (int i = -1; i < numPerfCounter; i++) {
            sopc << "add_module performance_counter performance_counter_"
                 << i + 1 << "\n"
                 << "add_connection pipeline_bridge_PERIPHERALS.m1 "
                    "performance_counter_" << i + 1 << ".s1\n"
                 << "set_avalon_base_address performance_counter_" << i + 1
                 << ".s1 " << perfCounterAddr << "\n";
            if (FPGABoard == "DE4") {
                sopc << "add_connection ddr2.sysclk performance_counter_"
                     << i + 1 << ".clockreset\n\n";
            } else {
                sopc << "add_connection clk.clk performance_counter_" << i + 1
                     << ".clockreset\n\n";
            }
            perfCounterAddr += 0x8;
        }
    }
}

// prints tcl commands to generate SOPC
void SwOnly::printSopcFile(raw_ostream &sopc, Function *F, wrapperType type,
                           unsigned long long baseAddr, int AccelCount,
                           int AccelIndex) {

    // for accelerator
    std::string moduleName = F->getName();
    stripInvalidCharacters(moduleName);

    sopc << "add_module " << F->getName() << " " << moduleName << "_"
         << AccelCount << "\n";

    // for DE4, connect the sysclk from ddr2 controller to the clock of
    // accelerator
    // for DE2, connect the system clock to the clock of accelerator
    if (FPGABoard == "DE4") {
        sopc << "add_connection ddr2.sysclk " << moduleName << "_" << AccelCount
             << ".clockreset\n";
    } else {
        sopc << "add_connection clk.clk " << moduleName << "_" << AccelCount
             << ".clockreset\n";
    }

    // connect accelerator to pipeline bridge
    sopc << "add_connection pipeline_bridge_PERIPHERALS.m1 " << moduleName
         << "_" << AccelCount << ".s1\n"
         << "set_avalon_base_address " << moduleName << "_" << AccelCount
         << ".s1 \"0x";
    sopc.write_hex(baseAddr);
    sopc << "\"\n";

    // int cachePortNum = ((AccelCount+1)%dcacheports);
    // for now we don't share the processor port with other accelerators
    // hence -1 and the -1
    int cachePortNum = (AccelIndex % (dcacheports - 1)) + 1;
    // if it is a sequential accelerator, connect directly to cache
    // otherwise connect to pipeline bridge
    // for testing
    //    if (type == seq) {
    // connect accelerator to data cache
    sopc << "add_connection " << moduleName << "_" << AccelCount
         << ".ACCEL data_cache_0.CACHE" << cachePortNum << "\n\n";
    //    } else {
    // connect to the pipeline bridge
    //    	sopc << "add_connection " << moduleName << "_" << AccelCount <<
    //    ".ACCEL pipeline_bridge_" << cachePortNum << ".s1\n\n";
    //    }

    // disable the 2nd port for now, all accelerator connect to one port of the
    // cache
    //	sopc << "add_connection " << moduleName << "_" << AccelCount << ".ACCEL
    // data_cache_0.CACHE1\n\n";

    // connect accelerator to mutex core
    if (lockUsed) {
        std::set<Function *> funcSet;
        // look through the mutex function map
        for (std::map<int, std::set<Function *>>::iterator it =
                 mutexFunctionMap.begin();
             it != mutexFunctionMap.end(); it++) {
            funcSet = (*it).second;

            // if you find this function in the map
            if (funcSet.find(F) != funcSet.end()) {
                // get the mutex number and connect this accelerator to the
                // mutex
                sopc << "add_connection " << moduleName << "_" << AccelCount
                     << ".ACCEL mutex_" << (*it).first << ".s1\n\n";
            }
        }
    }

    // connect accelerator to barrier core
    if (barrierUsed) {
        // sopc << "add_connection " << moduleName << "_" << AccelCount << ".API
        // legup_barrier_0.s1\n\n";
        sopc << "add_connection " << moduleName << "_" << AccelCount
             << ".ACCEL legup_barrier_0.s1\n\n";
    }
}

// initial part of qsys file
void SwOnly::printQSYSFileInitial(raw_ostream &qsys) {

    // Require the use the appropriate Qsys verion
    qsys << "package require -exact qsys 13.0\n";

    // Load the legup_system
    qsys << "load_system legup_system.qsys\n\n";

    // Update the cache to include accelerator port if available
    // qsys << "remove_instance data_cache_0\n"
    //     << "add_instance data_cache_0 data_cache\n";

    // BF: to be removed
    // add the clock connection
    // if (FPGABoard == "DE4") {
    //    qsys << "add_connection sysclk.out_clk data_cache_0.clockreset\n";
    //    qsys << "add_connection clk.clk_reset
    //    data_cache_0.clockreset_reset\n";
    //} else {
    //    qsys << "add_connection clk.clk data_cache_0.clockreset\n";
    //    qsys << "add_connection clk.clk_reset
    //    data_cache_0.clockreset_reset\n";
    //}

    // BF: to be removed
    // if (multiportedCache && dcacheType == "MP" && FPGABoard == "DE4") {
    //    qsys << "add_connection ddr2.auxfull data_cache_0.clockreset2X\n";
    //}

    // BF: to be removed
    // connect the other ports from data cache
    // qsys << "add_connection data_cache_0.PROC tiger_top_0.PROC\n";

    // BF: to be removed
    // for (int i = 0; i < dcacheports; i++) {
    //    qsys << "add_connection data_cache_0.dataMaster" << i << "
    //    sdram.s1\n";
    //}

    // BF: Not sure what any of this is. To be revisited
    // setting this to false for testing
    parallelAccelUsed = false;
    // if parallel accelerators are used, add pipeline bridge to improve Fmax
    if (parallelAccelUsed) {
        for (int i = 0; i < dcacheports - 1; i++) {

            // add pipeline bridge between accelerators and cache to improve
            // Fmax
            qsys << "add_instance pipeline_bridge_" << i + 1
                 << "altera_avalon_pipeline_bridge\n";
            if (FPGABoard == "DE4") {
                qsys << "add_connection sysclk.out_clk pipeline_bridge_"
                     << i + 1 << ".clockreset\n";
                qsys << "add_connection clk.clk_reset pipeline_bridge_" << i + 1
                     << ".clockreset_reset\n";
            } else {
                qsys << "add_connection clk.clk pipeline_bridge_" << i + 1
                     << ".clockreset\n";
                qsys << "add_connection clk.clk_reset pipeline_bridge_" << i + 1
                     << ".clockreset_reset\n";
            }
            // the bridge is pipelined from master-to-slave
            qsys << "set_instance_parameter_value pipeline_bridge_" << i + 1
                 << " dataWidth 128\n";

            // pipeline downstream (from accelerator to cache)
            qsys << "set_instance_parameter_value pipeline_bridge_" << i + 1
                 << " downstreamPipeline true\n";

            // pipeline upstream (from cache to accelerator)
            qsys << "set_instance_parameter_value pipeline_bridge_" << i + 1
                 << " upstreamPipeline true\n";

            // pipeline waitrequest
            qsys << "set_instance_parameter_value pipeline_bridge_" << i + 1
                 << " waitrequestPipeline true\n";

            qsys << "add_connection pipeline_bridge_" << i + 1
                 << ".m1 data_cache_0.CACHE" << i % (dcacheports - 1) + 1
                 << "\n";
            qsys << "set_avalon_base_address pipeline_bridge_" << i + 1
                 << ".s1 0x00\n";
            qsys << "set_connection_parameter_value data_cache_0.CACHE"
                 << i % (dcacheports - 1) + 1 << "/pipeline_bridge_" << i + 1
                 << ".s1 baseAddress \"0x00\"\n";
        }
    }

    // BF: to be removed
    // qsys << "add_connection tiger_top_0.CACHE data_cache_0.CACHE0\n";

    // add omp, lock, barrier, perf counter cores if needed
    printQSYSFileAPIcores(qsys);
}

// BF: This function needs to be fixed
void SwOnly::printQSYSFileAPIcores(raw_ostream &qsys) {
    const std::string SysClk{SysClkModule + "." + SysClkInterface + " "};
    const std::string SysRst{SysRstModule + "." + SysRstInterface + " "};
    const std::string SysMem{SysMemModule + "." + SysMemInterface};
    const std::string SysProcMaster{SysProcName + "." + SysProcDataMaster};

    // if locks are used, connect the mutex core to the processor
    if (lockUsed) {
        int mutexNum = mutexMap.size();
        for (int i = 0; i < mutexNum; i++) {

            qsys << "add_instance mutex_" << i << " mutex\n";

            // qsys << "add_connection clk.clk mutex_" << i << ".clock\n";
            // qsys << "add_connection clk.clk_reset mutex_" << i
            //     << ".reset\n";
            qsys << "add_connection " << SysClk << "mutex_" << i
                 << ".clockreset\n";
            qsys << "add_connection " << SysRst << "mutex_" << i
                 << ".clockreset_reset\n";
        }
    }

    // if barriers is needed, add it to the system and connect it to the
    // processor
    if (barrierUsed) {
        qsys << "add_instance legup_barrier_0 legup_barrier_sopc\n";
        qsys << "add_connection " << SysProcMaster << " legup_barrier_0.s1\n";
        qsys << "set_connection_parameter_value " << SysProcMaster
             << "/legup_barrier_0.s1 baseAddress ";
        if (ProcessorArchitecture == "ARMA9") {
            qsys << "\"0x05001000\"\n";
        } else {
            qsys << "\"0xC5001000\"\n";
        }
        qsys << "add_connection " << SysClk << "legup_barrier_0.clockreset\n";
        qsys << "add_connection " << SysRst
             << "legup_barrier_0.clockreset_reset\n";
    }

    // if perf counters are used, connect the perf counter core to the processor
    if ((perfCounterUsed) && (ProcessorArchitecture != "ARMA9")) {
        unsigned long long perfCounterAddr = 0xF1000000;
        for (int i = -1; i < numPerfCounter; i++) {
            qsys << "add_instance performance_counter_" << i + 1
                 << " performance_counter\n";
            qsys << "add_connection Tiger_MIPS.data_master "
                 << "performance_counter_" << i + 1 << ".s1\n";
            qsys << "set_connection_parameter_value "
                    "Tiger_MIPS.data_master/performance_counter_" << i + 1
                 << ".s1 baseAddress \"" << perfCounterAddr << "\"\n";

            qsys << "add_connection " << SysClk << "performance_counter_"
                 << i + 1 << ".clockreset\n";
            qsys << "add_connection " << SysRst << "performance_counter_"
                 << i + 1 << ".clock_reset\n\n";
            perfCounterAddr += 0x8;
        }
    }
}

// prints tcl commands to generate QSYS
void SwOnly::printQSYSFile(raw_ostream &qsys, Function *F, wrapperType type,
                           unsigned long long baseAddr, int AccelCount,
                           int AccelIndex, int addrBusWidth) {

    const std::string SysClk{SysClkModule + "." + SysClkInterface + " "};
    const std::string SysRst{SysRstModule + "." + SysRstInterface + " "};
    const std::string SysMem{SysMemModule + "." + SysMemInterface};
    const std::string SysProcMaster{SysProcName + "." + SysProcDataMaster};

    // for accelerator
    std::string moduleName = F->getName();
    stripInvalidCharacters(moduleName);

    std::string bridge_name{"legup_accelerator_bridge_" + moduleName + "_" +
                            std::to_string(AccelCount)};

    // BF: Temporary until the accelerators are avalon compliant
    // Add bridge
    qsys << "add_instance " << bridge_name << " legup_accelerator_bridge\n";

    qsys << "set_instance_parameter_value " << bridge_name << " ADDR_WIDTH "
         << (addrBusWidth + 2) << "\n";

    qsys << "add_connection " << SysClk << bridge_name << ".clock\n";
    qsys << "add_connection " << SysRst << bridge_name << ".reset\n";

    // Connect processor to bridge
    qsys << "add_connection " << SysProcMaster << " " << bridge_name
         << ".from_cpu\n";

    // Connect bridge to memory
    //    qsys << "add_connection "
    //         << "legup_accelerator_bridge_" << AccelCount
    //         << ".to_memory SDRAM.s1\n\n";
    qsys << "add_connection " << bridge_name << ".to_memory " << SysMem
         << "\n\n";
    qsys << "set_connection_parameter_value " << SysProcMaster << "/"
         << bridge_name << ".from_cpu baseAddress \"0x";
    if (ProcessorArchitecture == "ARMA9") {
        qsys.write_hex(baseAddr + 0x08000000); // TODO changed from 0xF0000000
        qsys << "\"\n";
    } else {
        qsys.write_hex(baseAddr + 0xF0000000);
        qsys << "\"\n";
    }

    //    qsys << "set_connection_parameter_value "
    //         << "legup_accelerator_bridge_" << AccelCount <<
    //         ".to_memory/SDRAM.s1"
    //         << " baseAddress \"0x00800000\"\n\n";
    qsys << "set_connection_parameter_value " << bridge_name << ".to_memory/"
         << SysMem << " baseAddress \"0x00000000\"\n\n";

    // Add accelerator
    qsys << "add_instance " << moduleName << "_" << AccelCount << " "
         << moduleName << "\n";

    qsys << "add_connection " << SysClk << moduleName << "_" << AccelCount
         << ".clockreset\n";
    qsys << "add_connection " << SysRst << moduleName << "_" << AccelCount
         << ".clockreset_reset\n\n";

    // connect accelerator to pipeline bridge
    //	qsys << "add_connection pipeline_bridge_PERIPHERALS.m0 " << moduleName
    //<< "_" << AccelCount << ".s1\n";
    //	qsys << "add_connection tiger_mips.data_master " << moduleName << "_" <<
    // AccelCount << ".s1\n";
    qsys << "add_connection " << bridge_name << ".to_accel " << moduleName
         << "_" << AccelCount << ".s1\n";
    // set the base address of the connection between accel and pipeline bridge
    //	qsys << "set_connection_parameter_value pipeline_bridge_PERIPHERALS.m0/"
    //<< moduleName << "_" << AccelCount << ".s1 baseAddress \"0x";
    qsys << "set_connection_parameter_value " << bridge_name << ".to_accel/"
         << moduleName << "_" << AccelCount << ".s1 baseAddress \"0x";
    qsys.write_hex(0);
    qsys << "\"\n\n";

    // Connect accelerator to bridge
    // int cachePortNum = ((AccelCount+1)%dcacheports);
    // for now we don't share the processor port with other accelerators
    // hence -1 and the -1
    // int cachePortNum = (AccelCount%(dcacheports-1))+1;
    // int cachePortNum = (AccelIndex%(dcacheports-1))+1;
    // if it is a sequential accelerator, connect directly to cache
    // otherwise connect to pipeline bridge
    // connect accelerator to data cache
    //    qsys << "add_connection " << moduleName << "_" << AccelCount <<
    //    ".ACCEL data_cache_0.CACHE" << cachePortNum << "\n\n";
    //    qsys << "add_connection " << moduleName << "_" << AccelCount <<
    //    ".ACCEL sdram.s1" << "\n\n";
    qsys << "add_connection " << moduleName << "_" << AccelCount << ".ACCEL "
         << bridge_name << ".from_accel"
         << "\n\n";
    // set Base Address of Cache/Accel connection
    //    qsys << "set_connection_parameter_value " << moduleName << "_" <<
    //    AccelCount << ".ACCEL/data_cache_0.CACHE" << cachePortNum << "
    //    baseAddress \"0x01000000\"\n\n";
    //    qsys << "set_connection_parameter_value " << moduleName << "_" <<
    //    AccelCount << ".ACCEL/sdram.s1" << " baseAddress \"0x01000000\"\n\n";
    qsys << "set_connection_parameter_value " << moduleName << "_" << AccelCount
         << ".ACCEL/" << bridge_name << ".from_accel"
         << " baseAddress \"0x00000000\"\n\n";

    // connect accelerator to mutex core
    if (lockUsed) {

        unsigned long long mutexAddr = 0xc5000000;
        std::set<Function *> funcSet;
        // look through the mutex function map
        for (std::map<int, std::set<Function *>>::iterator it =
                 mutexFunctionMap.begin();
             it != mutexFunctionMap.end(); it++) {
            funcSet = (*it).second;

            // if you find this function in the map
            if (funcSet.find(F) != funcSet.end()) {
                // get the mutex number and connect this accelerator to the
                // mutex
                // qsys << "add_connection legup_accelerator_bridge_" <<
                // AccelCount
                //     << ".to_memory mutex_" << (*it).first << ".s1\n\n";
                qsys << "add_connection " << moduleName << "_" << AccelCount
                     << ".ACCEL mutex_" << (*it).first << ".s1\n\n";
                // base Address
                // qsys << "set_connection_parameter_value "
                //     << "legup_accelerator_bridge_" << AccelCount
                //     << ".to_memory/mutex_" << (*it).first
                //     << ".s1 baseAddress \"" << mutexAddr << "\"\n\n";
                qsys << "set_connection_parameter_value " << moduleName << "_"
                     << AccelCount << ".ACCEL/mutex_" << (*it).first
                     << ".s1 baseAddress \"" << mutexAddr << "\"\n\n";
            }
        }
    }

    // Connect accelerator to barrier core
    if (barrierUsed) {
        // qsys << "add_connection legup_accelerator_bridge_" << AccelCount
        //     << ".to_memory legup_barrier_0.s1\n";
        qsys << "add_connection " << moduleName << "_" << AccelCount
             << ".ACCEL legup_barrier_0.s1\n";
    }
}

// prints tcl commands to generate QSYS
void SwOnly::printQSYSFilePCIeShared(raw_ostream &qsys, Function *F,
                                     unsigned long long baseAddr,
                                     int AccelCount, int addressSize) {

    // Runs for each accelerator
    std::string moduleName = F->getName();
    stripInvalidCharacters(moduleName);

    // qsys << "set num_threads " << AccelCount << "\n";
    qsys << "set baseAddress 0x";
    qsys.write_hex(baseAddr);
    qsys << "\n\n";

    // qsys << "for {set i 0} {$i < $num_threads} {incr i} {"
    //      << "\n";
    qsys << "\tadd_instance " << moduleName << "_" << AccelCount << " "
         << moduleName << "\n";

    // Add the accel_mem_bridge
    qsys << "\tadd_instance " << moduleName << "_accel_mem_bridge_"
         << AccelCount << " accel_to_mem_bridge\n";

    // Connect clock to accelerators
    qsys << "\t# Connect clock to accelerators\n";
    qsys << "\tadd_connection clk_0.clk " << moduleName << "_" << AccelCount
         << ".clockreset\n";
    qsys << "\tadd_connection clk_0.clk_reset " << moduleName << "_"
         << AccelCount << ".clockreset_reset\n";
    // qsys << "\tadd_connection pcie_ip.pcie_core_reset " << moduleName
    //      << "_$i.clockreset_reset\n";

    qsys << "\t# Connect clock to bridge\n";
    qsys << "\tadd_connection clk_0.clk " << moduleName << "_accel_mem_bridge_"
         << AccelCount << ".clock\n";
    qsys << "\tadd_connection clk_0.clk_reset " << moduleName
         << "_accel_mem_bridge_" << AccelCount << ".reset\n";
    // qsys << "\tadd_connection pcie_ip.pcie_core_reset " << moduleName
    //      << "_accel_mem_bridge_$i.reset\n";

    // Connect PCIe to each accelerator
    qsys << "\t# Connect PCIe to each accelerator\n";
    qsys << "\tadd_connection riffa_avalon_0.avalon_master " << moduleName
         << "_" << AccelCount << ".s1\n";
    qsys << "\tset_connection_parameter_value riffa_avalon_0.avalon_master/"
         << moduleName << "_" << AccelCount
         << ".s1 baseAddress \"[expr $baseAddress]\"\n";

    // Connect JTAG_to_FPAG to accelerators
    qsys << "\t# Connect JTAG_to_FPAG to each accelerator\n";
    qsys << "\tadd_connection JTAG_to_FPGA.master " << moduleName << "_"
         << AccelCount << ".s1\n";
    qsys << "\tset_connection_parameter_value JTAG_to_FPGA.master/"
         << moduleName << "_" << AccelCount
         << ".s1 baseAddress \"[expr $baseAddress]\"\n";

    // Connect each accelerator to the bridge
    qsys << "\t# Connect each accelerator to the bridge\n";
    qsys << "\tadd_connection " << moduleName << "_" << AccelCount << ".ACCEL "
         << moduleName << "_accel_mem_bridge_" << AccelCount << ".accel\n";
    qsys << "\tset_connection_parameter_value " << moduleName << "_"
         << AccelCount << ".ACCEL/" << moduleName << "_accel_mem_bridge_"
         << AccelCount << ".accel baseAddress \"0x0\"\n";

    // Connect the bridge to on-chip memory
    qsys << "\t# Connect the bridge to mem\n";
    qsys << "\tadd_connection " << moduleName << "_accel_mem_bridge_"
         << AccelCount << ".mem onchip_memory2_0.s1\n";
    qsys << "\tset_connection_parameter_value " << moduleName
         << "_accel_mem_bridge_" << AccelCount
         << ".mem/onchip_memory2_0.s1 baseAddress \"0x20000000\"\n";

    // Connect the bridge to ddr3
    qsys << "\t# Connect the bridge to ddr3\n";
    qsys << "\tadd_connection " << moduleName << "_accel_mem_bridge_"
         << AccelCount << ".mem mm_clock_crossing_bridge_0.s0\n";
    qsys << "\tset_connection_parameter_value " << moduleName
         << "_accel_mem_bridge_" << AccelCount
         << ".mem/mm_clock_crossing_bridge_0.s0 baseAddress \"0x40000000\"\n";
}

// include common signals signals for processor such as top, decode, branch, and
// cache signals and configure the modelsim wave file
void SwOnly::initWaveFile(raw_ostream &wave) {

    // add processor, instruction/data cache signals to waves
    //    if (LEGUP_CONFIG->getParameterInt("USE_QSYS")) {
    //        // waves for QSYS generated testbench
    //        wave << "onerror {resume}\n"
    //             << "quietly WaveActivateNextPane {} 0\n"
    //             << "add wave -noupdate -label clk -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/clk\n"
    //             << "add wave -noupdate -label pc -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/core/pc\n"
    //             << "add wave -noupdate -label ins -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/ins\n"
    //             << "add wave -noupdate -label reset_n -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/reset\n"
    //             << "add wave -noupdate -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/*\n"
    //             << "add wave -noupdate -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/data_cache_0/*\n"
    //             << "add wave -noupdate -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/data_cache_0/state\n"
    //             << "add wave -noupdate -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/InsCache/*\n"
    //             << "add wave -noupdate -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/InsCache/state\n"
    //             << "add wave -noupdate -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/core/*\n"
    //             << "add wave -noupdate -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/core/de/*\n"
    //             << "add wave -noupdate -radix hexadecimal "
    //                "/tiger_tb/tiger_inst/tiger_top_0/core/de/b/*\n\n";
    //
    //        if (lockUsed) {
    //            wave << "add wave -noupdate -radix hexadecimal "
    //                    "/tiger_tb/tiger_inst/mutex_0/*\n\n";
    //        }
    //        if (barrierUsed) {
    //            wave << "add wave -noupdate -radix hexadecimal "
    //                    "/tiger_tb/tiger_inst/legup_barrier_0/*\n\n";
    //        }
    //
    //        // add multi-ported caches to waves if they are used
    //        for (int i = 0; i < dcacheway; i++) {
    //            if (dcacheType == "MP") {
    //                wave << "add wave -noupdate -radix hexadecimal "
    //                        "/tiger_tb/tiger_inst/data_cache_0/loop_instantiateRAM["
    //                     << i << "]/dcacheMemIns/*\n\n";
    //            } else if (dcacheType == "LVT") {
    //                wave << "add wave -noupdate -radix hexadecimal "
    //                        "/tiger_tb/tiger_inst/data_cache_0/"
    //                        "loop_instantiateRAMArbiter[" << i
    //                     << "]/dcacheMemIns/*\n\n";
    //                wave << "add wave -noupdate -radix hexadecimal "
    //                        "/tiger_tb/tiger_inst/data_cache_0/"
    //                        "loop_instantiateRAMArbiter[" << i
    //                     << "]/cacheWrite_arbiter/*\n\n";
    //            }
    //        }
    //    } else {
    // waves for SOPC generated testbench
    wave << "onerror {resume}\n"
         << "quietly WaveActivateNextPane {} 0\n"
         << "add wave -noupdate -label clk -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/clk\n"
         << "add wave -noupdate -label pc -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/core/pc\n"
         << "add wave -noupdate -label ins -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/ins\n"
         << "add wave -noupdate -label reset_n -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/reset\n"
         << "add wave -noupdate -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/*\n"
         << "add wave -noupdate -radix hexadecimal "
            "/test_bench/DUT/the_data_cache_0/data_cache_0/*\n"
         << "add wave -noupdate -radix hexadecimal "
            "/test_bench/DUT/the_data_cache_0/data_cache_0/state\n"
         << "add wave -noupdate -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/*\n"
         << "add wave -noupdate -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/InsCache/state\n"
         << "add wave -noupdate -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/core/*\n"
         << "add wave -noupdate -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/core/de/*\n"
         << "add wave -noupdate -radix hexadecimal "
            "/test_bench/DUT/the_tiger_top_0/tiger_top_0/core/de/b/*\n\n";

        if (lockUsed) {
            wave << "add wave -noupdate -radix hexadecimal "
                    "/test_bench/DUT/the_mutex_0/mutex_0/*\n\n";
        }
        if (barrierUsed) {
            wave << "add wave -noupdate -radix hexadecimal "
                    "/test_bench/DUT/the_legup_barrier_0/legup_barrier_0/*\n\n";
        }

        // add multi-ported caches to waves if they are used
        for (int i = 0; i < dcacheway; i++) {
            if (dcacheType == "MP") {
                wave << "add wave -noupdate -radix hexadecimal "
                        "/test_bench/DUT/the_data_cache_0/data_cache_0/"
                        "loop_instantiateRAM[" << i << "]/dcacheMemIns/*\n\n";
            } else if (dcacheType == "LVT") {
                wave << "add wave -noupdate -radix hexadecimal "
                        "/test_bench/DUT/the_data_cache_0/data_cache_0/"
                        "loop_instantiateRAMArbiter[" << i
                     << "]/dcacheMemIns/*\n\n";
                wave << "add wave -noupdate -radix hexadecimal "
                        "/test_bench/DUT/the_data_cache_0/data_cache_0/"
                        "loop_instantiateRAMArbiter[" << i
                     << "]/cacheWrite_arbiter/*\n\n";
            }
        }
        //    }
}

// adding accelerator_top signals to modelsim wave file
void SwOnly::addAcceltoWaveFile(raw_ostream &wave, Function *F,
                                int AccelCount) {

    std::string AccelName = F->getName();
    stripInvalidCharacters(AccelName);
    wave << "add wave -noupdate -radix hexadecimal /test_bench/DUT/the_"
         << AccelName << "_" << AccelCount << "/";
    wave << AccelName << "_" << AccelCount << "/*\n\n";
}

// finishing modelsim wave file with some configurations
void SwOnly::finishWaveFile(raw_ostream &wave) {

    wave << "TreeUpdate [SetDefaultTree]\n"
         << "configure wave -namecolwidth 556\n"
         << "configure wave -valuecolwidth 145\n"
         << "configure wave -justifyvalue left\n"
         << "configure wave -signalnamewidth 0\n"
         << "configure wave -snapdistance 10\n"
         << "configure wave -datasetprefix 0\n"
         << "configure wave -rowmargin 4\n"
         << "configure wave -childrowmargin 2\n"
         << "configure wave -gridoffset 0\n"
         << "configure wave -gridperiod 1\n"
         << "configure wave -griddelta 40\n"
         << "configure wave -timeline 0\n"
         << "configure wave -timelineunits ns\n"
         << "WaveRestoreZoom {109112943 ps} {109266688 ps}\n"
         << "update\n";
}

// get pthread_create calls
void SwOnly::getPthreadFunction(CallInst *CI) {

    // find the function pointer for the actual pthread function
    Function *pthreadFuncPtr = CI->getCalledFunction();
    assert(pthreadFuncPtr);

    // get the number of threads
    int numThreads = getMetadataInt(CI, "NUMTHREADS");

    addAcceleratedFcts(pthreadFuncPtr, getWrapperType(CI), numThreads);
}

// given a call instruction, get the function type from metadata
// and return the corresponding wrapperType
wrapperType SwOnly::getWrapperType(CallInst *CI) {

    wrapperType type;
    std::string functionType = getMetadataStr(CI, "TYPE");

    if (functionType == "legup_wrapper_pthreadcall")
        type = pthreadcall;
    else if (functionType == "legup_wrapper_pthreadpoll")
        type = pthreadpoll;
    else if (functionType == "legup_wrapper_omp")
        type = ompcall;
    else
        type = seq;

    return type;
}

void SwOnly::generateParallelAccelConfigs() {

    formatted_raw_ostream *Out;
    std::string fileName = "parallelaccels.tcl";
    Out = GetOutputStream(fileName);
    assert(Out);
    raw_ostream &tcl = *Out;
    wrapperType type;

    if (!AcceleratedFcts.empty()) {
        for (std::vector<accelFcts>::const_iterator I = AcceleratedFcts.begin(),
                                                    E = AcceleratedFcts.end();
             I != E; ++I) {
            type = (*I).type;
            if (type == pthreadcall || type == ompcall) {
                std::string funcName = ((*I).fct)->getName();
                tcl << "set_parallel_accelerator_function \"" << funcName
                    << "\"\n";
            }
        }
    }

    delete Out;
}
/*
// returns true if it is a call to a parallel function
// false otherwise
bool SwOnly::isaCalltoParallelFunction(CallInst *CI) {

    std::string functionType = getMetadataStr(CI, "TYPE");

    if (functionType == "legup_wrapper_pthreadcall"
     || functionType == "legup_wrapper_pthreadpoll"
     || functionType == "legup_wrapper_omp"
     || functionType == "omp_function" )
        return true;
    else
        return false;
}
*/
void SwOnly::printCacheParametersFile() {

    // make file
    formatted_raw_ostream *file;
    std::string fileName = "cache_parameters.v";
    file = GetOutputStream(fileName);
    assert(file);
    raw_ostream &Out = *file;

    // assign the bitwidth of burst count
    int DburstCountWidth =
        6; // these are hard-coded to 10 bits for now (up to 1024 bursts)
    int IburstCountWidth = 6;
    int burstCountWidth_divider;
    int sdramWidth;

    if (FPGABoard == "DE4") {
        burstCountWidth_divider = 32;
        sdramWidth = 256;
    } else {
        burstCountWidth_divider = 4;
        sdramWidth = 32;
    }

    std::string NUMPORTS;
    if (dcacheports == 2) {
        NUMPORTS = "TWO_PORTS";
    } else if (dcacheports == 4) {
        NUMPORTS = "FOUR_PORTS";
    }

    Out << "`ifndef CACHE_PARAMETERS_H\n"
        << "`define CACHE_PARAMETERS_H\n\n";

    // defines cache parameters
    Out << "//define which board to use\n"
        << "`define " << FPGABoard << "\n\n";

    Out << "//defines the number of ports to the cache\n"
        << "//1 when only the processor, 2 when accelerator present\n"
        << "`define NUM_DCACHE_PORTS " << dcacheports << "\n"
        << "`define NUM_ICACHE_PORTS 1\n\n"

        << "//this is defined if an accelerator is present\n"
        << "`define ACCELERATOR_PORT\n"
        << "`define " << NUMPORTS << "\n\n";

    if (multiportedCache) {
        Out << "`define " << dcacheType << "\n";
    }

    Out << "//defines the associativity of the cache\n"
        << "`define DWAYS " << dcacheway << "\n"
        << "`define IWAYS " << icacheway << "\n\n";

    Out << "//define the cache size\n"
        << "`define DCACHE_SIZE " << dcachenumlines << "\n" // 1024-bit bus
        << "`define DBLOCKSIZE " << dcachelinesize << "\n"
        << "`define ICACHE_SIZE " << icachenumlines << "\n"
        << "`define IBLOCKSIZE " << icachelinesize << "\n\n"

        << "//external memory bus width (256 = DDR2 on DE4, 32 = SDRAM on "
           "DE2)\n"
        << "`define SDRAM_WIDTH " << sdramWidth << "\n"
        << "`define BURSTCOUNT_DIV " << burstCountWidth_divider << "\n"
        << "`define DBURSTCOUNTWIDTH " << DburstCountWidth << "\n"
        << "`define IBURSTCOUNTWIDTH " << IburstCountWidth << "\n\n";

    Out << "//other definitions\n"
        << "`define BYTE 8\n"
        << "`define BYTEEN_WIDTH `SDRAM_WIDTH/`BYTE\n\n"

        << "`define MP_cacheSize		`DCACHE_SIZE //log2(number of lines in "
           "cache)\n"
        << "`define MP_cacheDepth		(2 ** `DCACHE_SIZE) //number of lines "
           "in cache\n"
        << "`define MP_blockSize  `DBLOCKSIZE\n"
        << "`define MP_blockSizeBits	(8*(2**`MP_blockSize)) //total data "
           "bits per cache line\n"
        << "`define MP_cacheWidth		(`MP_blockSizeBits + `MP_tagSizeBits + "
           "1) //total bits per cache line\n\n"

        << "`define WORD_WIDTH	   `MP_cacheWidth\n"
        << "`define WORD			 [`WORD_WIDTH-1:0]\n\n"

        << "`define MEM_ADDR_WIDTH   `MP_cacheSize\n"
        << "`define MEM_ADDR		 [`MEM_ADDR_WIDTH-1:0]\n\n"

        << "`define MEM_DEPTH		`MP_cacheDepth\n"
        << "`define MEM			  [`MEM_DEPTH-1:0]\n\n"

        << "`define MP_tagSizeBits		31\n"
        << "`define HIGH 1'b1\n"
        << "`define LOW  1'b0\n\n"

        << "// Used for write enables\n"
        << "`define READ  `LOW\n"
        << "`define WRITE `HIGH\n\n"

        << "// Multipumping phases\n"
        << "`define PHASE_WIDTH	 1\n"
        << "`define PHASE		   [`PHASE_WIDTH-1:0]\n\n"

        << "`define PHASE_0 `PHASE_WIDTH'd0\n"
        << "`define PHASE_1 `PHASE_WIDTH'd1\n\n"

        << "`define BYTE_EN [((2**`MP_blockSize)+4)-1:0]\n\n"

        << "// Table pointing to which bank holds live register value\n"
        << "`define LVT_ENTRY_WIDTH   2\n"
        << "`define LVT_ENTRY		 [`LVT_ENTRY_WIDTH-1:0]\n\n"

        << "// One entry for each register\n"
        << "`define LVT_DEPTH		 (1 << `MEM_ADDR_WIDTH)\n\n"

        << "// This changes with number of ports in memory\n"
        << "`define ACCEL_0 `LVT_ENTRY_WIDTH'd0\n"
        << "`define ACCEL_1 `LVT_ENTRY_WIDTH'd1\n"
        << "`define ACCEL_2 `LVT_ENTRY_WIDTH'd2\n"
        << "`define ACCEL_3 `LVT_ENTRY_WIDTH'd3\n\n"

        << "`define HIGH 1'b1\n"
        << "`define LOW  1'b0\n\n"

        << "// Used for various write enables\n"
        << "`define READ  `LOW\n"
        << "`define WRITE `HIGH\n\n"

        << "`endif\n";

    delete file;
}

void SwOnly::setdefaultCacheParameters(const std::string cacheType,
                                       const int cachesize, int &cachelinesize,
                                       int &cacheway, int &cachenumlines) {

    // check that they are properly defined
    // the parameters for a type of cache need to be either all defined or none
    // defined
    // if none defined, they will use default values
    if (cachesize == 0 && cachelinesize == 0 &&
        cacheway ==
            0) { // if none of the parameters are defined, assign default values
        cacheway = 1;
        if (FPGABoard == "DE4") {
            cachenumlines = 9;
            cachelinesize = 7;
        } else {
            cachenumlines = 9;
            cachelinesize = 4;
        }
    } else if (cachesize != 0 && cachelinesize != 0 &&
               cacheway != 0) { // if they are all defined, change to log2 base
        cachenumlines = log2((cachesize * 1024) / (cachelinesize * cacheway));
        cachelinesize = log2(cachelinesize);
    } else {
        errs() << "If any of the parameters are defined for " << cacheType
               << " cache, they all need to be defined!\n";
        assert(0);
    }
}

void SwOnly::checkMaximumLinesize(const std::string cacheType,
                                  const int cachelinesize, const int maxValue) {
    if (cachelinesize > maxValue) {
        int maxBytes = pow(2, maxValue);
        errs() << cacheType << " cache line size too large! The maximum "
               << cacheType << " cache line size is " << maxBytes
               << " bytes!\n";
        assert(0);
    }
}

void SwOnly::checkLinesizes() {
    // check that the linesizes are smaller than the maximum allowed.
    // the burstcount width is set to 6 bits (since DDR2 controller allows max
    // burst of 64),
    // the maximum line size for DE4 is 256*64 = 16384 bits = 2048 bytes ->
    // log2(2048) = 11
    // the maximum line size for DE2 is 32*64 = 2048 bits = 256 bytes ->
    // log2(256) = 8
    if (FPGABoard == "DE4") {
        checkMaximumLinesize("data", dcachelinesize, 11);
        checkMaximumLinesize("instruction", icachelinesize, 11);
    } else {
        checkMaximumLinesize("data", dcachelinesize, 8);
        checkMaximumLinesize("instruction", icachelinesize, 8);
    }
}

void SwOnly::setCacheParameters() {

    // check that they are properly defined
    // the parameters for a type of cache need to be either all defined or none
    // defined
    // if none defined, they will use default values
    setdefaultCacheParameters("data", dcachesize, dcachelinesize, dcacheway,
                              dcachenumlines);
    setdefaultCacheParameters("instruction", icachesize, icachelinesize,
                              icacheway, icachenumlines);

    // set number of data cache ports
    if (dcacheports == 0) {
        dcacheports = 2; // default value is 2 if not specified
    }

    // check that the linesizes are smaller than the maximum allowed.
    // the burstcount width is set to 6 bits (since DDR2 controller allows max
    // burst of 64),
    // the maximum line size for DE4 is 256*64 = 16384 bits = 2048 bytes ->
    // log2(2048) = 11
    // the maximum line size for DE2 is 32*64 = 2048 bits = 256 bytes ->
    // log2(256) = 8
    checkLinesizes();
}

#if 0
// this checks over the mutex data structure
// if there are OpenMP internal accelerators (inside pthread accelerator) which
// use omp_critical
// then multiple locks should be instantiated for each indepedent avalon
// accelerator.
// this has be done after all functions have been replaced, so that all pthread
// functions would be included in pthreadFunctions
// NOTE: this can't be used right now, since in the hw.ll, there is only one
// copy of the function.
// since replacements are done statically compiled at time, the index number
// which determines which mutex to communicate with has to be the same across
// all internal accelerators within a pthread accelerators
void SwOnly::setMutexData() {

    //look through mutex map for OMP_atomic_
    //get the mutex number, then look at mutexFunctionMap to get its caller functions
    //check through the caller functions to see if there is a parent pthread function
    //if there is check in pthreadFunction set too see how many times that function is accelerated
    //if it is more than one, add more instances of the mutex to the data structures (mutexMap and mutexFunctionMap)
    for (std::map<std::string, int>::iterator it = mutexMap.begin(); it != mutexMap.end(); it++) {
        //copy the portion of string before the index (OMP_atomic_)
        std::string fullMutexName = (*it).first;
        std::string mutexName = fullMutexName.substr(0, 11);
        if (mutexName == "OMP_atomic_") {
            int mutexNum = it->second;
            std::set<Function*> callerSet = mutexFunctionMap[mutexNum];
            //look through the caller set to see if there are any pthread functions
            for (std::set<Function*>::iterator it2 = callerSet.begin(); it2 != callerSet.end(); it2++) {
                std::map<Function*,int>::iterator it3;
                it3 = pthreadFunctions.find(*it2);
                //if there is a parent function which is a pthread function
                if (it3 != pthreadFunctions.end()) {
                    //get the times the pthread function is accelerated
                    int numAccelerated = it3->second;
                    if (numAccelerated > 1) {
                        //if it is accelerated more than once, add more mutexes to the data structure
                        //one less than the number of pthreads since it has been already added once
                        for (int i=1; i<numAccelerated; i++) {
                            std::string newMutexName = "OMP_atomic_" + utostr(numOMPatomic);
                            numOMPatomic++;

                            int newMutexNum = mutexMap.size();
                            mutexMap.insert( std::pair<std::string, int>(mutexName, newMutexNum));
                            mutexFunctionMap[newMutexNum] = callerSet;
                        }
                    }
                }
            }
        }
    }
}
#endif

// Generate call to pcie library function
// pcie_read(int* buffer, size_t length, int offset)
void SwOnly::generatePCIeReadCall(IRBuilder<> &builder, Value *buffer,
                          Value *length, Value *offset) {

    assert(PCIeFunctions.find("pcie_read") != PCIeFunctions.end());
    generatePCIeCommunicationCall(builder, PCIeFunctions["pcie_read"],
                                  buffer, length, offset);
}

// Generate call to library function
// pcie_write(int* buffer, size_t length, int offset)
void SwOnly::generatePCIeWriteCall(IRBuilder<> &builder, Value *buffer,
                           Value *length, Value *offset) {

    assert(PCIeFunctions.find("pcie_write") != PCIeFunctions.end());
    generatePCIeCommunicationCall(builder, PCIeFunctions["pcie_write"],
                                  buffer, length, offset);
}

// generate call to pcie transfer function
void SwOnly::generatePCIeCommunicationCall(IRBuilder<> &builder, Function *f,
                                   Value *buffer, Value *length,
                                   Value *offset) {
    assert(f != NULL);
    assert(buffer != NULL);
    assert(length != NULL);
    assert(offset != NULL);

    Value *_buffer = buffer;
    Value *_length = length;
    Value *_offset = offset;

    // cast parameters to appropriate types if they doesn't match
    if (buffer->getType() != Type::getInt32PtrTy(getGlobalContext()))
        _buffer = builder.CreatePointerCast(
            buffer, Type::getInt32PtrTy(getGlobalContext()));

    if (length->getType() != Type::getInt32Ty(getGlobalContext()))
        _length = builder.CreateIntCast(
            length, Type::getInt32Ty(getGlobalContext()), true);

    if (offset->getType() != Type::getInt32Ty(getGlobalContext()))
        _offset = builder.CreateIntCast(
            offset, Type::getInt32Ty(getGlobalContext()), true);

    // generate the actual call
    builder.CreateCall3(f, _buffer, _length, _offset);
}

void SwOnly::generatePthreadYieldCall(IRBuilder<> &builder)
{
    assert(PCIeFunctions.find("pthread_yield") != PCIeFunctions.end());
    builder.CreateCall(PCIeFunctions["pthread_yield"]);
}

} // end of legup namespace
