/**
 * @file SDCScheduler.h
 * @author janders
 * @version
 *
 * @section LICENSE
 *
 * This file is distributed under the LegUp license. See LICENSE for details.
 *
 * @section DESCRIPTION
 *
 * This file implements the SDC-based scheduler.
 */

#ifndef SDCSCHEDULER_H
#define SDCSCHEDULER_H

#include "Scheduler.h"
#include "Allocation.h"
#include "llvm/IR/Instruction.h"
#include <map>

using namespace llvm;

extern "C" { typedef struct _lprec lprec; }

namespace legup {

class Scheduler;
class SchedulerDAG;
class SchedulerMapping;

/**
 * LegUp scheduler based on system of difference constraints (SDC).
 * The basic idea is to formulate the scheduling problem as a linear
 * programming (LP) problem.  In this case, we use the lpsolve package
 * to solve the LP (lpsolve.sourceforge.net).
 *
 * First published by Cong and Zhang, DAC 2006, "An Efficient
 * and Versatile Scheduling Algorithm Based on SDC Formulation"
 */
  class SDCScheduler : public Scheduler {

 public:

    /**
     * Constructor.
     */
    SDCScheduler (Allocation *alloc);

    /**
     * Key function that performs the scheduling.  Only the
     * high-level control is implemented here.
     */
    SchedulerMapping* createMapping(Function *F, SchedulerDAG *dag);
    /**
     * @return true if chaining is permitted for the design.
     */
    bool getChaining() { return chaining; }
    /**
     * Allows control over whether chaining is permitted in the design.
     * @param c Should be set to true if chaining is allows.
     */
    void setChaining(bool c) { chaining = c; }

 private:

    /**
     * The scheduler DAG keeps track of the dependencies
     * between LLVM instructions.  This is needed so things
     * are scheduled in the correct order.  The class also can be
     * queried for the delay of a particular instruction
     */
    SchedulerDAG *dag;

    /**
     * If set to true, chaining of operations in a cycle is permitted.
     * If false, no chaining is allowed -- design will be maximally pipeliend.
     * Default: false
     */
    bool chaining;

    /**
     * The LP problem formulation.  This data structure holds the
     * entire LP formulation: all the constraints, variables, etc.
     * Used by the lpsolve solver.
     */
    lprec *lp;

    /**
     * The number of variables in the LP formulation.
     */
    int numVars;

    /**
     * The number of instructions to be scheduled.
     */
    int numInst;

    /** 
     * Mapping between instructions and variable indicies in the LP formulation.
     * This map holds the starting variable indices for instructions.
     * 
     * Instructions that are not multi-cycle have only a single
     * variable in the LP.  Instructions that take N cycles to
     * complete (N > 1) have N variables in the LP.
     */
    std::map<InstructionNode*, unsigned> startVariableIndex;
    /**
     * Mapping between instructions and variable indices in the LP formulation.
     * This map holds the ending variable indices for instructions.
     */
    std::map<InstructionNode*, unsigned> endVariableIndex;

