//===-- Graph.cpp -----------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// A data flow graph representing a circuit. 
//
//===----------------------------------------------------------------------===//

#include "Graph.h"
using namespace legup;

//------------------------------------------------------------------------------
// Definition of Graph Class member functions
//------------------------------------------------------------------------------

/*******************************************************************************
 * Local Helper Functions
 ******************************************************************************/  

// Return the string corresponding to this instruction for the
// graph. Alternatively, alloc->getConfig->getOpnameFromInst can be used.
string get_instruction_string_for_graph (const Instruction * I) {
    string op_name = I->getOpcodeName();
    
    // If the instruction is icmp, concatenate compare condition to the end 
    // (e.g. icmp -> icmpeq)
    if (const ICmpInst *cmp = dyn_cast<ICmpInst>(I)) {
        switch (cmp->getPredicate()) {
        case ICmpInst::ICMP_EQ:  op_name += "eq"; break;
        case ICmpInst::ICMP_NE:  op_name += "ne"; break;
        case ICmpInst::ICMP_SLT: op_name += "slt"; break;
        case ICmpInst::ICMP_ULT: op_name += "ult"; break;
        case ICmpInst::ICMP_SLE: op_name += "sle"; break;
        case ICmpInst::ICMP_ULE: op_name += "ule"; break;
        case ICmpInst::ICMP_SGT: op_name += "sgt"; break;
        case ICmpInst::ICMP_UGT: op_name += "ugt"; break;
        case ICmpInst::ICMP_SGE: op_name += "sge"; break;
        case ICmpInst::ICMP_UGE: op_name += "uge"; break;
        default: ;
        }
    }

    // Both operands should have same type, concatenate it to operation name, 
    // e.g. change from mul to muli32, or icmpeq from above to icmpeqi64 
    op_name += "i" + legup::IntToString( I->getOperand(0)->getType()->getPrimitiveSizeInBits() ); 
    
    return op_name;
}

/*******************************************************************************
 * Private Member Functions
 ******************************************************************************/  

// Create, initialize and return a new node given an instruction
Node * Graph::NewNode(Instruction * _I) {
    N++;
    Node *temp = new Node (
                /* I= */ _I, 
                /* op= */ get_instruction_string_for_graph(_I), 
                /* label= */ 0
            );

    // Create nodes for the two predecessors. The left and right 
    // predecessors which are either "constants" or "inputs".

    // Check if the left predecessor of I is a constant or an input
    if ( isa<ConstantInt>( temp->I->getOperand(0) )) {
    
        // prepend "POS" or "NEG" to the const name
        string constantName;
        int value = cast<ConstantInt>(temp->I->getOperand(0))->getZExtValue();
        if (value<0) constantName = "NEG" + IntToString( value*-1 );
        else constantName = "POS" + IntToString(value*1);

        // if the constant has been found already, no need to create new node
        if (CONSTANTS.find(constantName) != CONSTANTS.end()) { 
            temp->p1 = CONSTANTS[constantName];
        }
        else { // create new const node
            Node * new_constant = new Node (
                    /* I= */ NULL, 
                    /* op= */ constantName, 
                    /* label= */ 0,
                    /* p1= */ NULL, 
                    /* p2= */ NULL
                );
            CONSTANTS.insert( pair<string,Node*>(constantName, new_constant) );
            temp->p1 = new_constant;
        }
    }
    else temp->p1 = INPUT;

    // Check if the right predecessor of I is a constant or an input
    if ( isa<ConstantInt>( temp->I->getOperand(1) )) {
    
        // prepend "POS" or "NEG" to the const name
        string constantName;
        int value = cast<ConstantInt>(temp->I->getOperand(1))->getZExtValue();
        if (value<0) constantName = "NEG" + IntToString( value*-1 );
        else constantName = "POS" + IntToString(value*1);

        // if the constant has been found already, no need to create new node
        if (CONSTANTS.find(constantName) != CONSTANTS.end()) { 
            temp->p2 = CONSTANTS[constantName];
        }
        else { // create new node
            Node * new_constant = new Node (
                    /* I= */ NULL, 
                    /* op= */ constantName, 
                    /* label= */ 0,
                    /* p1= */ NULL, 
                    /* p2= */ NULL
                );
            CONSTANTS.insert( pair<string,Node*>(constantName, new_constant) );
            temp->p2 = new_constant;
        }
    }
    else temp->p2 = INPUT;

    return temp;
}


void Graph::DeleteAllNodes() {
    GraphNodes_iterator i;
    // for each Node in the graph
    for (i=GraphNodes.begin(); i != GraphNodes.end(); ++i) 
        delete i->second;
    N=0;
}

/*******************************************************************************
 * Constructors and Destructor
 ******************************************************************************/  

Graph::Graph() 
  : N (0), NumInputs (2)
{
    INPUT = new Node (
                /* I= */ NULL, 
                /* op= */ "INPUT", 
                /* label= */ 0,
                /* p1= */ NULL, 
                /* p2= */ NULL
            );

    root = NULL;
    current = root;
}

Graph::Graph(Instruction * _I) 
  : N (0), NumInputs (2)
{
    INPUT = new Node (
                /* I= */ NULL, 
                /* op= */ "INPUT", 
                /* label= */ 0,
                /* p1= */ NULL, 
                /* p2= */ NULL
            );
    
    root = NewNode(_I);
    GraphNodes.insert( pair<Instruction*, Node*> (_I, root) ); 
    current = root;
}

// destructor
Graph::~Graph() {
    if (N!=0) {
        DeleteAllNodes();
    }
    delete INPUT;
    map<string,Node*>::const_iterator it  = CONSTANTS.begin();
    map<string,Node*>::const_iterator end = CONSTANTS.end();
    for ( ; it != end; ++it) {
        delete it->second;
    }
}

