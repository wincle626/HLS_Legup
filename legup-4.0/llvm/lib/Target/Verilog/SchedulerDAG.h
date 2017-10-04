//===-- SchedulerDAG.h -------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the data structures needed for scheduling.
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_SCHEDULERDAG_H
#define LEGUP_SCHEDULERDAG_H

#include "Allocation.h"
#include "LegupPass.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"

using namespace llvm;

namespace legup {
class FiniteStateMachine;
class LegupConfig;

/// InstructionNode - Container for Instructions to contain schedule-specific
/// information. Contains memory dependencies.
/// TODO: include pseudo-instructions, etc...
/// @brief Legup Instruction container class
class InstructionNode {
public:
    InstructionNode (Instruction *I) : inst(I) {}

    typedef SmallVector<InstructionNode*, 3> depType;
    typedef depType::iterator iterator;
    typedef depType::const_iterator const_iterator;

    // true data dependencies, for example:
    //    %1 = add %2, %3
    //    %4 = add %1, %5
    // %1 depends on %2 and %3
    // %4 depends on %1 and %5
    // another example:
    //    %a = load
    //    store %a
    // Here you would have to delay the store by two (if loads have a
    // 2 cycle latency)
    iterator       dep_begin()       { return depInsts.begin(); }
    const_iterator dep_begin() const { return depInsts.begin(); }
    iterator       dep_end()         { return depInsts.end(); }
    const_iterator dep_end() const   { return depInsts.end(); }

    // in the example above:
    // %2 and %3 are used by %1
    // %1 and %5 are used by %4
    iterator       use_begin()       { return useInsts.begin(); }
    const_iterator use_begin() const { return useInsts.begin(); }
    iterator       use_end()         { return useInsts.end(); }
    const_iterator use_end() const   { return useInsts.end(); }

    // dependencies due to alias analysis
    // these could be RAW, WAR, or WAR dependencies, for example:
    //    store %1
    //    store %2
    // would be a WAW dependency if pointers %1 and %2 alias
    // another example:
    //    %a = load
    //    store %b
    // could be a WAR dependency if %a and %b alias. In this
    // case you should delay the store by ONE cycle (not two as in the case
    // above) because you don't need to wait for the load to finish
    // For example:
    //    %... = load
    //    store %...
    // Calling mem_dep_begin() on the store would return the load since
    // there is a WAR dependency from load -> store. The store 'depends'
    // on the load instruction.
    // iterate over all instructions that "depend" on this memory instruction
    iterator       mem_dep_begin()       { return bbMemDepInsts.begin(); }
    const_iterator mem_dep_begin() const { return bbMemDepInsts.begin(); }
    iterator       mem_dep_end()         { return bbMemDepInsts.end(); }
    const_iterator mem_dep_end() const   { return bbMemDepInsts.end(); }

    // For example:
    //    %... = load
    //    store %...
    // Calling mem_use_begin() on the load would return the store since
    // there is a WAR dependency from load -> store. The load is 'used' by
    // the store instruction.
    // iterate over all instructions that "use" the result of this memory
    // instruction
    iterator       mem_use_begin()       { return bbMemUseInsts.begin(); }
    const_iterator mem_use_begin() const { return bbMemUseInsts.begin(); }
    iterator       mem_use_end()         { return bbMemUseInsts.end(); }
    const_iterator mem_use_end() const   { return bbMemUseInsts.end(); }

    void addDepInst(InstructionNode* in) { depInsts.push_back(in); }
    void addUseInst(InstructionNode* in) { useInsts.push_back(in); }
    void addMemDepInst(InstructionNode* in) { bbMemDepInsts.push_back(in); }
    void addMemUseInst(InstructionNode* in) { bbMemUseInsts.push_back(in); }

    void setDelay (float _delay) {
        delay = _delay;
        assert(delay <= getMaxDelay());
    }
    float getDelay () { return delay; }
    void setAtMaxDelay () { delay = getMaxDelay(); }
    static float getMaxDelay () {
        // 10 ns ie. 100 Mhz
        // 5 ns ie. 200 Mhz
        //return 5
        return LEGUP_CONFIG->getParameterInt("CLOCK_PERIOD");
    }

  // the ASAP methods were relevant with the LegUp 1.0 release
  // deprecated with the SDC scheduler in the LegUp 1.1 release
    void setAsapDelay (int _delay) { asapPropDelay = _delay; }
    float getAsapDelay () { return asapPropDelay; }

    Instruction* getInst() { return inst; }
private:
    depType depInsts;
    depType useInsts;
    depType bbMemUseInsts;
    depType bbMemDepInsts;

    Instruction* inst;

    // propogation delays, used for packing (will change when scheduled)
    // this is the state delay after the instruction executes
    float asapPropDelay;

    // intrinsic delay, calculated by constructor
    float delay;
};

/// LegupSchedulerPass - Builds memory dependencies and Instruction to
/// InstructionNode lookup.
/// @brief Legup Scheduler Memory Dependency Pass
class SchedulerDAG {
public:
    SchedulerDAG () {}
    ~SchedulerDAG();
    bool runOnFunction(Function &F, Allocation *_alloc);

    InstructionNode* getInstructionNode(Instruction *inst) {
        assert (nodeLookup.find(inst) != nodeLookup.end());
        return nodeLookup[inst];
    }

    // print a .dot file for the dependency DFG of basic block BB
    void printDFGDot(formatted_raw_ostream &out, BasicBlock *BB);

private:
    void regDataDeps(InstructionNode *iNode);
    void memDataDeps(InstructionNode *iNode);
    bool hasDependencyAA(Instruction *inst, Instruction *dep);
    void callDataDeps(InstructionNode *iNode);
    void updateDAGwithInst(Instruction *instr);
    void generateDependencies(Instruction *instr);

    AliasAnalysis            *AliasA;

    DenseMap<Instruction *, InstructionNode*> nodeLookup;
    Allocation *alloc;
};

/// SchedulerMapping - Holds InstructionNode to state # mapping.
/// Usage: Pass this mapping between schedulers to effectively chain them.
/// @brief Legup Scheduler data structure for Instruction to state # lookup.
class SchedulerMapping {
public:
    unsigned getState(InstructionNode *i) { return map[i]; }
    void setState(InstructionNode *i, unsigned u) { map[i] = u; }
    unsigned getNumStates(BasicBlock *bb) { return stateNum[bb]; }
    void setNumStates(BasicBlock *bb, unsigned u) { stateNum[bb] = u; }
    FiniteStateMachine *createFSM(Function *, SchedulerDAG *);

private:
    DenseMap<InstructionNode*, unsigned> map;
    DenseMap<BasicBlock *, unsigned> stateNum;
};

} // End legup namespace

#endif
