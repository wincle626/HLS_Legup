//===- PreLTO.cpp - LegUp uses this pass to lower intrinsics --------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This pass lowers intrinsics, allowing the llvm-ld to optimize
// functions calls to lowered intrinsics such as memset and memcpy.
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
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Debug.h"
#include "utils.h"
#include "llvm/Support/ErrorHandling.h"

#define DEBUG_TYPE "LegUp:PreLTO"

using namespace llvm;

namespace legup {

struct LegUp : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    LegUp() : FunctionPass(ID) {}

    virtual bool doInitialization(Module &M) {
       Mod = &M;

       TD = new DataLayout(&M);

       IL = new IntrinsicLowering(*TD);
       return false;
    }

    virtual bool runOnFunction(Function &F) {
        bool modified = false;

        // Examine all the instructions in this function to find the intrinsics
        // that need to be lowered.
        for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE; ++BB) {
            bool localChange = true;

            // the below functions can modify instruction CI invalidating the
            // iterator so we need to keep checking the BB until there are no
            // more changes
            while (localChange) {
                localChange = false;
                for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I !=
                        E; ++I) {
                    if (CallInst *CI = dyn_cast<CallInst>(I)) {

                        Function *calledFunction = CI->getCalledFunction();
                        // ignore indirect function calls
                        if (!calledFunction) continue;

                        localChange = lowerIfIntrinsic(CI, calledFunction);

                        // recheck the BB again
                        if (localChange) {
                            modified = true;
                            break;
                        }
                    }
                }
            }
        }

        return modified;
    }

private:
    Module *Mod;
    IntrinsicLowering *IL;
    const DataLayout* TD;
    bool lowerIfIntrinsic(CallInst *CI, Function *calledFunction);
    bool lowerLegupInstrinsic(CallInst *CI, Function *calledFunction);
    bool lowerOverflowIntrinsic(CallInst *CI, Function *calledFunction);
    void createOverflowSumCarry(CallInst *CI, Instruction* &sum,
            Instruction* &carry);
    void replaceOverflowIntrinsic(CallInst *CI, Instruction *sum, Instruction
            *carry);

    std::string getIntrinsicMemoryAlignment(CallInst *CI);
};

} // end of legup namespace

using namespace legup;


/// This is used to keep track of intrinsics that get generated to a lowered
/// function. We must generate the prototypes before the function body which
/// will only be expanded on first use (by the loop below).
/// Note: This function was adapted from lowerIntrinsics() in CBackend.cpp
bool LegUp::lowerIfIntrinsic(CallInst *CI, Function *calledFunction) {

    switch (calledFunction->getIntrinsicID()) {
      case Intrinsic::not_intrinsic:
      //case Intrinsic::memory_barrier: # replaced by fence atomic
      case Intrinsic::vastart:
      case Intrinsic::vacopy:
      case Intrinsic::vaend:
      case Intrinsic::returnaddress:
      case Intrinsic::frameaddress:
      case Intrinsic::setjmp:
      case Intrinsic::longjmp:
      case Intrinsic::prefetch:
      case Intrinsic::powi:
      case Intrinsic::x86_sse_cmp_ss:
      case Intrinsic::x86_sse_cmp_ps:
      case Intrinsic::x86_sse2_cmp_sd:
      case Intrinsic::x86_sse2_cmp_pd:
      case Intrinsic::ppc_altivec_lvsl:
          // We directly implement these intrinsics
          return false;

      case Intrinsic::dbg_declare:
      case Intrinsic::dbg_value:
    	  // This is just debug info - it doesn't affect program
    	  // behavior
    	  return false;

      case Intrinsic::memcpy:
      case Intrinsic::memmove:
      case Intrinsic::memset:
          return lowerLegupInstrinsic(CI, calledFunction);

      case Intrinsic::uadd_with_overflow:
          return lowerOverflowIntrinsic(CI, calledFunction);

      default:

          // All other intrinsic calls we must lower.
          DEBUG(errs() << "Lowering: " << *CI << "\n");
          IL->LowerIntrinsicCall(CI);

          return true;
    }

}

// determine the type of the memcpy, memmove, memset destination pointer
const Type *get_dest_ptr_type(CallInst *CI) {
    const Value *destOp = CI->getOperand(0);
    PointerType *destPtr;
    if (const BitCastInst *bci = dyn_cast<BitCastInst>(destOp)) {
        destPtr = dyn_cast<PointerType>(bci->getSrcTy());
    } else if (const GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(
                destOp)) {
        destPtr = dyn_cast<PointerType>(gep->getPointerOperand()->getType());
    } else if (PointerType *p = dyn_cast<PointerType>(destOp->getType())) {
        destPtr = p;
    } else {
        errs() << "CallInst: " << *CI << "\n";
        errs() << "Dest Pointer: " << *destOp << "\n";
        llvm_unreachable("Unknown pointer destination in intrinsic argument");
    }

    const Type *destType = destPtr->getElementType();
    while (const ArrayType *at = dyn_cast<ArrayType>(destType)) {
        destType = at->getElementType();
    }
    return destType;
}

