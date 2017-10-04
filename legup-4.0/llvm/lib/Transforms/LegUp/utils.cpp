//===- utils.cpp - LegUp pre-LTO helper functions -------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Legup helper functions
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Debug.h"
#include "LegupConfig.h"
#include "utils.h"
#include "../../Target/Verilog/Ram.h"
#include <string>
#include "llvm/Support/Signals.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Attributes.h"
//#include "llvm/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Support/FileSystem.h"


using namespace llvm;
using namespace std;

namespace legup {

bool isFSub(Instruction *instr) {
    return (instr->getOpcode() == Instruction::FSub);
}
bool isFAdd(Instruction *instr) {
    return (instr->getOpcode() == Instruction::FAdd);
}
bool isFAddSub(Instruction *instr) { return (isFAdd(instr) || isFSub(instr)); }
bool isIAdd(Instruction *instr) {
    return (instr->getOpcode() == Instruction::Add);
}
bool isISub(Instruction *instr) {
    return (instr->getOpcode() == Instruction::Sub);
}

RAM *createEmptyRAM(Module *M, Allocation *alloc, std::string name) {
    // got this LLVM code from the CPP backend:
    //    llc -march=cpp test.ll
    ArrayType* ArrayTy =
        ArrayType::get(IntegerType::get(M->getContext(), 8),
                name.length()+1);

    GlobalVariable* dummyArray = new GlobalVariable(/*Module=*/*M,
            /*Type=*/ArrayTy,
            /*isConstant=*/true,
            /*Linkage=*/GlobalValue::InternalLinkage,
            /*Initializer=*/0, // has initializer, specified below
            /*Name=*/name);

    Constant* init = ConstantDataArray::getString(M->getContext(), name, true);
    dummyArray->setInitializer(init);

    // create a RAM to represent the global shared memory
    RAM *globalRAM = new RAM(dummyArray, alloc);
    globalRAM->setName(name);
    return globalRAM;
}


//copy the metadata from old to new instruction
void copyMetaData(Instruction* Old, Instruction* New) {
    SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
    Old->getAllMetadata(MDs);
    for (SmallVectorImpl<std::pair<unsigned, MDNode *> >::iterator
        MI = MDs.begin(), ME = MDs.end(); MI != ME; ++MI) {
        New->setMetadata(MI->first, MI->second);
    }
}

// looks for calls in function F to any functions not in HwFcts
// if found, add the new function to HwFcts and call addCalledFunctions() on
// the new function
void addCalledFunctions(Function *F, std::set<GlobalValue *> &HwFcts) {

    for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
        for (BasicBlock::iterator I = BB->begin(), EE = BB->end(); I != EE;
                ++I) {
            if (CallInst *CI = dyn_cast<CallInst>(I)) {

                Function *calledFunction = CI->getCalledFunction();
                // ignore indirect function calls
                if (!calledFunction) continue;

                if (HwFcts.find(calledFunction) == HwFcts.end()) {
                    HwFcts.insert(calledFunction);
                    addCalledFunctions(calledFunction, HwFcts);
                }
            }
        }
    }
}

//adds the descendant functions to HwFcts which of a set of Function pointers
void addCalledFunctions2(Function *F, std::set<Function*> &HwFcts) {

    for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB) {
        for (BasicBlock::iterator I = BB->begin(), EE = BB->end(); I != EE;
                ++I) {
            if (CallInst *CI = dyn_cast<CallInst>(I)) {

                Function *calledFunction = CI->getCalledFunction();
                // ignore indirect function calls
                if (!calledFunction) continue;

                if (HwFcts.find(calledFunction) == HwFcts.end()) {
                    HwFcts.insert(calledFunction);
                    addCalledFunctions2(calledFunction, HwFcts);
                }
            }
        }
    }
}

/// get all non-accelerated functions to be deleted
void getAcceleratedFunctions(Module &M,
        std::set<GlobalValue*> &HwFcts) {

    for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        if (LEGUP_CONFIG->isAccelerated(*I) ||
	    (LEGUP_CONFIG->isParallelAccel(*I) && !LEGUP_CONFIG->isPCIeFlow())) {
          HwFcts.insert(I);
          addCalledFunctions(I, HwFcts);
        }
    }

}

