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

#ifndef LEGUP_SWONLY_H
#define LEGUP_SWONLY_H

#include "utils.h"
#include <set>

using namespace llvm;

namespace legup {

// SwOnly - Replace accelerated functions with wrappers and create
// legup_wrappers.c. Create tcl files (legup_sopc.tcl, _hw.tcl) to
// control SOPC builder to add the accelerator to the system.
class SwOnly : public ModulePass {
  public:
    static char ID; // Pass identification, replacement for typeid
    SwOnly()
        : ModulePass(ID),
          // initialize board parameter
          FPGABoard(LEGUP_CONFIG->getFPGABoard()),
          // initialize processor parameters
          ProcessorArchitecture(
              LEGUP_CONFIG->getParameter("SYSTEM_PROCESSOR_ARCHITECTURE")),
          // initialize system parameters
          SysClkModule(LEGUP_CONFIG->getParameter("SYSTEM_CLOCK_MODULE")),
          SysClkInterface(LEGUP_CONFIG->getParameter("SYSTEM_CLOCK_INTERFACE")),
          SysRstModule(LEGUP_CONFIG->getParameter("SYSTEM_RESET_MODULE")),
          SysRstInterface(LEGUP_CONFIG->getParameter("SYSTEM_RESET_INTERFACE")),
          SysMemModule(LEGUP_CONFIG->getParameter("SYSTEM_MEMORY_MODULE")),
          SysMemInterface(
              LEGUP_CONFIG->getParameter("SYSTEM_MEMORY_INTERFACE")),
          SysProcName(LEGUP_CONFIG->getParameter("SYSTEM_PROCESSOR_NAME")),
          SysProcDataMaster(
              LEGUP_CONFIG->getParameter("SYSTEM_PROCESSOR_DATA_MASTER")),
          // initialize constant cache parameters
          dcachesize(LEGUP_CONFIG->getCacheParameters("dcachesize")),
          icachesize(LEGUP_CONFIG->getCacheParameters("icachesize")),
          // initialize other parameters
          sharedMemory(false) {
        // initialize other cache parameters (these are not constant since if
        // they are not defined, they need to be set to default values later)
        dcachelinesize = LEGUP_CONFIG->getCacheParameters("dcachelinesize");
        dcacheway = LEGUP_CONFIG->getCacheParameters("dcacheway");
        dcacheports = LEGUP_CONFIG->getCacheParameters("dcacheports");
        icachelinesize = LEGUP_CONFIG->getCacheParameters("icachelinesize");
        icacheway = LEGUP_CONFIG->getCacheParameters("icacheway");
        dcacheType = LEGUP_CONFIG->getDCacheType();

        if (dcacheports > 2) {
            multiportedCache = true;
        } else {
            multiportedCache = false;
        }

        // DE2 processor does not support multi-ported caches due to CycloneII
        // chip size
        if (FPGABoard == "DE2") {
            multiportedCache = false;
            dcacheports = 2;
            dcacheType = "";
        }

        // ompUsed = false;
        lockUsed = false;
        barrierUsed = false;
        pthreadHeaderIncluded = false;
        perfCounterUsed = false;
        parallelAccelUsed = false;
        numPerfCounter = 0;
        numOMPatomic = 0;
        ompAtomicUsed = false;
    }

    virtual bool doInitialization(Module &M);
    virtual bool doFinalization(Module &M);
    virtual bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<LoopInfo>();
    }

  private:
    bool replaceHwCallWithWrapper(CallInst *CI, Function *calledFunction,
                                  wrapperType type);

    void printWrapperPCIe(raw_ostream &Out, Function *F,
                          const unsigned long long startAddress,
                          const wrapperType type, const int wrapperNum,
                          const int numAccelerated, const int AddressSize);
    void printWrapperPrototype(raw_ostream &Out, Function *F, bool &voidtype,
                               const wrapperType type);
    void printWrapperAccelID(raw_ostream &Out, Function *F,
                             const wrapperType type, const int numAccelerated,
                             const int AddressSize);
    void printDATApointer(raw_ostream &Out, const std::string AccelName,
                          const Type *rT, unsigned long long &CurAddr);
    void printSTATUSpointer(raw_ostream &Out, const std::string AccelName,
                            unsigned long long &CurAddr);
    void printARGpointers(raw_ostream &Out, const std::string AccelName,
                          const wrapperType type, Function *F,
                          unsigned long long &CurAddr);
    void printHWtcl(Function *F, wrapperType type, int AddressBusWidth);
    void printSopcFileInitial(raw_ostream &sopc);
    void printSopcFileAPIcores(raw_ostream &sopc);
    void printSopcFile(raw_ostream &sopc, Function *F, wrapperType type,
                       unsigned long long baseAddr, int AccelCount,
                       int AccelIndex);
    void printQSYSFileInitial(raw_ostream &qsys);
    void printQSYSFileAPIcores(raw_ostream &qsys);
    void printQSYSFile(raw_ostream &qsys, Function *F, wrapperType type,
                       unsigned long long baseAddr, int AccelCount,
                       int AccelIndex, int addrBusWidth);
    void printQSYSFilePCIe(raw_ostream &sopc, Function *F,
                           unsigned long long baseAddr, int AccelCount,
                           int addressSize);
    void printQSYSFilePCIeShared(raw_ostream &sopc, Function *F,
                                 unsigned long long baseAddr, int AccelCount,
                                 int addressSize);
    void printCacheParametersFile();
    void printCacheHWtcl();
    bool printSWfilesPCIe();
    void printQSYSFileEnd(raw_ostream &out);
    void printSopcFileEnd(raw_ostream &out);

