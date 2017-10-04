//===-- SchedulerDAG.cpp -----------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the data structures needed for scheduling.
//
//===----------------------------------------------------------------------===//

#include "FiniteStateMachine.h"
#include "SchedulerDAG.h"
#include "Scheduler.h"
#include "State.h"
#include "LegupPass.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/ErrorHandling.h"
#include "utils.h"
#include "LegupConfig.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/AliasAnalysis.h"

using namespace llvm;
using namespace legup;

namespace legup {

// Add dependencies for all the operands of iNode
// ie. %3 = add %1, %2
// %3 is dependent on %1 and %2
// %1 is used by %3
// %2 is used by %3
void SchedulerDAG::regDataDeps(InstructionNode *iNode) {
    Instruction *inst = iNode->getInst();
    // these instructions are not scheduled
    if (isa<AllocaInst>(inst) || isa<PHINode>(inst))
        return;
    for (User::op_iterator i = inst->op_begin(), e = inst->op_end(); i != e;
         ++i) {
        // we only care about operands that are created by other instructions
        Instruction *dep = dyn_cast<Instruction>(*i);
        // also ignore if the dependency is an alloca
        if (!dep || isa<AllocaInst>(dep))
            continue;

        // ignore operands from other basic blocks, these are
        // guaranteed to be in another state
        if (dep->getParent() != inst->getParent())
            continue;

        InstructionNode *depNode = getInstructionNode(dep);
        iNode->addDepInst(depNode);
        depNode->addUseInst(iNode);
    }
}

// returns true if there is a dependency from I1 -> I2
// based on alias analysis
bool SchedulerDAG::hasDependencyAA(Instruction *I1, Instruction *I2) {
    AliasAnalysis::Location Loc1, Loc2;

    if (isa<CallInst>(I1)) {
        // assume that any loads/stores after a call must indeed execute
        // AFTER the call
        return true;
    }

    // bool store = false;
    if (LoadInst *lInst = dyn_cast<LoadInst>(I1)) {
        Loc1 = AliasA->getLocation(lInst);
    } else if (StoreInst *sInst = dyn_cast<StoreInst>(I1)) {
        Loc1 = AliasA->getLocation(sInst);
        // store = true;
    } else {
        assert(0 && "Unexpected input");
    }

    if (isa<StoreInst>(I1) && isa<LoadInst>(I2)) {
        // RAW dependency:
        // I1 is a store and I2 is a load from potentially the same address
        LoadInst *lInst = dyn_cast<LoadInst>(I2);
        Loc2 = AliasA->getLocation(lInst);
        if (!AliasA->isNoAlias(Loc1, Loc2)) {
            return true;
        }

    } else if (isa<StoreInst>(I2)) {
        // WAW or WAR dependency:
        // I1 is a store OR a load and I2 is a store to
        // potentially same address
        StoreInst *sInst = dyn_cast<StoreInst>(I2);
        Loc2 = AliasA->getLocation(sInst);
        if (!AliasA->isNoAlias(Loc1, Loc2)) {
            return true;
        }
    } else {
        // RAR: no dependency
        assert(isa<LoadInst>(I1) && isa<LoadInst>(I2));
    }

    return false;
}

// find all memory dependencies: I1 -> I2 (where I2 is given)
void SchedulerDAG::memDataDeps(InstructionNode *I2Node) {
    Instruction *I2 = I2Node->getInst();
    BasicBlock *bb = I2->getParent();

    // loop over all candidates for I1 in the BB for dependencies I1 -> I2
    for (BasicBlock::iterator dep = bb->begin(), ie = bb->end(); dep != ie;
         ++dep) {
        Instruction *I1 = dep;

        // If we reach I2 then there are no more candidates for I1
        if (I1 == I2)
            break;

        if (!isa<LoadInst>(I1) && !isa<StoreInst>(I1) && !isa<CallInst>(I1))
            continue;

        if (isaDummyCall(I1))
            continue;

        if (hasDependencyAA(I1, I2)) {
            // errs() << "memDataDeps: I1 -> I2\n" <<
            //    "I1: " << *I1 << "\n" <<
            //    "I2: " << *I2 << "\n";
            InstructionNode *I1Node = getInstructionNode(I1);
            I2Node->addMemDepInst(I1Node);
            I1Node->addMemUseInst(I2Node);
        }
    }
}

// find all memory dependencies: I1 -> I2 (where I2 is given)
void SchedulerDAG::callDataDeps(InstructionNode *I2Node) {
    Instruction *I2 = I2Node->getInst();
    BasicBlock *b = I2->getParent();

    if (isaDummyCall(I2) && !isaPrintCall(I2))
        return;

    RAM *localMem = NULL;
    if (LEGUP_CONFIG->getParameterInt("LOCAL_RAMS") &&
        (isa<LoadInst>(I2) || isa<StoreInst>(I2))) {
        assert(alloc);
        localMem = alloc->getLocalRamFromInst(I2);
    }

    // loop over all candidates for I1 in the BB for dependencies I1 -> I2
    for (BasicBlock::iterator dep = b->begin(), ie = b->end(); dep != ie;
         ++dep) {
        Instruction *I1 = dep;

        // If we reach I2 then there are no more candidates for I1
        if (I1 == I2)
            break;

        if (!isa<LoadInst>(I1) && !isa<StoreInst>(I1) && !isa<CallInst>(I1))
            continue;

        if (isaDummyCall(I1) && !isaPrintCall(I1))
            continue;

        if (isa<LoadInst>(I1) || isa<StoreInst>(I1)) {
            RAM *depMem = alloc->getLocalRamFromInst(I1);
            if (localMem && depMem) {
                // only make them dependent if they point to the same memory
                if (localMem == depMem) {
                    if (LEGUP_CONFIG->getParameterInt("ALIAS_ANALYSIS")) {
                        if (!hasDependencyAA(I1, I2))
                            continue;
                    }
                    InstructionNode *I1Node = getInstructionNode(I1);
                    I2Node->addMemDepInst(I1Node);
                    I1Node->addMemUseInst(I2Node);
                }
                continue;
            }
        }

        InstructionNode *I1Node = getInstructionNode(I1);
        I2Node->addMemDepInst(I1Node);
        I1Node->addMemUseInst(I2Node);
    }
}

SchedulerDAG::~SchedulerDAG() {
    for (DenseMap<Instruction *, InstructionNode *>::iterator
             i = nodeLookup.begin(),
             e = nodeLookup.end();
         i != e; ++i) {
        delete i->second;
    }
}

// hasNoDelay - detect instructions which have no delay. For example a shift by
// constant will just be turned into a wire by Quartus
bool hasNoDelay(Instruction *instr) {
    if (instr->isShift()) {
        return (isa<ConstantInt>(instr->getOperand(1)));
    }
    if (isa<GetElementPtrInst>(instr)) {
        if (LEGUP_CONFIG->getParameterInt("DONT_CHAIN_GET_ELEM_PTR")) {
            return false;
        }
        for (unsigned i = 1; i < instr->getNumOperands(); i++) {
            if (!isa<ConstantInt>(instr->getOperand(i))) {
                return false;
            }
        }
        return true;
    }

    switch (instr->getOpcode()) {
    case (Instruction::And):
    case (Instruction::Or):
        return (isa<ConstantInt>(instr->getOperand(1)) ||
                getBitWidth(instr->getType()) == 1);
    }
    return (instr->isCast() || isa<PHINode>(instr) || isa<AllocaInst>(instr) ||
            instr->isTerminator() || isa<LoadInst>(instr) ||
            isa<CallInst>(instr));
}

void SchedulerDAG::updateDAGwithInst(Instruction *instr) {
    // generate Instruction to InstructionNode mapping
    InstructionNode *iNode = new InstructionNode(instr);
    nodeLookup[instr] = iNode;

    // set delay
    std::string opName = LEGUP_CONFIG->getOpNameFromInst(instr, alloc);
    if (opName.empty() || isMem(instr)) {
        if (isa<GetElementPtrInst>(instr)) {
            if (LEGUP_CONFIG->getParameterInt("DONT_CHAIN_GET_ELEM_PTR")) {
                iNode->setAtMaxDelay();
            } else if (hasNoDelay(instr)) {
                iNode->setDelay(0);
            } else {
                iNode->setAtMaxDelay();
            }
        } else if (hasNoDelay(instr)) {
            iNode->setDelay(0);
        } else {
            // errs() << "Empty: " << *instr << "\n";
            // assert(hasNoDelay(instr));
            iNode->setAtMaxDelay();
        }
    } else {
        Operation *Op = LEGUP_CONFIG->getOperationRef(opName);
        assert(Op);
        float critDelay = Op->getCritDelay();

        if (critDelay > InstructionNode::getMaxDelay()) {
            // errs() << "Warning delay " << critDelay <<
            //    "ns is greater than clock constraint of " <<
            //    InstructionNode::getMaxDelay() << "ns for instruction: " <<
            //    *instr << "\n";
            iNode->setAtMaxDelay();
        } else if (isMul(instr) &&
                   LEGUP_CONFIG->getParameterInt("MULTIPLIER_NO_CHAIN")) {
            iNode->setAtMaxDelay();
        } else {
            iNode->setDelay(critDelay);
        }
    }
}

void SchedulerDAG::generateDependencies(Instruction *instr) {
    InstructionNode *iNode = getInstructionNode(instr);

    // generate dependencies
    regDataDeps(iNode);

    if (isa<LoadInst>(*instr) || isa<StoreInst>(*instr)) {
        if (LEGUP_CONFIG->getParameterInt("ALIAS_ANALYSIS")) {
            memDataDeps(
                iNode); // create dependencies based on LLVM alias analysis
        } else {
            callDataDeps(iNode); // create dependences in same order as in IR
                                 // [LegUp 1.0 & 2.0 functionality]
        }
    } else if (isa<CallInst>(instr)) {
        callDataDeps(iNode);
    }
}

bool SchedulerDAG::runOnFunction(Function &F, Allocation *_alloc) {
    assert(_alloc);
    alloc = _alloc;
    AliasAnalysis *AA = alloc->getAA();
    assert(AA);
    AliasA = AA;

    for (Function::iterator b = F.begin(), be = F.end(); b != be; b++) {
        for (BasicBlock::iterator instr = b->begin(), ie = b->end();
             instr != ie; ++instr) {
            updateDAGwithInst(instr);
        }
    }

    for (Function::iterator b = F.begin(), be = F.end(); b != be; b++) {
        for (BasicBlock::iterator instr = b->begin(), ie = b->end();
             instr != ie; ++instr) {
            generateDependencies(instr);
        }
    }

    return false;
}

/// createStates - create new states for an FSM.
void createStates(unsigned begin, unsigned end,
                  std::map<unsigned, State *> &orderStates,
                  FiniteStateMachine *fsm) {
    assert(orderStates.find(begin - 1) != orderStates.end());
    for (; begin <= end; begin++) {
        orderStates[begin] = fsm->newState(orderStates[begin - 1]);
    }
}

/// setStateTransitions - determine the state transitions given a terminating
/// instruction
void setStateTransitions(State *lastState, const TerminatorInst *TI,
                         State *waitState,
                         std::map<BasicBlock *, State *> bbFirstState) {

    lastState->setTerminating(true);

    // unreachable could occur in a basic block like:
    // bb:                                               ; preds = %entry
    //   %2 = tail call i32 (i8*, ...)* @printf(i8* noalias getelementptr
    //    inbounds ([32 x i8]* @.str1, i32 0, i32 0)) nounwind ; <i32> [#uses=0]
    //   tail call void @exit(i32 1) noreturn nounwind
    //   unreachable
    if (isa<UnreachableInst>(TI) || TI->getOpcode() == Instruction::Ret) {
        lastState->setDefaultTransition(waitState);
        return;
    }

    lastState->setTransitionVariable(TI->getOperand(0));

    BasicBlock *Default;
    if (const SwitchInst *SI = dyn_cast<SwitchInst>(TI)) {

        for (unsigned i = 2, e = SI->getNumOperands(); i != e; i += 2) {
            Value *value = SI->getOperand(i);
            assert(value);
            BasicBlock *Succ = dyn_cast<BasicBlock>(SI->getOperand(i + 1));
            State *state = bbFirstState[Succ];

            lastState->addTransition(state, value);
        }

        Default = dyn_cast<BasicBlock>(SI->getDefaultDest());

    } else if (const BranchInst *B = dyn_cast<BranchInst>(TI)) {

        if (B->isConditional()) {
            Default = dyn_cast<BasicBlock>(TI->getSuccessor(1));

            BasicBlock *Succ = dyn_cast<BasicBlock>(B->getSuccessor(0));
            State *state = bbFirstState[Succ];

            lastState->addTransition(state);
        } else {
            Default = dyn_cast<BasicBlock>(TI->getSuccessor(0));
        }

    } else {
        llvm_unreachable(0);
    }

    State *state = bbFirstState[Default];
    lastState->setDefaultTransition(state);
}

/// createFSM - create a Finite State Machine object from the scheduler mapping.
FiniteStateMachine *SchedulerMapping::createFSM(Function *F,
                                                SchedulerDAG *dag) {
    // create FSM
    FiniteStateMachine *fsm = new FiniteStateMachine();

    // very first state
    State *waitState = fsm->newState();
    waitState->setDefaultTransition(waitState);

    // first state in each basic block
    std::map<BasicBlock *, State *> bbFirstState;
    // this map is used for labeling the states
    // each state name will have an index, from this map, as well as being
    // labeled with a function name, and a basic block name.
    std::map<BasicBlock *, unsigned> sCount;

    // create a FSM for the function where each basic block is assigned
    // an empty state as a placeholder
    unsigned bbNum = 0;
    for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b) {
        if (isEmptyFirstBB(b))
            continue;

        State *s = fsm->newState();
        s->setBasicBlock(b);
        bbFirstState[b] = s;
        sCount[b] = 0;

        bbNum++;
    }

