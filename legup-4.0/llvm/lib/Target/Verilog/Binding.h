//===-- Binding.h -----------------------------------------------*- C++ -*-===//
//
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

#ifndef LEGUP_BINDING_H
#define LEGUP_BINDING_H

#include "llvm/IR/Instructions.h"
#include <map>
#include <set>

using namespace llvm;

namespace legup {

class State;
class LiveVariableAnalysis;
class FiniteStateMachine;
class MinimizeBitwidth;

class LegupPass;
class Allocation;
class RAM;

/// Binding - For a given LLVM instruction returns the name of the shared
/// functional unit
/// @brief Legup Binding Class
class Binding {
public:
    Binding(Allocation *alloc, FiniteStateMachine *fsm,
            Function *Fp, MinimizeBitwidth *_MBW);

    typedef std::map<Instruction*, std::string> BindingMapType;

    //===------------------------------------------------------------------===//
    /// Binding iterator methods
    ///
    typedef BindingMapType::iterator       iterator;
    typedef BindingMapType::const_iterator const_iterator;

    inline iterator       begin()       { return BindingInstrFU.begin(); }
    inline const_iterator begin() const { return BindingInstrFU.begin(); }
    inline iterator       end  ()       { return BindingInstrFU.end();   }
    inline const_iterator end  () const { return BindingInstrFU.end();   }


    /// Assign operators to functional units
    virtual void operatorAssignment() = 0;
    virtual ~Binding () {};

    /// Is there a shared functional unit for this instruction?
    bool existsBindingInstrFU(Instruction* I) {
        return BindingInstrFU.find(I) != BindingInstrFU.end();
    }

    std::vector<Instruction*> &getInstructionsAssignedToFU(std::string FU) {
        assert(BindingFUInstrs.find(FU) != BindingFUInstrs.end());
        return BindingFUInstrs[FU];
    }


    /// What is the name of the shared functional unit for this instruction
    /// ie. "main_signed_multiply_32_7"
    std::string getBindingInstrFU(Instruction* I) {
        return BindingInstrFU[I];
    }

    void setBindingInstrFU(Instruction* I, std::string FU) {
        BindingInstrFU[I] = FU;
        BindingFUInstrs[FU].push_back(I);
    }

    // This needs to be public because it's called from GenerateRTL
    static void FindIndependentInstructions
    (
        std::set<Instruction*> &Instructions,
        std::map<Instruction*, std::set<Instruction*> > 
            &IndependentInstructions, 
        LiveVariableAnalysis *LVA, 
        FiniteStateMachine *fsm
    );

    static void FindIndependentInstructionsUsingLiveBlocks
    (
        std::map<Instruction*, std::set<BasicBlock*> > &LiveBlocks,
        std::map<Instruction*, std::set<Instruction*> >
            &IndependentInstructions, 
        LiveVariableAnalysis* LVA, 
        FiniteStateMachine *fsm
    );
    
    static void FindLiveBlocks 
    (
        std::set<Instruction*> &Instructions, 
        std::map<Instruction*, std::set<BasicBlock*> > & LiveBlocks, 
        LiveVariableAnalysis *LVA
    );

    static bool FromIndependentStates
    (
        Instruction *i1,
        Instruction *i2,
        std::set<BasicBlock*> &Intersection,
        LiveVariableAnalysis* LVA,
        FiniteStateMachine *fsm
    );
    
    static void getBBStateEncodings
    (
        Function *Fp,
        FiniteStateMachine *fsm,
        std::map<State*, int> &StateEncoding,
        std::map<BasicBlock*, int> &lastState
    );

protected:
    BindingMapType BindingInstrFU;
    Allocation *alloc;
    FiniteStateMachine *fsm;
    Function *Fp;
    LiveVariableAnalysis *LVA;
    MinimizeBitwidth *MBW;
    std::map<std::string, std::vector<Instruction*> > BindingFUInstrs;

};


} // End legup namespace

#endif
