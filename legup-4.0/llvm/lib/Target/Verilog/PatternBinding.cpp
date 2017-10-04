//===-- PatternBinding.cpp - Binding Patterns -------------------*- C++ -*-===//
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
//===----------------------------------------------------------------------===//

#include "PatternBinding.h"
#include "Allocation.h"
#include "LegupPass.h"
#include "Hungarian.h"
#include "GlobalNames.h"
#include "VerilogWriter.h"
#include "LegupConfig.h"
#include "MinimizeBitwidth.h"
#include "LVA.h"
#include "RTL.h"
#include "utils.h"
#include "Graph.h"
#include "PatternMap.h"
#include <fstream>
#include<cstdlib> // for system mkdir
#include<unistd.h> // for chdir
#include<cmath> // for ceil function
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/FileSystem.h"


using namespace llvm;
using namespace legup;
using namespace std;

namespace legup {

// Functions in this file are listed in the order that they're called
// (top-level functions on top and auxiliary functions below)

// Forward declaration of local helper functions. Declaring them here allows
// the functions to come after the class member function which calls them.
// This makes the code more readable from top-to-bottom because code appears
// in the order that it's called, but alternatively these forward declarations 
// can be removed and the local functions can be moved to above their call.

void create_dot_directory_for_function (std::string foldername);
bool check_predecessor_legality(Instruction *pred, Graph *g, 
    Function::iterator BB, MinimizeBitwidth *MBW, FiniteStateMachine *fsm,
    Allocation *alloc); 
bool sharing_is_enabled_for_this_instruction(Instruction *I);
bool check_validity_for_chaining(Instruction * I, std::string opName, 
    MinimizeBitwidth *MBW);
bool is_duplicate(map<Instruction *, PatternMap::GraphList> &GraphsFound, 
    Graph *g); 
bool are_graphs_identical(Graph * g1, Graph * g2); 
bool are_graphs_identical_bfs(Graph * g1, Graph * g2);
bool share_common_instruction (Graph *g1, Graph *g2);
bool within_bitwidth_threshold(Instruction *i1, Instruction *i2,
    MinimizeBitwidth *MBW); 
void map_labels_to_nodes(Graph *g, 
    std::map<unsigned, Instruction*> &Instruction_Labels,
    std::map<unsigned, Node*> &Node_Labels);
void adjust_cost_for_shared_input_variables(int &current_cost, Node *n1, 
    Node *n2);
bool sharing_graphs_creates_combinational_loop(Node *n1, Node *n2,
    map<Instruction*, Graph* > &InstructionAssignedGraph1, 
    map<Instruction*, Graph* > &InstructionAssignedGraph2,
    std::set<Instruction*> &InstructionsInGraphs);
void add_successor_FUs_for_graphs(Instruction *i1, Instruction *i2,
    set<Graph*> &successorFUs,
    map<Instruction*, Graph* > &InstructionAssignedGraph1, 
    map<Instruction*, Graph* > &InstructionAssignedGraph,
    std::set<Instruction*> &InstructionsInGraphs);
void remove_overlapping_graphs_from_data_structures(Graph *partner1,     
    Graph *partner2,
    std::map<unsigned, Instruction*> &Graph1_Labels,
    map<Instruction*, Graph* > &InstructionAssignedGraph1, 
    map<Instruction*, Graph* > &InstructionAssignedGraph2,
    std::map<Graph*, std::set<Graph*> > & IndependentGraphs,
    std::map<Value*, Value*> &AllBindingPairs,
    std::set<Instruction*> &InstructionsInGraphs,
    map<Instruction*, std::set<Graph*> > &InstructionGraphUses,
    map<Graph*, bool> &AlreadyUsed);
void update_graph_data_structures_with_pairing(Graph *partner1,
    Graph *partner2,
    std::map<unsigned, Instruction*> &Graph1_Labels,
    std::map<Graph*, std::set<Graph*> > &IndependentGraphs,
    std::map<Value*, Value*> &AllBindingPairs,
    std::set<Instruction*> &InstructionsInGraphs);
int file_counter();
void write_to_dot(Graph * g, string filename);
void write_to_verilog(Graph * g, string filename, Function *Fp, int MUX_WIDTH);
RTLModule *l_create_graph_rtl(Graph *g, int MUX_WIDTH, const char *module_name,
    int input_width);
RTLSignal *l_add_select_to_rtl(RTLModule *rtl, int MUX_WIDTH);
RTLWidth l_rtl_output_width(Graph *g);
void l_rtl_add_all_predecessors(Graph *g, vector<RTLSignal*> &intermediates,
    map<Node*,int> &OperationIndex, vector<RTLOp*> &op, vector<RTLSignal*> &w);


// ----- Top level function in Pattern Sharing. Called by RTLGenerator -----
//
// PatternAnalysis()
//
// Find patterns of instructions and represent them as Pattern Graphs (Graph.h)
// See Paper and slides for more information on the algorithm. Steps:
// 1. Fill a PatternMap container (see PatternMap.h) with all graphs in
//    the function,
// 2. Determine which can be paired together and bound to the same
//    functional units,
// 3. Select a matching of Graphs (pair them up)

void PatternBinding::PatternAnalysis
(
        PatternMap &Patterns,
        std::map<Value*, Value*> &AllBindingPairs,
        std::map<Graph*, Graph*> &GraphPairs,
        std::set<Instruction*> &InstructionsInGraphs
)
{
    if (LEGUP_CONFIG->getParameterInt("ENABLE_PATTERN_SHARING") == 0) return;
    if (LEGUP_CONFIG->getParameterInt("PS_MAX_SIZE") < 1) return;

    // Output a file describing all patterns discovered
    formatted_raw_ostream out(this->alloc->getPatternFile());
    string function_name = this->Fp->getName().str();
    out << "Function: " << function_name << "\n";
    create_dot_directory_for_function(function_name);

    // Maps every instruction to a set of Graphs which contain it
    map<Instruction*, set<Graph*> > InstructionGraphUses;

    // Discover all patterns and graphs in this function and store
    // it in the PatternMap
    DiscoverAllPatternsInFunction (
        Patterns,
        InstructionGraphUses
    );

    // Pair the graphs together, starting from the largest size found
    // and ending with graphs of size PS_MIN_SIZE (in legup.tcl).

    // Note that many overlapping nodes exist, so every time a pair is
    // formed, every graph which it is part of needs to be 'removed'
    // (or ignored from now on). This is done with the AlreadyUsed map
    std::map<Graph*, bool> AlreadyUsed; // could be a set

    // iterate from largest size to smallest (backwards in the map)
    // and pair together the graphs found above, taking into
    // consideration the liveness information of each graph
    // (from the Live Variable Analysis, or LVA, pass)
    PatternMap::PatternMap_iterator PMi = Patterns.end();
    PatternMap::PatternMap_iterator PMe = Patterns.begin();
    for ( ; PMi != PMe; )
    {
        --PMi;

        // Don't share for pattern sizes < PS_MIN_SIZE
        if (PMi->first < LEGUP_CONFIG->getParameterInt("PS_MIN_SIZE")) {
            continue;
        }

        GraphLVAPairing(
            PMi,
            PMi->first,
            AllBindingPairs,
            GraphPairs,
            InstructionsInGraphs,
            InstructionGraphUses,
            AlreadyUsed,
            function_name,
            out
        );
    }
    out << "\n";
}

// Make a new directory to store DOT files for each function
void create_dot_directory_for_function(string foldername) {
    int prevent_warnings = 0;
    if (LEGUP_CONFIG->getParameterInt("PS_WRITE_TO_DOT") ||
        LEGUP_CONFIG->getParameterInt("PS_WRITE_TO_VERILOG")) {
        string command = "if [ ! -e \"" + foldername + "\" ]; then mkdir " +
            foldername + "; fi";
        prevent_warnings += system(command.c_str());
        prevent_warnings += chdir(foldername.c_str());
        prevent_warnings += system("if [ -e \"*.dot\" ]; then rm *.dot; fi");
        prevent_warnings += system("if [ -e \"*.pdf\" ]; then rm *.pdf; fi");
        prevent_warnings += system("if [ -e \"*.v\" ]; then rm *.v; fi");
        prevent_warnings += chdir("..");
		assert(prevent_warnings == 0);
    }
}

//===----------------------------------------------------------------------===//
// Pattern Analysis - Step 1:
// Fill a PatternMap container  with all graphs in the function
//===----------------------------------------------------------------------===//

// Fill the Patterns PatternMap with all the graphs in the function
void PatternBinding::DiscoverAllPatternsInFunction 
(
    PatternMap &Patterns,
    map<Instruction*, set<Graph*> > &InstructionGraphUses
) 
{
    // organize graphs by root Instruction, and map a root to a vector 
    // of graphs which share that common root (to check for duplicate graphs)
    map<Instruction*, vector<Graph*> > GraphsFound; 

    // For each basic block
    Function::iterator BB  = this->Fp->begin();
    Function::iterator BBe = this->Fp->end();
    for ( ; BB != BBe; ++BB ) {
        DiscoverAllPatternsInBasicBlock(
            BB,
            Patterns, 
            InstructionGraphUses,
            GraphsFound
        );
    }
}

// Fill the Patterns PatternMap with all the graphs in the basic block
// See Resource Sharing Paper for algorithm description (Repeatedly uses BFS)
void PatternBinding::DiscoverAllPatternsInBasicBlock 
(
    Function::iterator BB,
    PatternMap &Patterns,
    map<Instruction*, set<Graph*> > &InstructionGraphUses,
    map<Instruction*, vector<Graph*> > &GraphsFound
) 
{
    queue <Graph*> Graphs;
    
    // Fill queue with all subgraphs of size 1
    DiscoverAllPatternsOfSizeOneInBasicBlock(
        BB, 
        Patterns, 
        Graphs, 
        InstructionGraphUses
    );

    int current_size = 1; // current size of subgraphs we are considering

    // Now that we have all graphs of size 1 in a queue, add every possible 
    // predecessor for every graph of size 1 to make every possible graph of
    // size 2. Place each new graph of size 2 discovered at the end of the
    // Graphs queue, then repeat this for all graphs of size 2 to produce 
    // graphs of size 3, etc.
    while (!Graphs.empty()) {
        if ( Graphs.front()->size() >=
             LEGUP_CONFIG->getParameterInt("PS_MAX_SIZE") )
            break; // stop growing

        // remove next graph from the queue and all possible predecessors
        Graph * g = Graphs.front(); Graphs.pop();

        if (g->size() > current_size) {
            current_size++; // beginning a new set of subgraphs

            // two graphs cannot be duplicates if they are not of the same size, 
            // so clear whenever the size of Graphs in the queue changes
            GraphsFound.clear(); 
        }

        // For this graph g of size N, create graphs of size N+1 by adding
        // one predecessor at a time to g and pushing each new graph 
        // onto to the queue
        DiscoverGraphsOfSizeOneGreater (
            BB,
            g,
            Patterns, 
            Graphs, 
            InstructionGraphUses,
            GraphsFound    
        );
    }
}


void PatternBinding::DiscoverAllPatternsOfSizeOneInBasicBlock
(
    Function::iterator BB,
    PatternMap &Patterns,
    queue <Graph*> &Graphs,
    map<Instruction*, set<Graph*> > &InstructionGraphUses
)
{
    // For each instruction in the basic block
    BasicBlock::iterator I = BB->begin();
    BasicBlock::iterator E = BB->end();
    for ( ; I != E; ++I) {  
        std::string opName = LEGUP_CONFIG->getOpNameFromInst(I, this->alloc);
        if (!check_validity_for_chaining(I, opName, this->MBW)) continue;
/*
        // iterate through successors and ensure all are from other states 
        // (only root graphs at end of states)
        bool EndOfState = true;
        for (Value::use_iterator j = I->use_begin(), end =
                I->use_end(); j != end; ++j) {
            Instruction * successor = dyn_cast<Instruction> ( *j );
            if (!successor) continue; // successor is not an instruction
            if ( fsm->getState(successor) == fsm->getState(I)) {
                EndOfState = false;
                break;
            }
        }
        if (EndOfState == false) continue;
*/
/*
        // Or, ensure every root instruction has a register
        if ( is_register_optimized_away(I, fsm->getState(I), fsm) ) {
            continue;
        }
*/
        Graph * g = new Graph(I);
        Graphs.push(g);
        Patterns.Add(g);

        // Graph g uses instruction I
        if (InstructionGraphUses.find(I) == InstructionGraphUses.end()) {
            InstructionGraphUses.insert( std::make_pair( I, set<Graph*>() ) );
        }
        InstructionGraphUses[I].insert(g);
    }
}


// Given graph g of size N, add all possible predecessors of every node
// to g, one by one, to create graphs of size N+1. Push these graphs
// on to the Graphs queue and add each new graph to the PatternMap

// Algorithm: starting from the root of g, perform a BFS of all
// the nodes in the graph. Every time a node is reached which
// does not have a predecessor in the graph, find what that
// predecessor is from the LLVM DFG and create a copy of g
// with that new predecessor. This copy has size one greater
// than g. Add the copy to Patterns and Graphs.

void PatternBinding::DiscoverGraphsOfSizeOneGreater 
(
    Function::iterator BB,
    Graph *g,
    PatternMap &Patterns,
    queue <Graph*> &Graphs,
    map<Instruction*, set<Graph*> > &InstructionGraphUses,
    map<Instruction*, vector<Graph*> > &GraphsFound
) 
{
    set<Node*> visited; // traversed nodes in the graph
    queue <Node*> Nodes; // queue of nodes to consider (BFS)
    Nodes.push(g->getRoot());                                        

    while (!Nodes.empty()) { // while still nodes to visit
    
        // set "current" to be the next top node in the queue
        g->setCurrent( Nodes.front() ); Nodes.pop();

        // For both predecessors of the current node
        for (int predecessor_number=0; predecessor_number < 2; ++predecessor_number)
        {
            Node *pred_node;
            if (predecessor_number==0) pred_node = g->getCurrent()->p1;
            else pred_node = g->getCurrent()->p2;

            // if predecessor is an operation in the graph g
            if (pred_node->is_op) { 
                // and if we haven't visited it, then add it to our BFS
                // queue to visit later, and move on
                if (visited.find(pred_node) == visited.end()) { 
                    Nodes.push(pred_node); // add to queue
                    visited.insert(pred_node); // mark as visited
                }
            }
            // Otherwise, the predecessor is not in the current graph g.
            // This means we can create a copy of g with this predecessor,
            // and add the copy to our data structures
            else { 
                Instruction * pred = dyn_cast<Instruction> (
                    g->getCurrent()->I->getOperand(predecessor_number) 
                );
                if (!check_predecessor_legality(pred, g, BB, this->MBW,
                            this->fsm, this->alloc)) {
                    continue;
                }
                CreateGraphWithNewPredecessor(
                    pred,
                    predecessor_number, // 0=left, 1=right
                    g,
                    Patterns, 
                    Graphs, 
                    InstructionGraphUses,
                    GraphsFound
                );
            }
        } // for both predecessors
    } // while
}


// Check legality of a predecessor. It must be an instruction, 
// in the same basic block as its successor, and can't have
// fanouts other than its successor in the same state
bool check_predecessor_legality
(
    Instruction *pred,
    Graph *g,
    Function::iterator BB,
    MinimizeBitwidth *MBW,
    FiniteStateMachine *fsm,
    Allocation *alloc
) 
{
    if (!pred) return false; // if it is not an instruction
    if (pred->getParent() != BB) return false; // or not in the same block
    string name = LEGUP_CONFIG->getOpNameFromInst(pred, alloc);
    if (!( check_validity_for_chaining(pred, name, MBW) )) return false; 

    // Final check: we don't want to include this instruction
    // in the graph if it has fanouts which are in the same
    // state and not in the current graph.
    bool IllegalFanouts = false;
    
    Value::use_iterator uit = pred->use_begin();
    Value::use_iterator end = pred->use_end();
    for ( ; uit != end; ++uit) {
        Instruction * successor = dyn_cast<Instruction> ( *uit );
        if (!successor) continue; // successor is not an instruction
        // if the successor is in the same state
        if ( fsm->getEndState(successor) == fsm->getEndState(pred) ) {
            // and if the graph does not contain the successor
            if ( g->GraphNodes.find(successor) == g->GraphNodes.end() ) {
                IllegalFanouts = true;
                break;
            }
        }
    }
    if (IllegalFanouts) return false;
    return true;
}


// Helper function for check_validity_for_chaining
bool sharing_is_enabled_for_this_instruction(Instruction *I) {
    if (isPipelined(I)) return false;

    if (isAdd(I)) {
        return LEGUP_CONFIG->getParameterInt("PATTERN_SHARE_ADD");
    }
    if (isSub(I)) {
        return LEGUP_CONFIG->getParameterInt("PATTERN_SHARE_SUB");
    }
    if (isShift(I)) {
        return LEGUP_CONFIG->getParameterInt("PATTERN_SHARE_SHIFT");
    }
    if (isBitwiseOperation(I)) {
        return LEGUP_CONFIG->getParameterInt("PATTERN_SHARE_BITOPS"); 
    }
    return false;
}

// Helper function to determine if an instruction should be shared
bool check_validity_for_chaining 
(
    Instruction * I, 
    std::string opName,
    MinimizeBitwidth *MBW
) 
{
    if (opName.empty()) return false;
    
    // Note we don't share /, % or *
    if (!sharing_is_enabled_for_this_instruction(I)) return false;

    // Now we know the instruction is a binary operation
    ConstantInt * ci = dyn_cast<ConstantInt>(I->getOperand(0));
    if(ci) return false;
    ci = dyn_cast<ConstantInt>(I->getOperand(1));
    if(ci) return false;

    unsigned minwidth = LEGUP_CONFIG->getParameterInt("PS_MIN_WIDTH");
    if ( MBW->getMinBitwidth(I) < minwidth ) return false;
    if ( MBW->getMinBitwidth(I->getOperand(0)) < minwidth ) return false;
    if ( MBW->getMinBitwidth(I->getOperand(1)) < minwidth ) return false;
/*
    Instruction *pred1 = dyn_cast<Instruction>(I->getOperand(0));
    Instruction *pred2 = dyn_cast<Instruction>(I->getOperand(1));

    // restrictions for successors
    for (Value::use_iterator i = I->use_begin(), end = I->use_end(); 
            i != end; ++i) {
        Instruction * s = dyn_cast<Instruction> ( *i );
        if (!s) continue;

        if (isBitwiseOperation(s) || s->getOpcode() == Instruction::Shl) {
            ci = dyn_cast<ConstantInt>(s->getOperand(0));
            if(ci) return false;
            ci = dyn_cast<ConstantInt>(s->getOperand(1));
            if(ci) return false;
        }
    }

    // restrictions for predecessors
    if (pred1) {
        if ( isBitwiseOperation(pred1) || 
                pred1->getOpcode() == Instruction::Shl) {
            ci = dyn_cast<ConstantInt>(pred1->getOperand(0));
            if(ci) return false;
            ci = dyn_cast<ConstantInt>(pred1->getOperand(1));
            if(ci) return false;
        }
    }

    if (pred2) {
        if ( isBitwiseOperation(pred2) || 
                pred2->getOpcode() == Instruction::Shl) {
            ci = dyn_cast<ConstantInt>(pred2->getOperand(0));
            if(ci) return false;
            ci = dyn_cast<ConstantInt>(pred2->getOperand(1));
            if(ci) return false;
        }
    }
*/
    return true;
}


// Given graph g of size N, create a copy of g with size N+1 and 
// add it to Patterns and Graphs
void PatternBinding::CreateGraphWithNewPredecessor 
(
    Instruction *pred,
    int predecessor_number, // 0 is left, 1 is right
    Graph *g,
    PatternMap &Patterns,
    queue <Graph*> &Graphs,
    map<Instruction*, set<Graph*> > &InstructionGraphUses,
    map<Instruction*, vector<Graph*> > &GraphsFound
) 
{
    Graph * new_subgraph = new Graph();
    (*new_subgraph) = *(g); // overloaded = operator to make a deep copy
    
    if (predecessor_number == 0) {
        pred = new_subgraph->getLeftInstruction();
        if (pred) new_subgraph->GrowLeft(pred);
    }
    else {
        pred = new_subgraph->getRightInstruction();
        if (pred) new_subgraph->GrowRight(pred);
    }

    if (pred && !is_duplicate(GraphsFound, new_subgraph)) {
        GraphsFound[new_subgraph->getRoot()->I].push_back(new_subgraph);
        Graphs.push(new_subgraph);
        Patterns.Add(new_subgraph);

        // Graph new_subgraph uses instruction pred
        if (InstructionGraphUses.find(pred) == InstructionGraphUses.end()) {
            InstructionGraphUses.insert( std::make_pair( pred, set<Graph*>() ) );
        }
        InstructionGraphUses[pred].insert(new_subgraph);
        // Also need to add all other nodes in the graph
        Graph::GraphNodes_iterator GNi = new_subgraph->GraphNodes.begin(); 
        Graph::GraphNodes_iterator end = new_subgraph->GraphNodes.end(); 
        for ( ; GNi != end; ++GNi) {
            InstructionGraphUses[GNi->first].insert(new_subgraph);
        }
    }
    else {
        // we have seen this exact graph before, it is a duplicate 
        delete new_subgraph; 
        new_subgraph = NULL;
    }
}


// check if a current graph has been found before while 
// discovering all graphs (this is possible in the BFS due to
// reconverging paths)
bool is_duplicate 
(
    map<Instruction *, PatternMap::GraphList> &GraphsFound, 
    Graph *g
) 
{
    if ( GraphsFound.empty() ) return false;
    PatternMap::Graph_iterator i   = GraphsFound[g->getRoot()->I].begin();
    PatternMap::Graph_iterator end = GraphsFound[g->getRoot()->I].end();
    for ( ; i != end; ++i ) {
        if (!*i) continue;
        if ( are_graphs_identical(g, *i) ) { // A different, helper function
            return true;
        }
    }
    return false;
}


// Check if two graphs are equal in terms of exact instruction objects
bool are_graphs_identical(Graph * g1, Graph * g2) 
{
    if ( (g1->size()==0) && (g2->size() == 0) ) return true;
    if ( g1->size() != g2->size() ) return false;
    if ( g1->getRoot()->I != g2->getRoot()->I) return false; 
    return are_graphs_identical_bfs(g1, g2);
}


// Check if two graphs are equal in terms of exact instruction objects
// Uses BFS to traverse all corresponding nodes in both graphs
bool are_graphs_identical_bfs(Graph * g1, Graph * g2) {

    // BFS data structures
    set <Node*> visited;
    queue <Node*> g1_nodes;
    queue <Node*> g2_nodes;
    
    // Initialize BFS
    g1_nodes.push(g1->getRoot());
    g2_nodes.push(g2->getRoot());
    visited.insert(g1->getRoot());

    Node *t1; // use these temp pointers as opposed to the graph's "current" pointer in order
    Node *t2; // to not change the value of the current pointer, which may be in use by another function

    // Main BFS Loop
    while (!g1_nodes.empty()) {
        // Pop next nodes from queue
        t1 = g1_nodes.front(); g1_nodes.pop();
        t2 = g2_nodes.front(); g2_nodes.pop();

        // Check equality of both the left and right predecessors
        Node *g1_pred;
        Node *g2_pred;
        for (int predecessor_number = 0; predecessor_number < 2; ++predecessor_number) {
            if (predecessor_number == 0) { // left predecessor
                g1_pred = t1->p1;
                g2_pred = t2->p1;
            }
            else { // right predecessor
                g1_pred = t1->p2;
                g2_pred = t2->p2;
            }
            if (g1_pred->is_op) { // If the predecessor is an operation
                if (!(g2_pred)) return false; // g1->current->p1 exists. but not g2->current->p1
                if ( (g1_pred->I) != (g2_pred->I) ) return false; // both exist, but node doesn't match
                if ( visited.find(g1_pred) == visited.end() ) {
                    g1_nodes.push(g1_pred);
                    g2_nodes.push(g2_pred);
                    visited.insert(g1_pred);
                }
            }
            else if (g2_pred->is_op) return false; // g2->current->p1 exists. but not g1->current->p1
        }
    }
    return true;
}


//===----------------------------------------------------------------------===//
// Pattern Analysis - Steps 2 and 3:
// 2. Determine which graphs can be paired together and bound to the same 
//    functional units.
// 3. Select a matching of Graphs (pair them up)
//===----------------------------------------------------------------------===//

// Given a PatternMap with graphs which are potential candidates for pairing,
// determine which graphs can be paired. To do this, liveness intervals are
// calculated for all instructions in the graphs. Then, select a matching.
void PatternBinding::GraphLVAPairing 
(
    PatternMap::PatternMap_iterator iter1,
    int current_size,
    std::map<Value*, Value*> &AllBindingPairs,
    std::map<Graph*, Graph*> &GraphPairs,
    std::set<Instruction*> &InstructionsInGraphs,
    map<Instruction*, set<Graph*> > &InstructionGraphUses,
    map<Graph*, bool> &AlreadyUsed, string & foldername,
    formatted_raw_ostream &out
) 
{
    // Hierarchy of iteration can be confusing. Recall a PatternMap maps 
    // an int (pattern size) to a list of patterns with that size.
    // But each pattern itself is a list of Graphs (all pattern instances).
    // See PatternMap.h for more information.

    // iter1, passed as a parameter, is an iterator to a PatternList 
    // (i.e. a list containing every pattern of that size)

     // iterate over every pattern with that size (e.g. all patterns of size 2)
    PatternMap::PatternList_iterator iter2;

    // iterate over every graph in that pattern (all graphs of this same pattern)
    PatternMap::Graph_iterator iter3;

    // iterate over every instruction in that graph
    Graph::GraphNodes_iterator iter4;

    // for each GraphList in the PatternList (Pattern list iterator)
    for (iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) { 
    
        if (iter2->size() == 1) continue; // Can't share if frequency 1

        // 0. Create a set of all Graphs in this pattern and all instructions 
        // in this pattern (every instruction in every graph)

        std::set<Graph*> Graphs; // set of all Graphs of this pattern
        std::set<Instruction*> Instructions; // all instructions in all graphs in this pattern

        // for each Graph in this Pattern (Graph iterator)
        for (iter3 = iter2->begin(); iter3 != iter2->end(); ++iter3) { 
            if ( AlreadyUsed[*iter3] ) continue;
            Graphs.insert( *iter3 );

            // for all instructions in this graph
            for (iter4 = (*iter3)->GraphNodes.begin(); 
                    iter4 != (*iter3)->GraphNodes.end(); ++iter4) { 
                Instructions.insert(iter4->first);
                assert(iter4->second->I == iter4->first);
            }
        }

        // 1. Create a map that maps every instruction in this pattern to every other
        //    instruction in this pattern with which it is independent (independent
        //    means in different states).
        std::map<Instruction*, std::set<Instruction*> > IndependentInstructions;

        // Fills the Independent instructions map (adjacency list)
        FindIndependentInstructions(Instructions, IndependentInstructions,
                this->LVA, this->fsm);

        // Now, every instruction in this pattern is mapped to a set of every
        // other instruction that it is independent with in terms of live
        // intervals

        // 2. Next, create a similar map as IndependentInstructions, but for
        // Graphs: this maps every Graph to every other Graph that it is
        // independent with (i.e. all instructions have non-overlapping lifetimes)
        std::map<Graph*, std::set<Graph*> > IndependentGraphs;

        std::set<Graph*>::iterator g  = Graphs.begin();
        std::set<Graph*>::iterator ge = Graphs.end();
        for ( ; g != ge; ++g ) {
            IndependentGraphs.insert(std::make_pair(*g, std::set<Graph*>()));
        }

        // As above with instructions, now fill IndependentGraphs
        FindIndependentGraphs(IndependentGraphs, IndependentInstructions);
        
        // 3. Finally, given independence information for every graph, pair them.
        // 
        // Consider the results of steps 0-2 above:
        //  - Step 1 mapped every Instruction in this pattern to every other 
        //    instruction with which it is independent
        //  - Step 2 used the information in Step 1 to map every LegUp Graph 
        //    in this pattern to every other LegUp Graph with which it is 
        //    independent.
        // 
        //  The result is IndependentGraphs, which itself can be thought of
        //  as an adjacency list representing a graph: IndependentGraphs
        //  maps every LegUp Graph to a set of other LegUp Graphs. Treating
        //  each LegUp Graph as a vertex and independence as edges, the result
        //  is an adjacency list representation of an "Independence graph"
        //  (in which nodes representing LegUp Graphs are adjacent if their 
        //  LegUp Graphs have independent lifetimes).
        // 
        //  The goal now is to pair up the maximum number of adjacent vertices 
        //  in this Independence graph (i.e. pair up as many independent LegUp 
        //  Graphs as possible for sharing). Moreover, edge weights can be
        //  added to the Independence graph reflecting preferred pairing for 
        //  LegUp Graphs based on bit width and other optimizations. 
        // 
        //  This weighted maximum matching problem is solved optimally by the 
        //  Blossom Algorithm: http://en.wikipedia.org/wiki/Blossom_algorithm,
        //  also called Jack Edmond's Maximum Matching Algorithm,
        //  which solves for a maximum matching in polytime. 
        // 
        //  Currently however, a sub-optimal algorithm is used in which graphs 
        //  are paired greedily and the optimizations mentioned above are 
        //  used as heuristics in a cost function to guide the greedy algorithm.
        //
        //  -- SHADJIS 2012
        
        int numPairs_old = GraphPairs.size();

        // Makes pairs of LegUp Graphs using a greedy algorithm
        MakePairs(
            IndependentGraphs, 
            AllBindingPairs, 
            GraphPairs, 
            InstructionsInGraphs, 
            InstructionGraphUses, 
            AlreadyUsed
        );
            
        int numPairs_new = GraphPairs.size();

        // 4. Now all pairs have been made. Write binding results to output files
        int numPairs = numPairs_new - numPairs_old;
        if (!numPairs) continue; // No new pairs added, so don't output anything
        
        // We need to select a graph whose nodes we can write to the text and 
        // dot files. Any graph will do, so arbitrarily pick the first one
        Graph *representative = iter2->front();
        assert(representative);
        
        WriteToOutputFiles(
            current_size,
            numPairs,
            representative,
            AlreadyUsed, 
            foldername,
            iter2,
            out
        );

        out << "\tInstructions:\n";
        for (std::set<Instruction*>::iterator I = InstructionsInGraphs.begin(),
                E = InstructionsInGraphs.end(); I != E; ++I) {
            out << "\t" << **I << "\n";
        }
        out << "\n";

        WriteDotAndVerilogFiles (
            current_size,
            numPairs,
            representative,
            foldername
        );

    }
}


//===----------------------------------------------------------------------===//
// Pattern Analysis - Step 2:
// 2. Determine which graphs can be paired together and bound to the same 
//    functional units.
//===----------------------------------------------------------------------===//


// Given the IndependentInstructions map above, this function creates 
// a similar map which maps every Graph to all other Graphs with which
// its lifetime is independent. Independence is determined by comparing 
// all equivalent nodes in each graph. 
// For example, if the pattern being considered is add->sub, then two 
// graphs are "independent" if their adds have non-overlapping liftimes 
// AND if their subtracts have non-overlapping lifetimes.
//
// Moreover, two instructions must have bit-widths within PS_BIT_DIFF_THRESHOLD 
// bits of one another (e.g. don't pair an 8-bit adder with a 64-bit adder)
void PatternBinding::FindIndependentGraphs 
(
    std::map<Graph*, std::set<Graph*> > & IndependentGraphs, 
    std::map<Instruction*, std::set<Instruction*> > & IndependentInstructions
) 
{
    // For all graphs (g1)
    std::map<Graph*, std::set<Graph*> >::iterator i   = IndependentGraphs.begin();
    std::map<Graph*, std::set<Graph*> >::iterator end = IndependentGraphs.end();
    for ( ; i != end; ++i) {
        
        // For all graphs (g2)
        std::map<Graph*, std::set<Graph*> >::iterator j = i;
        for ( ; j != end; ++j) {

            Graph *g1 = i->first, *g2 = j->first;

            if (IndependentGraphs[g1].find(g2) != IndependentGraphs[g1].end()) {
                continue;
            }

            // Since we allow overlapping nodes in graphs, it is
            // possible that two graphs share a common instruction. Make sure 
            // this is not the case.
            if (share_common_instruction(g1, g2)) continue;

            // Check if all instructions in these two graphs are independent
            if (CheckAllOperationsForIndependence(g1, g2, 
                  IndependentInstructions)) 
            {
                IndependentGraphs[g1].insert(g2);
                IndependentGraphs[g2].insert(g1);
            }
        }
    }

    // Remove every graph from its own independence set if it's there
    i   = IndependentGraphs.begin();
    end = IndependentGraphs.end();
    for ( ; i != end; ++i) {
        IndependentGraphs[i->first].erase(i->first); // erase self
    }
}


// Check if 2 graphs share a common instruction
bool share_common_instruction (Graph *g1, Graph *g2) 
{
    bool shareCommonInstruction = false;
    Graph::GraphNodes_iterator GNi = g1->GraphNodes.begin();
    Graph::GraphNodes_iterator GNe = g1->GraphNodes.end();
    for ( ; GNi != GNe; ++GNi) {
        if (g2->GraphNodes.find(GNi->first) != g2->GraphNodes.end()) {
            shareCommonInstruction = true;
            break;
        }                
    }
    return shareCommonInstruction;
}


// Given two graphs, check that all of their corresponding instructions
// are independent
bool PatternBinding::CheckAllOperationsForIndependence
(
    Graph *g1, 
    Graph *g2,
    std::map<Instruction*, std::set<Instruction*> > &IndependentInstructions    
)
{
    // In order to handle commutative operations, two maps will be created:

    // Recall that nodes in a graph are labeled with integers from
    // 1 to GraphSize such that two identical graphs with different
    // topology (due to commutative operations) will have corresponding
    // operations labelled consitently (i.e. the node labelled "1" on
    // both graphs will be the same operation, even if it is in a
    // different location)

    // Therefore, for accessibility convenience two maps are created
    // here mapping the label integer to the Instruction it represents
    // on both maps

    std::map<unsigned, Instruction*> Graph1_Labels;
    std::map<unsigned, Instruction*> Graph2_Labels;

    // Fill these maps

    Graph::GraphNodes_iterator GNi;
    for (GNi = g1->GraphNodes.begin(); GNi != g1->GraphNodes.end();
            ++GNi) {
        // GNi->second is a Node *, hence it has an int label, and 
        // GNi->first is the instruction of that Node
        Graph1_Labels[GNi->second->label] = GNi->first; 
        assert(GNi->second->I == GNi->first);
        assert(GNi->second->label > 0);
        assert(GNi->second->label <= g1->size());
    }
    for (GNi = g2->GraphNodes.begin(); GNi != g2->GraphNodes.end();
            ++GNi) {
        Graph2_Labels[GNi->second->label] = GNi->first;
        assert(GNi->second->I == GNi->first);
        assert(GNi->second->label > 0);
        assert(GNi->second->label <= g2->size());
    }

    // The two maps are now filled, so check every operation in both
    // graphs for live interval independence
    // If every operation in the graph is independent with its partner
    // in the other graph, then we can share these Graphs
    bool share_graphs = true;
    assert(g1->size() == g2->size());
    for (int index = 1; index <= g1->size(); ++index) {
        Instruction * i1 = Graph1_Labels[index];
        Instruction * i2 = Graph2_Labels[index];
        assert( strcmp(i1->getOpcodeName(), i2->getOpcodeName()) == 0 );

        // can't share if the operations are not independent
        if ( IndependentInstructions[i1].find(i2) ==
                IndependentInstructions[i1].end() ) { 
            share_graphs = false;
            break;
        }

        // or if their bit widths are too different
        if ( !within_bitwidth_threshold(i1, i2, this->MBW) ) {
            share_graphs = false;
            break;
        }
        
        // or if sharing the instructions causes a combinational loop
        if (combinationalLoop(i1, i2)) {
            share_graphs = false;
            break;
        }
    }
    return share_graphs;
}            


// Check if two instructions are within the defined bit width threshold
// for sharing
bool within_bitwidth_threshold 
(
    Instruction *i1, 
    Instruction *i2,
    MinimizeBitwidth *MBW
) 
{
    int threshold = LEGUP_CONFIG->getParameterInt("PS_BIT_DIFF_THRESHOLD");
    if ( abs( (int)(MBW->getMinBitwidth(i1) - MBW->getMinBitwidth(i2)) ) 
        > threshold) return false;

    // at this point, all predecessors should be either instructions or
    // return a valid bit width from the MinimizeBitwidthPass (e.g. 
    // input args into the function).

    Value *i1pred1 = i1->getOperand(0);
    Value *i1pred2 = i1->getOperand(1);
    Value *i2pred1 = i2->getOperand(0);
    Value *i2pred2 = i2->getOperand(1);

    if ( abs( (int)(MBW->getMinBitwidth(i1pred1)
            - MBW->getMinBitwidth(i2pred1)) ) > threshold )
        return false;
    if ( abs( (int)(MBW->getMinBitwidth(i1pred2)
            - MBW->getMinBitwidth(i2pred2)) ) > threshold )
        return false;

    return true;
}


// Check for combinational loops between patterns and previous bound FUs
// (multipliers/dividers). For instance:
//   %160 = add nsw i32 %140, %136 (state 78) - FU1
//   %161 = mul nsw i32 %160, 362 (state 78) - FU2
//   ...
//   %167 = mul i32 %142, -473 (state 80) - FU2
//   %168 = add i32 %167, %166 (state 80) - FU1
// The lifetimes don't overlap. But here if both mul are shared and both adds
// are shared then we have a combinational loop.  The add fans out to the mul
// and the mul fans out to the add.
bool PatternBinding::combinationalLoop(Instruction *i1,
    Instruction *i2) {
    set<std::string> successorFUs;

    add_successor_binding_FUs(i1, successorFUs);
    add_successor_binding_FUs(i2, successorFUs);

    // loop over predecessors - if a successor is a predecessor we have a loop
    for (User::op_iterator pi = i1->op_begin(), pe =
            i1->op_end(); pi != pe; ++pi) {
        Instruction *pred = dyn_cast<Instruction>(*pi);
        if (existsBindingInstrFU(pred)) {
            std::string predFU = getBindingInstrFU(pred);
            if (successorFUs.find(predFU) != successorFUs.end()) {
                return true;
            }
        }
    }
    for (User::op_iterator pi = i2->op_begin(), pe =
            i2->op_end(); pi != pe; ++pi) {
        Instruction *pred = dyn_cast<Instruction>(*pi);
        if (existsBindingInstrFU(pred)) {
            std::string predFU = getBindingInstrFU(pred);
            if (successorFUs.find(predFU) != successorFUs.end()) {
                return true;
            }
        }
    }
    return false;
}


// loop over successors - find all FUs that we fanout too
void PatternBinding::add_successor_binding_FUs(Instruction
        *I, set<std::string> &successorFUs) {
    for (Value::use_iterator si = I->use_begin(), send = I->use_end();
            si != send; ++si) {
        Instruction *succ = dyn_cast<Instruction>(*si);
        if (existsBindingInstrFU(succ)) {
            successorFUs.insert(getBindingInstrFU(succ));
        }
    }
}


//===----------------------------------------------------------------------===//
// Pattern Analysis - Step 3:
// 3. Select a pairing of Graphs
//===----------------------------------------------------------------------===//

// This function takes a graph defined by the IndependentInstructions adjacency
// list and greedily returns a set of pairs guided by bitwidths. Alternatively
// the Blossom algorithm solves this problem optimally, see the comments in
// GraphLVAPairing where this function is called.
void PatternBinding::MakePairs
(
    std::map<Graph*, std::set<Graph*> > & IndependentGraphs,
    std::map<Value*, Value*> &AllBindingPairs,
    std::map<Graph*, Graph*> &GraphPairs,
    std::set<Instruction*> &InstructionsInGraphs,
    map<Instruction*, std::set<Graph*> > &InstructionGraphUses,
    map<Graph*, bool> &AlreadyUsed
) 
{
    bool finish = false; // finish when every set is empty

    static map<Instruction*, Graph* > InstructionAssignedGraph1, InstructionAssignedGraph2;

    while (!finish) {
        finish = true;
        
        // For all graphs in IndependentGraphs
        for (std::map<Graph*, std::set<Graph*> >::iterator i =
                IndependentGraphs.begin(), ie =
                IndependentGraphs.end(); i != ie; ++i) {

            if (i->second.empty()) continue; // if Graph has no compatibilities

            Graph * partner1 = i->first;
            if (AlreadyUsed[partner1]) continue;

            // map the label of each node in this graph to the node itself
            std::map<unsigned, Instruction*> Graph1_Labels;
            std::map<unsigned, Node*> Graph1_NodeLabels;
            map_labels_to_nodes(partner1, Graph1_Labels, Graph1_NodeLabels);
            
            // Matching algoritm:
            //
            // Given a graph (partner1), consider every other graph that it can 
            // be paired with (according to independent lifetimes). 
            // Iterate over all their corresponding nodes and calculate a cost 
            // function associated to that pairing equal to the sum of the bit  
            // width differences of the operations. Then pair partner1 with
            // the candidate which minimizes the cost.

            Graph * partner2 = FindMinimumCostPartner(
                partner1,    // our current graph
                i->second,    // all possible partners
                Graph1_NodeLabels,               // These data structures hold a 1:1 map
                InstructionsInGraphs,          // of nodes in partner1 to all candidates
                InstructionAssignedGraph1, 
                InstructionAssignedGraph2,
                AlreadyUsed    
            );

            assert (partner1 != partner2);

            // nothing matches with partner1.
            // this can be caused by combinational loops
            if (!partner2) continue;

            GraphPairs.insert(std::make_pair(partner1, partner2));
            AlreadyUsed[partner1] = true;
            AlreadyUsed[partner2] = true;

            finish = false; // found at least another pair, so don't stop yet

            // Now that a pair of graphs has been found, update the data
            // structures to remove every other graph which has nodes that 
            // overlap with those in partner1 and partner2
            remove_overlapping_graphs_from_data_structures(
                partner1,
                partner2,
                Graph1_Labels,
                InstructionAssignedGraph1, 
                InstructionAssignedGraph2,
                IndependentGraphs,
                AllBindingPairs,
                InstructionsInGraphs,
                InstructionGraphUses,
                AlreadyUsed
            );
        }
    }
}


// Recall that each node is uniquely identified by an unsigned short "label".
// This function maps the label of each node in this g to the node itself
void map_labels_to_nodes
(
    Graph *g, 
    std::map<unsigned, Instruction*> &Instruction_Labels,
    std::map<unsigned, Node*> &Node_Labels
)
{
    Graph::GraphNodes_iterator GNi;
    for (GNi = g->GraphNodes.begin(); GNi !=
            g->GraphNodes.end(); ++GNi) {
        Instruction_Labels[GNi->second->label] = GNi->first;
        Node_Labels[GNi->second->label] = GNi->second; // GNi->second is a Node *
        assert(GNi->second->I == GNi->first);
    }
}


// Given a graph (partner1), a set of all pairing candidates for
// this graph, and data structures giving a 1:1 mapping from the
// nodes in partner1 to all candidates, return the best sharing
// partner candidate
Graph *PatternBinding::FindMinimumCostPartner
(
    Graph *partner1,
    set<Graph*> &pairing_candidates,
    std::map<unsigned, Node*> &Graph1_NodeLabels,
    std::set<Instruction*> &InstructionsInGraphs,
    map<Instruction*, Graph* > &InstructionAssignedGraph1,
    map<Instruction*, Graph* > &InstructionAssignedGraph2,
    map<Graph*, bool> &AlreadyUsed
) 
{
    // Select partner2 from all pairing candidates
    Graph *partner2 = NULL;
    int MIN_COST = 1000;

    // Iterate over all pairing candidates
    set<Graph*>::iterator Gi = pairing_candidates.begin();
    set<Graph*>::iterator Ge = pairing_candidates.end();
    for ( ; Gi != Ge; ++Gi ) {
        Graph *partnerCandidate = *Gi;
        if (partner1 == partnerCandidate) continue;
        if (AlreadyUsed[partnerCandidate]) continue;

        // Check for a combinational loop if these graphs are shared
        bool combinationalLoop = false;
        
        int cost = CalculatePairingCost(
            combinationalLoop,
            partnerCandidate,
            Graph1_NodeLabels,
            InstructionAssignedGraph1, 
            InstructionAssignedGraph2,
            InstructionsInGraphs
        );

        if (cost < MIN_COST && !combinationalLoop) {
            MIN_COST = cost;
            partner2 = partnerCandidate;
        }
    }
    
    return partner2;
}


// Given graph pairing candidate "partnerCandidate" and
// a mapping of all the nodes of partnerCandidate to
// the nodes of the original graph, return a cost associated
// with sharing these two graphs. 
// See Resource Sharing paper for more information on the cost 
// function and optimizations considered.
// Also return if a combinational loop is found as this
// disqualifies the pairing.
int PatternBinding::CalculatePairingCost
(
    bool &combinationalLoop,
    Graph *partnerCandidate,
    std::map<unsigned, Node*> &Graph1_NodeLabels,
    map<Instruction*, Graph* > &InstructionAssignedGraph1, 
    map<Instruction*, Graph* > &InstructionAssignedGraph2,
    std::set<Instruction*> &InstructionsInGraphs
)
{
    int current_cost = 0;
    
    // Iterate over all the nodes of the pairing candidate
    Graph::GraphNodes_iterator GNi;
    for (GNi = partnerCandidate->GraphNodes.begin(); GNi !=
            partnerCandidate->GraphNodes.end(); ++GNi) {
            
        // n2 is the node in the pairing candidate
        Node *n2 = GNi->second;
        
        // n1 is the equivalent node in Graph1 to n2
        Node *n1 = Graph1_NodeLabels[n2->label]; 

        // See paper for cost function (bit widhs of the node and predecessors)
        current_cost += abs( (int)(this->MBW->getMinBitwidth(n1->I) - this->MBW->getMinBitwidth(n2->I)) );
        current_cost += abs( (int)(this->MBW->getMinBitwidth(n1->I->getOperand(0)) - 
            this->MBW->getMinBitwidth(n2->I->getOperand(0))) );
        current_cost += abs( (int)(this->MBW->getMinBitwidth(n1->I->getOperand(1)) - 
            this->MBW->getMinBitwidth(n2->I->getOperand(1))) );

        // See paper for shared input variable optimization. If the two
        // nodes share a common input, the cost of a mux is saved.
        adjust_cost_for_shared_input_variables(current_cost, n1, n2);

        // Don't pair graphs that would form a combinational loop
        combinationalLoop = sharing_graphs_creates_combinational_loop(
                                n1,
                                n2,
                                InstructionAssignedGraph1,
                                InstructionAssignedGraph2,
                                InstructionsInGraphs
                            );
            
        if (combinationalLoop) return 0;
    }
    return current_cost;
}


// Adjust pairing cost if the two nodes share common inputs
void adjust_cost_for_shared_input_variables
(
    int &current_cost,
    Node *n1,
    Node *n2
)
{
    if (!n1->p1->is_op) { // if the left predecessor of these nodes is an input
        if (n1->I->getOperand(0) == n2->I->getOperand(0)) {
            current_cost -= 20; // approximation for mux, can use LegupConfig instead
        }
    }
    if (!n1->p2->is_op) { // if the right predecessor of these nodes is an input
        if (n1->I->getOperand(1) == n2->I->getOperand(1)) {
            current_cost -= 20;
        }
    }
}


// There is another function, combinationalLoop(I1, I2), which returns
// true if sharing 2 instructions causes a combinational loop. This
// function handles the more complex problem of checking whether sharing
// two graphs causes a combinational loop. Note that this function is
// not given two graphs but rather is called incrementally during 
// CalculatePairingCost, one node at a time
bool sharing_graphs_creates_combinational_loop
(
    Node *n1,
    Node *n2,
    map<Instruction*, Graph* > &InstructionAssignedGraph1, 
    map<Instruction*, Graph* > &InstructionAssignedGraph2,
    std::set<Instruction*> &InstructionsInGraphs
)
{
    Instruction *i1 = n1->I;
    Instruction *i2 = n2->I;
    set<Graph*> successorFUs;    
    
    // loop over successors - find all FUs that we fanout to
    add_successor_FUs_for_graphs(
        i1,
        i2,
        successorFUs,
        InstructionAssignedGraph1, 
        InstructionAssignedGraph2,
        InstructionsInGraphs
    );
    
    // loop over predecessors - if a successor is a predecessor we have a loop
    for (User::op_iterator pi = i1->op_begin(), pe =
            i1->op_end(); pi != pe; ++pi) {
        Instruction *pred = dyn_cast<Instruction>(*pi);
        if (InstructionsInGraphs.count(pred)) {
            Graph *fu = InstructionAssignedGraph1[pred];
            assert(fu);
            if (successorFUs.find(fu) != successorFUs.end()) {
                return true;
            }
            fu = InstructionAssignedGraph2[pred];
            assert(fu);
            if (successorFUs.find(fu) != successorFUs.end()) {
                return true;
            }
        }

    }
    for (User::op_iterator pi = i2->op_begin(), pe =
            i2->op_end(); pi != pe; ++pi) {
        Instruction *pred = dyn_cast<Instruction>(*pi);
        if (InstructionsInGraphs.count(pred)) {
            Graph *fu = InstructionAssignedGraph1[pred];
            assert(fu);
            if (successorFUs.find(fu) != successorFUs.end()) {
                return true;
            }
            fu = InstructionAssignedGraph2[pred];
            assert(fu);
            if (successorFUs.find(fu) != successorFUs.end()) {
                return true;
            }
        }
    }
    
    return false;
}


// loop over successors of both graphs and find all FUs that 
// they fanout to
void add_successor_FUs_for_graphs
(
    Instruction *i1,
    Instruction *i2,
    set<Graph*> &successorFUs,
    map<Instruction*, Graph* > &InstructionAssignedGraph1, 
    map<Instruction*, Graph* > &InstructionAssignedGraph2,
    std::set<Instruction*> &InstructionsInGraphs
)
{
    for (Value::use_iterator si = i1->use_begin(), send = i1->use_end();
            si != send; ++si) {
        Instruction *succ = dyn_cast<Instruction>(*si);
        if (InstructionsInGraphs.count(succ)) {
            Graph *fu = InstructionAssignedGraph1[succ];
            assert(fu);
            successorFUs.insert(fu),
            fu = InstructionAssignedGraph2[succ];
            assert(fu);
            successorFUs.insert(fu);
        }
    }
    for (Value::use_iterator si = i2->use_begin(), send = i2->use_end();
            si != send; ++si) {
        Instruction *succ = dyn_cast<Instruction>(*si);
        if (InstructionsInGraphs.count(succ)) {
            Graph *fu = InstructionAssignedGraph1[succ];
            assert(fu);
            successorFUs.insert(fu),
            fu = InstructionAssignedGraph2[succ];
            assert(fu);
            successorFUs.insert(fu);
        }
    }
}


// After two graphs are matched, remove this pair from IndependentGraphs
// and update all other data structures
void remove_overlapping_graphs_from_data_structures
(
    Graph *partner1,
    Graph *partner2,
    std::map<unsigned, Instruction*> &Graph1_Labels,
    map<Instruction*, Graph* > &InstructionAssignedGraph1, 
    map<Instruction*, Graph* > &InstructionAssignedGraph2,
    std::map<Graph*, std::set<Graph*> > & IndependentGraphs,
    std::map<Value*, Value*> &AllBindingPairs,
    std::set<Instruction*> &InstructionsInGraphs,
    map<Instruction*, std::set<Graph*> > &InstructionGraphUses,
    map<Graph*, bool> &AlreadyUsed
)
{
    // Update IndependentGraphs:
    
    // for every instruction in partner 1
    Graph::GraphNodes_iterator GNi;
    for (GNi = partner1->GraphNodes.begin(); GNi !=
            partner1->GraphNodes.end(); ++GNi) {
        InstructionAssignedGraph1[GNi->first] = partner1;
        InstructionAssignedGraph2[GNi->first] = partner2;
        // For every graph that this instruction is used in
        for (std::set<Graph*>::iterator Gi = InstructionGraphUses[GNi->first].begin();
                Gi != InstructionGraphUses[GNi->first].end(); ++Gi) {
            Graph *useGraph = *Gi;
            if (AlreadyUsed[useGraph]) continue;
            assert(useGraph != partner2);

            // We need to completely erase all other graphs which use 
            // this node. First, remove from IndepenentGraphs
            IndependentGraphs.erase(useGraph);
            for (std::map<Graph*, std::set<Graph*> >::iterator j =
                    IndependentGraphs.begin(), end = IndependentGraphs.end(); 
                    j != end; ++j) {
                IndependentGraphs[j->first].erase(useGraph);
            }
            // Finally, mark the graph (no longer needed)
            AlreadyUsed[useGraph] = true;
        }
    }

    // repeat for partner 2
    for (GNi = partner2->GraphNodes.begin(); GNi !=
            partner2->GraphNodes.end(); ++GNi) {
        InstructionAssignedGraph1[GNi->first] = partner1;
        InstructionAssignedGraph2[GNi->first] = partner2;
        for (std::set<Graph*>::iterator Gi = InstructionGraphUses[GNi->first].begin();
                Gi != InstructionGraphUses[GNi->first].end(); ++Gi) {
            Graph *useGraph = *Gi;
            if (AlreadyUsed[useGraph]) continue;
            assert(useGraph != partner1);

            IndependentGraphs.erase(useGraph);
            for (std::map<Graph*, std::set<Graph*> >::iterator j =
                    IndependentGraphs.begin(), end = IndependentGraphs.end(); 
                    j != end; ++j) {
                IndependentGraphs[j->first].erase(useGraph);
            }
            AlreadyUsed[useGraph] = true;
        }
    }

    // Update data structures InstructionsInGraphs, AllBindingPairs, IndependentGraphs
    update_graph_data_structures_with_pairing(
        partner1,
        partner2,
        Graph1_Labels,
        IndependentGraphs,
        AllBindingPairs,
        InstructionsInGraphs
    );
}


// After a graph pair is chosen, update data structures InstructionsInGraphs, 
// AllBindingPairs and IndependentGraphs
void update_graph_data_structures_with_pairing
(
    Graph *partner1,
    Graph *partner2,
    std::map<unsigned, Instruction*> &Graph1_Labels,
    std::map<Graph*, std::set<Graph*> > &IndependentGraphs,
    std::map<Value*, Value*> &AllBindingPairs,
    std::set<Instruction*> &InstructionsInGraphs
)
{
    // Update InstructionsInGraphs, AllBindingPairs
    std::map<unsigned, Instruction*> Graph2_Labels;
    Graph::GraphNodes_iterator GNi;
    for (GNi = partner2->GraphNodes.begin(); GNi !=
            partner2->GraphNodes.end(); ++GNi) {
        Graph2_Labels[GNi->second->label] = GNi->first;
        assert(GNi->second->I == GNi->first);
    }

    for (int index = 1; index <= partner1->size(); ++index) {
        Instruction * i1 = Graph1_Labels[index];
        Instruction * i2 = Graph2_Labels[index];
        AllBindingPairs.insert(std::make_pair(i1, i2));
        InstructionsInGraphs.insert(i1);
        InstructionsInGraphs.insert(i2);
    }

    // Lastly, remove the pair from every instruction's set
    IndependentGraphs[partner1].clear();
    IndependentGraphs[partner2].clear();
    for (std::map<Graph*, std::set<Graph*> >::iterator j =
            IndependentGraphs.begin(), end = IndependentGraphs.end(); 
            j != end; ++j) {
        IndependentGraphs[j->first].erase(partner1);
        IndependentGraphs[j->first].erase(partner2);
    }
}


//===----------------------------------------------------------------------===//
// Pattern Analysis - Write to sharing info file and dot / verilog files
//===----------------------------------------------------------------------===//

void PatternBinding::WriteToOutputFiles
(
    int current_size,
    int numPairs,
    Graph *representative,
    map<Graph*, bool> &AlreadyUsed, 
    string & foldername,
    PatternMap::PatternList_iterator iter2,
    formatted_raw_ostream &out
) 
{        
    // First write to the sharing log file
    out << "\tPattern Size: " << current_size << " (contents: ";

    Graph::GraphNodes_iterator GNi = representative->GraphNodes.begin(); 
    Graph::GraphNodes_iterator GNe = representative->GraphNodes.end();
    for ( ; GNi != GNe; ++GNi ) {
        assert(GNi->second->I == GNi->first);
        out << GNi->second->op << ", ";
    }

    // Next, count how many graphs we've found in this pattern to compare
    // to the number of pairs. We don't want to count overlaps however,
    // so the number of unique graphs can be found by:
    // #unique = #paired (i.e. 2*numPairs) + #unpaired but not overlaps
    int NumberOfGraphs = 2*numPairs; 
    // (note some may have contained overlaps)
    PatternMap::Graph_iterator iter3 = iter2->begin();
    PatternMap::Graph_iterator iend  = iter2->end();
    for ( ; iter3 != iend; ++iter3) {
        if ( AlreadyUsed[*iter3] ) continue;
        NumberOfGraphs++;
    }

    out << ")\n\tFrequency: " << NumberOfGraphs << "\n";
    out << "\tNumber of Pairs: " << numPairs << "\n";
    
}


void PatternBinding::WriteDotAndVerilogFiles
(
    int current_size,
    int numPairs,
    Graph *representative,
    string & foldername
)
{
    if ( (LEGUP_CONFIG->getParameterInt("PS_WRITE_TO_DOT") ||
            LEGUP_CONFIG->getParameterInt("PS_WRITE_TO_VERILOG")) &&
         (numPairs >= LEGUP_CONFIG->getParameterInt("FREQ_THRESHOLD")) )
    {
        int prevent_warnings = chdir(foldername.c_str());
        string filename = IntToString(file_counter()) + "_freq" + 
            IntToString(numPairs) + "_size" + IntToString(current_size);
        if (LEGUP_CONFIG->getParameterInt("PS_WRITE_TO_DOT")) {
            write_to_dot(representative, filename);
            prevent_warnings += system("if [ -e \"*.dot\" ]; then dot -Tpdf -O *.dot; fi");
        }
        if (LEGUP_CONFIG->getParameterInt("PS_WRITE_TO_VERILOG")) {
            write_to_verilog(representative, filename, this->Fp, 2); // mux width 2
        }
        prevent_warnings += chdir("..");
		assert(prevent_warnings == 0);
    }
}


// used to create unique file names for dot and verilog files
int file_counter () {
    static int c = 0;
    ++c;
    return c;
}


// write a graph to a dot file with a unique filename consisting of the 
// pattern's frequency and size, as well as a unique identification number
// Note: write_to_dot and write_to_verilog can be moved to utils.cpp
void write_to_dot(Graph * g, string filename)  {
    if (g->size()<=1) return; // Don't output patterns of size 1

    string full_filename = filename + ".dot";
    ofstream dot(full_filename.c_str());
    dot << endl << "digraph name {" << endl;

    set <Node*> visited;
    queue <Node*> q;
    Node * t;
    q.push(g->getRoot());
    visited.insert(g->getRoot());

    while (!q.empty()) {
        t = q.front(); q.pop();

        // Visit every node as in previous functions
        if (t->p1->is_op) {
            if ( visited.find(t->p1) == visited.end() ) { // not in set
                q.push(t->p1);
                visited.insert(t->p1);
            }
            dot << "  " << t->p1->op << (char)(t->p1->label + 64) << "->" 
                << t->op << (char)(t->label + 64) << ";" << endl; 
        }
        else dot << "  " << t->p1->op << "->" << t->op 
            << (char)(t->label + 64) << ";" << endl;

        if (t->p2->is_op) {
            if ( visited.find(t->p2) == visited.end() ) {
                q.push(t->p2);
                visited.insert(t->p2);
            }
            dot << "  " << t->p2->op << (char)(t->p2->label + 64) << "->" 
                << t->op << (char)(t->label + 64) << "[style=dotted];" 
                << endl; // dotted line = right predecessor
        }
        else dot << "  " << t->p2->op << "->" << t->op 
            << (char)(t->label + 64) << "[style=dotted];" << endl;
    }

/*  // NOTE: The code above visits all nodes by traversing the graph.
    // A simpler implementation would be to use the Graph's GraphNodes map to 
    // visit nodes. However, then the dot files are not written in a 
    // conventional order (root up). Currently this order is not being used but
    // may be convenient in the future for comparing if two graphs are 
    // equivalent based on their dot representations.
    // 
    // To change this, replace code above with the code below, starting from 
    // and including     set <Node*> visited;

    GraphNodes_iterator i;
    // for each Node in the graph
    for (i=g->GraphNodes.begin(); i != g->GraphNodes.end(); ++i) {
        Node * t = i->second;

        if (t->p1->is_op) {
            dot << "  " << t->p1->op << t->p1->label << "->" << t->op 
                << t->label << ";" << endl;
        }
        else dot << "  " << t->p1->op << "->" << t->op << t->label 
            << ";" << endl;

        if (t->p2->is_op) {
            dot << "  " << t->p2->op << t->p2->label << "->" << t->op 
                << t->label << "[style=dotted];" << endl;
        }
        else dot << "  " << t->p2->op << "->" << t->op << t->label 
            << "[style=dotted];" << endl;
    }
*/

    dot << "}" << endl;
    dot.close();
}


// Write a Verilog circuit for a Pattern Graph g with input muxing
void write_to_verilog
(
    Graph * g, 
    string filename, 
    Function *Fp, 
    int MUX_WIDTH
)  
{
    // Check for invalid circuit parameters
    
    if (g->size()<=1) return; // Don't output patterns of size 1
    if (MUX_WIDTH < 2) return;

    string full_filename = filename + ".v";
    string ErrorInfo;
    raw_fd_ostream Out(full_filename.c_str(), ErrorInfo, llvm::sys::fs::F_None);

    if (!ErrorInfo.empty()) {
        errs() << "Error: " << ErrorInfo << '\n';
        assert(0);
    }

    int input_width = g->getRoot()->I->getOperand(0)->getType()->getPrimitiveSizeInBits();
    if (input_width == 0) return;

    // Create the RTL and write it to Out
    Allocation * allocation = new Allocation(Fp->getParent());
    RTLModule *rtl = l_create_graph_rtl(
        g, MUX_WIDTH, filename.c_str(), input_width
    );
    allocation->addRTL(rtl);
    VerilogWriter *writer = new VerilogWriter(Out, allocation);
    writer->printRTL(rtl);
    
    // delete rtl;
    // delete allocation;
    // delete writer;
}


RTLModule *l_create_graph_rtl
(
    Graph *g, 
    int MUX_WIDTH, 
    const char *module_name,
    const int input_width
) 
{
    RTLModule *rtl = new RTLModule(module_name); 

    rtl->addIn("clk");

    // Determine select signal width
    RTLSignal *select = l_add_select_to_rtl (rtl, MUX_WIDTH);

    // Add the bit width
    rtl->addParam( "WIDTH", IntToString(input_width).c_str() );
    RTLWidth width("WIDTH-1");
    
    // Write all inputs
    std::map<int, RTLSignal*> inputs;
    vector<RTLSignal*> in; 
    vector<RTLSignal*>reg;

    for (int i = 1; i <=(g->getNumInputs() * MUX_WIDTH); i++) {
        std::string name = "data" + IntToString(i);
        in.push_back( rtl->addIn(name, width) );
        reg.push_back ( rtl->addReg(name + "_reg", width) );
        reg.back()->connect(in.back());
        inputs[i-1] = reg.back();
    }

    // Write output. Multiply has double bit width, compare has bit width 1.
    RTLSignal *dataout = rtl->addOutReg("dataout", l_rtl_output_width(g));
    
    // Muxing conditions (select==0, ==1, etc for each possibility in muxwidth)
    vector<RTLOp*> cond; // Vector of pointers to RTLOp objects
    cond.reserve(MUX_WIDTH);
    for (int i = 0; i < MUX_WIDTH; i++) {
        RTLOp * new_op = rtl->addOp(RTLOp::EQ);
        cond.push_back(new_op);
        cond.back()->setOperand(0, select); // select (LHS operand)
        cond.back()->setOperand(1, new RTLConst(IntToString(i)));
    }

    // intermediate variables for each operation result
    vector<RTLSignal*> intermediates;// (g->size(), NULL);
    for (int i = 0; i < g->size(); i++) {
        intermediates.push_back( rtl->addWire("i" + IntToString(i+1), width) );
    }

    // Signal connections
    vector<RTLSignal*> w;
    for (int i = 0; i < g->getNumInputs(); i++) {
        string wirename = "w" + IntToString(i+1);
        w.push_back( rtl->addWire(wirename.c_str(), width) );
        for (int j = 0; j < MUX_WIDTH; j++) {
          w.back()->addCondition(cond[j], inputs[i + j*(g->getNumInputs())]);
        }
    }

    // Operations. Now we must visit every instruction node in the graph
    vector<RTLOp*> op; // 1 operation for each node
    Graph::GraphNodes_iterator i;
    map<Node*,int> OperationIndex;
    int count = 0;

    // add all nodes to map and create an operation for them
    for (i=g->GraphNodes.begin(); i != g->GraphNodes.end(); ++i) {
        Node * t = i->second;
        OperationIndex[t] = count;
        count++;
        RTLOp * new_op = rtl->addOp( t->I );
        op.push_back( new_op );
    }

    // Now we have an operation in op[] for each node, so add all predecessors
    l_rtl_add_all_predecessors (g, intermediates, OperationIndex, op, w);
    
    dataout->connect(intermediates[ OperationIndex[g->getRoot()] ]);
    return rtl;
}

    
RTLSignal *l_add_select_to_rtl(RTLModule *rtl, int MUX_WIDTH) {
    int select_width = ceil(log(MUX_WIDTH)/log(2)); // log base 2
    RTLSignal *select;
    if (select_width < 2) {
        select = rtl->addIn("select");
    }
    else {
        RTLWidth selwidth(IntToString(select_width-1).c_str());
        select = rtl->addIn("select", selwidth);
    }
    return select;
}


RTLWidth l_rtl_output_width(Graph *g) 
{
    if (strcmp(g->getRoot()->I->getOpcodeName(), "mul") == 0) {
        RTLWidth output_width("2*WIDTH-1");
        return output_width;
    }
    if (strcmp(g->getRoot()->I->getOpcodeName(), "icmp") == 0) {
        RTLWidth output_width("1");
        return output_width;
    }
    RTLWidth output_width("WIDTH-1");
    return output_width;
}


void l_rtl_add_all_predecessors 
(
    Graph *g,
    vector<RTLSignal*> &intermediates,
    map<Node*,int> &OperationIndex,
    vector<RTLOp*> &op,
    vector<RTLSignal*> &w
)
{
    int input_count = 0;
   
    Graph::GraphNodes_iterator i;
    for (i=g->GraphNodes.begin(); i != g->GraphNodes.end(); ++i) {
        Node * t = i->second;
        intermediates[ OperationIndex[t] ] -> connect( op[ OperationIndex[t] ] );

        if (t->p1->is_op) {
            op[ OperationIndex[t] ]->setOperand(0, intermediates[ OperationIndex[t->p1] ]);
        }
        // else if const, set operand to be the new const we create
        else {
            op[ OperationIndex[t] ]->setOperand(0, w[input_count]);
            input_count++;
        }

        if (t->p2->is_op) {
            op[ OperationIndex[t] ]->setOperand(1, intermediates[ OperationIndex[t->p2] ]);
        }
        else {
            op[ OperationIndex[t] ]->setOperand(1, w[input_count]);
            input_count++;
        }
    }
}

} // End legup namespace