// find call instruction with metadata isInternalAccel
// get the called function and insert to internalAccels
void getInternalAccels(Module &M, std::set<Function*> &internalAccels) {

	for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
	    for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE; ++BB) {
			for (BasicBlock::iterator I = BB->begin(), EE = BB->end(); I != EE; ++I) {
			    if (CallInst *CI = dyn_cast<CallInst>(I)) {

                    if (getMetadataInt(CI, "isInternalAccel"))
                        internalAccels.insert(CI->getCalledFunction());
			    }
			}
		}
	}
}

CallInst *ReplaceCallWith(const char *NewFn, CallInst *CI,
								 vector<Value*> Args,
                                 Type *RetTy) {
	// If we haven't already looked up this function, check to see if the
	// program already contains a function with this name.
	Module *M = CI->getParent()->getParent()->getParent();
	// Get or insert the definition now.
	std::vector<Type *> ParamTys;
	std::vector<Value*> Params;
	if (!Args.empty()) {
		for (vector<Value*>::iterator it = Args.begin(); it != Args.end(); ++it) {
			ParamTys.push_back((*it)->getType());
		}
	}

	Constant* FCache = M->getOrInsertFunction(NewFn, FunctionType::get(RetTy, ParamTys, false));

	Instruction * Ins = CI;
	CallInst *NewCI = CallInst::Create(FCache, Args, "", Ins);
	NewCI->setName(CI->getName());
	if (!CI->use_empty()) {
		CI->replaceAllUsesWith(NewCI);
	}

	// CI doesn't get erased, so do it explicitly
	CI->eraseFromParent();
	return NewCI;
}

std::vector<Value*> copyArguments (User::op_iterator startIdx, User::op_iterator endIdx) {
	std::vector<Value*> newParam;
	for (User::op_iterator it = startIdx; it != endIdx; ++it) {
		newParam.push_back(*it);
	}
	return newParam;
}

//find the function pointer with the given function name
Function * findFuncPtr (Module &M, const char *funcName) {
	for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F) {
		if(strcmp(F->getName().str().c_str(), funcName) == 0) {
			return F;
		}
	}
//	errs() << "Function " << *funcName << "not found!\n";
//	assert(0);
	return NULL;
}

void set_all_linkage(Module &M, GlobalValue::LinkageTypes LT) {
  for (Module::global_iterator I = M.global_begin(), E = M.global_end(); I != E;
      ++I)
    if (!I->isDeclaration()) {
      I->setLinkage(LT);
    }
  for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I)
    if (!I->isDeclaration()) {
      I->setLinkage(LT);
    }
}

// copied isolateGV() and deleteGV() from ExtractGV
// TODO: How do I call another pass from here?
// opt: Pass.cpp:234: void<unnamed>::PassRegistrar::RegisterPass(const llvm::PassInfo&): Assertion `Inserted && "Pass registered multiple times!"' failed.
// ModulePass *GV = createGVExtractionPass(GVs, true, false);
// return GV->runOnModule(M);
bool isolateGV(Module &M, std::set<GlobalValue*> &Named) {

  // Mark all globals internal
  // FIXME: what should we do with private linkage?
  set_all_linkage(M, GlobalValue::InternalLinkage);

  // All of the functions may be used by global variables or the named
  // globals.  Loop through them and create a new, external functions that
  // can be "used", instead of ones with bodies.
  std::vector<Function*> NewFunctions;

  Function *Last = --M.end();  // Figure out where the last real fn is.

  for (Module::iterator I = M.begin(); ; ++I) {
    if (std::find(Named.begin(), Named.end(), &*I) == Named.end()) {
      Function *New = Function::Create(I->getFunctionType(),
                                       GlobalValue::ExternalLinkage);
      New->copyAttributesFrom(I);

      // If it's not the named function, delete the body of the function
      I->dropAllReferences();

      M.getFunctionList().push_back(New);
      NewFunctions.push_back(New);
      New->takeName(I);
    }

    if (&*I == Last) break;  // Stop after processing the last function
  }

  // Now that we have replacements all set up, loop through the module,
  // deleting the old functions, replacing them with the newly created
  // functions.
  if (!NewFunctions.empty()) {
    unsigned FuncNum = 0;
    Module::iterator I = M.begin();
    do {
      if (std::find(Named.begin(), Named.end(), &*I) == Named.end()) {
        // Make everything that uses the old function use the new dummy fn
        I->replaceAllUsesWith(NewFunctions[FuncNum++]);

        Function *Old = I;
        ++I;  // Move the iterator to the new function

        // Delete the old function!
        M.getFunctionList().erase(Old);

      } else {
        ++I;  // Skip the function we are extracting
      }
    } while (&*I != NewFunctions[0]);
  }

  // set function linkage back to external
  set_all_linkage(M, GlobalValue::ExternalLinkage);

  return true;
}

