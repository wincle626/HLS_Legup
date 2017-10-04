//===-- Scheduler.cpp - Scheduling Pass -------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Legup Scheduler passes.
//
//===----------------------------------------------------------------------===//

#include "RTL.h"
#include "Ram.h"
#include "llvm/ADT/StringExtras.h"
#include "Scheduler.h"
#include "LegupConfig.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace legup;


Allocation* Scheduler::alloc = NULL;

// getNumInstructionCycles - return the number of cycles for an instruction to
// complete
unsigned Scheduler::getNumInstructionCycles(Instruction *instr) {

    // store should always have 1 cycle of latency (unlike a load)
    if (isa<StoreInst>(instr)) {
        return 1;
    }

    assert(alloc);
    std::string FuName = LEGUP_CONFIG->getOpNameFromInst(instr, alloc);
    int constraint;

    if (LEGUP_CONFIG->getOperationLatency(FuName, &constraint)) {
        return constraint;
    }

    if (isa<CallInst>(instr)) {
        if (isaDummyCall(instr)) {
            return 0;
        } else {
            return 1;
        }
    }

    //if (dyn_cast<FPExtInst>(instr)){
    //    return 2;
    //}

    //if (dyn_cast<FPTruncInst>(instr)){
    //    return 3;
    //}

    //if (dyn_cast<SIToFPInst>(instr)){
    //    return 6;
    //}

    //if (dyn_cast<FPToSIInst>(instr)){
    //    return 6;
    //}

    if (const FCmpInst *cmp = dyn_cast<FCmpInst>(instr)){
        switch (cmp->getPredicate()) {
            case (FCmpInst::FCMP_OEQ):
            case (FCmpInst::FCMP_UEQ):
            case (FCmpInst::FCMP_ONE):
            case (FCmpInst::FCMP_UNE):
            case (FCmpInst::FCMP_OLE):
            case (FCmpInst::FCMP_ULE):
            case (FCmpInst::FCMP_OLT):
            case (FCmpInst::FCMP_ULT):
            case (FCmpInst::FCMP_OGE):
            case (FCmpInst::FCMP_UGE):
            case (FCmpInst::FCMP_OGT):
            case (FCmpInst::FCMP_UGT):
                return 1;
            default:
                return 0;
        }
    }

    switch (instr->getOpcode()) {
    case (Instruction::UDiv):
    case (Instruction::URem):
    case (Instruction::SDiv):
    case (Instruction::SRem): {
        unsigned pipelineDepth;

        pipelineDepth = getBitWidth(instr->getType());
        if (LEGUP_CONFIG->getParameter("DIVIDER_MODULE") == "generic") {
            pipelineDepth++;
        }
        return pipelineDepth;
    }
    case (Instruction::Mul):
        assert(alloc);
          return 0;
      case (Instruction::Store):
          return 1;
      case (Instruction::Load):
          {
              assert(alloc);
              RAM *ram = alloc->getLocalRamFromInst(instr);
              if (ram && !alloc->isRAMGlobal(ram)) {
                  // local memory
                  return ram->getLatency(alloc);
              } else {
                  // global memory
                  return getGlobalMemLatency();
              }
          }
//Floating point core latencies (fixed) chosen based on Jack's experiment.
//Lower latency causes Fmax to drop.  
//Latencies are specified through Quartus GUI when generating the cores
//      case (Instruction::FAdd):
//          return 14;
//      case (Instruction::FSub):
//          return 14;
//      case (Instruction::FMul):
//          return 11;
//      case (Instruction::FDiv):{
//          int width = instr->getOperand(0)->getType()->getPrimitiveSizeInBits();
//          if (width == 32)
//              return 33;
//          else
//              return 61;
//      }
      default:
	return 0;
   }
}

/**
 * Determine the initiation interval for a function.
 * This is used for shared resources and affects scheduling.
 * For example, one may want to have a divider functional unit
 * that takes, say 10 cycles to complete (latency), with 
 * the unit capable of starting a new division every 2 cycles (initiation interval).
 * 
 * @param instr The instruction we wish to find the II for.
 */
unsigned Scheduler::getInitiationInterval(Instruction *instr) {

  switch(instr->getOpcode()) {
    default:
      return 1;
  }
}
