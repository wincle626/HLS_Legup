//===-- SDCModuloScheduler.h -----------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation of SDC Modulo Scheduling based on:
//      Canis, Andrew, Stephen D. Brown, and Jason H. Anderson. "Modulo SDC
//      scheduling with recurrence minimization in high-level synthesis." Field
//      Programmable Logic and Applications (FPL), 2014 24th International
//      Conference on. IEEE, 2014.
// Also constains an implementation of:
//      Zhang, Zhiru, and Bin Liu. "SDC-based modulo scheduling for pipeline
//      synthesis." Proceedings of the International Conference on
//      Computer-Aided Design. IEEE Press, 2013.
//
// This class also implements Iterative modulo scheduling
//      based on the paper:
//          B. Ramakrishna Rau, "Iterative Modulo Scheduling", 1995
//      and also the M.Sc thesis:
//          Tanya Lattner, "An implementation of swing modulo scheduling with
//          extensions for superblocks", 2000
//
//===----------------------------------------------------------------------===//

#ifndef ITERATIVEMODULOSCHEDULING_H
#define ITERATIVEMODULOSCHEDULING_H

struct isl_set;

#include "ModuloScheduler.h"
#include "SDCSolver.h"

using namespace llvm;

namespace legup {

class SDCModuloScheduler {
    // class SDCModuloScheduler : public ScopPass {
    // Region *region;
    // Scop *S;
    DominatorTree *DT;
    // ScopDetection *SD;
    // CloogInfo *C;
    DataLayout *TD;
    // Region *R;
    std::vector<std::string> parallelLoops;
    ModuloScheduler moduloScheduler;
    SDCSolver sdcSolver;
    SchedulerDAG *dag;

  public:
    bool runOnLoop(Loop *L, LPPassManager &LPM);
    std::map<Instruction *, int> height;

    std::set<Instruction *> unscheduledInsts;
    std::set<Instruction *> unscheduledInstsConstrained;
    std::queue<Instruction *> unscheduledInstsConstrainedQueue;

    bool noChildren(Instruction *I) {
        return (I->user_begin() == I->user_end());
    }
    raw_fd_ostream &File() { return moduloScheduler.File(); }
    std::map<Instruction *, bool> neverScheduled;
    std::map<Instruction *, int> prevSchedTime;

    BasicBlock *BB;
    LoopInfo *LI;
    ScalarEvolution *SE;
    AliasAnalysis *AA;

    int origRecMII;

    void printLineBreak() {
        File() << "------------------------------------------------------------"
                  "--------------------\n";
    }

    typedef std::map<unsigned, std::list<Instruction *>> IntToInstMapTy;
    // sdcSchedInst maps a control step number to a list of instructions
    // scheduled at that step
    IntToInstMapTy sdcSchedInst;
    std::map<Instruction *, int> sdcSchedTime;
    std::map<Instruction *, int> perturbation;
    unsigned maxCstep;

    SDCModuloScheduler();
    void printModuloReservationTable();
    void computeHeight(Instruction *I);
    void updateHeight(Instruction *I, Instruction *child);
    void computeHeights();
    int numIssueSlots(std::string FuName);
    bool resourceConflict(Instruction *I, int timeSlot);
    int findTimeSlot(Instruction *I, int minTime, int maxTime);
    bool MRTSlotEmpty(int slot, std::string FuName);
    void unscheduleSDC(Instruction *I);
    void unschedule(Instruction *I);
    void schedule(Instruction *I, int timeSlot);
    int predecessorStart(Instruction *I, Instruction *pred);
    int calcEarlyStart(Instruction *I);
    void init();
    Instruction *getHighestPriorityInst();
    bool iterativeClassic(int budget);
    void scheduleSDCInstruction(Instruction *I, int timeSlot);
    bool findEmptySlot(Instruction *I, int timeSlot, int *port);
    bool schedulingConflictSDC(Instruction *I, int timeSlot);
    bool checkFeasible();
    // for instance constrainSDC(I, GE, 0.0)
    //    would constrain I to be scheduled into a cycle >= 0
    // returns a pointer to the constraint object created in the LP
    // to allow future deletion
    SDCSolver::Constraints *constrainSDC(Instruction *I, int constr_type,
                                         REAL constraint);
    // 1) add an equality constraint to the SDC for Instruction at time slot
    // 2) attempt to solve and determine feasibility
    // 3) remove the equality constraint and return feasibility
    // NOTE: this ignores resource conflicts
    bool schedulingConflictSDCIgnoreResources(Instruction *I, int timeSlot);

