//===-- BipartiteWeightedMatchingBinding.h ----------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// See paper: Data Path Allocation Based on Bipartite Weighted Matching, DAC'90
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_BIPARTITE_WEIGHTED_MATCHING_BINDING_H
#define LEGUP_BIPARTITE_WEIGHTED_MATCHING_BINDING_H

#include "Binding.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"


#include <map>
#include <set>

using namespace llvm;

namespace legup {

/// BipartiteWeightedMatchingBinding - A binding implementation
/// using bipartite weighted matching
class BipartiteWeightedMatchingBinding : public Binding {
public:
    BipartiteWeightedMatchingBinding (Allocation *alloc, FiniteStateMachine
            *fsm, Function *Fp, MinimizeBitwidth *_MBW) :
        Binding(alloc, fsm, Fp, _MBW) {}
    void operatorAssignment();
    ~BipartiteWeightedMatchingBinding() {}

    // is the instruction a candidate to be shared?
    static bool isInstructionSharable(Instruction *I, Allocation *alloc);

private:
    struct AssignmentInfo {
        std::map<std::string, int> muxInputs;
        std::map<Instruction*, std::set<Instruction*> > IndependentInstructions;
        std::map<std::string, std::set<Instruction*> > existingOperands;
        std::map<std::string, std::set<Instruction*> > existingInstructions;
    };
    typedef std::vector< std::vector<int> > Table;
    int** vector_to_matrix(Table &v, int rows, int cols);
    //void bindOperationsToFunctionalUnits (std::map <std::string, int> &minFUs);
    void constructWeights(raw_ostream &out, Instruction *I, int operationIdx,
            std::string funcUnitType, int numFuncUnitsAvail, AssignmentInfo
            &assigned, Table &weights);

    std::map<int, Instruction*> opInstr;

    void UpdateAssignments(raw_ostream &out, int numOperationsToShare,
            std::string funcUnitType, int numFuncUnitsAvail, AssignmentInfo
            &assigned, Table &assignments);
    void CheckAllWereAssigned(int numOperationsToShare, int numFuncUnitsAvail,
            Table &assignments);
    void bindFunctUnitInState(raw_ostream &out, State* state, std::string funcUnitType,
            int numFuncUnitsAvail, AssignmentInfo &assigned);
    bool shareInstructionWithFU(Instruction *I, std::string funcUnitType);
    void solveBipartiteWeightedMatching(Table &weights, Table &assignments);

    void printTable(raw_ostream &out, std::string funcUnitType, int
            numOperationsToShare, int numFuncUnitsAvail, Table &weights);
    
    // All pairs of instructions which are shared
    std::map<Value*, Value*> AllBindingPairs;
    

};

} // End legup namespace

#endif