std::string LegUp::getIntrinsicMemoryAlignment(CallInst *CI) {

    std::string legupFunctionAlign;

    const Type *destType = get_dest_ptr_type(CI);

    // use modified struct size if first argument is a struct pointer
    if (isa<StructType>(destType)) {
        // Get the alignment, and decide which memcpy to use

        // the following code would always work if clang supported the
        // -malign-double flag
        // we need the above code to handle 64-bit aligned accesses on a 32-bit
        // machine
        ConstantInt *align = dyn_cast<ConstantInt>(CI->getOperand(3));
        for (int i = 8; i > 0; i >>= 1) {
            if (align->equalsInt(i)) {
                legupFunctionAlign = utostr(i);
            }
        }
    } else if (const IntegerType *it = dyn_cast<IntegerType>(destType)) {
        legupFunctionAlign = utostr(it->getBitWidth() / 8);
    } else if (isa<PointerType>(destType)) {
        // pointers are 32 bits
        legupFunctionAlign = "4";
    } else if (destType->isFloatTy()) {
        // floats are 32 bits
        legupFunctionAlign = "4";
    } else if (destType->isDoubleTy()) {
        // double are 64 bits
        legupFunctionAlign = "8";
    } else {
        errs() << "CallInst: " << *CI << "\n";
        llvm_unreachable("unknown destination pointer type");
    }

    return legupFunctionAlign;
}

void LegUp::createOverflowSumCarry(CallInst *CI, Instruction* &sum,
        Instruction* &carry) {
    Value *op0 = CI->getArgOperand(0);
    Value *op1 = CI->getArgOperand(1);
    unsigned size = op0->getType()->getPrimitiveSizeInBits();
    unsigned newSize = size + 1;

    IntegerType * newType = IntegerType::get(Mod->getContext(),
            newSize);

    string name = "overflow_intrinsic";
    // assume llvm.uadd.* so we zero extend
    Instruction *zextOp0 = CastInst::CreateZExtOrBitCast(op0, newType,
            name, CI);
    Instruction *zextOp1 = CastInst::CreateZExtOrBitCast(op1, newType,
            name, CI);

    Instruction *add = BinaryOperator::Create(Instruction::Add, zextOp0,
            zextOp1, name, CI);

    sum = CastInst::CreateTruncOrBitCast(add, op0->getType(),
            name + "_sum", CI);

    APInt ap1 = APInt(newSize, size);
    Constant *one = ConstantInt::get(newType, ap1);
    Instruction *shift = BinaryOperator::Create(Instruction::LShr, add,
            one, name, CI);

    IntegerType * sizeOne = IntegerType::get(Mod->getContext(), 1);
    carry = CastInst::CreateTruncOrBitCast(shift, sizeOne,
            name + "_carry", CI);

}

void LegUp::replaceOverflowIntrinsic(CallInst *CI, Instruction *sum,
        Instruction *carry) {


    // find sum/carry extractvalue
    // sum: %108 = extractvalue %0 %uadd.i, 0
    // carry: %109 = extractvalue %0 %uadd.i, 1
    vector<Instruction*> dead;
    for (Value::user_iterator i = CI->user_begin(), e = CI->user_end(); i !=
            e; ++i) {
        if (ExtractValueInst *EV = dyn_cast<ExtractValueInst>(*i)) {
            Instruction *replace = NULL;
            // sum
            if (*EV->idx_begin() == 0) {
                replace = sum;
            // carry
            } else {
                assert(*EV->idx_begin() == 1);
                replace = carry;
            }
            DEBUG(errs() << "Replacing " << *EV << "\n");
            DEBUG(errs() << "With " << *replace << "\n");
            EV->replaceAllUsesWith(replace);
            dead.push_back(EV);

        } else {
            errs() << **i << "\n";
            llvm_unreachable("Expecting extractvalue for overflow intrinsic\n");
        }
    }

    // remove dead extractvalue instructions
    for (vector<Instruction*>::iterator i = dead.begin(), e =
            dead.end(); i != e; ++i) {
        (*i)->eraseFromParent();
    }
    CI->eraseFromParent();
}


// Handle uadd.with.overflow.* intrinsics. For example:
//      %uadd.i = call %0 @llvm.uadd.with.overflow.i64(i64 %105, i64 %106)
//      %108 = extractvalue %0 %uadd.i, 0
//      %109 = extractvalue %0 %uadd.i, 1
// Replace this n-bit addition with a (n + 1) bit addition and shift out the
// carry bit manually
bool LegUp::lowerOverflowIntrinsic(CallInst *CI, Function *calledFunction) {

    DEBUG(errs() << "Lowering overflow intrinsic: " << *CI << "\n");


    Instruction *sum = 0, *carry = 0;

    createOverflowSumCarry(CI, sum, carry);

    replaceOverflowIntrinsic(CI, sum, carry);

    return true;
}


// handle memcpy, memmove, memset by replacing with legup specific functions
bool LegUp::lowerLegupInstrinsic(CallInst *CI, Function *calledFunction) {

    DEBUG(errs() << "Lowering for LegUp: " << *CI << "\n");

    std::string legupFunction;
    switch (calledFunction->getIntrinsicID()) {
      case Intrinsic::memcpy:
          legupFunction = "legup_memcpy";
          break;
      case Intrinsic::memmove:
          legupFunction = "legup_memmove";
          break;
      case Intrinsic::memset:
          legupFunction = "legup_memset";
          break;
    }

    std::string fullLegupFunction =
        legupFunction + "_" + getIntrinsicMemoryAlignment(CI);

    // we only need the first 3 operands
    vector<Value *> Ops;
    Ops.push_back(CI->getOperand(0));
    Ops.push_back(CI->getOperand(1));
    Ops.push_back(CI->getOperand(2));

    if (CI->getOperand(2)->getType()->isIntegerTy(64))
        fullLegupFunction += "_i64";

    DEBUG(errs() << "Replacing calls with: " << fullLegupFunction << "\n");
    ReplaceCallWith(fullLegupFunction.c_str(), CI, Ops,
                    calledFunction->getReturnType());

    return true;
}
char LegUp::ID = 0;
static RegisterPass<LegUp> X("legup-prelto",
        "Pre-Link Time Optimization Pass to lower intrinsics");