    // is there a conflict when scheduling operation at time timeSlot?
    bool schedulingConflict(Instruction *I, int timeSlot);
    // save the ASAP schedule steps
    // assuming the SDC is initially solved to find the ASAP schedule for all
    // operations without the presence of resource constraints
    void findASAPTimeForEachInst(map<Instruction *, int> &instStepASAP);
    // the SDC solution was infeasible with the current
    // scheduling constraints for all possible positions
    // that were available for the instruction.
    // We must now backtrack by evicting an already scheduled
    // instruction
    void backtracking(Instruction *I, int cstep);
    // backtracking SDC modulo scheduling
    // can actually evict instructions to achieve the best II
    // when handling resource conflicts
    bool SDCWithBacktracking(int budget);
    // the step the instruction would be scheduled to under ASAP scheduling
    map<Instruction *, int> instStepASAP;

    // in the interests of saving development time, instead of getting this
    // working with STL datastructures I'm going to implement this priority
    // queue
    // naively using an O(n) traversal
    std::list<Instruction *> unscheduledInstPriorityQueue;
    // return the highest priority instr and remove it from the queue
    Instruction *unscheduledInstPriorityQueue_pop();
    void calculatePerturbation();
    bool SDCGreedy();
    // assign the operations that are *not* resource constrained to control
    // steps
    void assignUnconstrainedOperations();
    // budget - max num of operations to be scheduled before giving up
    // II - current initiation interval to attempt
    bool iterativeSchedule(int budget);
    void printMinDistDot(int II);
    int recurrenceMII_SDC(int resourceMII);
    // computing recMII is slightly expensive O(N^3) for N operations in the
    // loop
    // so start II = resourceMII
    // to make this more efficient double the recMII after each failure,
    // then upon success binary search backwards to find the lowest integer.
    int recurrenceMII(int resourceMII);
    // compute the longest path between every pair of vertices
    // return true if there is a positive cycle
    map<Instruction *, map<Instruction *, int>> minDist;
    map<Instruction *, map<Instruction *, int>> minDistCopy;
    // heightR[P] is equal to MinDist[P][STOP] where STOP
    // is a psuedo node that all the leaf operations without successors connect
    // to
    map<Instruction *, int> heightR;
    // initialize minDist for all pairs of operations that have a dependence
    // between them
    void initMinDist(int II);
    // Calculate the minimum number of cycles: minDist(i,j) between scheduling
    // instruction i and j.
    // ie. if instruction i was a load with latency 2 and instruction j used the
    // newly loaded value, then minDist(i,j) would be 2 (assuming i and j are in
    // the same iteration, that their dependency distance = 0)
    bool computeMinDist(int II);
    void resetMinDistForDetectingRecurrences();
    void restructureLoopRecurrences(int resMII);
    void saveMinDistForDetectingRecurrences(int recMII);
    int resourceMII();
    // Taken from LoopUnroll.cpp
    /// RemapInstruction - Convert the instruction operands from referencing the
    /// current values into those specified by VMap.
    typedef std::map<Value *, Value *> ValueToValueMapTy;
    void RemapInstruction(Instruction *I, ValueToValueMapTy &VMap);
    void printMap(map<int, ValueToValueMapTy> &valueMapIter, Value *v,
                  int iter);
    Function *F;
    Module *M;
    void initLoop();
    int getInitialMII();

    // ---------------------------------------------------------------------------
    // SDC scheduler
    // lots of duplicate code here from the standard SDC scheduler
    // ---------------------------------------------------------------------------

