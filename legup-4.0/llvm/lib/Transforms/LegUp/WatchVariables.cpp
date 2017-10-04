//===- WatchVariables.cpp - LegUp uses this pass to add profiling output --===//
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
#include <vector>
#include <queue>

using namespace llvm;

namespace legup {
struct LegUpWatch : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    LegUpWatch() : FunctionPass(ID) {};

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

    void addVariableToFormatString(Instruction * instr,
            std::string &printString) {
        printString += "Watch@  ";
        // print variable name
        if (instr->getName().empty()) {
            // read the instruction into LLVM's string replacement
            std::string str;
            raw_string_ostream varName(str);
            varName << *instr;
            // first two characters are spacing
            std::string count = varName.str().substr(2);
            // escape the % sign in the variable name
            printString += '%';
            // the variable name is until the first space
            for (int i = 0; count[i] != ' '; i++)
                printString += count[i];
        }
        else {
            printString += "%%" + instr->getName().str();
        }
        printString += '=';

        // push variable value to be added to printf arguments
        unsigned bitwidth = instr->getType()->getScalarSizeInBits();
        // print in hexadecimal format, and only up to the size of the variable
        if (bitwidth > 32) {
            printString += "%llx";
        }
        else if (bitwidth > 16) {
            printString += "%x";
        }
        else if (bitwidth > 8) {
            printString += "%hx";
        }
        else {
            printString += "%hhx";
        }
        printString += "\n";
    }

    static bool isAPrintCall(Instruction *instr) {
        CallInst *ci = dyn_cast<CallInst>(instr);
        if (!ci) {
            return false;
        }
        Function *calledFunc = ci->getCalledFunction();
        // ignore indirect function invocations
        if (!calledFunc) {
            return false;
        }
        std::string funcName = calledFunc->getName();
        return (funcName == "printf" || funcName == "putchar" ||
            funcName == "puts" || funcName == "mprintf");
    }

    void watchInstruction(Instruction *instr, std::vector<Value *> &Args,
            std::string &printString) {
        // store, and gep instructions will never be assigned an integer value
        if (isa<StoreInst>(instr) || isa<GetElementPtrInst>(instr)) {
            return;
        }
        if ((isa<IntegerType>(instr->getType()) || instr->getType()->isFloatingPointTy())
                && !isAPrintCall(instr)) {
            addVariableToFormatString(instr, printString);
            Args.push_back(instr);
            // replace UndefValue's with ConstantInt's equal to 0
            for (unsigned i = 0; i < instr->getNumOperands(); i++) {
                if (isa<UndefValue>(instr->getOperand(i))) {
                    Type *type = instr->getOperand(i)->getType();
                    Value *value = NULL;
                    if (isa<IntegerType>(type)) {
                        value = ConstantInt::get(type, 0);
                    } else if (type->isFloatingPointTy()) {
                        value = ConstantFP::get(type, 0);
                    }
                    assert(value && "Unsupported type\n");
                    instr->setOperand(i, value);
                }
            }
        }
    }

    void reachedFunctionTerminator(std::string &printString,
            std::vector<Value *> &Args,
            std::queue< std::vector<Value *> > &ArgsQueue,
            Function::iterator &BB, Function &F, Instruction *instr) {
        printString += '\n';
        IRBuilder<> Builder(BB);
        // printString is the first argument of printf(char *, ...)
        Args[0] = Builder.CreateGlobalStringPtr(printString.c_str());
        ArgsQueue.push(Args);
        // print all variables that have been assigned in that basic block
        while (!ArgsQueue.empty()) {
            CallInst::Create(PrintFunc, ArgsQueue.front(), "", instr);
            ArgsQueue.pop();
        }
    }

    virtual bool runOnFunction(Function &F) {
        for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE; ++BB) {
            // initialize string to print out
            std::string printString = "\nWatch@" + F.getName().str() + ":";
            printString += BB->getName().str() + '\n';
            std::vector<Value *> Args;
            // reserve first argument of printf for printString
            Args.push_back(NULL);
            std::queue< std::vector<Value *> > ArgsQueue;
            for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;
                    ++I) {
                Instruction *instr = I;
                // print variables values just before the basic block terminates
                if (instr->isTerminator()) {
                    reachedFunctionTerminator(printString, Args, ArgsQueue, BB,
                        F, instr);
                } else {
                    watchInstruction(instr, Args, printString);
                }
            }
        }
        // the bitcode will change as long as a basic block exists
        return true;
    }

private:
    Module *Mod;
    Constant *PrintFunc;
};

}

using namespace legup;

char LegUpWatch::ID = 0;
static RegisterPass<LegUpWatch> X("legup-watch",
    "Add print statements for assigned variables after each basic block");

