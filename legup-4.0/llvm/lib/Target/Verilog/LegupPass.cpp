//===-- LegupPass.cpp -----------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LegupPass object
//
//===----------------------------------------------------------------------===//

#include "Allocation.h"
#include "LegupPass.h"
#include "VerilogWriter.h"
#include "utils.h"
#include "Binding.h"
#include "RTL.h"
#include "GenerateRTL.h"
#include "LegupConfig.h"
#include "Scheduler.h"
#include "ResourceEstimator.h"
//#include "llvm/Analysis/ProfileInfo.h"
#include "Debug.h"
#include <fstream>
//NC changes
#include "Debugging.h"

#include "llvm/Support/FileSystem.h"

#define DEBUG_TYPE "LegUp:LegupPass"

using namespace llvm;
using namespace legup;

namespace legup {
Debugging dbger;

class FunctionSortingWrapper;

class FunctionSortingWrapper {
  public:
    FunctionSortingWrapper() {
		sortNumber = 0;
	}
	;
	FunctionSortingWrapper(unsigned _sortNumber) {
		sortNumber = _sortNumber;
	}
	;
    bool operator<(const FunctionSortingWrapper &rhs) const {
      return sortNumber < rhs.sortNumber;
	}
	;
	llvm::Function *getFunction() const {
		return function;
	}
	;
	void setFunction(Function *f) {
		function = f;
	}
	;

  private:
    unsigned sortNumber;
    Function *function;
  };

  // Returns a set of functions ordered such that the   
std::vector<Function*> LegupPass::getDepthFirstSortedFunctions(Module &M) {
  // TODO: Use a more efficient algorithm for this.
  size_t previousSetSize = 0;
  std::vector<Function*> sortedSet;
  std::vector<std::string> addedFunctionNames;
  size_t addedFunctionCount = 0;
  do {
    previousSetSize = sortedSet.size();
    for (Module::iterator f = M.begin(), FE = M.end(); f != FE; ++f) {
      Function &F = *f;
			if (std::find(addedFunctionNames.begin(), addedFunctionNames.end(),
					F.getName().str()) != addedFunctionNames.end())
	continue;
      bool allCalledFunctionsAddedToSet = true;
      
      for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
	
				for (BasicBlock::iterator instr = b->begin(), ie = b->end();
						instr != ie; ++instr) {
					if (isaDummyCall(instr))
						continue;
	  
	  if (CallInst *CI = dyn_cast<CallInst>(instr)) {	
	    llvm::Function *called = getCalledFunction(CI);
						if (std::find(addedFunctionNames.begin(),
								addedFunctionNames.end(),
								called->getName().str())
								== addedFunctionNames.end()) {
	      allCalledFunctionsAddedToSet = false;
	      break;
	    }
	  }
	} // end Block Iterator
				if (!allCalledFunctionsAddedToSet)
					break;
      } // end Function Iterator
      if (allCalledFunctionsAddedToSet) {
	  sortedSet.push_back(&F);
	  addedFunctionNames.push_back(F.getName().str());
	  addedFunctionCount ++;
      }
    } // end Module iterator
	} while (previousSetSize != sortedSet.size()
			&& addedFunctionCount <= M.getFunctionList().size());
  assert(addedFunctionCount == M.getFunctionList().size());
  return sortedSet;
}

bool LegupPass::doInitialization(Module &M) {
    allocation = new Allocation(&M);
    Scheduler::alloc = allocation;

    // no modification
    return false;
}

