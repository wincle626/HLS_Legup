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

#include "ElementaryCycles.h"
#include <assert.h>
#include <stdio.h>

#include "LegupConfig.h"

using namespace llvm;
using namespace legup;

namespace legup {


// return TRUE if value is NOT in the list
bool FindElementaryCycles::notInList (std::vector<int> &list, int val) {
    //assert(list != null);
    assert((unsigned)list[0] < list.size());
    for (int i = 1; i <= list[0]; i++) {
        if (list[i] == val)
            return false;
    }
    return true;
}

// return TRUE if value is in the list
bool FindElementaryCycles::inList (std::vector<int> &list, int val) {
    //assert(list != null);
    assert((unsigned)list[0] < list.size());
    for (int i = 1; i <= list[0]; i++) {
        if (list[i] == val)
            return true;
    }
    return false;
}

// empties a list by simply zeroing its size
void FindElementaryCycles::emptyList (std::vector<int> &list) {
    //assert(list != null);
    assert((unsigned)list[0] < list.size());
    list[0] = 0;
}

// adds on to the end (making extra space if needed)
void FindElementaryCycles::addToList (std::vector<int> &list, int val) {
    //assert(list != null);
    assert((unsigned)list[0] < list.size());
    int newPos = list[0] + 1;
    list[newPos] = val;
    list[0] = newPos;
}

// removes all occurences of val in the list
int FindElementaryCycles::removeFromList (std::vector<int> &list, int val) {
    //assert(list != null);
    assert((unsigned)list[0] < list.size());
    int nOccurrences = 0;
    for (int i = 1; i <= list[0]; i++) {
        if (list[i] == val) {
            nOccurrences++;
            for (int j = i; j<list[0]; j++) {
                list[j] = list[j+1];
            }
            --list[0]; // should be safe as list[0] is
                       // re-evaluated each time around the i-loop
            --i;
        }
    }
    return nOccurrences;
}


int FindElementaryCycles::countAkArcs () { // return number of Arcs in graph
    int nArcs = 0;
    for (int i =0; i<nVertices; i ++) {
        nArcs += Ak[i][0]; // zero’th element gives nArcs for i
    }
    return nArcs;
}

void FindElementaryCycles::unblock (int u) {
    blocked [u] = false;
    for (int wPos = 1; wPos <= B[u][0]; wPos++) {
        // for each w in B[u]
        int w = B[u][wPos];
        wPos -= removeFromList(B[u], w);
        if (blocked[w])
            unblock(w);
    }
}

// initialise the stack to some size max
void FindElementaryCycles::stackInit(int max) {
    stack.resize(max);
    //assert(stack != null);
    stackTop = 0;
}

// push an int onto the stack, extending if necessary
void FindElementaryCycles::stackPush (int val) {
    if ((unsigned)stackTop >= stack.size())
        stack.resize(stack.size() + 1);
    stack[stackTop++] = val;
}

int FindElementaryCycles::stackSize() {
    return stackTop;
}

int FindElementaryCycles::stackPop () {
    // pop an int off the stack
    assert(stackTop > 0);
    return stack[--stackTop];
}

void FindElementaryCycles::stackClear () {
    // clear the stack
    stackTop = 0;
}


void FindElementaryCycles::setupGlobals() {  // presupposes nVertices is set up
    //nVertices = std.conv.parse!int(args[1]);


    Ak.resize(nVertices); // Ak[i][0] is the number of members, Ak[i][1]..Ak[i][n] ARE the members, i>0
    B.resize(nVertices);  // B[i][0] is the number of members, B[i][1]..B[i][n] ARE the members , i>0
    blocked.resize(nVertices); // we use blocked [0]..blocked[n-1], i> = 0

    for (int i = 0; i < nVertices; i++) {
        Ak[i].resize(nVertices);
        B[i].resize(nVertices);
        blocked[i] = false;
    }

    /*
    char[] buf;
    while (stdin.readln(buf)) {
        string[] vertices = std.array.split(std.conv.to!string(buf), " ");
        int v1 = std.conv.parse!int(vertices[0]);
        int v2 = std.conv.parse!int(vertices[1]);
        addToList(Ak[v1], v2);
    }
    */


    lengthHistogram.resize(nVertices+1); // will use as [1]...[n] to histogram circuits by length
                                          // [0] for zero length circuits, which are impossible
    for (unsigned len = 0; len < lengthHistogram.size(); len++) // initialise histogram bins to empty
        lengthHistogram[len] = 0;
    stackInit(nVertices);
    vertexPopularity.resize(nVertices+1); // max elementary circuit length is exactly nVertices
    for (int len = 0; len <= nVertices; len++) {
        vertexPopularity[len].resize(nVertices);
        for (int j = 0; j < nVertices; j++) {
            vertexPopularity[len][j] = 0;
        }
    }
}


/*
 * to replicate the result from figure 10 in the paper, run the program as
 * follows:
 *
* echo "0 2\n0 10\n0 14\n1 5\n1 8\n2 7\n2 9\n3 3\n3 4\n3 6\n4 5\n4 13\n\
* 4 15\n6 13\n8 0\n8 4\n8 8\n9 9\n10 7\n10 11\n11 6\n12 1\n12 1\n12 2\n12 10\n12 12\n\
* 12 14\n13 3\n13 12\n13 15\n14 11\n15 0" | ./circuits_hawick 16
 */
/*
int main() {
    //if (args.length != 2) {
    //    std.stdio.writefln("usage: echo \"v1 v2\nv1 v3\n...\" | %s num_vertices", args[0]);
    //    return 1;
    //}
    setupGlobals();
    stackClear();
    start = 0;
    bool verbose = false;
    while (start < nVertices) {
        if (verbose && enumeration) printf("Starting s = %d\n", start);
        for (int i = 0; i < nVertices; i++) { // for all i in Vk
            blocked[i] = false;
            emptyList(B[i]);
        }
        circuit(start);
        start = start + 1;
    }
    return 0;
}
    */
//}

//namespace legup {

void FindElementaryCycles::solve() {
    nVertices = this->numVertices;
    setupGlobals();

    // add edges
    for (std::map<Instruction*, std::list<Instruction*> >::iterator i =
            edges.begin(), e = edges.end(); i != e; ++i) {
        Instruction *source = i->first;
        std::list< Instruction* > &path = i->second;
        for (std::list< Instruction* >::iterator pi = path.begin(), pe =
                path.end(); pi != pe; ++pi) {
            Instruction *sink = *pi;
            assert (v2index.find(source) != v2index.end());
            assert (v2index.find(sink) != v2index.end());
            addToList(Ak[v2index[source]], v2index[sink]);
            //errs() << "Connecting source -> sink\n";
            //errs() << "Connecting " << *source << " -> " << *sink << "\n";
            //errs() << "dot " << v2index[source] << " -> " << v2index[sink] << "\n";
        }
    }

    stackClear();
    start = 0;
    while (start < nVertices) {
        for (int i = 0; i < nVertices; i++) { // for all i in Vk
            blocked[i] = false;
            emptyList(B[i]);
        }
        circuit(start);
        start = start + 1;
    }
}

void FindElementaryCycles::stackPrint3d() {
    int i;
    std::list<Instruction*> newElementaryCycle;
    for (i = 0; i < stackTop; i++) {
        //printf("%d ", stack[i]);
        assert (index2v.find(stack[i]) != index2v.end());
        newElementaryCycle.push_back(index2v[stack[i]]);
    }
    //printf("\n");
    this->cycles.push_back(newElementaryCycle);
}

bool FindElementaryCycles::circuit(int v) { // based on Johnson ’s logical procedure CIRCUIT
    bool f = false;
    stackPush(v);
    blocked[v] = true;
    for (int wPos = 1; wPos <= Ak[v][0]; wPos++) { // for each w in list Ak[v]:
        int w = Ak[v][wPos];
        if (w < start) continue; // ignore relevant parts of Ak
        if (w == start) { // we have a circuit,
            if (enumeration) {
                stackPrint3d(); // print out the stack to record the circuit
            }
            assert (stackTop <= nVertices);
            ++lengthHistogram[stackTop]; // add this circuit ’s length to the length histogram
            nCircuits++; // and increment count of circuits found
            if (stackTop > lenLongest) { // keep a copy of the longest circuit found
                lenLongest = stackTop;
                longestCircuit = stack;
            }
            for (int i = 0; i < stackTop; i ++) // increment [circuit-length][vertex] for all vertices in this circuit
                 ++vertexPopularity[stackTop][stack[i]];
            f = true;
        } else if (!blocked[w]) {
            if (circuit(w)) f = true;
        }
    }
    if (f) {
        unblock (v);
    } else {
        for (int wPos = 1; wPos <= Ak[v][0]; wPos++) { // for each w in list Ak[v]:
            int w = Ak[v][wPos];
            if (w < start) continue;  // ignore relevant parts of Ak
            if (notInList(B[w], v)) addToList(B[w], v);
        }
    }
    v = stackPop();
    return f;
}

}