bool deleteGV(std::set<GlobalValue*> &Named) {

  // Let a later invocation of clang run Dead-Code Elimination for the PCIe flow rather than here
  if (LEGUP_CONFIG->isPCIeFlow()) {
    return false;
  }

  for (std::set<GlobalValue*>::iterator GI = Named.begin(), 
         GE = Named.end(); GI != GE; ++GI) {
    if (Function* NamedFunc = dyn_cast<Function>(*GI)) {
     NamedFunc->setLinkage(GlobalValue::ExternalLinkage);
     NamedFunc->deleteBody();
     assert(NamedFunc->isDeclaration() && "This didn't make the function external!");
   } else {
      if (!(*GI)->isDeclaration()) {
        cast<GlobalVariable>(*GI)->setInitializer(0);  //clear the initializer
        (*GI)->setLinkage(GlobalValue::ExternalLinkage);
      }
    }
  }
  return true;
}

/// GetOutputStream - return a stream to the given file
formatted_raw_ostream * GetOutputStream(string & OutputFilename) {

    // stdout
    if (OutputFilename == "-")
        return &fouts();

    // Make sure that the Out file gets unlinked from the disk if we get a
    // SIGINT
    //sys::RemoveFileOnSignal(sys::Path(OutputFilename));
    sys::RemoveFileOnSignal((OutputFilename));

    std::string error;
    raw_fd_ostream *FDOut =
        new raw_fd_ostream(OutputFilename.c_str(), error,
                //raw_fd_ostream::F_Binary);
                sys::fs::F_None);
    if (!error.empty()) {
        errs() << error << '\n';
        delete FDOut;
        exit(1);
    }
    formatted_raw_ostream *Out =
        new formatted_raw_ostream(*FDOut, formatted_raw_ostream::DELETE_STREAM);

    return Out;
}

Function* cloneFunction(Module &M, const Function *HwFuncPtr, std::set<GlobalValue*> &HwFcts) {

	std::string FuncName; 
	
//	for (int i=0; i<numOMPthreads; i++) {

		FuncName = "legup_" + HwFuncPtr->getName().str();
		ValueToValueMapTy VMap;
		ClonedCodeInfo *CodeInfo = NULL;
		Function* duplicateFunction = CloneFunctionWithNewName(FuncName, HwFuncPtr, VMap, /*ModuleLevelChanges=*/true, CodeInfo);
		HwFcts.insert(duplicateFunction);
		duplicateFunction->setLinkage(HwFuncPtr->getLinkage());
		//duplicateFunction->dump();
		M.getFunctionList().push_back(duplicateFunction);
//	}
	return duplicateFunction;
}

/// getWrapperName - gives the name of the legup wrapper for a function
/// wrapperType may be seq, call, poll, lock, or unlock
/// if wrapperType is seq, call, or poll, append the function name at the end
string getWrapperName(Function *F, wrapperType type) {
	string wrapperName = "legup_";
    string funcName;
	if (type != pthreadpoll) {
		funcName = F->getName();
	}
    stripInvalidCharacters(funcName);	

	if (type == pthreadcall) {
		wrapperName += "pthreadcall_" + funcName;
	} else if (type == pthreadpoll) {
		wrapperName += "pthreadpoll"; //there is only one pthread polling wrapper for all pthread functions
    } else if (type == ompcall) {
        // there is now only one type of wrapper for OMP
        // both calling and polling are done inside the same wrapper
        wrapperName += "omp_" + funcName;
    } else if (type == seq) {
        wrapperName += "sequential_" + funcName;
    } else if (type == pcie) {
        wrapperName += "pcie_" + funcName;
    } else {
        assert(0 && "Incorrect wrapperType!\n");
    }

    return wrapperName;
}

void deleteInstruction(Instruction *I) {

	if (!I->use_empty()) {
		I->replaceAllUsesWith(UndefValue::get(I->getType()));
	}
	I->eraseFromParent();
}

