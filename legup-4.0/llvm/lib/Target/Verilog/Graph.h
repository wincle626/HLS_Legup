//===-- Graph.h -------------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// A data flow graph representing a circuit. 
//
//===----------------------------------------------------------------------===//


#ifndef GRAPH_H
#define GRAPH_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "utils.h"

#include<string>
#include<queue>
#include<map>

using namespace std;

// Each graph representing an operation chain is composed of Nodes. Each node
// is an instruction/operation
struct Node {

public:

    Node 
    (
        Instruction * I_ = NULL, 
        string op_ = "", 
        unsigned short label_ = 0,
        Node *p1_ = NULL, 
        Node *p2_ = NULL
    ) : I (I_), 
        op (op_), 
        label (label_),
        p1 (p1_), 
        p2 (p2_)
    {
        is_op = (I != NULL);    // only false for INPUT / CONST nodes
    }
    
    Instruction * I; // pointer to the instruction object associated w/ this node
    string op; // the string name corresponding to the above instruction
    unsigned short label; // used to uniquely identify each node 
    Node * p1, * p2;    // pointers to predecessor nodes
    bool is_op; // is this an operation (equivalent to checking if I is not NULL)
};


//------------------------------------------------------------------------------
// Main class. All operation chains are represented as Graph objects. 
// These are directed graphs with one root node (operation).
//
// See the LegUp wiki for slides explaining the pattern sharing algorithm.
//------------------------------------------------------------------------------
class Graph {
  private:  
        /***********************************************************************
         * Private Variables and Data Structures
         **********************************************************************/  
        int N,         // # nodes in a graph
            NumInputs; // # inputs to the circuit the graph represents 
        
        Node * current, // A temporary pointer used for convenience
             * root,    // The final operation in the pattern                    
             * INPUT;   // Represents an input to an operation

        map<string,Node*> CONSTANTS; // store all constant value inputs
             
        /***********************************************************************
         * Private Helper Functions
         **********************************************************************/  
        // allocate memory and return a new node 
        Node * NewNode(Instruction * _I); 
        
        // called by destructor
        void DeleteAllNodes(); 

    public:
        /***********************************************************************
         * Public Variables and Data Structures
         **********************************************************************/  

        // Iterators:
         
        // This map is used to iterate over the nodes in a Graph.
        // It keeps track of every LLVM Instruction associated with a graph node, 
        // so when adding a new node this map is also searched to see if the 
        // instruction being added already exists in the graph.
        map<Instruction*, Node*> GraphNodes;
        typedef map<Instruction*, Node*>::const_iterator GraphNodes_iterator;

        /***********************************************************************
         * Public Member Functions
         **********************************************************************/  
        // Constructor and Destructor:
        Graph();
        Graph(Instruction * _I); // create a root node
        ~Graph(); // Delete all nodes

        // Deep copy of graph objects. g2 = g1 allocates memory for new nodes
        // so that g2 is the same size as g1 with unique nodes, while the data
        // in the nodes stays the same (LLVM Instructions, names, etc.).
        // Graphs can be thought of as "containers" for LLVM Instructions, so
        // this operator has the same functionality as e.g. copying a vector of
        // Instructions, i.e. vector <Instruction*> v2 = v1.
        Graph & operator = ( Graph & rhs ); 
        
        // Data Accessors:
        
        int size() { return N; }
        int getNumInputs() { return NumInputs; }
        Node * getRoot() { return root; }
        Node * getCurrent() { return current; }
        
        // "left" and "right" below refer to the left and right 
        // predecessors of the node pointed to by "current"
        Instruction * getLeftInstruction();
        Instruction * getRightInstruction();

        // Data Modifiers:
        
        void setCurrent(Node * n) { current = n; }

        // Add nodes to the graph with the given instruction to 
        // the left and right of the node that "current" points to
        void GrowLeft(Instruction * _I); 
        void GrowRight(Instruction * _I);
};

#endif

