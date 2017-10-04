//===- CustomVerilog.cpp - LegUp pre-LTO pass -----------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// The CustomVerilog pass strips away custom verilog functions
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LLVMContext.h"
#include "utils.h"

using namespace llvm;

namespace legup {

class CustomVerilog  : public ModulePass {
public:
    static char ID; // Pass identification, replacement for typeid
    CustomVerilog() : ModulePass(ID) {

    }

 

    virtual bool runOnModule(Module &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    }

private:

    std::set<GlobalValue *> getGlobalValueSetForVector(std::vector<Function*> &HwFcts);
    void getNonCustomVerilogFunctions(Module &M, std::vector<Function*> &HwFcts);
    void getCustomVerilogFunctions(Module &M, std::vector<Function*> &HwFcts);
    
};

char CustomVerilog::ID = 0;
static RegisterPass<CustomVerilog> Y("legup-custom-verilog",
        "Strip away custom verilog functions");

struct FunctionWithCallCount {

    Function *func;
    int callCount;
    
};

bool operator==(const FunctionWithCallCount& lhs, const FunctionWithCallCount& rhs)
{
    return (lhs.func == rhs.func);
}

void removeBasicBlocksFromFunction(Function &F) {

    for (Function::iterator b = F.begin(), be = F.end(); b != be; ) {

	
	for (BasicBlock::iterator II = b->begin(); II != b->end();) {
            Instruction * insII = &(*II);
	    II++;

	    if (!(insII->isTerminator())) {
		insII->dropAllReferences();
	    }
        }
	
	BasicBlock *toRemove = b++;
	toRemove->removeFromParent();

    }

    /*BasicBlock* block = BasicBlock::Create(getGlobalContext(), "return", &F);
    IRBuilder<> builder(block);
    ReturnInst *ret = builder.CreateRetVoid();
    
    Type *returnType = F.getReturnType();
    Value *returnVal = ret->getReturnValue();
    returnVal->mutateType(returnType); */

    //    builder.CreateRetVoid();

}

std::vector<FunctionWithCallCount>::iterator functionWithCallCountForFunctionInVector(Function *F, std::vector<FunctionWithCallCount> &fv) {

    for (std::vector<FunctionWithCallCount>::iterator it = fv.begin(); it != fv.end(); ++it) {

	if (it->func == F) {
	    return it;
	}

    }

    return fv.end();
}

void removeUncalledFunctionsFromVector(Function *function, std::vector<FunctionWithCallCount> &functions) {

    for (Function::iterator b = function->begin(), be = function->end(); b != be; ++b) {
	for (BasicBlock::iterator instr = b->begin(), ie = b->end(); instr !=
		 ie; ++instr) {
	    if (isaDummyCall(instr)) continue;
	    
	    if (CallInst *CI = dyn_cast<CallInst>(instr)) {	
		Function *called = getCalledFunction(CI);

		// Get the existing FunctionWithCallCount if one exists
		//
		std::vector<FunctionWithCallCount>::iterator fcc =
		    functionWithCallCountForFunctionInVector(called, functions);

		// We will be removing 'function', so we must decrement the callCount
		// of any of the functions it called
		//
		fcc->callCount --;
		
		if (!(fcc->callCount)) {

		    // Save function pointer from iterator and increment iterator so
		    // that we don't destroy our iterator when we (potentially) erase
		    // functions from our vector.
		    //
		    FunctionWithCallCount functionToDelete = *fcc;

		    removeUncalledFunctionsFromVector(called, functions);
		    std::vector<FunctionWithCallCount>::iterator iteratorToDelete =
			std::find(functions.begin(), functions.end(), functionToDelete);

		    // There is something wrong with the code if this was asserted.
		    // There should be no case where the iterator is not in the
		    // vector.
		    //
		    assert(iteratorToDelete != functions.end());
		    
		    functions.erase(iteratorToDelete);
		}
	    }
	}
    }
}

void removeFunctionsFromVectorIfNotCalled(std::vector<Function *> &functions) {

    unsigned int previousFunctionCount = functions.size();
    std::vector<FunctionWithCallCount> calledFunctions;
 
    for (std::vector<Function *>::iterator it = functions.begin(); it != functions.end(); ++it) {
	for (Function::iterator b = (*it)->begin(), be = (*it)->end(); b != be; ++b) {
	    for (BasicBlock::iterator instr = b->begin(), ie = b->end(); instr !=
		 ie; ++instr) {
		if (isaDummyCall(instr)) continue;
	    
		if (CallInst *CI = dyn_cast<CallInst>(instr)) {	
		    Function *called = getCalledFunction(CI);

		    // Get the existing FunctionWithCallCount for the called function
		    // if one exists
		    //
		    std::vector<FunctionWithCallCount>::iterator existing =
			functionWithCallCountForFunctionInVector(called, calledFunctions);

		    // If it does exist, increment its callCount
		    //
		    if (existing != calledFunctions.end())
			existing->callCount++;
		    else {

			// It does not exist, so we make a new one and add it to our
			// vector
			//
			FunctionWithCallCount newFunctionWithCallCount;
			newFunctionWithCallCount.func = called;
			newFunctionWithCallCount.callCount = 1;
			calledFunctions.push_back(newFunctionWithCallCount);
		    }
		}
	    }
	}
    }


    // Keep going through functions and removing ones that aren't called until
    // the size of our list of functions stops changing
    //
    do {

	previousFunctionCount = functions.size();

	for (std::vector<Function*>::iterator it = functions.begin(); it != functions.end();) {

	    // We removing 'main' would cause all functions to be removed, but,
	    // there aren't any functions that call 'main', so we would remove
	    // it if we didn't have this condition
	    //
	    if ((*it)->getName() != "main") {

		// Get the iterator pointing to the location of the current function
		// in our list of called functions
		//
		std::vector<FunctionWithCallCount>::iterator fcc =
		    functionWithCallCountForFunctionInVector(*it, calledFunctions);

		// If the function was not in our list of called functions then we
		// can remove it
		//
		if (fcc == calledFunctions.end()) {

		    // Save function pointer from iterator and increment iterator so
		    // that we don't destroy our iterator when we (potentially) erase
		    // functions from our vector.
		    //
		    std::vector<Function*>::iterator savedIterator = it;
		    Function *function = *it;
		    it++;

		    // Before we remove our function we must decrement the call counts
		    // of any functions it calls (this function is recursive and will
		    // decrement the call counts of all of the child functions)
		    //
		    removeUncalledFunctionsFromVector(function, calledFunctions);
		    it = std::find(functions.begin(),functions.end(),function);
		    if (it == functions.end())
			it = functions.begin();

		    // Remove the current function from the vector
		    //
		    functions.erase(savedIterator);
		    
		}
		else {

		    // We must still increment the iterator
		    //
		    it++;

		}
	    }
	    else {

		// We must still increment the iterator
		//
		it++;
		
	    }
	}

	
    } while (functions.size() != previousFunctionCount);

}

void warnIfCustomVerilogNotCalled(Module &M) {

    std::vector<Function*> cvFunctions;

    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        if (LEGUP_CONFIG->isCustomVerilog(*I)) {
	    cvFunctions.push_back(I);
	}
    }