// print out some statistics:
// number of instructions and basic blocks in each loop/function
void LegupPass::printBBStats(Function &F) {
	if (!LEGUP_CONFIG->getParameterInt("PRINT_BB_STATS"))
		return;

    errs() << "Statistics\n";
    // note: the loop stats will double count some basic blocks that are
    // part of multiple loops. For instance, both the inner and the outer
    // body of a loop nest will count. This may affect the calculated
    // median/average
    errs() << "Function: " << F.getName() << "\n";
    LoopInfo &LI = getAnalysis<LoopInfo>(F);
    int totalBBs = 0;
    int totalInst = 0;
    int totalMemInst = 0;
    int numLoops = 0;
    for (LoopInfo::iterator li = LI.begin(), le = LI.end(); li != le; ++li) {
        numLoops++;
        Loop *loop = *li;
        BasicBlock *loopPreheader;
        loopPreheader = loop->getLoopPreheader();
		errs() << "Loop " << numLoops << " preheader name: "
				<< getLabel(loopPreheader) << "\n";
        int numBBs = 0;
        int numInstLoop = 0;
        int numMemInstLoop = 0;
        for (Loop::block_iterator bb = loop->block_begin(), eb =
                loop->block_end(); bb != eb; ++bb) {
            BasicBlock *BB = *bb;
            int numInst = 0;
            int numMemInst = 0;
			for (BasicBlock::iterator I = BB->begin(), ie = BB->end(); I != ie;
					++I) {
                numInst++;
				if (isMem(I))
					numMemInst++;
            }
            errs() << "Number of Instructions (Loop,BB): " << numInst << "\n";
			errs() << "Number of Mem Instructions (Loop,BB): " << numMemInst
					<< "\n";

            numInstLoop += numInst;
            numMemInstLoop += numMemInst;
            numBBs++;
        }
        errs() << "Number of Instructions (Loop): " << numInstLoop << "\n";
		errs() << "Number of Mem Instructions (Loop): " << numMemInstLoop
				<< "\n";
        errs() << "Number of Basic Blocks (Loop): " << numBBs << "\n";
        totalInst += numInstLoop;
        totalMemInst += numMemInstLoop;
        totalBBs += numBBs;
    }
    errs() << "Number of Loops: " << numLoops << "\n";
    errs() << "Number of Mem Instructions (Func): " << totalMemInst << "\n";
    errs() << "Number of Basic Blocks (Func): " << totalBBs << "\n";
	if (totalBBs > 0)
		errs() << "Inst/Basic Blocks: " << totalInst / totalBBs << "\n";

    errs() << "Function: " << F.getName() << "\n";
    totalBBs = 0;
    totalInst = 0;
    totalMemInst = 0;
    for (Function::iterator BB = F.begin(), be = F.end(); BB != be; ++BB) {
        int numInst = 0;
        int numMemInst = 0;
		for (BasicBlock::iterator I = BB->begin(), ie = BB->end(); I != ie;
				++I) {
            numInst++;
			if (isMem(I))
				numMemInst++;
        }
        errs() << "Number of Instructions (BB): " << numInst << "\n";
        errs() << "Number of Mem Instructions (BB): " << numMemInst << "\n";

        totalInst += numInst;
        totalMemInst += numMemInst;
        totalBBs++;
    }
    errs() << "Number of Instructions (Func): " << totalInst << "\n";
    errs() << "Number of Mem Instructions (Func): " << totalMemInst << "\n";
    errs() << "Number of Basic Blocks (Func): " << totalBBs << "\n";
    errs() << "Inst/Basic Blocks: " << totalInst/totalBBs << "\n";
}

