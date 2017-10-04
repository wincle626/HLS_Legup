//===- PostLTO.cpp - LegUp may use this pass after linking ----------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This pass lowers certain instructions after link time optimizations.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace legup {
struct PostLTO : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  PostLTO() : FunctionPass(ID) {};

virtual bool doInitialization(Module &M) {
  Mod = &M;
  return false;
}

/// lowerSDIV - Given an SDiv expressing a divide by constant,
/// replace it by multiplying by a magic number.  See:
/// <http://the.wall.riscom.net/books/proc/ppc/cwg/code2.html>
bool lowerSDiv(Instruction *inst) const {
  ConstantInt *Op1 = dyn_cast<ConstantInt>(inst->getOperand(1));
  // check if dividing by a constant and not by a power of 2
  if (!Op1 || inst->getType()->getPrimitiveSizeInBits() != 32 ||
          Op1->getValue().isPowerOf2()) {
      return false;
  }

  BasicBlock::iterator ii(inst);
  Value *Op0 = inst->getOperand(0);
  APInt d = Op1->getValue();

  APInt::ms magics = Op1->getValue().magic();

  IntegerType * type64 = IntegerType::get(Mod->getContext(), 64);

  Instruction *sext = CastInst::CreateSExtOrBitCast(Op0, type64, "", inst);
  APInt m = APInt(64, magics.m.getSExtValue());
  Constant *magicNum = ConstantInt::get(type64, m);

  Instruction *magInst = BinaryOperator::CreateNSWMul(sext, magicNum, "", inst);

  APInt ap = APInt(64, 32);
  Constant *movHiConst = ConstantInt::get(type64, ap);
  Instruction *movHi = BinaryOperator::Create(Instruction::AShr, magInst, 
    movHiConst, "", inst);
  Instruction *trunc = CastInst::CreateTruncOrBitCast(movHi, inst->getType(),
    "", inst);
  if (d.isStrictlyPositive() && magics.m.isNegative()) {
    trunc = BinaryOperator::Create(Instruction::Add, trunc, Op0, "", inst);
  } else if (d.isNegative() && magics.m.isStrictlyPositive()) {
    trunc = BinaryOperator::Create(Instruction::Sub, trunc, Op0, "", inst);
  }
  if (magics.s > 0) {
    APInt apS = APInt(32, magics.s);
    Constant *magicShift = ConstantInt::get(inst->getType(), apS);
    trunc = BinaryOperator::Create(Instruction::AShr, trunc,
      magicShift, "", inst);
  }

  APInt ap31 = APInt(32, 31);
  Constant *thirtyOne = ConstantInt::get(inst->getType(), ap31);
  // get sign bit
  Instruction *sign = BinaryOperator::Create(Instruction::LShr, trunc,
    thirtyOne, "", inst);
  
  Instruction *result = BinaryOperator::Create(Instruction::Add, trunc, sign,
    "");
  ReplaceInstWithInst(inst->getParent()->getInstList(), ii, result);
  return true;
}

/// lowerUDIV - Given an UDiv expressing a divide by constant,
/// replace it by multiplying by a magic number.  See:
/// <http://the.wall.riscom.net/books/proc/ppc/cwg/code2.html>
bool lowerUDiv(Instruction *inst) const {
  ConstantInt *Op1 = dyn_cast<ConstantInt>(inst->getOperand(1));
  // check if dividing by a constant
  if (!Op1 || inst->getType()->getPrimitiveSizeInBits() != 32) {
      return false;
  }

  BasicBlock::iterator ii(inst);
  Value *Op0 = inst->getOperand(0);

  APInt::mu magics = Op1->getValue().magicu();

  IntegerType * type64 = IntegerType::get(Mod->getContext(), 64);

  Instruction *zext = CastInst::CreateZExtOrBitCast(Op0, type64, "", inst);
  APInt m = APInt(64, magics.m.getZExtValue());
  Constant *magicNum = ConstantInt::get(type64, m);

  Instruction *magInst = BinaryOperator::CreateNSWMul(zext, magicNum, "", inst);

  if (!magics.a) {
    APInt apS = APInt(64, 32 + magics.s);
    Constant *magicShift = ConstantInt::get(type64, apS);
    Instruction *bigResult = BinaryOperator::Create(Instruction::LShr, magInst,
      magicShift, "", inst);
    Instruction *result = CastInst::CreateTruncOrBitCast(bigResult,
      inst->getType(), "");

    ReplaceInstWithInst(inst->getParent()->getInstList(), ii, result);
  } else {
    APInt ap = APInt(64, 32);
    Constant *movHiConst = ConstantInt::get(type64, ap);
    Instruction *movHi = BinaryOperator::Create(Instruction::LShr, magInst, 
      movHiConst, "", inst);
    Instruction *trunc = CastInst::CreateTruncOrBitCast(movHi, inst->getType(),
      "", inst);
    Instruction *sub = BinaryOperator::Create(Instruction::Sub, Op0, trunc, "",
      inst);
    APInt ap1 = APInt(32, 1);
    Constant *one = ConstantInt::get(inst->getType(), ap1);
    Instruction *shift = BinaryOperator::Create(Instruction::LShr, sub, one, "",
      inst);
    Instruction *add = BinaryOperator::Create(Instruction::Add, shift, trunc,
      "", inst);
    APInt apShr = APInt(32, magics.s - 1);
    Constant *shr = ConstantInt::get(inst->getType(), apShr);
    Instruction *result = BinaryOperator::Create(Instruction::LShr, add,
      shr, "");

    ReplaceInstWithInst(inst->getParent()->getInstList(), ii, result);
  }
  return true;
}

virtual bool runOnFunction(Function &F) {
  bool isChanged = false;
  for (Function::iterator BB = F.begin(), EE = F.end(); BB != EE; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E;) {
        Instruction *instr = I;
      ++I;
      if (instr->getOpcode() == Instruction::UDiv) {
        isChanged |= lowerUDiv(instr);
      } else if (instr->getOpcode() == Instruction::SDiv) {
        isChanged |= lowerSDiv(instr);
      }
    }
  }
  return isChanged;
}

private:
  Module *Mod;
};

}

using namespace legup;

char PostLTO::ID = 0;
static RegisterPass<PostLTO> X("legup-postlto",
  "this pass lowers some instructions as optimizations");