    std::vector<Function*> calledFunctions;

    // Yeah, this is a mess...

    for (Module::iterator it = M.begin(), E = M.end(); it != E; ++it) {
	for (Function::iterator b = it->begin(), be = it->end(); b != be; ++b) {
	    for (BasicBlock::iterator instr = b->begin(), ie = b->end(); instr !=
		 ie; ++instr) {
		if (isaDummyCall(instr)) continue;
	    
		if (CallInst *CI = dyn_cast<CallInst>(instr)) {	
		    Function *called = getCalledFunction(CI);

		    if (LEGUP_CONFIG->isCustomVerilog(*called)) {

			if (std::find(calledFunctions.begin(), calledFunctions.end(), called) == calledFunctions.end()) {

			    calledFunctions.push_back(called);

			}
		    }
		}
	    }
	}
    }

    // ^ ew...

    if (calledFunctions.size() != cvFunctions.size()) {

	std::vector<Function*>::iterator it, b, e, cb, ce;
	b = cvFunctions.begin();
	e = cvFunctions.end();

	cb = calledFunctions.begin();
	ce = calledFunctions.end();

	for (it = cvFunctions.begin(); it != e; ++it) {

	    if (std::find(cb, ce, *it) == ce) {

		errs() << "Warning: Custom Verilog Function \"";
		errs().write_escaped((*it)->getName());
		errs() << "\" was never called.\n";
		errs() << "Hint: try adding \"noinline\" and \"used\" attributes and adding \"volatile int a = 0;\" to the body of the function\n";
	    }
	}

    }
    
    
}

/// remove all functions that are called by custom verilog
/// Adapted from GlobalDCE pass
bool CustomVerilog::runOnModule(Module &M) {

    std::vector<Function*> HwFcts;

    // We don't want to synthesize functions called by custom verilog
    // functions, but we do want to synthesize the custom verilog
    // functions so that they can be added to the rtl.
    //
    // To accomplish this, we will get all of the non-custom verilog
    // functions and remove all of the functions that aren't called
    // by any of the functions in the vector.  Then, we will add in
    // all of the custom verilog functions.
    //
    getNonCustomVerilogFunctions(M, HwFcts);
    removeFunctionsFromVectorIfNotCalled(HwFcts);
    getCustomVerilogFunctions(M, HwFcts);
    warnIfCustomVerilogNotCalled(M);

    std::set<GlobalValue *> HwFctsSet = getGlobalValueSetForVector(HwFcts);

    return isolateGV(M, HwFctsSet);

}

void CustomVerilog::getNonCustomVerilogFunctions(Module &M,
        std::vector<Function*> &HwFcts) {

    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        if (!LEGUP_CONFIG->isCustomVerilog(*I)) {
          HwFcts.push_back(I);
	}
    }
}

void CustomVerilog::getCustomVerilogFunctions(Module &M,
        std::vector<Function*> &HwFcts) {

    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        if (LEGUP_CONFIG->isCustomVerilog(*I)) {
	    HwFcts.push_back(I);
	}
    }
}

std::set<GlobalValue *> CustomVerilog::getGlobalValueSetForVector(std::vector<Function*> &HwFcts) {

    std::set<GlobalValue *> set;

    for (std::vector<Function *>::iterator i = HwFcts.begin(); i != HwFcts.end(); ++i) {
	set.insert(*i);
    }

    return set;
}

}

