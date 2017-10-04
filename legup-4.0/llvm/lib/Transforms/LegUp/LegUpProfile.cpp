//===- LegUpProfile.cpp ---------------------------------------------------===//
// LegUp pass that adds profiling instrumentation calls
//===----------------------------------------------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Adds a call to __legup_prof_init() at beginning of main()
// Adds calls to  __legup_prof_begin() at the beginning of each function
// Adds calls to  __legup_prof_end() at the end of each function
// Adds a call to __legup_prof_print() at the end of main()
//
// NOTE: the backend for ARM, MIPS, etc. may move the put some prologue/epilogue
// instructions before/after these calls.
//
// Eg. for ARM you may see:
// <called_function>:
//      push {lr}
//      ...
//      call __legup_prof_begin
//      ...
//      call __legup_prof_end
//      ...
//      pop {sp}
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

namespace legup {

// LegUpProfile
struct LegUpProfile : public FunctionPass {
    static char ID; // Pass identification

    LegUpProfile() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
        Module *M = F.getParent();
        Constant *c;

        c = M->getOrInsertFunction(
            "__legup_prof_begin",               // function name
            Type::getVoidTy(M->getContext()),   // return value (void)
            Type::getInt8PtrTy(F.getContext()), // pass char pointer for fn name
            (Type *)0);                         // end variable args with NULL
        Function *__legup_prof_begin = cast<Function>(c);

        // skip over any allocas in the entry block
        Function::iterator Entry = F.begin();
        BasicBlock::iterator InsertPos = Entry->begin();
        while (isa<AllocaInst>(InsertPos))
            ++InsertPos;

        IRBuilder<> frontBuilder(InsertPos);

        /*
        // if this is the main function, add additional function call to
        // initialize profiling
        if (F.getName() == "main") {
            // build init function
            c = M->getOrInsertFunction(
                "__legup_prof_init",              // function name
                Type::getVoidTy(M->getContext()), // return value (void)
                (Type *)0);                       // end variable args with NULL
            Function *__legup_prof_init = cast<Function>(c);

            // insert instruction at beginning of function
            frontBuilder.CreateCall(__legup_prof_init);
        }
        */

        Value *strPtr = frontBuilder.CreateGlobalStringPtr(F.getName(), ".str");
        frontBuilder.CreateCall(__legup_prof_begin, strPtr);

        // attempt to find return instruction of function
        // we want to add the profiling end() functions at the very end of
        // each
        // function
        for (Function::iterator BB = F.begin(), E = F.end(); BB != E; BB++) {
            for (BasicBlock::iterator InsertPos = BB->begin(), EE = BB->end();
                 InsertPos != EE; InsertPos++) {
                // if it is a return instruction, we want to add function calls
                // just before it
                if (isa<ReturnInst>(&(*InsertPos))) {
                    IRBuilder<> backBuilder(InsertPos);
                    // profiling function call to signal end of function
                    c = M->getOrInsertFunction(
                        "__legup_prof_end",               // function name
                        Type::getVoidTy(M->getContext()), // return value (void)
                        (Type *)0); // end variable args with NULL
                    Function *__legup_prof_end = cast<Function>(c);

                    // insert instruction just before 'return'
                    backBuilder.CreateCall(__legup_prof_end);

                    if (F.getName() != "main") {
                        break;
                    }

                    c = M->getOrInsertFunction(
                        "__legup_prof_print",             // function name
                        Type::getVoidTy(M->getContext()), // return value (void)
                        (Type *)0); // end variable args with NULL
                    Function *__legup_prof_print = cast<Function>(c);

                    // insert instruction just before 'return'
                    backBuilder.CreateCall(__legup_prof_print);

                    break;
                }
            }
        }

        return true; // each function should have been modified
    }
};

char LegUpProfile::ID = 0;
static RegisterPass<LegUpProfile>
    X("legup-profile", "Add LegUp profiling instrumentation calls");

// LegUpProfile-main-only
struct LegUpProfile_main_only : public FunctionPass {
    static char ID; // Pass identification

    LegUpProfile_main_only() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
        if (F.getName() != "main")
            return false; // nothing changed

        Module *M = F.getParent();
        Constant *c;

        c = M->getOrInsertFunction(
            "__legup_prof_begin",               // function name
            Type::getVoidTy(M->getContext()),   // return value (void)
            Type::getInt8PtrTy(F.getContext()), // pass char pointer for fn name
            (Type *)0);                         // end variable args with NULL
        Function *__legup_prof_begin = cast<Function>(c);

        // skip over any allocas in the entry block
        Function::iterator Entry = F.begin();
        BasicBlock::iterator InsertPos = Entry->begin();
        while (isa<AllocaInst>(InsertPos))
            ++InsertPos;

        IRBuilder<> frontBuilder(InsertPos);

        // if this is the main function, add additional function call to
        // initialize profiling
        // build init function
        c = M->getOrInsertFunction(
            "__legup_prof_init",              // function name
            Type::getVoidTy(M->getContext()), // return value (void)
            (Type *)0);                       // end variable args with NULL
        Function *__legup_prof_init = cast<Function>(c);

        // insert instruction at beginning of function
        frontBuilder.CreateCall(__legup_prof_init);

        Value *strPtr = frontBuilder.CreateGlobalStringPtr(F.getName(), ".str");
        frontBuilder.CreateCall(__legup_prof_begin, strPtr);

        // attempt to find return instruction of function
        // we want to add the profiling end() functions at the very end of
        // each
        // function
        for (Function::iterator BB = F.begin(), E = F.end(); BB != E; BB++) {
            for (BasicBlock::iterator InsertPos = BB->begin(), EE = BB->end();
                 InsertPos != EE; InsertPos++) {
                // if it is a return instruction, we want to add function calls
                // just before it
                if (isa<ReturnInst>(&(*InsertPos))) {
                    IRBuilder<> backBuilder(InsertPos);
                    // profiling function call to signal end of function
                    c = M->getOrInsertFunction(
                        "__legup_prof_end",               // function name
                        Type::getVoidTy(M->getContext()), // return value (void)
                        (Type *)0); // end variable args with NULL
                    Function *__legup_prof_end = cast<Function>(c);

                    // insert instruction just before 'return'
                    backBuilder.CreateCall(__legup_prof_end);

                    c = M->getOrInsertFunction(
                        "__legup_prof_print",             // function name
                        Type::getVoidTy(M->getContext()), // return value (void)
                        (Type *)0); // end variable args with NULL
                    Function *__legup_prof_print = cast<Function>(c);

                    // insert instruction just before 'return'
                    backBuilder.CreateCall(__legup_prof_print);

                    break;
                }
            }
        }

        return true; // main function has been modified
    }
};

char LegUpProfile_main_only::ID = 0;
static RegisterPass<LegUpProfile_main_only>
    Y("legup-profile-main-only", "Add LegUp profiling instrumentation calls"
                                 "only to main function");
}
