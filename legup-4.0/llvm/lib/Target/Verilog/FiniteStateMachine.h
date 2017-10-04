//===-- FiniteStateMachine.cpp - Store finite state machine -----*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the FiniteStateMachine class.
// The FSM is stored as a doubly-linked list of States.
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_FINITESTATEMACHINE_H
#define LEGUP_FINITESTATEMACHINE_H

#include "State.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"

using namespace llvm;

namespace legup {

/// FiniteStateMachine - Represents a finite state machine. Composed of a
/// list of State objects
/// @brief Legup Finite State Machine Representation
class FiniteStateMachine {
public:
    /// newState() - Create a new State within the state machine.
    State* newState(State *InsertAfter = 0, std::string Name="LEGUP");

    /// StateListType - Doubly-linked list of State objects
    typedef llvm::iplist<State> StateListType;

    /// State iterators
    typedef StateListType::iterator                              iterator;
    typedef StateListType::const_iterator                  const_iterator;

    //===------------------------------------------------------------------===//
    /// State iterator methods
    ///
    inline iterator                begin()       { return StateList.begin(); }
    inline const_iterator          begin() const { return StateList.begin(); }
    inline iterator                end  ()       { return StateList.end();   }
    inline const_iterator          end  () const { return StateList.end();   }

    inline size_t                   size() const { return StateList.size();  }
    inline bool                    empty() const { return StateList.empty(); }
    inline const State            &front() const { return StateList.front(); }
    inline       State            &front()       { return StateList.front(); }
    inline const State             &back() const { return StateList.back();  }
    inline       State             &back()       { return StateList.back();  }

    iterator erase(iterator i) { return StateList.erase(i); }

    /// getStateList() - Return the underlying state list container.  You
    /// need to access it directly if you want to modify it currently.
    ///
    const StateListType &getStateList() const { return StateList; }
          StateListType &getStateList()       { return StateList; }

    /// getNumStates() - Return the number of states
    unsigned getNumStates() { return StateList.size(); }

    void setStartState(const Instruction *i, State *s) { map[i] = s; }
    void setEndState(const Instruction *i, State *s) { endState[i] = s; }

    State *getStartState(const Instruction *i) { return map[i]; }
    State *getEndState(const Instruction *i) { return endState[i]; }
    int getStateNum(const State *state);

    void pushInstructionToNextState(Instruction *);
    void pushExtraStates(Instruction *, unsigned);

    // print a dot graph representing the dependency information (both normal
    // and memory) for this FSM
    void printDot(formatted_raw_ostream &out);

    // clone the FSM into newFSM. Performs a
    // deep copy and creates new State objects
    // for each state
    void cloneFSM(FiniteStateMachine *newFSM);

    // NC changes...
    bool EndStateExists(const Instruction *i) {
        return (endState.find(i) != endState.end());
    }

    void buildStatePredecessors();

  private:
    StateListType StateList;
    DenseMap<const Instruction *, State *> map, endState;
};

} // End legup namespace

#endif
