//===-- PatternMap.cpp ------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// A PatternMap is a container for Graph objects used by Binding.cpp and 
// PatternFind.cpp. See PatternMap.h for a more in depth explanation.
//
//===----------------------------------------------------------------------===//

#include "PatternMap.h"
#include<set>

/*******************************************************************************
 * Local Helper Functions
 ******************************************************************************/  

// Label all the nodes in a graph
void Label (Graph * g) {
    unsigned label = 1;
     std::map<Instruction*, Node*>::iterator GNi = g->GraphNodes.begin();
     std::map<Instruction*, Node*>::iterator GNe = g->GraphNodes.end(); 
    for ( ; GNi != GNe; ++GNi) {
        GNi->second->label = label;
        label++;
    }
}

// Return true if the two nodes are of the same type.
// Node types are the same if either:
//    - both nodes are operations of the same type,
//    - both nodes are the INPUT node,
//    - both nodes are constants with the same value
// Also return a mismatch if either node is NULL.
bool node_types_mismatch(Node *n1, Node *n2) {
    // Return mismatch if either node is NULL
    if (n1 == NULL || n2 == NULL) return true;
    
    // If both nodes are operations or both are not operations,
    // then there is a mismatch if their type strings are unequal
    // - for operations these strings are e.g. "and32", and
    // - for non-operations (INPUT or CONSTANT) the strings are 
    //   either "INPUT" or e.g. "POS3", "NEG4", etc.
    if ( (n1->is_op  && n2->is_op) ||
         (!n1->is_op && !n2->is_op) )
    {
        return (n1->op != n2->op); // return mismatch if unequal
    }
    
    // One instruction is an operation and the other is not,
    // so there is a mismatch
    return true;
}

/*******************************************************************************
 * Private Helper Functions
 ******************************************************************************/

// Used to copy the node labelling from one graph to another
void PatternMap::CopyLabels (Graph * Old, Graph * New) {
    assert (Old->size() == New->size());
    assert (Old->size() != 0);

    New->getRoot()->label = Old->getRoot()->label;
    std::set <Node*> visited; // nodes visited in the Old graph
    visited.insert(Old->getRoot());

    queue <Node*> old_nodes;
    old_nodes.push(Old->getRoot()); // queue of nodes to visit in Old graph
    queue <Node*> new_nodes;
    new_nodes.push(New->getRoot()); // queue of nodes to visit in New graph

    // use these temp pointers as opposed to the graph's "current" 
    // pointer in order to not change the value of the current pointer, 
    // which may be in use by another function
    Node * t1; 
    Node * t2; 

    while (!old_nodes.empty()) {
        t1 = old_nodes.front(); old_nodes.pop();
        t2 = new_nodes.front(); new_nodes.pop();

        // If the predecessors are operations and haven't
        // been visited yet, copy their label
        if (t1->p1->is_op) {
            if (visited.find(t1->p1) == visited.end()) {
                t2->p1->label = t1->p1->label;
                visited.insert(t1->p1);
                old_nodes.push(t1->p1);
                new_nodes.push(t2->p1);
            }
        }
        if (t1->p2->is_op) {
            if (visited.find(t1->p2) == visited.end()) {
                t2->p2->label = t1->p2->label;
                visited.insert(t1->p2);
                old_nodes.push(t1->p2);
                new_nodes.push(t2->p2);
            }
        }
    }
}


// Adds all equivalent graphs to the given GraphList (recursive)
// This function recursively adds all equivalent copies of the base 
// Graph to the list
void PatternMap::AddEquivalentCopies 
(
    Graph * base, 
    Instruction * start, 
    GraphList * list
) 
{
    Graph::GraphNodes_iterator i;
    bool begin = false;
    for (i = base->GraphNodes.begin(); i != base->GraphNodes.end(); ++i) {
        // only start swapping commutative operations from the 
        // one sent as a parameter
        if ( (*i).first == start ) {
            begin = true; 
        }
        
        // Note this is not equivalent to if ((*i).first != start) continue, 
        // since begin never resets
        if (!begin) continue;

        // Note: LLVM does not consider icmpeq or icmpne to be commutative
        if ( ((*i).first)->isCommutative() ) {
            // create a new graph which will be identical to "this" 
            // graph except the operator corresponding to (*i)->first 
            // will have its predecessors swapped
            Graph * g = new Graph ();
            (*g) = (*base);
            Label(g); // label all the nodes arbitrarily 

            // commutative, so swap the predecessors
            Node * temp = g->GraphNodes[(*i).first]->p1;
            g->GraphNodes[(*i).first]->p1 = g->GraphNodes[(*i).first]->p2;
            g->GraphNodes[(*i).first]->p2 = temp;
    
            // add new graph to the list
            list->push_back(g);
    
            // and add all its equivalent versions as well, starting 
            // from the next instruction
            Graph::GraphNodes_iterator temp_iterator = i;
            ++temp_iterator;
            if ( temp_iterator != base->GraphNodes.end() ) {
                AddEquivalentCopies(g, (*temp_iterator).first, list);
            }
        }
    }
}