    bool generateWrappersAndPrintSideFiles();
    void generateWrapper(Function *F, const wrapperType type,
                         const int StartAddr, const int AddressSize,
                         const int numAccelerated);
    void generatePthreadCallingWrapper(Function *F, const wrapperType type,
                                       int StartAddr, const int AddressSize,
                                       const int numAccelerated);
    void generatePthreadPollingWrapper(Module &M, bool pthreadReturn);
    Value *generateOMPWrapperCallingLoop(IRBuilder<> &builder, Function *F,
                                         BasicBlock *loopBB, BasicBlock *prevBB,
                                         const int numAccelerated,
                                         const int AddressSize, int &StartAddr);
    Value *generateOMPWrapperPollingLoop(IRBuilder<> &builder, Function *F,
                                         BasicBlock *prevBB,
                                         const int numAccelerated,
                                         const int AddressSize,
                                         const int StatusAddr);
    void generatePthreadCallingWrapperStoreAccelAddr(IRBuilder<> &builder,
                                                     Value *threadVarPtr,
                                                     const int intBaseAddr,
                                                     Value *offset);
    void generateOMPWrapper(Function *F, const wrapperType type, int StartAddr,
                            const int AddressSize, const int numAccelerated);
    void generateSequentialWrapper(Function *F, const wrapperType type,
                                   int StartAddr);
    void generatePCIeWrapper(Function *F, const wrapperType type,
                                   int StartAddr, int numAccelerated);
    Value * getAccel(IRBuilder<> &builder, Value * sched);
    void freeAccel(IRBuilder<> &builder, Value * sched, Value * handle);
    GlobalVariable* createSchedulerStorage(Module &M, int totalAccels, std::string name);
    Function* generatePCIeWrapperPrototype(Function *F, const wrapperType type);
    void setUpPCIeAcceleratorArguments(IRBuilder<> &builder, Function * wrapperF,
        Value* paramsAddr);
    void sendStartSignalAndWaitForResult(IRBuilder<> &builder, Function * wrapperF,
        Value* statusAddr);
    void generateWrapperArgumentTransfer(IRBuilder<> &builder, Value *argPtr,
                                         Value *addrOffset, int address);
    Value *generateGEPfromIntAddressandValueOffset(IRBuilder<> &Builder,
                                                   const int addr,
                                                   Value *offset,
                                                   const int bitWidth = 32);
    void generateParallelAccelConfigs();
    Value *generatePtrfromIntAddress(IRBuilder<> &builder, const int addr,
                                     const int bitWidth = 32);
    Value *generateAccelOffsetfromThreadIDandAddrSize(IRBuilder<> &builder,
                                                      Value *threadID,
                                                      int addrSize);
    void generateWrapperGiveStartSignaltoAccel(IRBuilder<> &builder,
                                               const int intBaseAddr,
                                               Value *offset);
    void generateWrapperStoreArgument(IRBuilder<> &builder, Value *value,
                                      const int baseAddr, Value *offset,
                                      const int bitWidth = 32,
                                      bool isVolatile = true);
    Value *generateWrapperLoadValue(IRBuilder<> &builder, const int baseAddr,
                                    Value *offset, const int bitWidth = 32,
                                    bool isVolatile = true);
    Value *generateWrapperArgumentStore(IRBuilder<> &builder, Value *arg,
                                        Value *addrOffset, int address);
    BasicBlock *generateBBandSetIncomingBr(IRBuilder<> &builder, Function *F,
                                           const std::string BBName,
                                           bool isConditionalBr = false,
                                           BasicBlock *prevBB = NULL,
                                           Value *cond = NULL);

    unsigned long long calculateMemorySpace(Function *F,
                                            unsigned long long CurAddr,
                                            wrapperType type);
    int calculateAddressSize(unsigned long long currAddr,
                             unsigned long long prevAddr, int &addrBusWidth);
    unsigned long long printMemAddr(raw_ostream &Out, Function *F,
                                    unsigned long long CurAddr,
                                    wrapperType type,
                                    unsigned countOfAccelType = 1);

    void initWaveFile(raw_ostream &wave);
    void addAcceltoWaveFile(raw_ostream &wave, Function *F, int AccelCount);
    void finishWaveFile(raw_ostream &wave);

    void getOMPParallel(Module &M, CallInst *CI);
    void getPthreadFunction(CallInst *CI);
    bool getAndReplaceCalls(Module &M);
    bool getAndReplaceParallelFunctionCalls(Module &M, CallInst *CI,
                                            BasicBlock::iterator &I,
                                            bool &modified);
    bool getOMPFunctions(Module &M, CallInst *CI);
    bool getSynchronizationFunctions(Function *F, CallInst *CI,
                                     Function *calledFunction);
    accelFcts getAccelFcts(Function *F, wrapperType functionType,
                           int numAccelerated = 1);
    wrapperType getWrapperType(CallInst *CI);
    Function *getWrapperFunctionAndDeleteBody(Function *F, wrapperType type);