bool LegupPass::runOnModule(Module &M) {

    // NC changes
    // Debugging dbger;
    if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG") ||
        LEGUP_CONFIG->getParameterInt("INSPECT_ONCHIP_BUG_DETECT_DEBUG")) {
        std::cout << "***LegUp Inspect-Debug mode is selected***" << std::endl;
        std::cout << "it is assumed that you already set NO_OPT, NO_INLINE and "
                     "DEBUG_G_FLAG Makefile variables to 1.";
        std::cout << " If not, the inspect debug behavior will be unknown."
				<< std::endl;
		if (LEGUP_CONFIG->getParameterInt("LOCAL_RAMS") != 0
				|| LEGUP_CONFIG->getParameterInt("GROUP_RAMS") != 0
				|| LEGUP_CONFIG->getParameterInt("NO_ROMS") == 0) {
			std::cout << "LOCAL_RAMS: "
					<< LEGUP_CONFIG->getParameterInt("LOCAL_RAMS") << std::endl;
			std::cout << "GROUP_RAMS: "
					<< LEGUP_CONFIG->getParameterInt("GROUP_RAMS") << std::endl;
			std::cout << "NO_ROMS: " << LEGUP_CONFIG->getParameterInt("NO_ROMS")
					<< std::endl;
			std::cout
					<< "LegUp Configuration for Inspect-Debug mode is not correct."
					<< " In order to run LegUp on Inspect-Debug mode set LOCAL_RAMS = 0; GROUP_RAMS = 0; and NO_ROMS = 1; in legup.tcl file"
					<< std::endl;
            exit(1);
        }
        
        /*Timer timer;
        timer.init(StringRef("timer"));
        timer.startTimer();*/
        dbger.initializeDatabase();
        dbger.initialize();
        /*timer.stopTimer();
        std::cout << "inspect debug database initialized: " << std::endl;*/
    }

	pipelineLabelSanityCheck(M);

	std::vector<Function *> sortedFunctionSet =
			this->getDepthFirstSortedFunctions(M);

	for (std::vector<Function *>::iterator fw = sortedFunctionSet.begin(), FWE =
			sortedFunctionSet.end(); fw != FWE; ++fw) {
	  Function &F = *(*fw);

        // can't call a function analysis pass on a function declaration
        // without a body
		if (F.isDeclaration() || LEGUP_CONFIG->isCustomVerilog(F))
			continue;

        DEBUG(errs() << "Entering function: " << F.getName() << "\n");

        // debugging: view a dot graph of function control flow graph:
        //F->viewCFG();
        
        //NC changes
		if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")
				|| LEGUP_CONFIG->getParameterInt(
						"INSPECT_ONCHIP_BUG_DETECT_DEBUG")) {
            dbger.fillDebugDB(&F);
        }

        // Do not codegen any 'extern' functions at all, they have
        // definitions outside the translation unit.
        if (F.hasAvailableExternallyLinkage()) {
            DEBUG(errs() << "Skipping function (extern)\n");
            continue;
        }

        // Create an RTL Generator for this function
        allocation->createGenerateRTL(&F);

        printBBStats(F);
    }
    
    allocation->addAA(&getAnalysis<AliasAnalysis>());	
  	
    // If software profiling was done in this compilation, read in the  
    // llvmprof.out file and cache some info about the execution
	// TODO LLVM 3.4 update.  profileInfo no longer exists.  commenting out for now.
    //if (LEGUP_CONFIG->getParameterInt("LLVM_PROFILE")) {
        //allocation->addPI(&getAnalysis<ProfileInfo>());
    //}

    // Schedule the operations in each function
    for (Allocation::hw_iterator i = allocation->hw_begin(), ie =
            allocation->hw_end(); i != ie; ++i) {
        GenerateRTL *HW = *i;
	HW->scheduleOperations();
    }

    // Calculate the required functional units (multipliers/dividers) required
    // This requires scheduling information to get a complete picture of the
    // overall resource usage
    allocation->calculateRequiredFunctionalUnits();

    // Gather debugger information
    if (allocation->getDbgInfo()->isDatabaseEnabled()) {
        allocation->getDbgInfo()->generateVariableInfo();
        allocation->getDbgInfo()->analyzeProgram();
    }

    // Generate the RTL
    for (Allocation::hw_iterator i = allocation->hw_begin(), ie =
            allocation->hw_end(); i != ie; ++i) {
        GenerateRTL *HW = *i;
        Function *F = HW->getFunction();

        allocation->addLVA(F, &getAnalysis<LiveVariableAnalysis>(*F));
        MinimizeBitwidth *MBW = &getAnalysis<MinimizeBitwidth>(*F);
        allocation->addLI(F, &getAnalysis<LoopInfo>(*F));

        RTLModule* rtl = HW->generateRTL(MBW);

        // Store the RTL for this module in the Allocation object
        allocation->addRTL(rtl);

        // Pair the rtl with its equivalent C function
        allocation->setModuleForFunction(rtl, F);

        // NC changes...
        if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
            dbger.mapIRsToStates(HW);
        }
    }

    // LLVM 3.4 update: doFinalization is called by the pass manager now?
    // doFinalization(M);

    // print the latency mismatch warnings
    LEGUP_CONFIG->printLatencyWarnings();

    // no modifications to IR so return false
    return false;
}

void LegupPass::pipelineLabelSanityCheck(Module &M) {
	std::set<std::string> pipelinedLabels;

    for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F) {
        for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b) {
			TerminatorInst *TI = b->getTerminator();
			if (getMetadataInt(TI, "legup.pipelined")) {
				std::string label = getMetadataStr(TI, "legup.label");
				pipelinedLabels.insert(label);
			}
		}
	}

	int totalLoopsPipelined = pipelinedLabels.size();

	int expected = LEGUP_CONFIG->numLoopPipelines();

	if (totalLoopsPipelined != expected) {
		errs() << "Error: Expected to find " << expected
			<< " loops to pipeline but only pipelined "
			<< totalLoopsPipelined << "\n";
		errs() << "Note: loop pipeline labels only work in the main .c file\n";
	}

	std::map<std::string, LegupConfig::LOOP_PIPELINE> &loop_pipelines =
		LEGUP_CONFIG->getAllLoopPipelines();

	for (std::map<std::string, LegupConfig::LOOP_PIPELINE>::iterator i =
			loop_pipelines.begin(), ie = loop_pipelines.end(); i != ie;
			++i) {
		std::string label = i->first;
		if (pipelinedLabels.find(label) == pipelinedLabels.end()) {
			errs() << "Couldn't pipeline loop with label: " << label << "\n";
		}
	}
}

