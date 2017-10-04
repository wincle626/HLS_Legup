//===- NonAcceleratedOnly.cpp - LegUp pre-LTO pass -------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// The NonAcceleratedOnly pass strips away all accelerated functions including
// those still used in SW part.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IRBuilder.h"
#include "LegupConfig.h"
#include "utils.h"

using namespace llvm;

namespace legup {

class NonAcceleratedOnly  : public ModulePass {
public:
    static char ID; // Pass identification, replacement for typeid
    NonAcceleratedOnly() : ModulePass(ID) {}

    virtual bool runOnModule(Module &F);
	virtual void preserveGlobalVariablesUsedInHW(Module &M);
	virtual void findUsedValues(GlobalVariable *LLVMUsed,
                            SmallPtrSetImpl<GlobalValue *> &UsedValues);
    };

char NonAcceleratedOnly::ID = 0;
static RegisterPass<NonAcceleratedOnly> Y("legup-non-accelerated-only",
        "Strip away non-accelerated functions");

// copied from SwOnly.cpp
// this function find all the global variables which are stored
// in the llvm.used variable
void NonAcceleratedOnly::findUsedValues(GlobalVariable *LLVMUsed,
                            SmallPtrSetImpl<GlobalValue *> &UsedValues) {
                            //SmallPtrSet<GlobalValue *, 8> &UsedValues) {
    if (!LLVMUsed)
        return;
	UsedValues.insert(LLVMUsed);

    ConstantArray *Inits = cast<ConstantArray>(LLVMUsed->getInitializer());
    for (unsigned i = 0, e = Inits->getNumOperands(); i != e; ++i)
        if (GlobalValue *GV = dyn_cast<GlobalValue>(
                Inits->getOperand(i)->stripPointerCasts()))
            UsedValues.insert(GV);
}


// copied from SwOnly.cpp
// this functions add all global variables used in HW
// to llvm.used intrinsic global variable, which prohibits the compiler
// from optimizing the global variable away
// this is need since the SW IR needs to keep the global variable, so that
// its address can be parsed later in the Verilog backend
void NonAcceleratedOnly::preserveGlobalVariablesUsedInHW(Module &M) {

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

/// remove the accelerating function
bool NonAcceleratedOnly::runOnModule(Module &M) {

    std::set<GlobalValue*> HwFcts;
    bool modified;

    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I)
        if (! LEGUP_CONFIG->isAccelerated(*I))
            HwFcts.insert(I);

    modified = isolateGV(M, HwFcts);

    preserveGlobalVariablesUsedInHW(M);

    return modified;
}

} // end of legup namespace

