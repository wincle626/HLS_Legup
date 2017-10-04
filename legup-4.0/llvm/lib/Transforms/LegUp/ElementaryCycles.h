//===------ ElementaryCycles.cpp ------------------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Enumerating Circuits and Loops in Graphs with Self-Arcs and Multiple-Arcs
// K.A. Hawick and H.A. James
// Computer Science, Institute for Information and Mathematical Sciences,
// Massey University, North Shore 102-904, Auckland, New Zealand
// k.a.hawick@massey.ac.nz; heath.james@sapac.edu.au
// Tel: +64 9 414 0800
// Fax: +64 9 441 8181
// Technical Report CSTN-013
//
// Original D Implementation from:
//      https://github.com/josch/cycles_hawick_james
// Converted to C++ by Andrew Canis
//
//===----------------------------------------------------------------------===//

#ifndef ELEMENTARYCYCLES_H
#define ELEMENTARYCYCLES_H

#include "llvm/IR/Instructions.h"
#include <vector>
#include <list>
#include <map>

using namespace llvm;

namespace legup {

// this has to be declared in the header due to templates
//template <typename T>
class FindElementaryCycles
{
public:
    FindElementaryCycles() :
        numVertices(0),
        nVertices(0),
        start(0),
        nCircuits(0),
        lenLongest(0),
        enumeration(true),
        stackTop(0)
        {
        }



    void addEdge(Instruction* source, Instruction* sink) {
        if (v2index.find(source) == v2index.end()) {
            v2index[source] = numVertices;
            index2v[numVertices] = source;
            numVertices++;
        }
        if (v2index.find(sink) == v2index.end()) {
            v2index[sink] = numVertices;
            index2v[numVertices] = sink;
            numVertices++;
        }
        edges[source].push_back(sink);
    }

    void clear() {
        numVertices = 0;
        v2index.clear();
        index2v.clear();
        edges.clear();
        cycles.clear();

        nVertices = 0;
        start = 0;
        nCircuits = 0;
        lenLongest = 0;
        enumeration = true;
        stackTop = 0;

        Ak.clear();
        B.clear();
        blocked.clear();
        lengthHistogram.clear();
        vertexPopularity.clear();
        longestCircuit.clear();
        stack.clear();
    }

    //std::list< std::list< Instruction* > > & solve();
    void solve();


    typedef std::list< std::list< Instruction* > > CycleListTy;
    CycleListTy cycles;

private:
    void stackPrint3d();
    bool circuit(int v); // based on Johnson ’s logical procedure CIRCUIT
    void addToList (std::vector<int> &list, int val);

    int numVertices;
    std::map<Instruction*, int> v2index;
    std::map<int, Instruction*> index2v;
    std::map<Instruction*, std::list<Instruction*> > edges;

    int nVertices;           // number of vertices
    int start;               // starting vertex index
    std::vector< std::vector<int> > Ak; // integer array size n of lists
                                 // ie the arcs from the vertex
    std::vector< std::vector<int> > B; // integer array size n of lists
    std::vector< bool > blocked; // logical array indexed by vertex
    int nCircuits;         // total number of circuits found;
    std::vector< int > lengthHistogram;    // histogram of circuit lengths
    std::vector< std::vector<int> > vertexPopularity; // adjacency table of occurrences of
                                 // vertices in circuits of each length
    std::vector< int > longestCircuit;    // histogram of circuit lengths
    int lenLongest;          // its length
    bool enumeration;     // explicitly enumerate circuits
    std::vector< int > stack; // stack of integers
    int stackTop;     // the number of elements on the stack
                                 // also the index "to put the next one"

    bool notInList (std::vector<int> &list, int val);
    bool inList (std::vector<int> &list, int val);
    void emptyList (std::vector<int> &list);
    int removeFromList (std::vector<int> &list, int val);
    int countAkArcs ();
    void unblock (int u);
    void stackInit(int max);
    void stackPush (int val);
    int stackSize();
    int stackPop ();
    void stackClear ();
    void setupGlobals();

};

} // end of legup namespace

#endif