    /**
     * Creates the mapping from LLVM instructions to variables in the
     * LP formulation.  An instruction that takes N cycles, will have
     * N variables in the LP formulation.  Variables in the LP
     * formulation are represented by numerical indicies.
     *
     * The variables will hold the cycle #s to which instructions
     * are assigned in the schedule.
     */
    void createLPVariables(Function *F);
    /**
     * @return Number of variables in the LP.
     */
    int getNumVariables() {return numVars;}
    /**
     * Add constraints so the LP variables of a multi-cycle
     * instruction are assigned to contiguous clock cycles.
     * See Section 3.1 of Cong's article.  
     */
    void addMulticycleConstraints(Function *F);
    /**
     * Add constraints to the LP formulation to implement correct
     * ordering of operations.
     *
     * These constraints are MANDATORY.
     */
    void addDependencyConstraints(Function *F);
    /**
     * Add the dependency constraints to the LP for a
     * particular instruction.
     *
     * @param in Instruction for which to add dependency constraints.
     */
    void addDependencyConstraints(InstructionNode *in);
    /**
     * Add constraints to the LP formulation to realize the clock
     * period constraints on this design.  This uses the delays of
     * LLVM instructions and the clockPeriodConstraint on the design
     * to schedule things such that the timing constraints are met.
     *
     * These constraints are OPTIONAL.
     *
     * ??? janders what to do about interconnect delay estimations?
     */
    void addTimingConstraints(Function *F);
    /**
     * This recursive function adds timing constraints into the math
     * formulation between a Root node and the dependent nodes of a
     * Curr node (which is in Root's transitive fanin). It then makes
     * a recursive call to itself, where the Curr node is replaced
     * with its dependant nodes.  The partial path delay is the
     * cumulative combinational delay between the Root node and the
     * Curr node.
     * 
     * @param Root A node in the DFG.  
     * @param Curr A node in the DFG in the transitive fanin of the Root.  
     * @param PartialPathDelay The total combinational path delay from Curr to Root.
     */
    void addTimingConstraints(InstructionNode* Root, InstructionNode* Curr, float PartialPathDelay);

    /**
     * Add constraints to the LP formulation so that that "allocation"
     * constraints on the design are met.  For example, we may want to
     * schedule the design such that only a limited # of multipliers
     * are available -- such a constraint is called the "allocation"
     * in HLS nomenclature.
     *
     * @param constrainedFuName is the name of functional unit to constrain
     * @param constraint is the number of functional units available
     *
     * These constraints are semi-OPTIONAL.  Meaning, there may
     * be a limited amount of resources on the device that 
     * make these constraints mandatory.
     */
    void addResourceConstraint(Function *F, std::string
            constrainedFuName, unsigned constraint);
    /**
     * Perform ASAP OR ALAP scheduling.  The workhorse function.
     * 
     * @param ASAP should be set to TRUE if ASAP scheduling is desired; otherwise, ALAP scheduling will be invoked.
     */
    void scheduleAXAP(bool ASAP = true);
    void scheduleASAP();
    void scheduleALAP();

    void addDbgValueInstConstraints(Function *F);

    /**
     * Add constraints so that ALAP scheduling doesn't
     * give an UNBOUNDED solution.
     */
    void addALAPConstraints(Function *F);
    /**
     * Perform slack-driven scheduling.  Maximimize the amount
     * of slack in the final schedule.  This idea is presented
     * in Cong's paper.
     *
     * NOT YET IMPLEMENTED -- placeholder for future development.
     */
    void scheduleSlackDriven();
    /**
     * Perform latency-driven scheduling, driven by a trace-based
     * profiling of how many times certain basic blocks are executed.
     *
     * NOT YET IMPLEMENTED -- this is a placeholder for future development.
     */
    void scheduleLatency();
    /**
     * Print the schedule to the screen.
     */
    void depositSchedule(Function *F);


    /**
     * The functions below are used to perform a second scheduling phase
     * that removes registers for all data paths in certain basic blocks.
     */
    void multicycle_and_modify_schedule(Function *F);
    void save_instruction_states(Function *F);
    void find_instructions_to_delay(Function *F,
            std::map<Instruction*, int> & Instruction_to_state);
    void find_instructions_to_delay_in_BB(BasicBlock *b, 
            std::map<Instruction*, int> & Instruction_to_state);
    void do_post_scheduling_steps(Function *F);
    bool should_scheduling_be_repeated_to_add_latency();


    /**
     * Holds the schedule, i.e., the instruction->cycle# assignment.
     * Also responsible for FSM generation.
     * ??? janders: unclear whether will use this, since it is 
     * tied to basic block scheduling
     */
    SchedulerMapping *map;

    // print out debugging information
    bool SDCdebug;

    void addMultipumpConstraints(Function *F);
    bool isDependent(InstructionNode *source, InstructionNode *dest);
    void add_lp_constraints_for_sdc_multipump(Function *F);

    unsigned* maxCycle;
    Allocation *alloc;

 protected:

};

}

#endif
