//===------ LoopPipeline.cpp ---------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Loop Pipelining
//
//===----------------------------------------------------------------------===//

/*
#define DEBUG_TYPE "polly-codegen"

#include "polly/LinkAllPasses.h"
#include "polly/Support/GICHelper.h"
#include "polly/Support/ScopHelper.h"
#include "polly/Cloog.h"
#include "polly/Dependences.h"
#include "polly/ScopInfo.h"
#include "polly/TempScopInfo.h"
*/
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/FileSystem.h"
#include "SchedulerDAG.h"
#include "Scheduler.h"
#include "utils.h"
#include "Ram.h"
#include "LegupConfig.h"
#include "ModuloScheduler.h"

//#define CLOOG_INT_GMP 1
//#include "cloog/cloog.h"
//#include "cloog/isl/cloog.h"

#include <sstream>
#include <iomanip>
#include <vector>
#include <utility>
#include <lp_lib.h>
#include <math.h>
#include <algorithm>

#include "SDCSolver.h"
#include "SDCModuloScheduler.h"

// using namespace polly;
using namespace llvm;
using namespace legup;

struct isl_set;

namespace llvm {
void initializeLoopPipelinePass(llvm::PassRegistry &);
}

namespace {

class LoopPipeline : public LoopPass {
    // Region *region;
    // Scop *S;
    DominatorTree *DT;
    ScalarEvolution *SE;
    // ScopDetection *SD;
    // CloogInfo *C;
    LoopInfo *LI;
    DataLayout *TD;
    SDCModuloScheduler IMS;

  public:
    static char ID;

    // SDCModuloScheduler() : ScopPass(ID) {
    LoopPipeline() : LoopPass(ID) {}

    bool runOnLoop(Loop *L, LPPassManager &LPM) {

        IMS.LI = &getAnalysis<LoopInfo>();
        IMS.AA = &getAnalysis<AliasAnalysis>();
        IMS.SE = &getAnalysis<ScalarEvolution>();

        return IMS.runOnLoop(L, LPM);
    }

    using llvm::Pass::doInitialization;
    bool doInitialization(Loop *L, LPPassManager &LPM) override {
        return false;
    }

    using llvm::Pass::doFinalization;

    bool doFinalization() override { return false; }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<LoopInfo>();
        AU.addRequired<AliasAnalysis>();
        AU.addRequired<ScalarEvolution>();
        // AU.setPreservesAll();
        // does not preserve loopinfo?
        // AU.addPreserved<MemoryDependenceAnalysis>();
        // AU.addPreserved<AliasAnalysis>();

        /*
        AU.addRequired<CloogInfo>();
        AU.addRequired<Dependences>();
        AU.addRequired<DominatorTree>();
        AU.addRequired<RegionInfo>();
        AU.addRequired<ScopDetection>();
        AU.addRequired<ScopInfo>();
        AU.addRequired<DataLayout>();

        AU.addPreserved<CloogInfo>();
        AU.addPreserved<Dependences>();
        AU.addPreserved<DominatorTree>();
        AU.addPreserved<PostDominatorTree>();
        AU.addPreserved<ScopDetection>();
        AU.addPreserved<ScalarEvolution>();
        AU.addPreserved<RegionInfo>();
        AU.addPreserved<TempScopInfo>();
        AU.addPreserved<ScopInfo>();
        AU.addPreservedID(IndependentBlocksID);
        */
    }
};
}

/*
*/
char LoopPipeline::ID = 1;

INITIALIZE_PASS_BEGIN(LoopPipeline, "loop-pipeline", "LegUp Loop Pipelining",
                      false, false)
// INITIALIZE_PASS_DEPENDENCY(SchedulerDAG)
INITIALIZE_PASS_END(LoopPipeline, "loop-pipeline", "LegUp Loop Pipelining",
                    false, false)

static RegisterPass<LoopPipeline> Z("loop-pipeline", "LegUp Loop Pipelining");

/*
using namespace llvm;
char LoopPipeline::ID = 0;
INITIALIZE_PASS(LoopPipeline, "modulo-schedule",
                "LegUp Iterative Modulo Scheduling PrePass",
                false, false)
namespace llvm {

LoopPass *createSDCModuloSchedulerPass() {
  return new LoopPipeline();
}
*/

//}
/*
namespace polly {
Pass* createModuloSchedulePass() {
    //return 0;
  return new LoopPipeline();
}
}
*/