//this function was copied from CloneFunction in Transforms/Utils/CloneFunction.cpp
Function *CloneFunctionWithNewName(std::string newName, const Function *F, ValueToValueMapTy &VMap,
                              bool ModuleLevelChanges,
                              ClonedCodeInfo *CodeInfo) {
  std::vector<Type*> ArgTypes;

  // The user might be deleting arguments to the function by specifying them in
  // the VMap.  If so, we need to not add the arguments to the arg ty vector
  //
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I)
    if (VMap.count(I) == 0)  // Haven't mapped the argument to anything yet?
      ArgTypes.push_back(I->getType());

//	 Type *Int32Ty = Type::getInt32Ty(F->getParent()->getContext());
//	ArgTypes.push_back(Int32Ty);
  // Create a new function type...
  FunctionType *FTy = FunctionType::get(F->getFunctionType()->getReturnType(),
                                    ArgTypes, F->getFunctionType()->isVarArg());

  // Create the new function...
  Function *NewF = Function::Create(FTy, F->getLinkage(), newName);

//Value *newARG = --NewF->arg_end();
//newARG->setName("numThreads");

  // Loop over the arguments, copying the names of the mapped arguments over...
  Function::arg_iterator DestI = NewF->arg_begin();
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I)
    if (VMap.count(I) == 0) {   // Is this argument preserved?
      DestI->setName(I->getName()); // Copy the name over...
      VMap[I] = DestI++;        // Add mapping to VMap
    }

  SmallVector<ReturnInst*, 8> Returns;  // Ignore returns cloned.
  CloneFunctionInto(NewF, F, VMap, ModuleLevelChanges, Returns, "", CodeInfo);
  return NewF;
}

void replaceCallSites(Function *oldF, Function *newF){

    std::vector<Value*> Args;
    llvm::Instruction *New;
    llvm::Instruction *Before;

    //iterate through all call sites for this function
    while (!oldF->use_empty()) {
        CallSite CS(oldF->user_back());
        //get the call instruction
        Instruction *Call = CS.getInstruction();
        //get call the arguments
        Args.assign(CS.arg_begin(), CS.arg_begin());

        Before = Call;

        //create the new call instruction
        New = llvm::CallInst::Create(newF, Args, "", Before);
        //copy calling convection and attributes
        llvm::cast<llvm::CallInst>(New)->setCallingConv(CS.getCallingConv());
        llvm::cast<llvm::CallInst>(New)->setAttributes(CS.getAttributes());
        //copy all metadata
        copyMetaData(Before, New);

        if (llvm::cast<llvm::CallInst>(Call)->isTailCall())
          llvm::cast<llvm::CallInst>(New)->setTailCall();
        //replace the call
        if (!Call->use_empty())
            Call->replaceAllUsesWith(New);
        //take the name
        New->takeName(Call);
        //delete the old call instruction
        Call->eraseFromParent();
    }
}

//this function copies a function into a new function with the same function body but with an extra argument of type argType
//it returns the pointer to the new function
Function* insertNewArgument(Module &M, std::string funcName, std::string argName, Type *argType) {

	Function * F =	findFuncPtr(M, funcName.c_str());
	assert(F);

    // Start by computing a new prototype for the function, which is the same as
	// the old function, but has an extra argument.
	const FunctionType *FTy = F->getFunctionType();

	/* Copy the argument types and add an extra argument */
	std::vector<Type*> params(FTy->param_begin(), FTy->param_end());
	params.push_back(argType);

	// Make a new parameter attribute list (they are immutable) that has the new
	// argument marked as byval. Since the parameter attributes include the
	// return type parameters at index 0, we don't use size() - 1, but just
	// size() as index.
	//AttrListPtr PAL = F->getAttributes();

	// Create the new function type based on the recomputed parameters.
	FunctionType *NFTy = FunctionType::get(F->getReturnType(), params, false);

	// Create the new function body and insert it into the module...
	//Function *NF = Function::Create(NFTy, F->getLinkage());
	Function *NF = Function::Create(NFTy, Function::ExternalLinkage);
	NF->copyAttributesFrom(F);
	//NF->setAttributes(PAL);
	NF->setAttributes(F->getAttributes());
	F->getParent()->getFunctionList().insert(F, NF);
	//M.getFunctionList().insert(F, NF);
	NF->takeName(F);
	//std::string newfuncName = "legup" + F->getName().str();
	//NF->setName(newfuncName);
	//copy the names of the arguments
	for (Function::arg_iterator AI = F->arg_begin(), AE = F->arg_end(),
		  NAI = NF->arg_begin(); AI != AE; ++AI, ++NAI)
	  NAI->takeName(AI);
	
	Value *newARG = --NF->arg_end();
	newARG->setName(argName);

    //replaceCallSites(F, NF);
    // Since we have now created the new function, splice the body of the old    
	// function right into the new function, leaving the old rotting hulk of the 
	// function empty.
	NF->getBasicBlockList().splice(NF->begin(), F->getBasicBlockList());

	// Replace all uses of the old arguments with the new arguments
	for (llvm::Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
		   NI = NF->arg_begin(); I != E; ++I, ++NI)
	  I->replaceAllUsesWith(NI);


//	return newARG;
	return NF;
}