/*******************************************************************************
 * Data Accessors
 ******************************************************************************/  

// return the left predecessor of the "current" node
Instruction * Graph::getLeftInstruction() {
    Instruction * predecessor = dyn_cast<Instruction> ( current->I->getOperand(0) );
    return predecessor;
}

// return the right predecessor of the "current" node
Instruction * Graph::getRightInstruction() { 
    Instruction * predecessor = dyn_cast<Instruction> ( current->I->getOperand(1) );
    return predecessor;
}

/*******************************************************************************
 * Modifiers
 ******************************************************************************/  

// Adds a new left predecessor to "current" with the given Instruction 
void Graph::GrowLeft(Instruction * _I) {
    // if we have not yet added this instruction
    if ( GraphNodes.find(_I) == GraphNodes.end() ) { 
        current->p1 = NewNode(_I);

        // add this instruction and node to our map
        GraphNodes.insert( pair<Instruction*, Node*> (_I, current->p1) ); 
        assert(current->p1->I == _I);
        NumInputs++;
    }
    else {
        current->p1 = GraphNodes[_I];
        NumInputs--;
    }
}

// Adds a new right predecessor to "current" with the given Instruction 
void Graph::GrowRight(Instruction * _I) {
    // if we have not yet added this instruction
    if ( GraphNodes.find(_I) == GraphNodes.end() ) { 
        current->p2 = NewNode(_I);

        // add this instruction and node to our map
        GraphNodes.insert( pair<Instruction*, Node*> (_I, current->p2) );
        assert(current->p2->I == _I);
        NumInputs++;
    }
    else {
        current->p2 = GraphNodes[_I];
        NumInputs--;
    }
}

/*******************************************************************************
 * Assignment Operator
 ******************************************************************************
 * 
 * Deep copy of graph objects. g2 = g1 allocates memory for new nodes
 * so that g2 is the same size as g1 with unique nodes, while the data
 * in the nodes stays the same (LLVM Instructions, names, etc.).
 * Graphs can be thought of as "containers" for LLVM Instructions, so
 * this operator has the same functionality as e.g. copying a vector of
 * Instructions, i.e. vector <Instruction*> v2 = v1.
 *
 *  Frequently with Graph object we need to traverse all nodes in the graph and 
 *    perform an operation. 
 * 
 *    This is the algorithm used (BFS):
 *
 *     1. Create a set of nodes visited. The set initially contains only the root
 *     2. Create also a queue of nodes to visit, and insert the root
 *     3. While the queue is not empty, pop the front element. If its left and 
 *            right predecessors are operations and have not been visited, add them to
 *            the queue and mark them visited.
 *
 ******************************************************************************/  

Graph & Graph::operator = ( Graph & rhs     ) { // g2 = g1, rhs is g1
    if ( this != &rhs ) { // Check for self-assignment (if so just return)

        // This map maps nodes in the old graph (rhs) to its copy (this). 
        // It also keeps track of visited nodes during the BFS of the rhs graph.
        map <Node*, Node*> visited;
        this->DeleteAllNodes();

        if (rhs.size() == 0) return (*this);

        this->root = this->NewNode(rhs.getRoot()->I); // copy the root operation first

        // add this instruction and node to our map
        this->GraphNodes.insert( pair<Instruction*, Node*> (this->root->I, this->root) ); 
        this->current = this->root;

        queue <Node*> lhs_nodes; // queue of Nodes we still need to visit, in the "this" (new) graph
        queue <Node*> rhs_nodes; // queue of Nodes we still need to visit, in rhs (old) graph
        Node * t2; // unlike before, while we still do not want to change the 
                   // current pointer of the rhs graph, the current pointer of 
                   // the "this" graph cannot be in use elsewhere yet and hence 
                   // may be used now

        rhs_nodes.push(rhs.root); // initialize our 2 queues
        lhs_nodes.push(this->root);
        visited.insert( pair<Node*, Node*> (rhs.getRoot(), this->root) ); // insert the roots

        // while still more nodes to process (rhs and lhs queues always 
        // have same size, no need to check both)
        while (!rhs_nodes.empty()) { 
            this->current = lhs_nodes.front(); lhs_nodes.pop(); // pop the top node from lhs node queue
            t2 = rhs_nodes.front(); rhs_nodes.pop(); // pop the top node from rhs node queue

            // left predecessor

            // if the node we just popped has a left predecessor in the rhs 
            // graph, we have to add the left predecessor to the lhs graph too
            if (t2->p1->is_op) { 
                // if we haven't visited that predecessor before, then we can 
                // create a new node in lhs graph
                if ( visited.find(t2->p1) == visited.end() ) { 
                    this->GrowLeft(t2->p1->I);    // add left to our new subtree
                    rhs_nodes.push(t2->p1);          // push it onto both queues
                    lhs_nodes.push(this->current->p1); // the node we just added

                    // mark this pair as visited
                    visited.insert( pair<Node*, Node*> (t2->p1, this->current->p1) ); 
                }
                else { // case where the predecessor exists but has been visited
                    this->current->p1 = visited[t2->p1];
                }
            }

            // repeat for right predecessor
            if (t2->p2->is_op) {
                if ( visited.find(t2->p2) == visited.end() ) {
                    this->GrowRight(t2->p2->I);
                    rhs_nodes.push(t2->p2);
                    lhs_nodes.push(this->current->p2);
                    visited.insert( pair<Node*, Node*> (t2->p2, this->current->p2) );
                }
                else {
                    this->current->p2 = visited[t2->p2];
                }
            }
        }// while    

        // Finally, set this->current to be the node in this corresponding to 
        // rhs.current, so the graphs are completely similar
        this->current = visited[rhs.current]; 

    }
    return * this;
}

