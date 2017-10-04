//===-- VerilogWriter.h -----------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// VerilogWriter takes an RTLModule and prints out the corresponding
// Verilog along with necessary memory controllers and avalon signals.
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_VERILOG_WRITER_H
#define LEGUP_VERILOG_WRITER_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "RTL.h"
#include <set>

#include <sstream>

using namespace llvm;

namespace legup {

class HwModule;
class Allocation;
class RAM;

/// VerilogWriter - Prints the Verilog for a RTLModule.
/// Also handles printing memory controller, test suite, and avalon interface
/// @brief VerilogWriter Class
class VerilogWriter {
public:
    //NC changes...
    VerilogWriter(raw_ostream &StreamOut, Allocation *alloc, std::set<const
		  Function*> AcceleratedFcts) : StreamOut(StreamOut), alloc(alloc),
						AcceleratedFcts(AcceleratedFcts), usesSynchronization(false) {}
    VerilogWriter(raw_ostream &StreamOut, Allocation *alloc) : StreamOut(StreamOut),
							 alloc(alloc), usesSynchronization(false) {}
    void print();
    
    void printRTL(const RTLModule *rtl);
    
    //NC changes...    
    void setRTL(const RTLModule* rtl) { this->rtl = rtl;}
    
    void printSignal(const RTLSignal *signal);
    void printRamInstance(RAM *R);
    void printSignalConditionForInstruction(const RTLSignal* signal, const Instruction* I);
    void clearStringStreamBuffer() { this->Out.str(""); }
    std::string getStringStreamOut() { return this->Out.str(); }
    //
    
private:
    void printValue(const RTLSignal *sig,unsigned w=0, bool zeroExtend=false);
    void printValueMinBW(const RTLSignal *sig, unsigned w, bool zeroExtend);

    bool stripRAM(RAM *R);

    void printCaseFSM(const RTLSignal *signal, std::string assignOp);
    void caseConditions(const RTLSignal *condition, 
			std::vector<const RTLSignal *> &clausesToKeep);
    void getStateName(const RTLSignal *condition, std::string &param);

    void printComments(const Instruction *I, std::string prefix="");
    //NC changes.. 
    //void printSignal(const RTLSignal *signal);
    void printIndividualCondition(const RTLSignal* signal, int conditionNum, std::string assignOp, bool printCmnts);
    
    void printConditions(const RTLSignal *signal, std::string assignOp);
    std::string bitsForSignalAndModuleName(const RTLSignal *sig, std::string name);
    void printModuleInstance(std::stringstream &Out, const RTLModule *mod);
    void printMemCtrlModuleHeader();
    void printMemCtrlVariablesSignals(std::string postfix);
    void printMemCtrlVariables();
    void printMemCtrlRAMs();
    void printAddrAlignmentCheck(std::string postfix);
    void printPrevAddr(RAM *R, std::string postfix, std::string name);
    void printAlwaysTrigger(const RTLSignal *signal, const RTLSignal *driver);
    bool isConst(const RTLSignal *sig);
    
    void printTop(const Function * F);
    void printRAMModule();
    void printInferredRAMModule(bool readonly);
    void printAltSyncRAMModule(bool readonly);
    void printPLLModule();
    void printClkFollowerModule();
    void printMultipumpModule();
    void printDebugModule();
    void printDebugModuleInstance(std::string postfix);
    void printDebugModuleSignals(std::string postfix);
    void printArbiterModule();
    void printPriorityArbiterModule();
    void printMemoryControllerSignals(std::string postfix);
    void printMemoryController();
    void printMIFFiles();
    void printSimpleMemoryController();
    bool currentBoardHasPin(std::string pinName);
    void printBoardPortList();
    void printBoardSignalDeclarations();
    void printBoardLogic();
    void printBoardTopSignals();
    void printVerilogAtSpaceSeparatedPaths(char *);
    void printDebuggerInstance();
    void printTraceSchedulerInstance();
    void printDbgStateMuxer();
    void printBoardUtils();
    void printBoardTops();
    void printDE2();
    void printDE4();
    void printHex();
    void printVerilog(std::stringstream &Out, const HwModule *module);
    void printModuleDeclaration(const RTLModule *rtl);
    void printVerilogTestbench();
    void printMemoryVariablesSignals(std::string busName, std::string inputPrefix,
				     std::string outputPrefix, std::string postfix);
    void printMemoryVariables(bool top);
    void printBlankDefaultCase(std::string indent);
    void printRAMTag(RAM *R, std::string postfix);
    void printRAMSignals(std::string postfix);
    void printRAMSignalsHelper(std::string postfix, std::string
			       name, unsigned datawidth, std::string tag, bool isStruct);
    void printPrevTagCase(std::string postfix);
    void printRAMTagCase(std::string postfix);

