//===-- Scheduler.h - Scheduler ---------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains class declarations used by scheduling.
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_SCHEDULER_H
#define LEGUP_SCHEDULER_H

#include "FiniteStateMachine.h"
#include "SchedulerDAG.h"
#include <map>
#include <set>

using namespace llvm;

namespace legup {

/// Scheduler - Generic Scheduler interface used for all Legup schedulers
/// Returns a Finite State Machine object for every LLVM function
/// @brief Legup Scheduler Pass interface
class Scheduler {
  public:
    Scheduler() : mapping(0) {}
    virtual ~Scheduler() {
        if (mapping)
            delete mapping;
    }

    /// getFSM - get the finite state machine for a function.
    FiniteStateMachine *getFSM(Function *F) {
        if (fsm.find(F) == fsm.end()) {
            fsm[F] = new FiniteStateMachine();
        }
        return fsm[F];
    }

    // shedule the function and create the fsm
    void schedule(Function *F, SchedulerDAG *dag) {
        mapping = createMapping(F, dag);

        assert(mapping);
        fsm[F] = mapping->createFSM(F, dag);
    }

    // create the instruction -> cycle mapping
    virtual SchedulerMapping *createMapping(Function *F, SchedulerDAG *dag) = 0;

    /**
     * @return Clock period constraint for the design.
     */
    float getClockPeriodConstraint() { return clockPeriodConstraint; }
    /**
     * Allow user to set the target clock period for the design.
     * @param c Target clock period in nanoseconds.
     */
    void setClockPeriodConstraint(float c) { clockPeriodConstraint = c; }

    static bool isaCheapCall(Instruction *instr);
    static bool canChainBefore(Instruction *instr);
    static bool canChainAfter(Instruction *instr);
    static bool canChainAfter(Instruction *instr, Instruction *before);
    static unsigned getNumInstructionCycles(Instruction *instr);
    static unsigned getInitiationInterval(Instruction *instr);
    static Allocation *alloc;

  protected:
    SchedulerMapping *mapping;
    std::map<Function *, FiniteStateMachine *> fsm;
    std::map<BasicBlock *, State *> bbFirstState;
    State *waitState;

    /**
     * Clock period constraint on the design.
     * (from user or automatically-generated).
     */
    float clockPeriodConstraint;
};

} // End legup namespace

#endif