//this function looks through the call graph to find all parent functions of function F
//but stops going up the graph once it finds an argument name given in Arg
void getParentFunctions(Function *F, std::string Arg, CallGraph &CG, std::set<Function*> &callerSet) {

    std::string funcName = F->getName();
	CallGraphNode *parentNode = CG[findFuncPtr(*(F->getParent()), funcName.c_str())];
	//CallGraphNode *parentNode = CG[F];
	Function *parentF = parentNode->getFunction();
    //errs () << "parentF name " << parentF->getName() << "\n\n";
    //if you find a parent function which has the argument name given, then return
    for (Function::const_arg_iterator I = parentF->arg_begin(), E = parentF->arg_end(); I != E; ++I) {
        if (I->getName() == Arg) 
            return;
    }
    //if you didn't find that argument, then add the function to the set and recurse
    callerSet.insert(parentF);
    getParentFunctions(parentF, Arg, CG, callerSet);
}

void findCallerFunctions(Function *F, std::set<Function *> &callerSet) {

    Function *CallerF;
    Instruction *Ins;

    for (User *UI : F->users()) {
        if ((Ins = dyn_cast<Instruction>(UI))) {
            CallSite CS(Ins);
            Instruction *Call = CS.getInstruction();
            if (Call) {
                // get the current function which contains this call instruction
                CallerF = Call->getParent()->getParent();
                assert(CallerF);
                // else insert into set
                callerSet.insert(CallerF);
                // recurse
                findCallerFunctions(CallerF, callerSet);
            }
        }
    }
}

unsigned int getLoopTripCount(Function::iterator BB, LoopInfo *LI) {

//	LoopInfo *LI;
//	LI = &getAnalysis<LoopInfo>(*pF);
	assert(LI);

	// By default, if there is a hardware accel, its 1 so far
	unsigned int loopTripCount = 1;

	// Check if it actually has a loop
	if (Loop *loop = LI->getLoopFor(BB))
	{
		// Instantiate this many hardware accelerators

		// TODO: LLVM 3.4 udpate changes
		//loopTripCount = loop->getSmallConstantTripCount();

		loopTripCount = 0;

		Value *tripCountVal = 0;

		PHINode *IV = loop->getCanonicalInductionVariable();
		if (IV == 0 || IV->getNumIncomingValues() != 2)
		{
			tripCountVal = 0;
		}
		else
		{
			bool P0InLoop = loop->contains(IV->getIncomingBlock(0));
			Value *Inc = IV->getIncomingValue(!P0InLoop);
			BasicBlock *BackedgeBlock = IV->getIncomingBlock(!P0InLoop);

			if (BranchInst *BI = dyn_cast<BranchInst>(BackedgeBlock->getTerminator())) {
				if (BI->isConditional()) {
					if (ICmpInst *ICI = dyn_cast<ICmpInst>(BI->getCondition())) {
						if (ICI->getOperand(0) == Inc) {
							if (BI->getSuccessor(0) == loop->getHeader()) {
								if (ICI->getPredicate() == ICmpInst::ICMP_NE)
									tripCountVal = ICI->getOperand(1);
							} else if (ICI->getPredicate() == ICmpInst::ICMP_EQ) {
								tripCountVal = ICI->getOperand(1);
							}
						}
					}
				}
			}
		}

		Value* TripCount = tripCountVal;
		if (TripCount) {
			if (ConstantInt *TripCountC = dyn_cast<ConstantInt>(TripCount)) {
				// Guard against huge trip counts.
				if (TripCountC->getValue().getActiveBits() <= 32) {
					loopTripCount = (unsigned)TripCountC->getZExtValue();
				}
			}
		}
		// TODO: end LLVM 3.4 update changes

		// errs() << "\n\n Num Loop Iterations = " << numCallsToHardwareAccel << "\n\n";
	}
	assert(loopTripCount != 0 && "Could not analyze number of threads for the accelerated function");

	return loopTripCount;
}

