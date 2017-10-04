//===-- State.h - Scheduling Pass -------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the State class.
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_STATE_H
#define LEGUP_STATE_H

#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/IR/Instructions.h"
#include "utils.h"
#include <map>
#include <list>
#include <vector>

using namespace llvm;

namespace legup {

class FiniteStateMachine;
class RTLSignal;

/// State - Represents one state of a finite state machine. Composed of a
/// sequential list of instructions.
/// Transitions are represented as follows:
/// Every state should have a default next state transition. The rest of the
/// transitions depend on a single LLVM variable. This variable is checked
/// against the LLVM value associated with each transition to determine the
/// next state
/// @brief Legup State Representation
class State : public ilist_node<State> {
public:
    struct Transition {
        Value* variable;
        RTLSignal *signal;
        std::vector<Value *> values;
        std::vector<State *> states;
        std::set<State *> predecessors;
        State *transitionDefault;
        Transition() : variable(0), signal(0), transitionDefault(0){};
    };

    // future: store instructions as a doubly-linked list
    //typedef llvm::iplist<Instruction> InstListType;
    typedef std::list<Instruction*> InstListType;
    typedef InstListType::iterator iterator;
    typedef InstListType::const_iterator const_iterator;

    State() : name(), BB(0), terminating(0), waitingForPipeline(false) {}

    /// setFSM - connect state to its FSM
    void setFSM(FiniteStateMachine *_fsm) { parent = _fsm; }

    /// push_back - add an instruction to the end of the state.
    void push_back(Instruction* I);

    /// remove - remove an instruction from the state.
    iterator remove(Instruction *I);

    /// getName - return the state's name
    std::string getName() { return name; }
    void setName(std::string n) { name = n; }

    /// getBasicBlock - get the basic block this state was derived from.
    /// Used for phi instructions. See printPHICopiesForSuccessor()
    const BasicBlock* getBasicBlock() const  { return BB; }
          BasicBlock* getBasicBlock()        { return BB; }
    void setBasicBlock(BasicBlock *b) { BB = b; }

    /// getTransitionVariable - The variable determining the state's transition
    Value* getTransitionVariable()           { return transition.variable; }
    void   setTransitionVariable(Value *v)   { transition.variable = v; }
    RTLSignal* getTransitionSignal()           { return transition.signal; }
    void   setTransitionSignal(RTLSignal *v)   { transition.signal = v; }

    /// addTransition - Add a transition to another state when
    /// getTransitionVariable() is equal to value.
    void addTransition (State *state, Value *value = 0) {
        transition.states.push_back(state);
        transition.values.push_back(value);
    }

    void addPredecessor(State *state) { transition.predecessors.insert(state); }

    /// getTransitionValue - get the value of a specific transition
    Value *getTransitionValue(unsigned i) { return transition.values[i]; }

    /// getTransitionState - get the state of a specific transition
    State*    getTransitionState(unsigned i) { return transition.states[i]; }

    /// getDefaultTransition - get the state of the default transition
    void    setDefaultTransition(State *s) { transition.transitionDefault = s; }
    State*  getDefaultTransition()         { return transition.transitionDefault; }

    /// getNumTransitions - the number of possible transition from this state.
    /// Includes the default transition
    unsigned getNumTransitions() { 
        assert(getDefaultTransition());
        // +1 is for the default transition
        return transition.states.size() + 1;
    }

    std::set<State *> *getPredecessors() { return &transition.predecessors; }

    /// getTransition - get the entire state transition
    void setTransition(Transition &t) { transition = t; }
    Transition &getTransition() { return transition; }

    /// getNumInstructions - the number of instructions executed in this state
    unsigned getNumInstructions() { 
        // -1 accounts for sentinel
        return InstList.size()-1; 
    }

    /// setTerminating - mark state as a basic block terminating state
    void setTerminating(bool t) { terminating = t; }

    /// isTerminating - return whether this state terminates a basic block
    bool isTerminating() { return terminating; }

    /// isPipelined - is this state simply waiting for a loop pipeline to finish?
    bool isWaitingForPipeline() {
        return waitingForPipeline;
    }

    void setWaitingForPipeline(bool p) {
        waitingForPipeline = p;
    }

    //===------------------------------------------------------------------===//
    /// State iterator methods
    ///
    inline iterator       begin()       { return InstList.begin(); }
    inline const_iterator begin() const { return InstList.begin(); }
    inline iterator       end  ()       { return InstList.end();   }
    inline const_iterator end  () const { return InstList.end();   }
    inline bool           empty() const { return InstList.empty(); }
    inline unsigned       size() const  { return InstList.size(); }

    iterator erase(iterator i) { return InstList.erase(i); }

    // print a textual version of this state's transition
    void printTransition(formatted_raw_ostream &out);

    /// ------------------------------------------------------------------------
    // Debugger
    /// ------------------------------------------------------------------------
    //    void setFunctionCall(bool val) { functionCall = val; }
    //    bool isFunctionCall() { return functionCall; }
    Function *getCalledFunction();

  private:
    InstListType InstList;

    std::string name;
    BasicBlock *BB;
    Transition transition;
    bool terminating;
    FiniteStateMachine *parent;
    bool waitingForPipeline;
    //    bool functionCall;
};

} // End legup namespace

#endif
