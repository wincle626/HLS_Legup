//===-- LegupPass.h - Legup Pass --------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the main LLVM pass for Legup
//
//===----------------------------------------------------------------------===//


#ifndef LEGUP_PASS_H
#define LEGUP_PASS_H

#include "llvm/Pass.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/CommandLine.h"
#include "MinimizeBitwidth.h"
#include "MinimizeBitwidth.h"
#include "LVA.h"
#include "LegupConfig.h"
#include "llvm/Analysis/AliasAnalysis.h"
//#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "GenerateRTL.h"
#include <set>

using namespace llvm;

namespace legup {

static std::string LEGUP_VERSION = "4.0";

class Allocation;
class HwModule;
class FunctionSortingWrapper;

/// LegupPass - This class is the LLVM pass that converts LLVM IR into
/// Verilog
/// @brief Legup Backend Pass
class LegupPass : public ModulePass, public InstVisitor<LegupPass> {
public:

    LegupPass(raw_ostream &o) : ModulePass(ID), Out(o), allocation(0) {}

    ~LegupPass();

    virtual const char *getPassName() const { return "LegupPass backend"; }

    /// doInitialization - Allocate RAMs for global variables
    virtual bool doInitialization(Module &M);

    /// runOnFunction - schedule each function and create HwModule object
    bool runOnModule(Module &M);

    /// doFinalization - print the verilog
    virtual bool doFinalization(Module &M);

private:

	void pipelineLabelSanityCheck(Module &M);
    void printVerilog(const std::set<const Function*> &AcceleratedFcts);
    void printResourcesFile(std::string fileName);
    void printBBStats(Function &F);
    std::vector<Function*> getDepthFirstSortedFunctions(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<LoopInfo>();
        AU.addRequired<LiveVariableAnalysis>();
        AU.addRequired<MinimizeBitwidth>();
        AU.addRequired<AliasAnalysis>();
        //if (LEGUP_CONFIG->getParameterInt("LLVM_PROFILE")) {
            //AU.addRequired<ProfileInfo>();
        //}
        AU.setPreservesAll();
    }

    raw_ostream &Out;
    static char ID;
    Allocation *allocation;
    LiveVariableAnalysis *LVA;
    MinimizeBitwidth *MBW;
};

} // End legup namespace

#endif
