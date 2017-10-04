//===-- PatternBinding.h - Binding Patterns ---------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Find patterns of instructions and represent them as Pattern Graphs (Graph.h)
// See Paper and slides for more information on the algorithm:
//      S. Hadjis, A. Canis, J.H. Anderson , J. Choi , K. Nam, S. Brown, T.
//      Czajkowski, "Impact of FPGA Architecture on Resource Sharing in
//      High-Level Synthesis," ACM/SIGDA International Symposium on Field
//      Programmable Gate Arrays (FPGA), Monterey, CA, February 2012.
// Steps:
// 1. Fill a PatternMap container (see PatternMap.h) with all graphs in
//    the function,
// 2. Determine which can be paired together and bound to the same
//    functional units,
// 3. Select a matching of Graphs (pair them up)
//
//===------------------------------------------------------------------===//

#ifndef LEGUP_PATTERN_BINDING_H
#define LEGUP_PATTERN_BINDING_H

#include "Binding.h"
#include "llvm/IR/Instructions.h"
#include "PatternMap.h"
#include <map>
#include <set>

using namespace llvm;

namespace legup {

class PatternBinding : public Binding {
public:
    PatternBinding (Allocation *alloc, FiniteStateMachine
            *fsm, Function *Fp, MinimizeBitwidth *_MBW) :
        Binding(alloc, fsm, Fp, _MBW) {}
    void operatorAssignment() {};
    ~PatternBinding() {}

    // Top-level pattern sharing function:
    // Discover all pattern graphs in a function and pair them for sharing
    void PatternAnalysis 
    (
        PatternMap &Patterns,
        std::map<Value*, Value*> &AllBindingPairs,
        std::map<Graph*, Graph*> &GraphPairs,
        std::set<Instruction*> &InstructionsInGraphs
    );

private:


    // Pattern Sharing - Step 1: Discover all graphs in the function
    void DiscoverAllPatternsInFunction 
    (
        PatternMap &Patterns,
        map<Instruction*, set<Graph*> > &InstructionGraphUses
    );

    void DiscoverAllPatternsInBasicBlock 
    (
        Function::iterator BB,
        PatternMap &Patterns,
        map<Instruction*, set<Graph*> > &InstructionGraphUses,
        map<Instruction*, vector<Graph*> > &GraphsFound
    );
    
    void DiscoverAllPatternsOfSizeOneInBasicBlock
    (
        Function::iterator BB,
        PatternMap &Patterns,
        queue <Graph*> &Graphs,
        map<Instruction*, set<Graph*> > &InstructionGraphUses
    );

    // Given a graph of size N, find all graphs of size N+1
    void DiscoverGraphsOfSizeOneGreater 
    (
        Function::iterator BB,
        Graph *g,
        PatternMap &Patterns,
        queue <Graph*> &Graphs,
        map<Instruction*, set<Graph*> > &InstructionGraphUses,
        map<Instruction*, vector<Graph*> > &GraphsFound
    );
    
    void CreateGraphWithNewPredecessor 
    (
        Instruction *pred,
        int predecessor_number, // 0 is left, 1 is right
        Graph *g,
        PatternMap &Patterns,
        queue <Graph*> &Graphs,
        map<Instruction*, set<Graph*> > &InstructionGraphUses,
        map<Instruction*, vector<Graph*> > &GraphsFound
    );
    
    // Pattern Sharing - Steps 2-3: Discover all graphs in the function
    void GraphLVAPairing 
    (
        PatternMap::PatternMap_iterator iter1,
        int current_size,
        std::map<Value*, Value*> &AllBindingPairs,
        std::map<Graph*, Graph*> &GraphPairs,
        std::set<Instruction*> &InstructionsInGraphs,
        map<Instruction*, set<Graph*> > &InstructionGraphUses,
        map<Graph*, bool> &AlreadyUsed, string & foldername,
        formatted_raw_ostream &out
    );

    // Pattern Sharing - Step 2: 
    //         Determine which graphs can be paired together and 
    //         bound to the same functional units.
    void FindIndependentGraphs 
    (
        std::map<Graph*, std::set<Graph*> > & IndependentGraphs, 
        std::map<Instruction*, std::set<Instruction*> > & IndependentInstructions
    );
    
    bool CheckAllOperationsForIndependence
    (
        Graph *g1, 
        Graph *g2,
        std::map<Instruction*, std::set<Instruction*> > &IndependentInstructions    
    );

    bool combinationalLoop(Instruction *i1, Instruction *i2);
    void add_successor_binding_FUs(Instruction *I, std::set<std::string> &successorFUs);
    
    // Pattern Analysis - Step 3: Select a matching of Graphs
    void MakePairs
    (
        std::map<Graph*, std::set<Graph*> > & IndependentGraphs,
        std::map<Value*, Value*> &AllBindingPairs,
        std::map<Graph*, Graph*> &GraphPairs,
        std::set<Instruction*> &InstructionsInGraphs,
        map<Instruction*, std::set<Graph*> > &InstructionGraphUses,
        map<Graph*, bool> &AlreadyUsed
    );

    Graph *FindMinimumCostPartner
    (
        Graph *partner1,
        set<Graph*> &pairing_candidates,
        std::map<unsigned, Node*> &Graph1_NodeLabels,
        std::set<Instruction*> &InstructionsInGraphs,
        map<Instruction*, Graph* > &InstructionAssignedGraph1,
        map<Instruction*, Graph* > &InstructionAssignedGraph2,
        map<Graph*, bool> &AlreadyUsed
    );

    int CalculatePairingCost
    (
        bool &combinationalLoop,
        Graph *partnerCandidate,
        std::map<unsigned, Node*> &Graph1_NodeLabels,
        map<Instruction*, Graph* > &InstructionAssignedGraph1, 
        map<Instruction*, Graph* > &InstructionAssignedGraph2,
        std::set<Instruction*> &InstructionsInGraphs
    );

    // Writing sharing results and graphs to output files
    
    void WriteToOutputFiles
    (
        int current_size,
        int numPairs,
        Graph *representative,
        map<Graph*, bool> &AlreadyUsed, 
        string & foldername,
        PatternMap::PatternList_iterator iter2,
        formatted_raw_ostream &out
    );
    
    void WriteDotAndVerilogFiles
    (
        int current_size,
        int numPairs,
        Graph *representative,
        string & foldername
    );
    
    
};

} // End legup namespace

#endif
