//===-- Binding.cpp - Binding -----------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// The Binding class provides a mapping between LLVM instructions and
// shared functional units, and also finds patterns of LLVM instructions
// and pairs them together to be shared (in GenerateRTL.cpp)
//
//===----------------------------------------------------------------------===//

#include "Binding.h"
#include "Allocation.h"
#include "LegupPass.h"
#include "GlobalNames.h"
#include "VerilogWriter.h"
#include "LegupConfig.h"
#include "MinimizeBitwidth.h"
#include "LVA.h"
#include "RTL.h"
#include "FiniteStateMachine.h"
#include "utils.h"

using namespace llvm;
using namespace legup;

namespace legup {

Binding::Binding(Allocation *alloc, FiniteStateMachine *fsm,
            Function *Fp, MinimizeBitwidth *_MBW) : alloc(alloc), fsm(fsm),
    Fp(Fp) {
    LVA = alloc->getLVA(Fp);
    assert(LVA);
    MBW = _MBW;
    assert(fsm);
}

bool is_register_optimized_away(Instruction *I, State *state, 
    FiniteStateMachine *fsm);
bool check_for_successors_in_other_states(Instruction *I, State *state, 
    FiniteStateMachine *fsm, std::set<Instruction*> &PHIs);
void visit_all_transition_states(State *state, std::set<Instruction*> &PHIs);
void add_instruction_def_states(BasicBlock *BB, Instruction *i1, 
    vector<int> &i1_states, Instruction *i2, vector<int> &i2_states,
    FiniteStateMachine *fsm, map<State*, int> &StateEncoding);
void add_instruction_use_states(BasicBlock *BB, Instruction *i1, 
    vector<int> &i1_states, Instruction *i2, vector<int> &i2_states,
    FiniteStateMachine *fsm, map<State*, int> &StateEncoding,
    const unsigned LAST_STATE);
void add_instruction_live_in_live_out_states(LiveVariableAnalysis* LVA,
    BasicBlock *BB, Instruction *i1, vector<int> &i1_states, Instruction *i2,
    vector<int> &i2_states, const unsigned FIRST_STATE, 
    const unsigned LAST_STATE);

// Given a set of instructions (Instructions), 
// - calculate liveness information for each instruction
// - fill a map (IndependentInstructions) which maps every instruction 
//   to a set of other instruction with which its lifetime is independent
void Binding::FindIndependentInstructions
(
    std::set<Instruction*> &Instructions,
    std::map<Instruction*, std::set<Instruction*> > 
        &IndependentInstructions, 
    LiveVariableAnalysis *LVA, 
    FiniteStateMachine *fsm
) 
{
    // 1. First calculate liveness information for the instructions,
    // which later will be used to pair graphs

    // For each instruction, make a set of every BB where it is live
    std::map<Instruction*, std::set<BasicBlock*> > LiveBlocks;
    FindLiveBlocks(Instructions, LiveBlocks, LVA);

    for (std::map<Instruction*, std::set<BasicBlock*> >::iterator i =
            LiveBlocks.begin(), end = LiveBlocks.end(); i != end; ++i) {
        IndependentInstructions.insert(std::make_pair(i->first,
                    std::set<Instruction*>()));
    }

    // 2. Fill the Independent instructions map
    FindIndependentInstructionsUsingLiveBlocks(LiveBlocks,
            IndependentInstructions, LVA, fsm);

    for (std::map<Instruction*, std::set<Instruction*> >::iterator i =
            IndependentInstructions.begin(), end =
            IndependentInstructions.end(); i != end; ++i) {
        IndependentInstructions[i->first].erase(i->first); // erase self
    }
}
        

// Given a LiveBlocks map which contains a set of live blocks for each
// instruction, convert this to a map which maps every instruction to a set of
// other instructions with which it is independent Note, in addition to
// considering only live blocks, states in the FSM are also considered when
// determining independence
//
// @brief: this function fills the IndependentInstructions map
void Binding::FindIndependentInstructionsUsingLiveBlocks
(
    std::map<Instruction*, std::set<BasicBlock*> > &LiveBlocks,
    std::map<Instruction*, std::set<Instruction*> > &IndependentInstructions, 
    LiveVariableAnalysis* LVA, 
    FiniteStateMachine *fsm
) 
{
    // We need to consider every instruction with every other instruction, i.e.
    // every pair of instructions. So, iterate over all instructions, and in
    // each iteration iterate over all instructions again, but starting from
    // the next iteration to avoid repetition

    if (LEGUP_CONFIG->getParameterInt("DISABLE_REG_SHARING")) {
        return;
    }

    for (std::map<Instruction*, std::set<BasicBlock*> >::iterator i =
            LiveBlocks.begin(), end = LiveBlocks.end(); i != end; ++i) {

        for (std::map<Instruction*, std::set<BasicBlock*> >::iterator j = i; j
                != end; ++j) {

            Instruction *i1 = i->first, *i2 = j->first;

            if (IndependentInstructions[i1].find(i2) !=
                    IndependentInstructions[i1].end() ) {
                continue;
            }

            // only pair if neither or both will be optimized away
            if ( is_register_optimized_away(i1, fsm->getEndState(i1), fsm) !=
                 is_register_optimized_away(i2, fsm->getEndState(i2), fsm) ) {
                continue;
            }

            std::set<BasicBlock*> Intersection;

            std::set_intersection(i->second.begin(), i->second.end(),
                    j->second.begin(), j->second.end(),
                    std::inserter(Intersection, Intersection.begin()) );

            if (Intersection.empty()) { // two instructions are independent
                IndependentInstructions[i1].insert(i2);
                IndependentInstructions[i2].insert(i1);
                continue;
            }

            if (FromIndependentStates(i1, i2, Intersection, LVA, fsm)) {
                // two instructions are independent
                IndependentInstructions[i1].insert(i2);
                IndependentInstructions[i2].insert(i1);
            }
        }
    }
}

        
// The Live Variable Analysis pass (LVA is a reference to this pass) calculated
// live blocks for every instruction. This function uses this analysis,
// taking a set of Instructions (Instructions) as input and returning a map
// from every instruction to a set of basic blocks it is live in (LiveBlocks)
// @brief: this function fills the LiveBlocks map
void Binding::FindLiveBlocks 
(
    std::set<Instruction*> &Instructions, 
    std::map<Instruction*, std::set<BasicBlock*> > & LiveBlocks, 
    LiveVariableAnalysis *LVA
)
{
    for (std::set<Instruction*>::iterator i = Instructions.begin(), ie =
            Instructions.end(); i != ie; ++i) {
        LiveBlocks.insert( std::make_pair(*i, std::set<BasicBlock*>()) );

        int index = LVA->instToLatticeBit[*i];

        for (LiveVariableAnalysis::BlockInfoMapping::iterator it =
                LVA->blockToInfo.begin(); it != LVA->blockToInfo.end(); ++it) {
            BasicBlock *bb = it->first;
            LiveVariableAnalysis::BasicBlockLivenessInfo *info = it->second;
            // instruction is live if it is a use, def, in or our of the block
            if( (info->use->test(index)) 
                    || (info->def->test(index)) 
                    || (info->in->test(index)) 
                    || (info->out->test(index)) )
                LiveBlocks[*i].insert( bb );
        }
    }
}

// Determines if the register assigned to an instruction will be optimized away.
// This happens if the instruction is not used across states. However, it is
// also possible that an instruction is used across states but its register is
// still optimized away if the use is a phi into another block and the phi is 
// in a transition state of the instruction (see below)
bool is_register_optimized_away
(
    Instruction *I, 
    State *state, 
    FiniteStateMachine *fsm
)
{
    std::set<Instruction*> PHIs; // all Phi nodes using the instruction I
    if (check_for_successors_in_other_states(I, state, fsm, PHIs)) {
        return false; // can't optimize register away
    }

    // Now, all successors of I are either PHIs or in the same state
    if (PHIs.empty()) return true;

    // All successors are PHIs. However, if the PHIs are in a transition state
    // of I, then the register will not be used
    // Visit all transition states and remove all from the PHIs set.
    visit_all_transition_states (state, PHIs);

    // Finally, if PHIs is empty now, then all the uses of I use the wire, not
    // the reg. But if PHIs is not empty, at least one of its PHI successors
    // is in a non-transition state
    if (PHIs.empty()) return true;
    return false;
}

// Checks for successors in other states and also filles the PHIs
// map with all PHI successors found
bool check_for_successors_in_other_states 
(
    Instruction *I, 
    State *state, 
    FiniteStateMachine *fsm,
    std::set<Instruction*> &PHIs
)
{
    // First, iterate over all successors. If they are all either PHI's or
    // in the same state, keep iterating, else return true
    for (Value::use_iterator i = I->use_begin(), e = I->use_end(); i!=e; ++i) {
        Instruction *successor = dyn_cast<Instruction>(*i);
        if (!successor) continue;
        if (isa<PHINode>(*successor)) {
            PHIs.insert(successor);
            continue;
        }
        // Not a PHI, check if it's in the same state
        if (fsm->getEndState(successor) != state) return true;
    }
    return false;
}

// Remove all PHIs from transition states from the PHIs map
void visit_all_transition_states
(
    State *state, 
    std::set<Instruction*> &PHIs
)
{ 
    // visit default transition first
    State *ts = state->getDefaultTransition();   
    for (State::iterator i = ts->begin(), e = ts->end(); i != e; ++i) {
        if (!isa<PHINode>(*(*i))) break;
        PHIs.erase(*i);
    }

    if (state->getNumTransitions() > 1) {
        for (unsigned t = 0, te = state->getNumTransitions()-1; t != te; ++t) {
            State *ts = state->getTransitionState(t);
            for (State::iterator i = ts->begin(), e = ts->end(); i != e; ++i) {
                if (!isa<PHINode>(*(*i))) break;
                PHIs.erase(*i);
            }
        }
    }
}

// These two instructions are live in at least one of the same BB.
// However, the pair may may still be independent at the state level
// Therefore, iterate through all overlapping BB and check each
// If they are independent in that BB, remove it from Intersection
// If intersection is empty at the end, then share the pair
bool Binding::FromIndependentStates
(
    Instruction *i1,
    Instruction *i2,
    std::set<BasicBlock*> &Intersection,
    LiveVariableAnalysis* LVA,
    FiniteStateMachine *fsm
) 
{
    bool IndependentStates = true;

    // fix: these shouldn't be static variables
    static FiniteStateMachine *prevFSM = NULL;
    static std::map<State*, int> StateEncoding;
    static std::map<BasicBlock*, int> lastState;
    if (prevFSM != fsm) {
        BasicBlock *BB = *Intersection.begin();
        Function *Fp = BB->getParent();
        StateEncoding.clear();
        lastState.clear();
        getBBStateEncodings(Fp, fsm, StateEncoding, lastState);
        prevFSM = fsm;
    }

    if (!Intersection.empty()) {
    }

    // Enumerate all states in Basic Block
    for (std::set<BasicBlock*>::iterator bb = Intersection.begin(),
            BBend = Intersection.end(); bb != BBend; ++bb) {
        BasicBlock *BB = *bb;

        assert(lastState.find(BB) != lastState.end());

        const unsigned FIRST_STATE = 0;
        const unsigned LAST_STATE = lastState[BB];

        // Keep track of all states where instruction is used
        std::vector<int> i1_states, i2_states;

        // First, add Defs to the 2 vectors above
        // But only add if def is in this BB (can also use getParent())
        add_instruction_def_states(
            BB, 
            i1, 
            i1_states,
            i2, 
            i2_states,
            fsm,
            StateEncoding
        );

        // Second, add all uses in this BB
        add_instruction_use_states(
            BB, 
            i1, 
            i1_states,
            i2, 
            i2_states,
            fsm,
            StateEncoding,
            LAST_STATE
        );

        // Finally, if an instruction is live in/out of this basic block, add the 
        // first/terminating instruction of this basic block
        add_instruction_live_in_live_out_states(
            LVA,
            BB,
            i1,
            i1_states,
            i2,
            i2_states,
            FIRST_STATE,
            LAST_STATE
        );
                
        /*
           errs() << "First instruction: " << *i1 << "\n";
           for (std::vector<int>::iterator k = i1_states.begin(), ke =
           i1_states.end(); k != ke; ++k) {
           errs() << "state: " << *k << "\n";
           }
           errs() << "Second instruction: " << *i2 << "\n";
           for (std::vector<int>::iterator k = i2_states.begin(), ke =
           i2_states.end(); k != ke; ++k) {
           errs() << "state: " << *k << "\n";
           }
           */

        // Given these states for i1 and i2 now, check if the intervals
        // overlap. If they don't, then they are independent
        assert(!i1_states.empty());
        assert(!i2_states.empty());
        if (
            ( *(std::min_element(i1_states.begin(), i1_states.end())) >
              *(std::max_element(i2_states.begin(), i2_states.end()))   ) || 
            ( *(std::max_element(i1_states.begin(), i1_states.end())) <
              *(std::min_element(i2_states.begin(), i2_states.end()))   )
            ) 
        {
            continue;
        } else  {
            IndependentStates = false; 
            break;
        }
    }
    return IndependentStates;
}

// Calculate the order of a state within a basic block:
//      State: LEGUP_function_call_58 order: 0
//      State: LEGUP_function_call_59 order: 1
//      State: LEGUP_F_main_BB_49_60 order: 2
//      State: LEGUP_F_main_BB_49_61 order: 3
//      State: LEGUP_F_main_BB_49_62 order: 4
//      State: LEGUP_F_main_BB_49_63 order: 5
//      State: LEGUP_F_main_BB_49_64 order: 6
//      State: LEGUP_F_main_BB_49_65 order: 7
//      State: LEGUP_function_call_66 order: 8
//      State: LEGUP_function_call_67 order: 9
//      State: LEGUP_F_main_BB_49_68 order: 10
// last state for BB %49 would be set to 10
void Binding::getBBStateEncodings(Function *Fp,
        FiniteStateMachine *fsm,
        std::map<State*, int> &StateEncoding,
        std::map<BasicBlock*, int> &lastState) {

    for (Function::iterator BB = Fp->begin(), be = Fp->end(); BB != be; ++BB) {
        unsigned order=0;

        State *state = NULL;
        for (FiniteStateMachine::iterator stateIter = fsm->begin(); stateIter
                != fsm->end(); stateIter++) {
            if (stateIter->getBasicBlock() == BB) {
                // will handle cases where the basic block is empty
                state = stateIter;
                break;
            }
            for (State::iterator instr = stateIter->begin(), ie =
                    stateIter->end(); instr != ie; ++instr) {
                Instruction *I = *instr;
                if (I->getParent() == BB) {
                    state = stateIter;
                    break;
                }
            }
            if (state) break;
        }
        assert(state);

        while (state) {
            if ( StateEncoding.find(state) == StateEncoding.end() ) {
                StateEncoding[state] = order;
                order++;
            }
            //errs() << "State: " << state->getName() << " order: " <<
            //    StateEncoding[state] << "\n";
            if (state->isTerminating()) break;

            if (state == state->getDefaultTransition()) {
                // special case for function calls
                assert(state->getNumTransitions() == 2);
                state = state->getTransitionState(0);
            } else {
                state = state->getDefaultTransition();
            }
        }
        lastState[BB] = order-1;
    }
}

// Add i1 and i2's def states to the state vectors
void add_instruction_def_states
(
    BasicBlock *BB,
    Instruction *i1,
    vector<int> &i1_states,
    Instruction *i2,
    vector<int> &i2_states,
    FiniteStateMachine *fsm,
    map<State*, int> &StateEncoding
)
{
    if (i1->getParent() == BB) {
        assert (StateEncoding.find(fsm->getEndState(i1)) != StateEncoding.end());
        i1_states.push_back( StateEncoding[fsm->getEndState(i1)] );
    }
    if (i2->getParent() == BB) {
        assert (StateEncoding.find(fsm->getEndState(i2)) != StateEncoding.end());
        i2_states.push_back( StateEncoding[fsm->getEndState(i2)] );
    }
}


// Add i1 and i2's use states to the state vectors
void add_instruction_use_states
(
    BasicBlock *BB,
    Instruction *i1,
    vector<int> &i1_states,
    Instruction *i2,
    vector<int> &i2_states,
    FiniteStateMachine *fsm,
    map<State*, int> &StateEncoding,
    const unsigned LAST_STATE
)
{
    for (Value::user_iterator use = i1->user_begin(),
            use_end = i1->user_end(); use != use_end; ++use) {
        Instruction * successor = dyn_cast<Instruction> ( *use );
        if (!successor) continue; // successor is not an instruction

        if (successor == BB->getTerminator() ) { // This is the case for ret and br
            i1_states.push_back( LAST_STATE );
            continue;
        }

        if (StateEncoding.find(fsm->getEndState(successor)) != StateEncoding.end()) {
            i1_states.push_back( StateEncoding[fsm->getEndState(successor)] );
        }
    }

    for (Value::user_iterator use = i2->user_begin(),
            use_end = i2->user_end(); use != use_end; ++use) {
        Instruction * successor = dyn_cast<Instruction> ( *use );
        if (!successor) continue; // successor is not an instruction

        if (successor == BB->getTerminator() ) { // This is the case for ret and br
            i2_states.push_back( LAST_STATE );
            continue;
        }

        if (StateEncoding.find(fsm->getEndState(successor)) != StateEncoding.end()) {
            i2_states.push_back( StateEncoding[fsm->getEndState(successor)] );
        }
    }
}


// If i1 and i2 are live in/out of the basic block, add the first/terminating 
// instruction of this basic block to the state vectors
void add_instruction_live_in_live_out_states
(
    LiveVariableAnalysis* LVA,
    BasicBlock *BB,
    Instruction *i1,
    vector<int> &i1_states,
    Instruction *i2,
    vector<int> &i2_states,
    const unsigned FIRST_STATE,
    const unsigned LAST_STATE
)
{
        if ( LVA->blockToInfo[BB]->out->test(LVA->instToLatticeBit[i1]) ) {
            i1_states.push_back( LAST_STATE );
        }
        if ( LVA->blockToInfo[BB]->in->test(LVA->instToLatticeBit[i1]) ) {
            i1_states.push_back( FIRST_STATE );
        }
        if ( LVA->blockToInfo[BB]->out->test(LVA->instToLatticeBit[i2]) ) {
            i2_states.push_back( LAST_STATE );
        }
        if ( LVA->blockToInfo[BB]->in->test(LVA->instToLatticeBit[i2]) ) {
            i2_states.push_back( FIRST_STATE );
        }
}


} // End legup namespace