    /**
     * Clock period constraint on the design.
     * (from user or automatically-generated).
     */
    float clockPeriodConstraint;
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
    // lprec *lp;

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
    std::map<InstructionNode *, unsigned> startVariableIndex;
    /**
     * Mapping between instructions and variable indices in the LP formulation.
     * This map holds the ending variable indices for instructions.
     */
    std::map<InstructionNode *, unsigned> endVariableIndex;

    // print out debugging information
    bool SDCdebug;

    // create variables in the LP formulation for
    // each of the loop kernel
    void createLPVariables() {
        assert(BB);

        if (sdcSolver.lp != NULL)
            delete_lp(sdcSolver.lp);

        numVars = 0; // LP isn't constructed yet

        // add space for the dummy node
        numVars++;

        numInst = 0; // the number of LLVM instructions to be scheduled
        startVariableIndex.clear();
        endVariableIndex.clear();

        // iterate over the instructions in a BB
        for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
             i++) {
            numInst++;
            InstructionNode *iNode = dag->getInstructionNode(i);
            startVariableIndex[iNode] = numVars;
            int delay = Scheduler::getNumInstructionCycles(i);
            if (isa<StoreInst>(i)) {
                // store you need an extra cycle for the memory to be ready
                delay = 1;
            }
            endVariableIndex[iNode] = numVars + delay;

            if (SDCdebug)
                File() << "Start Index: " << startVariableIndex[iNode]
                       << " End Index: " << endVariableIndex[iNode]
                       << " I: " << *i << "\n";

            numVars += (1 + Scheduler::getNumInstructionCycles(i));
        }

        if (SDCdebug)
            File() << "SDC: # of variables: " << numVars
                   << " # of instructions: " << numInst << "\n";

        // build an empty LP instance with the right # of variables
        sdcSolver.lp = make_lp(0, numVars);

        sdcSolver.clear();

        int idx[1];
        REAL coeff[1];
        idx[0] = 1;     // variable indicies
        coeff[0] = 1.0; // variable coefficients

        // constrain dummy node at index 1 to equal 0
        add_constraintex(sdcSolver.lp, 1, coeff, idx, EQ, 0.0);
    }

    // add multi-cycle instruction constraints
    // these are the the math constraints for the instructions
    // that take multiple cycles to complete, for example, the multiply
    // instruction
    // in the Cyclone II FPGA
    void addMulticycleConstraints() {

        int col[2];
        REAL val[2];

        for (std::map<InstructionNode *, unsigned>::iterator
                 i = startVariableIndex.begin(),
                 e = startVariableIndex.end();
             i != e; i++) {

            InstructionNode *iNode = i->first;
            // Instruction *I = iNode->getInst();
            unsigned startIndex = i->second;

            unsigned endIndex = endVariableIndex[iNode];

            if (startIndex == endIndex)
                continue; // not a multicycle instruction

            // File() << "Multi-cycle operation startIndex: " << startIndex <<
            //    " endIndex: " << endIndex << " I: " << *I << "\n";

            // add constraints so that the variable corresponding to each
            // cycle of a multiple cycle instruction gets assigned to
            // contiguous states.
            for (unsigned j = startIndex + 1; j <= endIndex; j++) {
                col[0] = 1 + j; // variable indicies
                col[1] = 1 + (j - 1);
                val[0] = 1.0; // variable coefficients
                val[1] = -1.0;

                if (SDCdebug) {
                    File() << "Adding constraint: s(" << col[0] - 1 << ") == s("
                           << col[1] - 1 << ") + 1 cycle\n";
                }

                // there must be EXACTLY 1 cycle delay between variable j and
                // j-1
                sdcSolver.addConstraintIncremental(sdcSolver.lp, 2, val, col,
                                                   EQ, 1.0);
            }
        }
    }

