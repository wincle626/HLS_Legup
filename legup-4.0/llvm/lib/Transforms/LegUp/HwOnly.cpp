//===- HwOnly.cpp - LegUp pre-LTO pass ------------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// The HwOnly pass strips away software only (non-accelerated) functions
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "utils.h"
#include <set>
#include <map>
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Attributes.h"
//#include "llvm/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/Analysis/CallGraph.h"
using namespace llvm;

namespace legup {

class HwOnly  : public ModulePass {
public:
    static char ID; // Pass identification, replacement for typeid
    HwOnly() : ModulePass(ID) {
    }

    virtual bool runOnModule(Module &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    }

private:
	void getParallelAcceleratedFunctions(Module &M, std::set<GlobalValue*> &HwFcts);
    bool getParallelFunctions(Module &M, CallInst *CI, std::set<GlobalValue*> &HwFcts);

    std::set<Function*> pthreadFunctions;
	std::set<Function*> internalAccels;    
};

char HwOnly::ID = 0;
static RegisterPass<HwOnly> Y("legup-hw-only",
        "Strip away non-accelerated functions");

/// remove all functions that are s/w only (not accelerated)
/// Adapted from GlobalDCE pass
bool HwOnly::runOnModule(Module &M) {

    std::set<GlobalValue*> HwFcts;

    getParallelAcceleratedFunctions(M, HwFcts);
    getAcceleratedFunctions(M, HwFcts);

    return isolateGV(M, HwFcts);
}

void HwOnly::getParallelAcceleratedFunctions(Module &M,
        std::set<GlobalValue*> &HwFcts) {

	// skip in PCIe flow
	if (LEGUP_CONFIG->isPCIeFlow()) return;

	getInternalAccels(M, internalAccels);
	
	//designate all omp functions as hardware accelerators
	for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
	    for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE; ++BB) {
	        for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;) {

	            if (CallInst *CI = dyn_cast<CallInst>(I++)) {

	                Function *calledFunction = CI->getCalledFunction();

	                // ignore indirect function calls
	                if (!calledFunction) continue;

                    if (getParallelFunctions(M, CI, HwFcts)) continue;

		        }
		    }
		}
	}
}

bool HwOnly::getParallelFunctions(Module &M, CallInst *CI, std::set<GlobalValue*> &HwFcts) {
    
	bool replaced = false;
	std::string HwFuncName;
    int parallelInstances = getMetadataInt(CI, "NUMTHREADS");
    std::string type = getMetadataStr(CI, "TYPE");

    // all pthread/openmp intrinsic function are replaced by this point
    // just check if it's a parallel function by checking the NUMTHREADS metadata
	if (parallelInstances != 0) {
        
        Function *calledFunction = CI->getCalledFunction();
		//get the parallel function
		HwFuncName = calledFunction->getName().str();
		//add noinline attribute to the OMP function, since this should never be inlined
	//	HwFuncPtr->addFnAttr(1<<11);

        // if pthread function, insert into set (there is no wrapper for pthreads)
        // for openmp, this current function is just the wrapper
        // so start inserting from its descendants
        if (type == "legup_wrapper_pthreadcall")
            HwFcts.insert(calledFunction);

		//add descendant functions
        addCalledFunctions(calledFunction, HwFcts);

		replaced = true;
	} 
	
	return replaced;
}

} // end of legup namespace