// check if operations in two graphs are equal
bool PatternMap::checkEquality(Graph * g1, Graph * g2) {
    if ( (g1->size()==0) && (g2->size() == 0) ) return true;
    if ( g1->size() != g2->size() ) return false;
    if ( g1->getRoot()->op != g2->getRoot()->op ) return false;

    return checkEqualityThroughBFS(g1, g2);
}


bool PatternMap::checkEqualityThroughBFS(Graph * g1, Graph * g2) {

    // BFS data structures
    set <Node*> g1_visited;     // visited nodes in each graph 
    set <Node*> g2_visited;
    queue <Node*> g1_nodes;     // queues used during traversal
    queue <Node*> g2_nodes;

    // Initialize BFS
    g1_nodes.push(g1->getRoot()); // root of g1
    g2_nodes.push(g2->getRoot()); // root of g2
    g1_visited.insert(g1->getRoot()); 
    g2_visited.insert(g2->getRoot()); 

    // use these temp pointers as opposed to the graph's "current" 
    // pointer in order to not change the value of the current pointer, 
    // which may be in use by another function
    Node * t1; 
    Node * t2; 

    // Main BFS loop
    while (!g1_nodes.empty()) {
        // Pop next nodes from queue
        t1 = g1_nodes.front(); g1_nodes.pop();
        t2 = g2_nodes.front(); g2_nodes.pop();
          
        // Cache the predecessors of t1 and t2
        Node *t1_predecessors [2], *t2_predecessors [2];
        t1_predecessors[0] = t1->p1;  // left predecessors
        t2_predecessors[0] = t2->p1;
        t1_predecessors[1] = t1->p2;  // right predecessors
        t2_predecessors[1] = t2->p2;
          
        // Check equality of both the left and right predecessors
        for (int predecessor_number = 0; predecessor_number < 2; ++predecessor_number) {
               Node *g1_pred = t1_predecessors[predecessor_number];
               Node *g2_pred = t2_predecessors[predecessor_number];
            
            // Check for mismatching types of the predecessors
            if (node_types_mismatch(g1_pred, g2_pred)) {
                return false; // The two predecessors have mismatching types
            }

            // The two predecessors have matching types. If they are operations
            // (rather than constants / inputs), consider adding them to the queue,
            // as long as neither node has been visited before 
            if (g1_pred->is_op) {
            
                // This node has not yet been visited in g1
                if ( g1_visited.find(g1_pred) == g1_visited.end() ) {
                
                    // If it has been visited in g2, there is a mismatch
                    if ( g2_visited.find(g2_pred) != g2_visited.end() ) return false;

                    // At this point, the current predecessors in both graphs are:
                    //    - Both instructions
                    //    - The same instruction
                    //    - Both have not been visited
                    // However, lastly we need to consider registers. E.g. if t1 and g1_pred are
                    // in the same state, but t2 and g2_pred are not, then there is a register
                    // in the way and we can't share, so these graphs are NOT equal

                    // The condition below checks the following:
                    // If (both t1 and t2 are in the same states as their predecessors) OR
                    //    (neither t1 and t2 are in the same states as their predecessors)
                    if ( ( fsm->getEndState(t1->I) == fsm->getEndState(g1_pred->I) ) !=
                         ( fsm->getEndState(t2->I) == fsm->getEndState(g2_pred->I) ) ) 
                    { 
                        return false; 
                    }

                    g1_nodes.push(g1_pred);
                    g2_nodes.push(g2_pred);
                    g1_visited.insert(g1_pred);
                    g2_visited.insert(g2_pred);
                }
                // The node has been visited in g1, so if it hasn't been 
                // visited in g2, return mismatch
                else if ( g2_visited.find(g2_pred) == g2_visited.end() ) {
                    return false;
                }
            }
        }    
    }
    return true;
}

           
/*******************************************************************************
 * Destructor
 ******************************************************************************/  
           