    // setup wait state transitions
    // setTransitionSignal(rtl->find("start"))
    // will be done later when we have access
    // to the RTL module
    Function::iterator firstBB = F->begin();
    if (BasicBlock *succ = isEmptyFirstBB(firstBB)) {
        // first BB was empty with just an unconditional branch,
        // so the waitstate should point to the next state, for instance:
        // BB_0:
        //   br label %BB_2
        // also need to set the basic block so phi's are still handled properly
        // for the empty BB
        waitState->setTerminating(true);
        waitState->setBasicBlock(firstBB);
        assert(bbFirstState.find(succ) != bbFirstState.end());
        waitState->addTransition(bbFirstState[succ]);
    } else {
        assert(bbFirstState.find(firstBB) != bbFirstState.end());
        waitState->addTransition(bbFirstState[firstBB]);
    }

    for (Function::iterator B = F->begin(), BE = F->end(); B != BE; ++B) {
        std::map<unsigned, State *> orderStates;
        if (isEmptyFirstBB(B))
            continue;

        orderStates[0] = bbFirstState[B];
        unsigned lastState = getNumStates(B);

        // int pipelined = getMetadataInt(B->getTerminator(),
        // "legup.pipelined");

        // errs() << "BB: " << getLabel(B) << " lastState: " << lastState <<
        // "\n";
        createStates(1, lastState, orderStates, fsm);

        for (BasicBlock::iterator instr = B->begin(), ie = B->end();
             instr != ie; ++instr) {
            Instruction *I = instr;
            unsigned order = getState(dag->getInstructionNode(I));

            orderStates[order]->push_back(I);

            // need to ensure multi-cycle instructions finish in the basic block
            unsigned delayState = Scheduler::getNumInstructionCycles(I);

            // Normally, loads take two cycles and the loaded values are stored
            // in shared memory controller output registers (port A or port B).
            //
            // In some flows however, we want each load to be stored in a
            // separate
            // register (e.g. to enable multi-cycle paths). But storing each
            // load
            // in a unique register and keeping the register on the output of
            // the
            // memory controller would make loads have a latency of 3, which is
            // not needed. Instead, when a separate register is created for each
            // load, make the FSM "think" that loads take 1 cycle (this is done
            // below). Then the second register is placed at the output of each
            // load (this is done in GenerateRTL.cpp, visitLoadInst);
            if (isa<LoadInst>(I) && LEGUP_CONFIG->duplicate_load_reg()) {
                delayState = 1; // Instead of normal 2 for loads
            }

            if (delayState == 0) {
                fsm->setEndState(I, orderStates[order]);
                continue;
            }

            delayState += order;
            if (delayState > lastState) {

                /*
                // assume iterative module scheduler has already handled
                // multi-cycle instructions
                // can't insert a new state - assume its ready in the first
                // state of next basic block
                if (pipelined) {
                    // this doesn't work for the kernel
                    assert(isa<LoadInst>(I));
                    //++B;
                    //assert(B != BE);
                    //fsm->setEndState(I, bbFirstState[B]);
                    //--B;

                    // all loads are assumed to be wires
                    fsm->setEndState(I, orderStates[order]);

                    continue;
                }
                */

                createStates(lastState + 1, delayState, orderStates, fsm);
                lastState = delayState;
            }

            fsm->setEndState(I, orderStates[delayState]);
        }

        setStateTransitions(orderStates[lastState], B->getTerminator(),
                            waitState, bbFirstState);
        orderStates[lastState]->setBasicBlock(B);

        for (unsigned i = 0; i < lastState; i++) {
            assert(orderStates.find(i) != orderStates.end());

            State *s = orderStates[i];
            s->setBasicBlock(B);
            s->setDefaultTransition(orderStates[i + 1]);
        }
    }

