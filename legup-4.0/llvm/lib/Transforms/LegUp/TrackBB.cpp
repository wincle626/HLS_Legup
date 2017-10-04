//===- TrackBB.cpp - LegUp uses this pass to track memory access --===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This pass prints out assigned variables after each basic block ends.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/StringExtras.h"
#include "utils.h"
#include <vector>
#include <queue>

using namespace llvm;

namespace legup
{
    struct LegUpTrackBB : public FunctionPass
    {
        static char ID;             // Pass identification, replacement for typeid
        LegUpTrackBB() : FunctionPass(ID) {};

        virtual bool doInitialization(Module &M) {
            Mod = &M;

            // get constants needed to create a printf call
            Type *charPointerType = PointerType::get(IntegerType::
                get(Mod->getContext(), 8), 0);
            FunctionType *printfTy = FunctionType::get(IntegerType::
                get(Mod->getContext(), 32), std::vector<Type*>
                (1, charPointerType), true);

            // insert function prototype if needed
            PrintFunc = M.getOrInsertFunction("printf", printfTy);
            return false;
        }

        std::string line_prefix(Function &F, BasicBlock *BB, int num_printf_added) {
                std::string BB_name = getLabel(BB).erase(0,1);
                int label_num = atoi(BB_name.c_str());
                if ( label_num != 0 && BB_name != "0" ) {
                    std::string str;
                    raw_string_ostream ss(str);
                    ss << ( label_num - num_printf_added );
                    BB_name = ss.str();
                }
                return "\nTrack@<"+F.getName().str()+">:<%%"+BB_name+">\n";
        }

        virtual bool runOnFunction(Function &F) {
            int num_printf_added = 0;
            for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE; ++BB) {
				std::vector<Value *> Args;
				IRBuilder<> Builder(BB);

                // initialize string to print out
                std::string printString = line_prefix(F, BB, num_printf_added);

                for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
                    Instruction *instr = I;
                    if (CallInst *CI = dyn_cast<CallInst>(instr) ) {
//                        if (!CI->getCalledFunction() || !CI->getCalledFunction()->hasExternalLinkage() ) {
                        // skip llvm intrinsics e.g. llvm.lifetime.start
                        if (CI->getCalledFunction()->isIntrinsic())
                            continue;
                        if (!CI->getCalledFunction() || CI->getCalledFunction()->getName().str() != "printf" ) {
                            printString += "Track@<Calling by " + F.getName().str() + ">\n";
                            Args.push_back( Builder.CreateGlobalStringPtr(printString.c_str()) );
                            CallInst::Create(PrintFunc, Args, "", instr);
                            Args.clear();
                            printString = "";
                            num_printf_added += 1;
                        }
                    }
                    else if (dyn_cast<ReturnInst>(instr)) {
                        printString += "Track@<Returning by " + F.getName().str() + ">\n";
                        Args.push_back( Builder.CreateGlobalStringPtr(printString.c_str()) );
                        CallInst::Create(PrintFunc, Args, "", instr);
                        Args.clear();
                        printString = "";
                        num_printf_added += 1;
                    }
                    
                    if (instr->isTerminator() && printString != "") {
                        // This BB has no CallInst or ReturnInst, print at end of BB
                        Args.push_back( Builder.CreateGlobalStringPtr(printString.c_str()) );
                        CallInst::Create(PrintFunc, Args, "", instr);
                        Args.clear();
                        printString = "";
                        num_printf_added += 1;
                    }
                }
            }
            return true;
        }
        
        private:
            Module *Mod;
            Constant *PrintFunc;
    };

}


using namespace legup;

char LegUpTrackBB::ID = 0;
static RegisterPass<LegUpTrackBB> X("legup-track-bb",
"Add print statements after each basic block and before every callInst");