bool LegupPass::doFinalization(Module &M) {
    std::set<const Function*> AcceleratedFcts;

    if (M.begin() == M.end()) {
        llvm_unreachable("No functions exist in the module!\n");
    }

	for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
        if (LEGUP_CONFIG->isAccelerated(*F)) {
        	AcceleratedFcts.insert(F);
		}
	}

    allocation->addGlobalDefines();

    if (allocation->getDbgInfo()->isDatabaseEnabled()) {
        // Assign instance IDs to each module instance
        allocation->getDbgInfo()->assignInstances();
    }

    if (allocation->getDbgInfo()->isDebugRtlEnabled()) {
        allocation->getDbgInfo()->addDebugRtl();
    }

    if (allocation->getDbgInfo()->isDatabaseEnabled()) {
        allocation->getDbgInfo()->outputDebugDatabase();
    }

    // TODO - instantiate local RAMs in Generate RTL data structure instead of
    // verilog writer and re-enable this
    if (!LEGUP_CONFIG->getParameterInt("LOCAL_RAMS")) {
        if (!LEGUP_CONFIG->getParameterInt("KEEP_SIGNALS_WITH_NO_FANOUT")) {
            for (Allocation::const_rtl_iterator i = allocation->rtl_begin(), e =
                    allocation->rtl_end(); i != e; ++i) {
                RTLModule *rtl = *i;

                // delete register signals that are unconnected
                rtl->removeSignalsWithoutFanout();

                rtl->verifyConnections(allocation);

            }
        }
    }

    if (!LEGUP_CONFIG->getParameterInt("NO_LOOP_PIPELINING")) {
        for (Allocation::const_rtl_iterator i = allocation->rtl_begin(), e =
                allocation->rtl_end(); i != e; ++i) {
            RTLModule *rtl = *i;
            if (rtl->getName() == "main") {
                rtl->buildCircuitStructure();
                formatted_raw_ostream out(allocation->getPipelineDotFile());
                rtl->printPipelineDot(out);
            }
        }
    }

    printVerilog(AcceleratedFcts);

    printResourcesFile("resources.legup.rpt");

	std::list<GenerateRTL::PATH *> *overallPaths =
			allocation->getOverallLongestPaths();
    std::string fileName = "timingReport.overall.legup.rpt";
    std::string Error = "Error in printing timing report\n";
    raw_fd_ostream overallReport(fileName.c_str(), Error,
                                 llvm::sys::fs::F_None);
    overallReport << getFileHeader();
    GenerateRTL::printPath(overallReport, overallPaths);

    if (LEGUP_CONFIG->getParameterInt("INSPECT_DEBUG")) {
        dbger.fillHardwareInfo(allocation);
        dbger.fillSignals(allocation);
        dbger.fillVariables(allocation);

        dbger.StateStoreInfoMapping(allocation);
    } else if (LEGUP_CONFIG->getParameterInt(
                   "INSPECT_ONCHIP_BUG_DETECT_DEBUG")) {
        dbger.fillCurStateAndFinishSignals(allocation);
    }

    return false;
}

LegupPass::~LegupPass() {
    assert(allocation);
    delete allocation;
}

void LegupPass::printVerilog(const std::set<const Function*> &AcceleratedFcts) {
    VerilogWriter writer(Out, allocation, AcceleratedFcts);
    writer.print();
}

void LegupPass::printResourcesFile(std::string fileName) {
    std::string EstimateError = "Error in printing early resource estimate\n";
    raw_fd_ostream resourceFile(fileName.c_str(), EstimateError, llvm::sys::fs::F_None);
    resourceFile << getFileHeader();

    ResourceEstimator estimator(allocation);
    estimator.print(resourceFile);
}

// we can't put this RegisterPass in MinimizeBitwidth.cpp because otherwise
// this statement will get linked into the shared library LLVMLegUp.so when
// compiling ../../Transforms/LegUp. This will cause an error that the
// MinimizeBitwidth pass has been registered twice when running:
//      opt -load=../../llvm/Debug+Asserts/lib/LLVMLegUp.so
char MinimizeBitwidth::ID = 0;
static RegisterPass<MinimizeBitwidth> X("legup-minimize-bitwidth",
        "Pre-Link Time Optimization Pass to shrink integer bitwidth to arbritrary precision");

} // End legup namespace

