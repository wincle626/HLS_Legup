//===- utils.h - LegUp pre-LTO helper functions -------------------------===//
//
/// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Legup helper functions
//
//===----------------------------------------------------------------------===//


#ifndef LEGUP_UTILSTRANSFORMS_H
#define LEGUP_UTILSTRANSFORMS_H

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Debug.h"
#include "../../Target/Verilog/LegupConfig.h"
#include <string>
#include "llvm/Support/Signals.h"
#include <sstream>
#include "llvm/Transforms/Utils/Cloning.h"
#include "../../Target/Verilog/utils.h"

using namespace llvm;
using namespace std;

namespace legup {

class RAM;

RAM *createEmptyRAM(Module *M, Allocation *alloc, std::string name);

enum wrapperType { seq, pthreadcall, pthreadpoll, ompcall, pcie};

struct accelFcts {
    Function *fct;
    wrapperType type;
    // number of times this function is accelerated
    // used for parallel functions (pthreads, openMP)
    int numAccelerated;
    // overloading operator == to check if two structs are equal
    bool operator==(const accelFcts &a) const {
        // if ((type == a.type) && (fct == a.fct) && (numAccelerated ==
        // a.numAccelerated)) {
        if ((type == a.type) && (fct == a.fct)) {
			return true;
		} else {
			return false;
		}
    }
};

bool isFSub(Instruction *instr);
bool isFAdd(Instruction *instr);
bool isFAddSub(Instruction *instr);
bool isIAdd(Instruction *instr);
bool isISub(Instruction *instr);

std::string getLabel(const Value *v);

void addCalledFunctions(Function *F, std::set<GlobalValue *> &HwFcts);

void addCalledFunctions2(Function *F, std::set<Function*> &HwFcts);

/// get all non-accelerated functions to be deleted
void getAcceleratedFunctions(Module &M, std::set<GlobalValue*> &HwFcts);

//void replaceAll(std::string &haystack, const std::string &needle, const std::string &replace);

//void stripInvalidCharacters(std::string &str);

//Function * findFuncPtr (Module &M, std::string funcName);
Function * findFuncPtr (Module &M, const char *funcName);

bool isolateGV(Module &M, std::set<GlobalValue*> &Named);

bool deleteGV(std::set<GlobalValue*> &Named);

//void copyArguments (Value* startIdx, Value* endIdx, std::vector<Value*>& newParam);
//std::vector<Value*> copyArguments (Value* startIdx, Value* endIdx);
std::vector<Value*> copyArguments (User::op_iterator startIdx, User::op_iterator endIdx);

/// GetOutputStream - return a stream to the given file
formatted_raw_ostream * GetOutputStream(string & OutputFilename);

/// ReplaceCallWith - This function is used when we want to lower an intrinsic
/// call to a call of an external function.  This handles hard cases such as
/// when there was already a prototype for the external function, and if that
/// prototype doesn't match the arguments we expect to pass in.

// Note: This is defined in a header file due to the template parameter
// Copied from CodeGen/IntrinsicLowering.cpp, added eraseFromParent() call
//template <class ArgIt>

CallInst *ReplaceCallWith(const char *NewFn, CallInst *CI,
                                 //ArgIt ArgBegin, ArgIt ArgEnd,
								 vector<Value*> Args,
								 //std::vector<Value*>::iterator startIdx, std::vector<Value*>::iterator endIdx,
                                 Type *RetTy);

unsigned getBitWidth(const Type* T);

void getInternalAccels(Module &M, std::set<Function *> &internalAccels);

bool replaceAll(std::string &haystack, const std::string &needle,
                const std::string &replace);

void stripInvalidCharacters(std::string &str);

string getWrapperName(Function *F, wrapperType type);

void deleteInstruction(Instruction *I);

Function *CloneFunctionWithNewName(std::string newName, const Function *F, ValueToValueMapTy &VMap,
                              bool ModuleLevelChanges,
                              ClonedCodeInfo *CodeInfo);

Function* insertNewArgument(Module &M, std::string funcName, std::string argName, Type *argType);
//Function *insertNewArgumentsAndReplaceCalls2(Function &Fn, std::set<GlobalValue*> &HwFct, const std::vector<std::string> &argNames, const std::vector<Value*> &argValues);
void replaceCallSites(Function *oldF, Function *newF);

Function *cloneFunction(Module &M, const Function *HwFuncPtr,
                        std::set<GlobalValue *> &HwFcts);

void findCallerFunctions(Function *F, std::set<Function *> &callerSet);
void getParentFunctions(Function *F, std::string Arg, CallGraph &CG,
                        std::set<Function *> &callerSet);

void replaceCallInstructions(Function &Fn);
void copyMetaData(Instruction* Old, Instruction* New);
unsigned int getLoopTripCount(Function::iterator BB, LoopInfo *LI);
std::string printType(const Type * Ty, bool MemAddr);

const char * printIntType (const Type * Ty);
const char * printFloatType (const Type* Ty);

Instruction *preserveLLVMInstruction(Value *V, Instruction *insertBefore);

bool isUsedinFunction(Value *V, Function *F);

raw_ostream &initFileStream(formatted_raw_ostream *&file, std::string fileName);

bool isaCalltoParallelFunction(CallInst *CI);

} // end of legup namespace

#endif