// Delete every graph
// This required iterating over every PatternList, every GraphList in that
// PatternList, then every Graph in that GraphList (see PatternMap.h)
// The destructor of each Graph object then deletes every node in the graph
PatternMap::~PatternMap() {
    PatternMap_iterator i;
    PatternList_iterator j;
    Graph_iterator k;

    if (!Patterns.empty()) {
        // for each Pattern size (map, so i->first is the size 
        // and i->second points to a list of GraphLists)
        for (i = Patterns.begin(); i != Patterns.end(); ++i) { 
        
            // for each pair of GraphLists in this PatternList
            for (j = (i->second).begin(); j != (i->second).end(); ++j) { 
            
                // for each Graph in the 1st GL
                for (k = j->begin(); k != j->end(); ++k) { 
                    // Note that j is a pointer to an element in the 
                    // GraphList vector, which contains pointers to 
                    // graph objects. Hence *j is a pointer to a Graph object
                    delete (*k); 
                    (*k) = NULL;
                }
            }
        }

        // Same iterations as above
        for (i = EquivalentCopies.begin(); i != EquivalentCopies.end(); ++i) {
            for (j = (i->second).begin(); j != (i->second).end(); ++j) {
                for (k = j->begin(); k != j->end(); ++k) {
                    delete (*k);
                    (*k) = NULL;
                }
            }
        }
    }
}

/*******************************************************************************
 * Modifiers
 ******************************************************************************/  

// Add a new graph to our PatternMap
void PatternMap::Add( Graph*g ) {
    PatternList_iterator i;
    Graph_iterator j;

    // if the map is empty or does not contain this size, add it 
    if ( Patterns.find(g->size()) == Patterns.end() ) 
    { 
        Label(g); // label all the nodes arbitrarily
        Patterns[g->size()].push_back( GraphList() );
        EquivalentCopies[g->size()].push_back( GraphList() );
        Patterns[g->size()].back().push_back(g);

        Graph * g_copy = new Graph;
        *(g_copy) = *g;
        Label(g_copy); // label all the nodes arbitrarily
        EquivalentCopies[g->size()].back().push_back(g_copy);
        // and also add all equivalent copies
        AddEquivalentCopies( 
            g, 
            g->GraphNodes.begin()->first, 
            &( EquivalentCopies[g->size()].back() ) 
        );    
        return;
    }

    // otherwise the size exists. Check for the graph in every GraphList of  
    // every Pattern list for this size, and if it is found then the pattern 
    // exists so increment its frequency.

    // Note this is where the EquivalentCopies GraphList is used: we need to 
    // determine if the pattern of Graph g has been found before, but it could
    // be that it was found but in a different (equivalent) form
    
    // for each GraphList in the vector
    for (    PatternList_iterator i1 = Patterns[g->size()].begin(), 
            i2 = EquivalentCopies[g->size()].begin(); 
            i1 != Patterns[g->size()].end(); 
            ++i1, ++i2
        ) 
    {
        // for each graph in the GraphList
        for (Graph_iterator j = i2->begin(); j != i2->end(); ++j) { 
            if ( checkEquality(g, *j) ) {
                // We have a match, i.e. the 2 graphs are functionally the same, 
                // but not necessarily topologically. To ensure
                CopyLabels (*j, g); 
                i1->push_back(g);   
                // The labels are consistent, i.e. label "1" always corresponds 
                // to the same add
                // Copy the labels now then add the graph
                return;
            }
        }
    }

    // the pattern has not been found, so add it as above

    Label(g);    // label all the nodes arbitrarily 
    Patterns[g->size()].push_back( GraphList() );
    EquivalentCopies[g->size()].push_back( GraphList() );
    Patterns[g->size()].back().push_back(g);

    Graph * g_copy = new Graph;
    *(g_copy) = *g;
    Label(g_copy); // label all the nodes arbitrarily 
    EquivalentCopies[g->size()].back().push_back(g_copy);
    // and also add all equivalent copies
    AddEquivalentCopies( 
        g, 
        g->GraphNodes.begin()->first, 
        &( EquivalentCopies[g->size()].back() ) 
    );    
}