    bool replacePthreadFunctions(Module &M, CallInst *CI);
    // bool replaceCallsPCIe(Module &M);
    bool replacePthreadFunctionsCalls(Module &M, CallInst *CI);
    bool replaceSequentialFunction(CallInst *CI, Function *calledFunction);

    bool addPerformanceCounters(CallInst *CI, Function *calledFunction);

    //    bool isaCalltoParallelFunction(CallInst *CI);

    CallInst *createNewCalltoPthreadFunction(Module &M, Function *F,
                                             CallInst *CI);

    void setCacheParameters();
    void setdefaultCacheParameters(const std::string cacheType,
                                   const int cachesize, int &cachelinesize,
                                   int &cacheway, int &cachenumlines);
    void checkLinesizes();
    void checkMaximumLinesize(const std::string cacheType,
                              const int cachelinesize, const int maxValue);

    void addAcceleratedFct(accelFcts accel);
    void addAcceleratedFcts(Function *calledFunction,
                            wrapperType functionType = seq,
                            unsigned numAccelerated = 0);
    void updateAcceleratedFct(accelFcts accel);
    void updateAcceleratedFcts(Function *calledFunction,
                               unsigned numAccelerated);
    void updateAccelCount(Function *F, unsigned accelCount);

    // void checkSharedFunction(Function *calledFunction);
    // void identifyPthreadFunctionsPCIe(Module &M, CallInst *CI,
    //                                   unsigned loopTripCount);

    void setMutexData();

    void preserveGlobalVariablesUsedInHW(Module &M);
    void findUsedValues(GlobalVariable *LLVMUsed,
                        SmallPtrSet<GlobalValue *, 8> &UsedValues);

    // variables which hold cache parameters and type of fpga board
    const std::string FPGABoard;
    const std::string ProcessorArchitecture;
    const std::string SysClkModule;
    const std::string SysClkInterface;
    const std::string SysRstModule;
    const std::string SysRstInterface;
    const std::string SysMemModule;
    const std::string SysMemInterface;
    const std::string SysProcName;
    const std::string SysProcDataMaster;
    const int dcachesize, icachesize;
    int dcachelinesize, icachelinesize;
    int dcacheway, icacheway;
    int dcachenumlines, icachenumlines;
    int dcacheports;
    int numPerfCounter;
    int numOMPatomic;
    std::string dcacheType;
    bool multiportedCache;
    bool lockUsed, barrierUsed, perfCounterUsed, parallelAccelUsed,
        ompAtomicUsed;
    bool pthreadHeaderIncluded;

    LoopInfo *LI;
    bool sharedMemory;

    std::map<std::string, int> mutexMap;
    std::map<int, std::set<Function *>> mutexFunctionMap;
    std::map<Function *, int> pthreadFunctions;
    std::map<std::string, Function *> PCIeFunctions;

    void initializePCIeFunctions(Module &M);
    void generatePthreadYieldCall(IRBuilder<> &builder);
    void generatePCIeCommunicationCall(IRBuilder<> &builder, Function *f,
                                       Value *buffer, Value *length,
                                       Value *offset);

    void generatePCIeWriteCall(IRBuilder<> &builder, Value *buffer,
                               Value *length, Value *offset);
    void generatePCIeWriteCall(IRBuilder<> &builder, Value *buffer,
                               const int length, Value *offset) {

        generatePCIeWriteCall(builder, buffer,
            ConstantInt::get(Type::getInt32Ty(getGlobalContext()), length),
            offset);
    }
    void generatePCIeWriteCall(IRBuilder<> &builder, Value *buffer,
                               const int length, const int offset) {
        generatePCIeWriteCall(builder, buffer,
            ConstantInt::get(Type::getInt32Ty(getGlobalContext()), length),
            ConstantInt::get(Type::getInt32Ty(getGlobalContext()), offset));
    }

    void generatePCIeReadCall(IRBuilder<> &builder, Value *buffer,
                              Value *length, Value *offset);
    void generatePCIeReadCall(IRBuilder<> &builder, Value *buffer,
                              const int length, Value *offset) {
        generatePCIeReadCall(builder, buffer,
            ConstantInt::get(Type::getInt32Ty(getGlobalContext()), length), offset);
    }
    void generatePCIeReadCall(IRBuilder<> &builder, Value *buffer,
                              const int length, const int offset) {
        generatePCIeReadCall(builder, buffer,
            ConstantInt::get(Type::getInt32Ty(getGlobalContext()), length),
            ConstantInt::get(Type::getInt32Ty(getGlobalContext()), offset));
    }

    // set storing internal HW functions
    std::set<Function *> internalAccels;
    // vector of structs accelFcts which hold the functions to be accelerated
    std::vector<accelFcts> AcceleratedFcts;
};

} // end of legup namespace

#endif
