//===-- SDCSolver.h -------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Constraint Class for LP solver
// these functions help manage the LP constraints by keeping track
// of the row number for each constraint
// the big problem with the constraint row number is it changes every
// time you del_constraint on the LP
// the LP formulation for constraints are indexed by row number
// the index starts at 1
// we *only* keep track of the SDC resource conflict resolution
// constraints (i.e. loads/stores) because these might need to be modified
// during backtracking
//
//===----------------------------------------------------------------------===//

#ifndef SDCSOLVER_H
#define SDCSOLVER_H

#include "SDCScheduler.h"
#include <lp_lib.h>

using namespace llvm;

namespace legup {

class SDCGraph {
  public:
    SDCGraph() : invalid(true), numEdges(0), numConstraints(0), file(NULL) {
        vertices.insert(0);
    }
    void setFile(raw_fd_ostream &File) { file = &File; }

    raw_fd_ostream &File() {
        assert(file);
        return *file;
    }

    void clear() {
        invalid = true;
        numEdges = 0;
        numConstraints = 0;
        edges.clear();
        edges_all.clear();
        vertices.clear();
        vertices.insert(0);
    }
    struct Node {
        int v;
        float dist;
    };

    struct myCompNode {
        bool operator()(const Node *a, const Node *b) const {
            return a->dist > b->dist;
        }
    };

    struct myComp {
        bool operator()(const float &a, const float &b) const { return a > b; }
    };

    std::map<int, float> minPath;
    bool invalid;
    int prevSource;

    int numEdges;
    int numConstraints;

    bool hasNegativeCycle;

    bool shortestPathSrc();
    bool shortestPathSrcUpdateFeasible(std::map<int, float> &FeasibleSoln);
    float shortestPathBellmanFord(int source, int sink);
    float shortestPathDijkstra(int source, int sink);
    void addEdge(int source, int sink, float c);
    void removeEdge(int source, int sink, float c);

    float length(int source, int sink) {
        assert(edges[source].find(sink) != edges[source].end() &&
               "Can't find edge");
        return edges[source][sink];
    }

    std::map<int, float> &getSuccessors(int x) { return edges[x]; }
    int getNumConstraints();
    void printConstraints(raw_fd_ostream &File);

  private:
    // u -> v (length)
    std::set<int> vertices;
    std::map<int, std::map<int, float>> edges;
    // minimum priority queue that holds all edge constraints added to
    // each edge -- this is for handling multiple constraints
    // between the same edges...
    std::map<int, std::map<int, std::vector<float>>> edges_all;

    raw_fd_ostream *file;
    /*
    // variables have a 1-to-1 mapping to vertices
    // and are indexed from 0
    class Node {
    public:
    Node(int index) : index(index) {}
    int index;
    };
    */
};

class SDCSolver {

  public:
    void clear();
    struct Constraint {
        Constraint() : u(0), v(0) {}
        int u;
        int v;
        float c;
    };

    struct Constraints {
        std::vector<Constraint *> list;
    };

    typedef std::list<Constraint *> ConstraintListTy;

    // since we don't track *all* the constraints, only those related
    // to the iterative resource resolution, the first constraint
    // row is at index initialRow (which will be greater than 0)
    int initialRow;

    ConstraintListTy constraintRows;
    ConstraintListTy constraintsAll;
    std::map<Instruction *, ConstraintListTy> instrConstraints;
    // ConstraintListTy GEConstraints;

    // get the row offset of an LP constraint in the LP problem
    int getRowNum(Constraint *C);

    static void printGraphNodeLabel(raw_ostream &out, int *i) { out << *i; }

    SDCGraph feasibleConstraintGraph;
    std::map<int, float> FeasibleSoln;
    std::list<Constraint *> unprocessed;
    std::list<Constraint *> processed;
    // how many variables were affected by the last constraint
    std::map<int, int> affected;
    // bool isFeasible;

    void deleteConstraints(Constraints *constraints);

    void print();

    float getD(std::map<int, float> &d, int i);
    void setD(std::map<int, float> &d, int i, float v);
    void printFeasibleSoln();
    void verifyFeasibleSoln();
    void removeFromFeasible(int v, int u, float c);
    void incrRemoveConstraint(int v, int u, REAL c);
    bool incrAddConstraint(int v, int u, REAL c);
    bool addToFeasible(int v, int u, REAL c);
    void addToFeasibleVerify(int v, int u, float c);
    void addConstraintSDCForm(lprec *lp, int v_index, int u_index, REAL rh,
                              Constraints *constraints, Instruction *I);
    void addConstraintIncremental(lprec *lp, int count, REAL *row, int *colno,
                                  int constr_type, REAL rh,
                                  Constraints *constraints = NULL,
                                  Instruction *I = NULL);
    SchedulerDAG *dag;
    bool SDCdebug;
    lprec *lp;
    raw_fd_ostream *file;

    raw_fd_ostream &File() { return *file; }

    // assuming you always call addConstraint right after creating a constraint
    // with addConstraintIncremental, so this constraint should be the last row
    // of the LP
    Constraint *addConstraint(Instruction *I);

    // search constraintRows for a constraint and return an iterator to it
    // returns constraintRows.end() if not found
    // O(n) search
    std::list<Constraint *>::iterator
    findConstraint(std::list<Constraint *> &rows, Constraint *C);

    // remove a constraint from the LP problem and constraintRows
    void deleteConstraint(Constraint *C);
    void deleteAllConstraints(ConstraintListTy &list);

    // delete all of the constraints for a given instruction
    // this is called when unscheduling an instruction from the SDC
    void deleteAllInstrConstraints(Instruction *I);
};

class FeasibleQueue {
  public:
    FeasibleQueue(raw_fd_ostream &File) : file(File) {}
    void print();
    void findAndDeleteMin(int *x, float *dist_x);
    float hashKey(int i);
    void insertHeap(int vertex, float key);
    void adjustHeap(int vertex, float key);
    bool empty() { return queue.empty(); }

  private:
    struct HeapNode {
        HeapNode(int v, float k) : vertex(v), key(k) {}
        int vertex;
        float key;
    };
    struct myComp {
        bool operator()(const HeapNode &a, const HeapNode &b) const {
            // return a.key < b.key;
            return a.key > b.key;
        }
    };
    std::vector<HeapNode> queue;
    std::map<int, float> v2key;

    raw_fd_ostream &file;
};

} // End legup namespace

#endif