std::string printType(const Type * Ty, bool MemAddr) {
	std::string str_front = "";
	std::string str_back = "";

	if (dyn_cast<IntegerType>(Ty)) {
		str_front += printIntType(Ty);
	}
	else if (Ty->isFloatingPointTy()) {
		str_front += printFloatType(Ty);
	}
	else if (const PointerType *PTy = dyn_cast<PointerType>(Ty)) {
		const Type * ETy = PTy->getElementType();
		str_front = printType(ETy, MemAddr);
		if (!MemAddr) {
			str_back = "*";
			str_front.append(str_back);
		}
	}
	else if (const ArrayType *ATy = dyn_cast<ArrayType>(Ty)) { //if argument is an array
		const Type * ETy = ATy->getElementType();
		str_front = printType(ETy, MemAddr);
		if (!MemAddr) {
			str_back = "*";
			str_front.append(str_back);
		}	
	}
	else if (dyn_cast<StructType>(Ty)) {
		str_front = "void ";
	}
	else {
		assert(0 && "Unsupported Argument for Accelerator\n");
	}
	return str_front;
}

const char* printIntType (const Type * Ty) {
	int NumBits;
	const char * type;
	const IntegerType *ITy = dyn_cast<IntegerType>(Ty);
	NumBits = ITy->getBitWidth();
	if (NumBits == 1) 
	  	type = "bool ";
	else if (NumBits <= 8)
		type = "char ";
	else if (NumBits <= 16)
		type = "short ";
	else if (NumBits <= 32)
		type = "int ";
	else if (NumBits <= 64)
		type = "long long ";
	else {
		assert(0 && "Unsupported Integer type!\n");
	}
	return type;	
}

const char* printFloatType (const Type * Ty){
	const char * type;
	if (Ty->isFloatTy()) 
		type = "float ";
	else if (Ty->isDoubleTy())
		type = "double ";
	else {
		assert(0 && "Unsupported Floating point type!\n");
	}
	return type;	
}

// this function inserts a dummy call to the IR 
// with the value (that needs to be preserved) as an argument
// this is to prevent LLVM from optimizing away an instruction (which is needed sometimes)
// since it's a dummy call, it won't actually do anything in HW
// in Target/Verilog/utils.cpp, isaDummyCall function detects that legup_preserve_value is a dummy call
Instruction *preserveLLVMInstruction (Value *V, Instruction *insertBefore) {

    std::vector<Value*>params;
    params.push_back(V);

    std::vector<Type*>paramType;
    paramType.push_back(V->getType());

    Module *M = insertBefore->getParent()->getParent()->getParent();
    //create the dummy function definition
    Constant* FCache = M->getOrInsertFunction("__legup_preserve_value", FunctionType::get(Type::getVoidTy(getGlobalContext()), paramType, false));
    //insert the call to the new function
    CallInst *CI = CallInst::Create(FCache, params, "", insertBefore);

    return CI;
}

bool isUsedinFunction(Value *V, Function *F) {

    for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE; ++BB) {
        if (V->isUsedInBasicBlock(BB))
            return true;
    }

    return false;
}

raw_ostream &initFileStream(formatted_raw_ostream *&file,
                            std::string fileName) {

    // make file stream
    file = GetOutputStream(fileName);
    assert(file);
    return *file;
}

// returns true if it is a call to a parallel function
// false otherwise
bool isaCalltoParallelFunction(CallInst *CI) {

    int numThreads = getMetadataInt(CI, "NUMTHREADS");
    // std::string functionType = getMetadataStr(CI, "TYPE");

    if (numThreads > 0)
        return true;
    /*    if (functionType == "legup_wrapper_pthreadcall"
         || functionType == "legup_wrapper_pthreadpoll"
         || functionType == "legup_wrapper_omp"
         || functionType == "omp_function" )
            return true;*/
    else
        return false;
}

} // end of legup namespace
