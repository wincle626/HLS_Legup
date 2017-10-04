//===-- State.cpp -----------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Implements State class
//
//===----------------------------------------------------------------------===//
#include "FiniteStateMachine.h"
#include "State.h"

namespace legup {

void State::push_back(Instruction* I) {
    InstList.push_back(I);
    parent->setStartState(I, this);
}

State::iterator State::remove(Instruction *I) {
    State::iterator it;
    bool found = false;
    for (iterator i = begin(), e = end(); i != e; ++i) {
        if (*i == I) {
            assert(!found);
            found = true;
            it = i;
        }
    }
    assert(found);
    return erase(it);
}

// print a textual version of this state's transition
void State::printTransition(formatted_raw_ostream &out) {

    assert(getDefaultTransition());

    out << "Transition: ";
    // uncond
    if (getNumTransitions() == 1) {
        // fall through

        // cond branch
    } else if (getNumTransitions() == 2) {
        // true condition
        out << "if (" << getTransitionOp(this) << ")";
        out << ": " << getTransitionState(0)->getName() << " ";

    } else {
        // switch
        assert(getNumTransitions() > 0);

        out << "switch (" << getTransitionOp(this) + ") ";
        for (unsigned i = 0, e = getNumTransitions()-1; i != e; ++i) {
            out << getLabel(getTransitionValue(i)) + ": ";
            out << getTransitionState(i)->getName() << " ";
        }

    }
    out << "default: " << getDefaultTransition()->getName();
}

// Returns pointer to the Function called in this state
Function *State::getCalledFunction() {
    CallInst *CI = NULL;
    if (getNumInstructions() == 0) {
        CI = dyn_cast<CallInst>(*(InstList.begin()));
        if (CI && !isaDummyCall(CI)) {
            return legup::getCalledFunction(CI);
        }
    }
    return NULL;
}
}