    void printModuleHeader();
    void printVerilogOperator(const RTLOp *op, unsigned w=0);
    // Detects all placeholders in the string and returns a vector of booleans
    // indicating whether the placeholder is unsigned.
    // Currently only "%u" is considered as unsigned.
    std::vector<bool> parseDisplayString(const std::string &display_string);
    // Replaces C-style placeholder to Verilog placeholder and prints it out.
    void printDisplayString(std::string display_string);
    void printDisplay(const RTLOp *op);
    void printVerilogOpcode(RTLOp::Opcode opcode);

    void printTopHybrid(const Function * F, unsigned dataSize);
    void printMemTag(unsigned dataSize);
    void printGlobalTags(RAM *R);
    void printLocalTags(RAM *R, unsigned dataSize, unsigned &tagIndex);
    void printAvalonInterface(std::string ModuleName);
    int getAvalonBusWidth(const Function * F);
    void printIODeclarations(int AddressBusWidth);
    bool printSignalDeclarations(const Function * F, std::vector<int> &arg_bitwidth);
    void printArgumentSignalDeclarations(const Function * F, std::vector<int> &arg_bitwidth);
    void printAssignStatements(int NumParams, bool return64, bool isParallel);
    void printACCELassignStatements(bool isParallel);
    void printMemoryAssignStatements(int NumParams, bool return64, bool isParallel);
    void printArgsReceivers(int NumParams, const std::vector<int>
			    &arg_bitwidth);
    void printStartDoneSignals();
    void printMemorySignals();
    void printMemoryRegisters(std::string postfix);
    bool printReturnValSignals(bool return64, const Function * F, bool isParallel);
    void printMemoryControllerInstance();
    void printAccelInstance(const Function * F, int NumParams, bool voidtype);
    std::string searchMIPSAddress(RAM *R);
    //std::string parseMIPSdisassembly(const char * disassembly, std::string varName);
    void parseMIPSdisassembly();
    void printModelsimSignals(bool voidtype);
    void printDeclaration(const RTLSignal *signal);
    void printVerilogBitwidthPrefix(const RTLSignal *sig);
    //NC changes...
    //void printRamInstance(RAM *R);
    void printMainInstance(const bool usesPthreads);
    void printLockInstance(int lockIndex);
    void printLockModule();
    void printBarrierInstance(int barrierIndex);
    void printBarrierModule();
    void printMemoryDataReadySignals();
    void printMemoryDataReceivers();
    void printMemoryShiftRegisters(std::string postfix);
    void printMemoryControllerAssignStatements(std::string postfix);
    void printMemoryDataSignals();
    void printMemoryStateMachine();
    void printMemoryDataReceivers(std::string postfix);
    void printMemorySignalDeclarations(std::string postfix);
    void printoffChipMemoryFlags(std::string postfix);
    
    void printSynchronizationControllerVariables();
    void printSynchronizationControllerInstance();
    void printSynchronizationController();
    void printSyncCtrlModuleHeader();
    void printSyncCtrlVariables();
    void printSyncCtrlCoresVariables();
    void printSyncCtrlSignals();
    void printSyncCtrlCoreInstances();
    void printSyncTagCase(std::string syncType);
    void printSyncTag(const std::string syncType, const int index);
    void printSynchronizationControllerShiftReg(std::string postfix);
    void printSynchronizationControllerVariableDeclarations();
    void printSynchronizationControllerMuxLogic();
    std::string getMemoryOutputRegisters();

    std::string indent, indent0;
    raw_ostream &StreamOut;
    std::stringstream Out;
    Allocation *alloc;
    const RTLModule *rtl;
    const std::set<const Function*> AcceleratedFcts;
    
    //std::map<std::string, std::set<std::string> > syncMap;
    std::map<std::string, int> syncMap;
    std::map<std::string, std::string> globalAddresses;
    bool usesSynchronization;

};

} // End legup namespace

#endif