    FiniteStateMachine::iterator stateIter = fsm->begin();
    for (; stateIter != fsm->end(); stateIter++) {
        State *state = stateIter;
        if (!state->getBasicBlock()) {
            assert(state == waitState);
            continue;
        }

        sCount[state->getBasicBlock()] += 1;
        std::string newStateName = std::string("LEGUP_F_") +
                                   F->getName().str().data() + "_BB_" +
                                   getLabelStripped(state->getBasicBlock());

        stripInvalidCharacters(newStateName);
        state->setName(newStateName);
    }

    if (fsm->begin() != fsm->end())
        fsm->begin()->setName("LEGUP");

    return fsm;
}

void printNodeLabel(raw_ostream &out, InstructionNode *I) {
    out << *I->getInst();
}

// print a dot graph representing the dependency information (both normal and
// memory) for a basic block
void SchedulerDAG::printDFGDot(formatted_raw_ostream &out, BasicBlock *BB) {

    dotGraph<InstructionNode> graph(out, printNodeLabel);
    graph.setLabelLimit(40);

    bool ignoreDummyCalls =
        !LEGUP_CONFIG->getParameterInt("DFG_SHOW_DUMMY_CALLS");
    for (BasicBlock::iterator I = BB->begin(), ie = BB->end(); I != ie; ++I) {
        InstructionNode *op = getInstructionNode(I);
        if (ignoreDummyCalls && isaDummyCall(I))
            continue;

        std::string label = "label=\"D:" + ftostr(op->getDelay()) + "ns L:" +
                            utostr(Scheduler::getNumInstructionCycles(I)) +
                            "\",";
        for (Value::use_iterator use = I->use_begin(), e = I->use_end();
             use != e; ++use) {
            if (Instruction *child = dyn_cast<Instruction>(*use)) {
                if (ignoreDummyCalls && isaDummyCall(child))
                    continue;
                graph.connectDot(out, op, getInstructionNode(child),
                                 label + "color=blue");
            }
        }

        for (InstructionNode::iterator use = op->mem_use_begin(),
                                       e = op->mem_use_end();
             use != e; ++use) {
            if (ignoreDummyCalls && isaDummyCall((*use)->getInst()))
                continue;
            graph.connectDot(out, op, *use, label + "color=red");
        }
    }
}

} // End legup namespace