    void addTimingConstraintsForKernel() {
        assert(BB);

        // iterate over the instructions in a BB
        for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
             i++) {
            InstructionNode *iNode = dag->getInstructionNode(i);
            std::vector<Instruction *> path;
            path.push_back(i);
            addTimingConstraints(iNode, iNode, iNode->getDelay(), path);
        }
    }

    void addDependencyConstraintsForKernel(int II) {
        assert(BB);

        // iterate over the instructions in a BB
        for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
             i++) {
            addDependencyConstraints(dag->getInstructionNode(i));
        }

        for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
             ++i) {
            for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
                 ++j) {
                if (!moduloScheduler.dependent(i, j))
                    continue;
                int dist = moduloScheduler.distance(i, j);
                if (!dist)
                    continue;

                int col[2];
                REAL val[2];

                // cross iteration dependence from i -> j
                col[0] = 1 + startVariableIndex[dag->getInstructionNode(j)];
                val[0] = 1.0;
                col[1] = 1 + endVariableIndex[dag->getInstructionNode(i)];
                val[1] = -1.0;

                // if chaining is permitted, then the instructions can be in the
                // SAME cycle
                // if chaining is NOT permitted, a dependent instruction is
                // moved to a
                // LATER cycle
                int chainingLatency = chaining ? 0.0 : 1.0;

                if (LEGUP_CONFIG->getParameterInt("SDC_ONLY_CHAIN_CRITICAL")) {
                    // if (moduloScheduler.onCriticalPath(i) &&
                    // moduloScheduler.onCriticalPath(j)) {
                    if (moduloScheduler.onCriticalPath(i)) {
                        // errs() << "sdc critical delay=" <<
                        // moduloScheduler.delay(i) << " :" << *i << "\n";
                        chainingLatency = 0;
                        // assert(moduloScheduler.delay(i) == 0);
                    } else {
                        // errs() << "sdc non-critical delay=" <<
                        // moduloScheduler.delay(i) << " :" << *i << "\n";
                        chainingLatency = 1;
                        // assert(moduloScheduler.delay(i) >= 1);
                    }
                }

                if (isa<StoreInst>(i)) {
                    // already looking at end state
                    assert(chainingLatency == 0);
                }

                // ensure the right ordering or instructions based on
                // dependencies
                //      i -> j
                // constraint:
                //      start of 'j' - end of 'i' >= chaining - II*distance(i,
                //      j)
                // equivalent to:
                //      start of 'j' >= end of 'i' + chaining - II*distance(i,
                //      j)
                File() << "Cross-iteration constraint: start of 'j' >= end of "
                          "'i' + chaining "
                       << "- II*distance(i, j)\n";
                File() << "\tchaining: " << chainingLatency << " II: " << II
                       << " distance: " << dist << "\n";
                File() << "\ti: " << *i << "\n";
                File() << "\tj: " << *j << "\n";
                sdcSolver.addConstraintIncremental(
                    sdcSolver.lp, 2, val, col, GE, chainingLatency - II * dist);
            }
        }
    }

    // add the constraints to the LP so that instructions
    // are scheduled in cycles >= the cycles where their
    // dependencies are computed
    void addDependencyConstraints(InstructionNode *in) {

        int col[2];
        REAL val[2];

        // First make sure each instruction is scheduled into a cycle >= 0
        col[0] = 1 + startVariableIndex[in];
        val[0] = 1.0;
        sdcSolver.addConstraintIncremental(sdcSolver.lp, 1, val, col, GE, 0.0);

        // Now handle the dependencies between instructions: producer/consumer
        // relationships
        for (InstructionNode::iterator i = in->dep_begin(), e = in->dep_end();
             i != e; ++i) {
            // Dependency: depIn -> in
            InstructionNode *depIn = *i;

            col[0] = 1 + startVariableIndex[in];
            val[0] = 1.0;
            col[1] = 1 + endVariableIndex[depIn];
            val[1] = -1.0;

            // if chaining is permitted, then the instructions can be in the
            // SAME cycle
            // if chaining is NOT permitted, a dependent instruction is moved to
            // a
            // LATER cycle
            int chainingLatency = chaining ? 0.0 : 1.0;

            if (LEGUP_CONFIG->getParameterInt("SDC_ONLY_CHAIN_CRITICAL")) {
                // if (moduloScheduler.onCriticalPath(i) &&
                // moduloScheduler.onCriticalPath(j)) {
                Instruction *I = depIn->getInst();
                if (moduloScheduler.onCriticalPath(I)) {
                    // errs() << "sdc critical 1: " << *I << "\n";
                    chainingLatency = 0;
                } else {
                    // errs() << "sdc non-critical 1: " << *I << "\n";
                    chainingLatency = 1;
                }
            }

            // ensure the right ordering or instructions based on data
            // dependency:
            //      depIn -> in
            // constraint:
            //      start of 'in' - end of 'depIn' >= chaining
            // equivalent to:
            //      start of 'in' >= end of 'depIn' + chaining
            int dist =
                moduloScheduler.distance(depIn->getInst(), in->getInst());
            assert(dist == 0);

            Instruction *I1 = in->getInst();
            Instruction *I2 = depIn->getInst();
            if (SDCdebug) {
                File()
                    << "Adding dependency constraint: start(I1) >= end(I2) + "
                    << chainingLatency << " cycle(s). \n"
                    << "\tI1: " << *I1 << "\n"
                    << "\tI2: " << *I2 << "\n";
            }

            sdcSolver.addConstraintIncremental(sdcSolver.lp, 2, val, col, GE,
                                               chainingLatency);

            // sdcSolver.addConstraintIncremental(lp, 2, val, col, GE, chaining
            // ?
            // 0.0 : 1.0);
        }

        for (InstructionNode::iterator i = in->mem_dep_begin(),
                                       e = in->mem_dep_end();
             i != e; ++i) {

            // dependency from memDepIn -> in
            InstructionNode *memDepIn = *i;

            col[0] = 1 + startVariableIndex[in];
            val[0] = 1.0;
            col[1] = 1 + endVariableIndex[memDepIn];
            val[1] = -1.0;

            Instruction *I1 = memDepIn->getInst();
            Instruction *I2 = in->getInst();

            if (SDCdebug) {
                File() << "Adding memory dependency constraint (I1->I2): "
                       << "start(I2) >= end(I1)\n"
                       << "\tI1: " << *I1 << "\n"
                       << "\tI2: " << *I2 << "\n";
            }

            // cross-iteration constraints are handled elsewhere
            // TODO: refactor cross-iteration constraints to be handled here
            assert(moduloScheduler.dependent(I1, I2));
            if (moduloScheduler.distance(I1, I2)) {
                // File() << "Skipping due to distance = " << dist << "\n";
                continue;
            }

            sdcSolver.addConstraintIncremental(sdcSolver.lp, 2, val, col, GE,
                                               0.0);
        }
    }

    void addTimingConstraints(InstructionNode *Root, InstructionNode *Curr,
                              float PartialPathDelay,
                              std::vector<Instruction *> path) {
        int col[2];
        REAL val[2];

        // don't constraint multi-cycle operations
        // dependency has more than 1 cycle latency, so this dependency will
        // already be in another cycle.
        if (Scheduler::getNumInstructionCycles(Root->getInst()) > 0)
            return;
        if (Scheduler::getNumInstructionCycles(Curr->getInst()) > 0)
            return;

        // Walk through the dependencies
        for (InstructionNode::iterator i = Curr->dep_begin(),
                                       e = Curr->dep_end();
             i != e; ++i) {
            // dependency from depNode -> Curr
            InstructionNode *depNode = *i;

            if (Scheduler::getNumInstructionCycles(depNode->getInst()) > 0)
                continue;

            float delay = PartialPathDelay + depNode->getDelay();
            unsigned cycleConstraint =
                ceil((float)delay / (float)clockPeriodConstraint);

            if (cycleConstraint > 0)
                cycleConstraint--;

            path.push_back(depNode->getInst());
            if (cycleConstraint > 0) {
                Instruction *I1 = depNode->getInst();
                Instruction *I2 = Root->getInst();

                if (SDCdebug) {
                    File() << "Found path:\n";
                    for (std::vector<Instruction *>::reverse_iterator
                             j = path.rbegin(),
                             je = path.rend();
                         j != je; ++j) {
                        Instruction *p = *j;
                        File() << "\tdelay: "
                               << dag->getInstructionNode(p)->getDelay() << " "
                               << *p << "\n";
                    }
                    File() << "Path delay:" << delay << "\n";
                }

                // don't add any timing constraints for nodes on recurrences,
                // we want to prioritize getting the best II over meeting
                // timing constraints...
                if (moduloScheduler.onCriticalPath(I1) &&
                    moduloScheduler.onCriticalPath(I2)) {
                    File()
                        << "Skipping timing constraint due to loop recurrence "
                        << "on this path\n";
                    continue;
                }

                if (SDCdebug) {
                    File() << "Adding cycle constraint: I2 >= I1 + "
                           << cycleConstraint
                           << " cycle(s). Detected delay: " << delay
                           << " (period: " << clockPeriodConstraint
                           << ") between I1 -> I2\n"
                           << "\tI1: " << *I1 << "\n"
                           << "\tI2: " << *I2 << "\n";
                }

                // if cycleConstraint == 0, we don't need to add the constraint.
                // the reason is that such constraints are ALREADY present in
                // the LP
                // formulation, as they are depedency constraints  --
                // constraints
                // that express that an operation must happen AFTER the
                // operations
                // producuing results that it depends on
                col[0] = 1 + startVariableIndex[Root];
                val[0] = 1.0;
                col[1] = 1 + endVariableIndex[depNode];
                val[1] = -1.0;
                sdcSolver.addConstraintIncremental(sdcSolver.lp, 2, val, col,
                                                   GE, cycleConstraint);

            } else {
                addTimingConstraints(
                    Root, depNode, delay,
                    path); // recursive call to discover other instructions
            }
        }
    }

    // save SDC schedule result in member variable: sdcSchedInst
    // lpSolve = false if this was just an incremental SDC solution.
    // lpSolve = true if we solved the LP and have a new minimized feasible
    // solution
    void saveSchedule(bool lpSolve) {
        REAL *variables = new REAL[numVars];
        if (lpSolve) {
            get_variables(sdcSolver.lp, variables);
        }

        sdcSchedInst.clear();

        maxCstep = 0;

        // iterate over the instructions in a BB
        for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie;
             i++) {
            int idx = startVariableIndex[dag->getInstructionNode(i)];
            assert(idx < numVars);

            // File() << "Before rounding: " << variables[idx] << "\n";
            unsigned cstep;
            if (lpSolve) {
                cstep = (unsigned)variables[idx];
            } else {
                int cstep_incr =
                    sdcSolver.getD(sdcSolver.FeasibleSoln, idx + 1);
                assert(cstep_incr >= 0);
                cstep = cstep_incr;
            }
            // File() << "After rounding: " << cstep << "\n";
            // int cstep_incr = sdcSolver.getD(sdcSolver.FeasibleSoln, idx + 1);

            // File() << "cstep for var(" << idx << ") sdc: " << cstep << "
            // incr:
            // "
            //    << cstep_incr << "\n";
            // if (LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC")) {
            //    cstep = (unsigned)sdcSolver.getD(sdcSolver.FeasibleSoln, idx);
            //} else {
            //}

            if (SDCdebug)
                File() << "INSTR OPCODE: " << i->getOpcodeName()
                       << " (IDX: " << idx + 1
                       << ") CLOCK CYCLE ASSIGNED: " << cstep << "\n";

            sdcSchedTime[i] = cstep;
            maxCstep = std::max(maxCstep, cstep);

            // only need to save *resource constrained* instructions
            if (!moduloScheduler.isResourceConstrained(i))
                continue;

            sdcSchedInst[cstep].push_back(i);
        }

        if (lpSolve && LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC")) {
            File()
                << "Resetting incremental feasible solution to LP solution\n";
            assert((int)sdcSolver.FeasibleSoln.size() == numVars);
            for (int i = 0; i < numVars; i++) {
                // reset feasible soln:
                unsigned val = (unsigned)variables[i];

                float old = sdcSolver.getD(sdcSolver.FeasibleSoln, i + 1);

                if (old != val) {
                    // changing
                    // File() << "changing Feasible soln idx: " << i+1 << " " <<
                    // old <<
                    //    " -> " << val << "\n";
                }
                sdcSolver.setD(sdcSolver.FeasibleSoln, i + 1, val);
            }

            sdcSolver.verifyFeasibleSoln();

            delete[] variables;
        }
    }

    //
    bool runLPSolver() {

        // In this case, we simply minimize the sum of the starting cycles
        // for all instructions.

        int *variableIndices = new int[numInst];
        REAL *variableCoefficients = new REAL[numInst];

        int count = 0;

        for (std::map<InstructionNode *, unsigned>::iterator
                 i = startVariableIndex.begin(),
                 e = startVariableIndex.end();
             i != e; i++) {

            unsigned varIndex = i->second;
            assert(count < numInst);
            variableIndices[count] = 1 + varIndex;
            variableCoefficients[count] = 1.0;
            count++;
        }

        assert(count == numInst);

        set_obj_fnex(sdcSolver.lp, count, variableCoefficients,
                     variableIndices);
        // if (ASAP)
        set_minim(sdcSolver.lp);
        // else
        // set_maxim(sdcSolver.lp);
        if (SDCdebug)
            write_LP(sdcSolver.lp, stderr);

        if (!SDCdebug)
            set_verbose(sdcSolver.lp, 1);

        int ret = solve(sdcSolver.lp);

        if (SDCdebug) {
            File() << "SDC solver status: " << ret << "\n";
        }

        delete[] variableCoefficients;
        delete[] variableIndices;

        if (ret != 0) {
            File() << "  LP solver returned: " << ret << "\n";
            File() << "  LP solver could not find an optimal solution\n";
            // report_fatal_error("LP solver could not find an optimal
            // solution");
            return false;
        }

        return true;
    }

    void initializeSDC(int II) {
        printLineBreak();
        File() << "initializing SDC constraints for II = " << II << "\n";

        chaining = false; // default is no chaining -- maximally pipelined
        clockPeriodConstraint = -1.0; // default is no clock period constraint

        createLPVariables();

        addMulticycleConstraints();

        chaining = true;

        // avoid chaining for now - along with timing constraints
        // chaining = false;

        if (LEGUP_CONFIG->getParameterInt("SDC_NO_CHAINING")) {
            chaining = false; // no chaining means that the design will be
                              // pipelined as much as possible
        }

        addDependencyConstraintsForKernel(II);

        clockPeriodConstraint = 15; // 66 MHz
        if (LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD")) {
            clockPeriodConstraint =
                (float)LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD");
        }

        if (chaining && clockPeriodConstraint > 0) {
            if (!LEGUP_CONFIG->getParameterInt("SDC_NO_TIMING_CONSTRAINTS")) {
                addTimingConstraintsForKernel();
            }
        }
        printLineBreak();
    }

    // tries to schedule the SDC
    bool scheduleSDC() {
        File()
            << "  Solving SDC LP problem (modulo resource constraints are not "
               "modeled in the formulation)\n";

        assert(sdcSolver.lp && "Must initialize SDC");

        // if (LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC")) {
        //    bool isFeasible = sdcSolver.unprocessed.empty();
        //    if (!isFeasible) {
        //        File() << "Incremental SDC constraints are infeasible\n";
        //        return false;
        //    }
        //    File() << "  Found incremental solution to SDC LP problem\n";
        //    saveSchedule();
        //    return true;
        //}

        // do the scheduling
        bool success = runLPSolver();

        if (success) {
            File() << "  Found solution to SDC LP problem\n";
            saveSchedule(/*lpSolve=*/true);
        }

        if (LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC")) {
            bool isFeasible = sdcSolver.unprocessed.empty();
            File() << "Incremental: " << isFeasible << "\n";
            File() << "Actual: " << success << "\n";
            if (isFeasible != success) {
                // if (isFeasible != success) {
                errs()
                    << "UNEXPECTED! Incremental SDC didn't match LP solver\n";

                sdcSolver.print();
            }
        }

        return success;
    }
};

} // End legup namespace

#endif
