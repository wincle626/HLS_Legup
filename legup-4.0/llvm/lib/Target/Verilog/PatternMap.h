//===-- PatternMap.h --------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// A PatternMap is a container for Graph objects used by Binding.cpp.
// It organizes patterns of operations by isomorphic equivalence,
// taking commutative operations into account.
//
// Each Pattern (e.g. add->add) is represented by a vector of graphs. For example 
// if the add->add pattern occurs with frequency 5 in a program, then the 
// "add->add" pattern will be a vector with 5 graphs. Each of these
// is called a GraphList:
//
//         GraphList    =     vector<Graph*>, representing all the
//                                            graphs of that pattern
//
// Moreover, the graphs in a PatternList are all checked for isomorphic (not only 
// topological) equivalence, by checking all commutative operations.
//
// Since there are many patterns which can exist (add->sub, mul->add, etc) a vector 
// of Patterns, i.e. a vector of GraphLists. is required. 
// This is represented by a PatternList:
//
//        PatternList   =     vector<GraphList>
//
// For example a program may have 5 "add->add"s and 3 "add->sub"s, in which case 
// the PatternList is of size 2, corresponding to a GraphList of size 5 and a 
// GraphList of size 3
//
// Finally, for faster lookup, PatternLists are separated by size using a map:
// NOTE: lookup can be made even faster by also creating bins for the root 
//       operation (group all PatternLists by size and by root operation)
//
//         PatternMap   =      map<int (pattern size), PatternList>
//
// E.g. PatternMap[1] contains all patterns of size 1, etc.
//
// Hence a PatternMap is:
//
//        PatternMap    =    map<int, vector< vector<Graph*> > >
//     
// However, the implementation is abstracted, so the only required operation to
// insert each Graph into a PatternMap is the Add() operation. This adds the 
// graph to the required PatternList and GraphList, creating lists as necessary, 
// and checking for isomorphic equivalence.
//
// Then, the iterators PatternMap_iterator, PatternList_iterator and 
// GraphList_iterator can be used to iterate over every graph as needed.
//
//===----------------------------------------------------------------------===//

#ifndef PATTERNMAP_H
#define PATTERNMAP_H

#include "Graph.h"
#include "FiniteStateMachine.h"

using namespace std;
using namespace llvm;
using namespace legup;

class PatternMap {

    public:
        /***********************************************************************
         * Definitions
         **********************************************************************/  
        // All lists of Graphs for that pattern
        typedef vector<Graph*> GraphList; 
        typedef GraphList::iterator Graph_iterator;

        // List of GrapLists, i.e. a list of list of graphs
        typedef vector<GraphList> PatternList; 
        typedef PatternList::iterator PatternList_iterator;

        // for iterating through a PatternMap
        typedef map<int, PatternList>::iterator PatternMap_iterator;

        /***********************************************************************
         * Public Variables and Data Structures
         **********************************************************************/  
        // organizes patterns by size, i.e. maps size 
        // to a vector of PatternLists
        map<int, PatternList> Patterns;    

        // keeps track of Equivalent copies of a pattern, to check commutativity
        map<int, PatternList> EquivalentCopies;    

        /***********************************************************************
         * Public Member Functions
         **********************************************************************/  
        // Constructor / Destructor
        PatternMap(FiniteStateMachine *fsm) : fsm(fsm) {}
        ~PatternMap();
        
        // Iterators and Searching
        map<int, PatternList>::iterator begin() { return Patterns.begin(); }
        map<int, PatternList>::iterator end() { return Patterns.end(); }
        map<int, PatternList>::iterator find(int i) { return Patterns.find(i); }
        
        // Accessors
        unsigned size() { return Patterns.size(); }
        bool empty() { return Patterns.empty(); }
        
        // Modifiers
        void Add( Graph * g ); // Add a graph to the container

    private:    
        /***********************************************************************
         * Private Variables and Data Structures
         **********************************************************************/  
        FiniteStateMachine *fsm; // needed to determine if two graphs have 
                // operations in the same states

        /***********************************************************************
         * Private Helper Functions
         **********************************************************************/  
        // Used to copy the node labelling from one graph to another
        void CopyLabels (Graph * Old, Graph * New);

        // To check if two graphs are equivalent despite commutativity, a new 
        // GraphList is created for every pattern to store all equivalent versions
        // of that pattern
        void AddEquivalentCopies (Graph * base, Instruction * start, GraphList * list);

        // check if operations in two graphs are equal
        bool checkEquality( Graph *g1, Graph *g2 );
        
        // Helper function for checkEquality
        bool checkEqualityThroughBFS( Graph *g1, Graph *g2 );
};

#endif

