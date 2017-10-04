//===------ SDCSolver.cpp ---------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// See header for details
//
//===----------------------------------------------------------------------===//

#include "SDCSolver.h"

using namespace legup;

int SDCSolver::getRowNum(Constraint *C) {
    int rowCount = initialRow;
    for (std::list<Constraint *>::iterator i = constraintRows.begin(),
                                           e = constraintRows.end();
         i != e; ++i) {
        Constraint *cur = *i;
        if (cur == C)
            return rowCount;
        rowCount++;
    }
    assert(rowCount - 1 == get_Nrows(lp));
    assert(0 && "Couldn't find the constraint!");
}

bool SDCGraph::shortestPathSrc() {
    /*
    // src node is 0
    int v = 0;
    for (std::set<int>::iterator i = vertices.begin(), e =
            vertices.end(); i != e; ++i) {
        int x = *i;
        assert(x != 0);
        v = x;
    }
    */
    // empty graph
    // if (!v) return true;
    // start from source to all nodes
    // shortestPath updates minPath
    shortestPathBellmanFord(0, 0);
    if (hasNegativeCycle)
        return false;

    /*
    for (std::set<int>::iterator i = vertices.begin(), e =
            vertices.end(); i != e; ++i) {
        int x = *i;
        if (x == 0) continue;
        removeEdge(0, x, 0.0);
    }
    vertices.erase(0);
    */
    return true;
}

bool
SDCGraph::shortestPathSrcUpdateFeasible(std::map<int, float> &FeasibleSoln) {
    bool feasible = shortestPathSrc();
    if (!feasible) {
        return false;
    }
    for (std::set<int>::iterator i = vertices.begin(), e = vertices.end();
         i != e; ++i) {
        int x = *i;
        assert(x != 0);
        FeasibleSoln[x] = minPath[x];
        // errs() << "D(" << x << ") = " << FeasibleSoln[x] << "\n";
        assert(FeasibleSoln[x] < 1e10);
    }
    return true;
}

float SDCGraph::shortestPathBellmanFord(int source, int sink) {
    File() << "Starting BellmanFord : " << vertices.size() << " x " << numEdges
           << "\n";

    if (!invalid && source == prevSource) {
        return minPath[sink];
    }

    if (vertices.find(sink) == vertices.end())
        return 1e20;
    std::set<int> seen;
    std::vector<Node *> toVisit;
    std::map<int, Node *> nodes;

    minPath.clear();
    hasNegativeCycle = false;

    for (std::set<int>::iterator i = vertices.begin(), e = vertices.end();
         i != e; ++i) {
        int x = *i;

        if (x == source) {
            minPath[x] = 0;
        } else {
            minPath[x] = 1e20;
        }
    }

    // edge relaxation
    for (unsigned n = 0; n < vertices.size(); n++) {
        for (std::set<int>::iterator i = vertices.begin(), e = vertices.end();
             i != e; ++i) {
            int x = *i;

            std::map<int, float> &successors = getSuccessors(x);

            for (std::map<int, float>::iterator si = successors.begin(),
                                                ei = successors.end();
                 si != ei; ++si) {
                int y = si->first;
                float w = si->second;

                // errs() << "edge " << x << " -> " << y << " = " << w << "\n";
                if (minPath[x] + w < minPath[y]) {
                    minPath[y] = minPath[x] + w;
                }
            }
        }
    }

    /*
    for (std::map<int, float>::iterator si = minPath.begin(),
            ei = minPath.end(); si != ei; ++si) {
        int y = si->first;
        float length = si->second;
        errs() << "minPath(" << source << ", " << y << ") = " << length << "\n";
    }
    */

    invalid = false;
    prevSource = source;

    // check for negative-weight cycles
    for (std::set<int>::iterator i = vertices.begin(), e = vertices.end();
         i != e; ++i) {
        int x = *i;

        std::map<int, float> &successors = getSuccessors(x);

        for (std::map<int, float>::iterator si = successors.begin(),
                                            ei = successors.end();
             si != ei; ++si) {
            int y = si->first;
            float w = si->second;

            if (minPath[x] + w < minPath[y]) {
                File() << "SDC Graph contains a negative-weight cycle";
                // assert(0);
                hasNegativeCycle = true;
                invalid = true;
                return -1e20;
            }
        }
    }
    float shortest = minPath[sink];

    File() << "Ending\n";

    // assert(shortest == shortestPathDijkstra(source, sink));
    return shortest;
}

float SDCGraph::shortestPathDijkstra(int source, int sink) {
    if (vertices.find(sink) == vertices.end())
        return 1e20;
    std::set<int> seen;
    std::vector<Node *> toVisit;
    std::map<int, Node *> nodes;
    minPath.clear();

    Node *sourceN = new Node;
    sourceN->v = source;
    sourceN->dist = 0;
    minPath[source] = 0;
    toVisit.push_back(sourceN);
    std::push_heap(toVisit.begin(), toVisit.end(), myCompNode());
    nodes[source] = sourceN;

    for (std::set<int>::iterator i = vertices.begin(), e = vertices.end();
         i != e; ++i) {
        int x = *i;
        if (x != source) {
            Node *v = new Node;
            v->v = x;
            v->dist = 1e20;
            minPath[x] = 1e20;
            toVisit.push_back(v);
            std::push_heap(toVisit.begin(), toVisit.end(), myCompNode());
            nodes[x] = v;
        }
    }

    while (!toVisit.empty()) {
        Node *minNode = toVisit.front();

        std::pop_heap(toVisit.begin(), toVisit.end(), myCompNode());
        toVisit.pop_back();

        // get all paths
        if (minNode->dist == 1e20)
            continue;

        // only visit nodes once
        // if (seen.find(x) != send.end()) continue;

        // go thru successors
        std::map<int, float> &successors = getSuccessors(minNode->v);

        for (std::map<int, float>::iterator si = successors.begin(),
                                            ei = successors.end();
             si != ei; ++si) {
            int y = si->first;
            float length = si->second;

            Node *succ = nodes[y];
            assert(succ);

            // check v has not yet been removed from toVisitQ
            bool foundInQ = false;
            for (std::vector<Node *>::iterator vi = toVisit.begin(),
                                               ve = toVisit.end();
                 vi != ve; ++vi) {
                Node *inQ = *vi;
                if (inQ == succ) {
                    foundInQ = true;
                    break;
                }
            }
            if (!foundInQ)
                continue;
            /*
            */

            assert(length > 0);
            float newPath = minNode->dist + length;

            if (newPath < succ->dist) {
                succ->dist = newPath;
                minPath[succ->v] = newPath;
                std::make_heap(toVisit.begin(), toVisit.end(), myCompNode());
            }
        }
    }
    Node *sinkN = nodes[sink];
    assert(sinkN);
    return sinkN->dist;
}

void SDCGraph::addEdge(int source, int sink, float c) {
    assert(sink != 0);
    invalid = true;
    if (source != 0)
        numConstraints++;
    numEdges++;
    // if (edges[source].find(sink) != edges[source].end()) {
    // adding a constraint twice, take the minimum
    // edges[source][sink] = std::min(c, edges[source][sink]);

    //}
    // edges_all[source][sink].push_back(c);

    edges_all[source][sink].push_back(c);
    std::push_heap(edges_all[source][sink].begin(),
                   edges_all[source][sink].end(), myComp());

    // grab the minimum edge
    float minC = edges_all[source][sink].front();
    // if (c != minC) {
    //    errs() << "edge weight: " << minC << "\n";
    //}

    // errs() << "add edge " << source << " -> " << sink << " w: " << minC <<
    // "\n";

    // for (unsigned i = 0; i < edges_all[source][sink].size(); i++) {
    //    assert (edges_all[source][sink].at(i) >= minC);
    //}

    assert(!edges_all[source][sink].empty());
    edges[source][sink] = minC;

    if (vertices.find(source) == vertices.end()) {
        assert(source != 0);
        vertices.insert(source);
        assert(vertices.find(source) != vertices.end());
        addEdge(0, source, 0.0);
    }
    if (vertices.find(sink) == vertices.end()) {
        vertices.insert(sink);
        addEdge(0, sink, 0.0);
    }
}

void SDCGraph::removeEdge(int source, int sink, float c) {
    numEdges--;
    assert(source != 0);
    assert(sink != 0);
    numConstraints--;
    invalid = true;

    // if (edges[source].find(sink) != edges[source].end()) {
    assert(edges[source].find(sink) != edges[source].end());

    assert(edges_all[source].find(sink) != edges_all[source].end());

    // delete the constraint
    for (unsigned i = 0; i < edges_all[source][sink].size(); i++) {
        if (edges_all[source][sink].at(i) == c) {
            edges_all[source][sink].erase(edges_all[source][sink].begin() + i);
            break;
        }
    }

    // remake the heap
    std::make_heap(edges_all[source][sink].begin(),
                   edges_all[source][sink].end(), myComp());

    edges[source].erase(sink);

    if (!edges_all[source][sink].empty()) {
        // grab the new minimum edge
        float minC = edges_all[source][sink].front();

        for (unsigned i = 0; i < edges_all[source][sink].size(); i++) {
            assert(edges_all[source][sink].at(i) >= minC);
        }

        edges[source][sink] = minC;
    }

    // assert(edges[source].find(sink) != edges[source].end() && "Can't find
    // edge");
    // assert(c == edges[source][sink]);
    // assert(c == edges[source][sink]);
}

int SDCGraph::getNumConstraints() {
    return numConstraints;
    int num = 0;
    for (std::set<int>::iterator i = vertices.begin(), e = vertices.end();
         i != e; ++i) {
        int x = *i;

        std::map<int, float> &successors = getSuccessors(x);

        for (std::map<int, float>::iterator si = successors.begin(),
                                            ei = successors.end();
             si != ei; ++si) {
            int y = si->first;
            // float w = si->second;
            if (x != 0 && y != 0) {
                // if (edges_all[x][y].size() > 1) {
                //    errs() << "Extra constraints\n";
                //}
                num += edges_all[x][y].size();
                assert(!edges_all[x][y].empty());
                // errs() << "num = " << num << " s(" << y << ") - s(" <<
                //    x << ") <= " << w << "\n";
            }
        }
    }
    assert(num == numConstraints);
    return num;
}

void SDCGraph::printConstraints(raw_fd_ostream &File) {
    for (std::set<int>::iterator i = vertices.begin(), e = vertices.end();
         i != e; ++i) {
        int x = *i;

        std::map<int, float> &successors = getSuccessors(x);

        for (std::map<int, float>::iterator si = successors.begin(),
                                            ei = successors.end();
             si != ei; ++si) {
            int y = si->first;
            float w = si->second;
            if (x != 0 && y != 0) {
                File << "s(" << y << ") - s(" << x << ") <= " << w << "\n";
            }
        }
    }
}

/*
void printDFGFile(const char* fileName) {
    { // Need to go out of scope to write to file properly
        std::string FileError;
        // print out the dependence graph
        raw_fd_ostream outFile(fileName, FileError);
        assert(FileError.empty() && "Error opening log files");
        formatted_raw_ostream out(outFile);
        printDot(out);
    }
}


void printDot(formatted_raw_ostream &out) {

    dotGraph<int> graph(out, printGraphNodeLabel);
    graph.setLabelLimit(40);

    std::map<int, int*> ptr;

    for (std::map<int, std::map<int, int> >::iterator i = edges.begin(),
            e = edges.end(); i != e; ++i) {
        int vertex = i->first;
        std::map<int, int> &successors = i->second;
        for (std::map<int, int>::iterator si = successors.begin(),
                ei = successors.end(); si != ei; ++si) {
            int succVertex = si->first;
            float length = si->second;

            std::stringstream label;
            label << "label=\"" << length << "\"";
            label.flush();
            // dotGraph uses pointer addresses to uniquely print off
            // node names, so use new int here:
            graph.connectDot(out, new int(vertex), new
                    int(succVertex), label.str());
        }
    }
}

// variables have a 1-to-1 mapping to vertices
// and are indexed from 0
struct GraphNode {
        GraphNode(int index) : index(index) {}
        int index;
};


static void printGraphNodeLabel(raw_ostream &out, SDCGraph::GraphNode *n) {
    out << n->index;
}

void printDot(formatted_raw_ostream &out) {

    dotGraph<GraphNode> graph(out, SDCGraph::printGraphNodeLabel);
    graph.setLabelLimit(40);

    for (std::map<int, std::map<int, int> >::iterator i = edges.begin(),
            e = edges.end(); i != e; ++i) {
        int vertex = i->first;
        std::map<int, int> &successors = i->second;
        for (std::map<int, int>::iterator si = successors.begin(),
                ei = successors.end(); si != ei; ++si) {
            int succVertex = si->first;
            float length = si->second;

            std::stringstream label;
            label << "label=\"" << length << "\"";
            label.flush();
            graph.connectDot(out, new GraphNode(vertex), new
                    GraphNode(succVertex), label.str());
        }
    }
}
*/

void SDCSolver::print() {

    bool feasible = feasibleConstraintGraph.shortestPathSrc();
    File() << "new system feasible? " << feasible << "\n";
    assert(feasible &&
           "feasibleConstraintGraph should always be feasible by definition");

    File() << "SDC constraints:\n";
    for (ConstraintListTy::iterator i = constraintsAll.begin(),
                                    e = constraintsAll.end();
         i != e; ++i) {
        Constraint *C = *i;
        File() << "s(" << C->v << ") - s(" << C->u << ") <= " << C->c << "\n";
    }

    File() << "SDC graph constraints:\n";
    feasibleConstraintGraph.printConstraints(File());

    File() << "feasibleConstraintGraph.getNumConstraints() = "
           << feasibleConstraintGraph.getNumConstraints() << "\n";
    File() << "processed.size()= " << processed.size() << "\n";
    File() << "unprocessed.size()= " << unprocessed.size() << "\n";
    File() << "constraintsAll.size()= " << constraintsAll.size() << "\n";

    assert(feasibleConstraintGraph.getNumConstraints() ==
           (int)processed.size());
    assert(feasibleConstraintGraph.getNumConstraints() +
               (int)unprocessed.size() ==
           (int)constraintsAll.size());
}

void FeasibleQueue::print() {
    file << "Queue size: " << queue.size() << " contains:\n";
    unsigned i;
    for (i = 0; i < queue.size(); i++) {
        file << "\tkey (dist): " << queue[i].key
             << " -> vertex: " << queue[i].vertex << "\n";
    }
}

// returns the item in queue that has the minimum key as well as its key and
// deletes the item from the heap
void FeasibleQueue::findAndDeleteMin(int *x, float *dist_x) {
    assert(!queue.empty());
    HeapNode node = queue.front();

    // print();
    // file << "Min: " << node.key << ":" << node.vertex << "\n";

    *x = node.vertex;
    *dist_x = node.key;

    std::pop_heap(queue.begin(), queue.end(), myComp());
    queue.pop_back();
    assert(v2key.find(node.vertex) != v2key.end());
    v2key.erase(node.vertex);
}

// returns the key of vertex i
float FeasibleQueue::hashKey(int i) {
    if (v2key.find(i) != v2key.end()) {
        return v2key[i];
    } else {
        return 1e20;
    }
}
// inserts an vertex into queue with priority key
void FeasibleQueue::insertHeap(int vertex, float key) {
    assert(v2key.find(vertex) == v2key.end());
    // file << "Inserting into heap key: " << key << " vertex: " << vertex <<
    //    "\n";
    queue.push_back(HeapNode(vertex, key));
    std::push_heap(queue.begin(), queue.end(), myComp());
    v2key[vertex] = key;
}
// inserts an item i into heap with key k if i is not in heap
// and decreases the key of item i in heap to k if i is in the heap
void FeasibleQueue::adjustHeap(int vertex, float key) {
    // file << "Adjusting heap vertex: " << vertex << " length: " << key <<
    //    "\n";
    if (v2key.find(vertex) != v2key.end()) {
        // vertex exists in heap

        bool found = false;
        float newkey = -1e20;
        for (unsigned i = 0; i < queue.size(); i++) {
            if (queue.at(i).vertex == vertex) {
                assert(!found);
                found = true;
                assert(queue.at(i).key > key);
                assert(v2key.find(vertex) != v2key.end());
                v2key[vertex] = key;

                queue.at(i).key = key;
                newkey = key;
            }
        }
        assert(found);

        for (std::vector<HeapNode>::iterator i = queue.begin(),
                                             e = queue.begin();
             i != e; ++i) {
            HeapNode &node = *i;
            if (node.vertex == vertex) {
                assert(node.key == newkey);
            }
        }

        // remake the heap
        std::make_heap(queue.begin(), queue.end(), myComp());
    } else {
        // not found: insert into heap
        insertHeap(vertex, key);
    }
}

float SDCSolver::getD(std::map<int, float> &d, int i) {
    // remember, when adding new variable to the SDC (with
    // no constraint) the system is still feasible.  Just
    // initialize the variable to zero in the feasible
    // solution
    assert(d.find(i) != d.end());
    return d[i];
}
void SDCSolver::setD(std::map<int, float> &d, int i, float v) { d[i] = v; }

void SDCSolver::printFeasibleSoln() {
    return;
    File() << "Feasible solution:\n";
    File() << FeasibleSoln.size() << "\n";
    for (std::map<int, float>::iterator i = FeasibleSoln.begin(),
                                        e = FeasibleSoln.end();
         i != e; ++i) {
        int v = i->first;
        float d = i->second;
        // assert(d >= 0);
        File() << "D(" << v << ") = " << d << "\n";
    }
}

void SDCSolver::verifyFeasibleSoln() {
    if (!LEGUP_CONFIG->getParameterInt("DEBUG_VERIFY_INCR_SDC")) {
        return;
    }

    // this is slow...
    for (std::list<Constraint *>::iterator i = processed.begin(),
                                           e = processed.end();
         i != e; ++i) {
        Constraint *C = *i;
        // v - u <= c
        float Dv = getD(FeasibleSoln, C->v);
        float Du = getD(FeasibleSoln, C->u);
        float c = C->c;

        // verify constraint
        if (Dv - Du <= c)
            continue;

        errs() << "var(" << C->v << ") - "
               << "var(" << C->u << ") <= " << ftostr(C->c) << "\n";
        errs() << "v - u <= c not satisified\n";
        errs() << "v: " << Dv << " u: " << Du << " c: " << c << "\n";
        assert(0 && "Feasible soln is not actually feasible!\n");
    }

    // bool feasible = feasibleConstraintGraph.shortestPathSrc();
    // File() << "system feasible? " << feasible << "\n";
    // assert(feasible);
}

// note: the system always remains feasible after removing a constraint
void SDCSolver::removeFromFeasible(int v, int u, float c) {

    feasibleConstraintGraph.removeEdge(u, v, c);

    // sanity check
    // bool feasible = feasibleConstraintGraph.shortestPathSrc();
    // File() << "new system feasible? " << feasible << "\n";
    // assert(feasible &&
    //      "feasibleConstraintGraph should always be feasible by definition");

    // File() << "Removing v: " << v << " to u: " << u << " c: " << c << "\n";
    bool found = false;
    for (std::list<Constraint *>::iterator i = processed.begin(),
                                           e = processed.end();
         i != e; ++i) {
        Constraint *C = *i;
        // File() << "Constraint v: " << C->v << " to u: " << C->u << " c: " <<
        // C->c << "\n";
        if (C->u == u && C->v == v && C->c == c) {
            // remove from processed
            // File() << "REMOVING from processed\n";
            processed.erase(i);
            found = true;
            break;
        }
    }
    assert(found);
}

void SDCSolver::clear() {
    initialRow = 0;
    constraintRows.clear();
    constraintsAll.clear();
    instrConstraints.clear();

    // incremental SDC
    feasibleConstraintGraph.clear();
    FeasibleSoln.clear();
    unprocessed.clear();
    processed.clear();
    affected.clear();
    // isFeasible = true;
    // dummy node in SDC formulation
    setD(FeasibleSoln, 1, 0);

    // GEConstraints.clear();
}

void SDCSolver::incrRemoveConstraint(int v, int u, REAL c) {
    if (!LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC"))
        return;
    assert(u);
    assert(v);

    File() << "incrRemoveConstraint: "
           << " v: " << v << " u: " << u << " c: " << c << "\n";
    bool found = false;
    for (std::list<Constraint *>::iterator i = unprocessed.begin(),
                                           e = unprocessed.end();
         i != e; ++i) {
        Constraint *C = *i;
        // File() << "Unprocessed Constraint v: " << C->v << " to u: " << C->u
        // << " c: " << C->c << "\n";
        if (C->u == u && C->v == v && C->c == c) {
            // remove from unprocessed
            // File() << "REMOVING from processed\n";
            unprocessed.erase(i);
            found = true;
            break;
        }
    }
    if (!found) {
        removeFromFeasible(v, u, c);

        // see if the any of the unprocessed constraints work now
        for (std::list<Constraint *>::iterator i = unprocessed.begin(),
                                               e = unprocessed.end();
             i != e; ++i) {
            Constraint *C = *i;
            if (addToFeasible(C->v, C->u, C->c)) {
                i = unprocessed.erase(i);
            } else {
                // infeasible
                break;
            }
        }
    }

    assert(feasibleConstraintGraph.getNumConstraints() ==
           (int)processed.size());
    assert(feasibleConstraintGraph.getNumConstraints() +
               (int)unprocessed.size() ==
           (int)constraintsAll.size());
}

bool SDCSolver::incrAddConstraint(int v, int u, REAL c) {
    if (!LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC"))
        return false;

    // File() << "incrAddConstraint: " << " v: " << v << " u: "
    //       << u << " c: " << c << "\n";

    Constraint *C = new Constraint;
    C->v = v;
    C->u = u;
    C->c = c;

    if (unprocessed.empty()) {
        // system is currently feasible
        if (addToFeasible(v, u, c)) {
            verifyFeasibleSoln();
            // File() << "New constraint feasible\n";
            return true;
        } else {
            // File() << "New constraint causes infeasability\n";
        }
    } else {
        // File() << "System is currently infeasible\n";
    }
    unprocessed.push_back(C);
    return false;
}

// add a new constraint:
//    v - u <= c
// to the SDC system of difference constraints
// return true if the the system is still feasible and false if the system is
// now infeasible
bool SDCSolver::addToFeasible(int v, int u, REAL c) {
    // return true;
    // SDC constraint: v - u <= c

    // bool origFeasible = feasibleConstraintGraph.shortestPathSrc();
    // assert(origFeasible);

    // origFeasible = feasibleConstraintGraph.shortestPathSrc();
    // assert(origFeasible);

    /*
    */

    feasibleConstraintGraph.setFile(File());
    feasibleConstraintGraph.addEdge(u, v, c);

    // File() << "Adding edge u: " << u << " to v: " << v << " c: " << c <<
    // "\n";
    formatted_raw_ostream out(*file);
    // feasibleConstraintGraph.printDot(out);

    if (FeasibleSoln.find(u) == FeasibleSoln.end()) {
        FeasibleSoln[u] = 0;
    }
    if (FeasibleSoln.find(v) == FeasibleSoln.end()) {
        FeasibleSoln[v] = 0;
    }

    std::map<int, float> newD;

    // File() << FeasibleSoln.size() << "\n";
    // File() << newD.size() << "\n";
    // printFeasibleSoln();

    // sanity checks:
    verifyFeasibleSoln();
    float dummyVal = getD(FeasibleSoln, 1);
    assert(dummyVal == 0);

    affected.clear();
    FeasibleQueue queue(File());
    queue.insertHeap(v, 0);
    bool negCycle = false;

    float ScalingFactor1 = getD(FeasibleSoln, u) +
                           feasibleConstraintGraph.length(u, v) +
                           -getD(FeasibleSoln, v);

    // int findAndDeleteMinOps = 0;
    while (!queue.empty()) {
        int x;
        float dist_x;
        // queue.print();
        queue.findAndDeleteMin(&x, &dist_x);
        // findAndDeleteMin
        // float new_dist = getD(FeasibleSoln, u) +
        // feasibleConstraintGraph.length(u, v) +
        //    (getD(FeasibleSoln, x) + dist_x - getD(FeasibleSoln, v));

        float new_dist = dist_x + getD(FeasibleSoln, x) + ScalingFactor1;

        float d_x;
        if (newD.find(x) != newD.end()) {
            d_x = getD(newD, x);
        } else {
            d_x = getD(FeasibleSoln, x);
        }

        // File() << "dist_x: " << dist_x << "\n";
        // File() << "new_dist: " << new_dist << "\n";
        // if (new_dist < getD(newD, x)) {
        if (new_dist < d_x) {
            if (x == u) { // || x == 1) {
                // if (x == 1) {
                //    File() << "Dummy node modified to: " << new_dist << "\n";
                //}

                // if x == 1 then we are changing the dummy node, which should
                // always stay equal to 0
                File() << "Infeasible system: reject new constraint\n";

                feasibleConstraintGraph.removeEdge(u, v, c);

                verifyFeasibleSoln();

                // float distGvu =
                // feasibleConstraintGraph.shortestPathBellmanFord(v, u);
                // assert(distGvu != -1e20);
                // bool infeasible = (distGvu + c < 0.0);

                // assert(infeasible);

                // File() << "Confirmed Infeasible system with Bellman ford\n";

                // File() << "v = " << v << "\n";
                // File() << "u = " << u << "\n";
                // File() << "distGvu = " << distGvu << "\n";
                // File() << "c = " << c << "\n";

                return false;
            } else {
                // should only have one iteration for each affected
                // vertex
                // assert(affected.find(x) == affected.end());
                if (affected.find(x) != affected.end()) {

                    errs() << "Warning: Incremental algorithm breakdown, "
                              "variable: " << x
                           << " affected twice: old_dist: " << getD(newD, x)
                           << " new_dist: " << new_dist << "\n";
                    errs() << "Switching to bellman ford approach\n";

                    float distGvu =
                        feasibleConstraintGraph.shortestPathBellmanFord(v, u);
                    assert(distGvu != -1e20);
                    bool infeasible = (distGvu + c < 0.0);

                    if (infeasible) {

                        feasibleConstraintGraph.removeEdge(u, v, c);

                        // sanity check
                        bool feasible =
                            feasibleConstraintGraph.shortestPathSrc();
                        File() << "new system feasible? " << feasible << "\n";
                        assert(feasible && "feasibleConstraintGraph should "
                                           "always be feasible by definition");

                        return false;

                    } else {

                        bool feasible =
                            feasibleConstraintGraph
                                .shortestPathSrcUpdateFeasible(FeasibleSoln);
                        assert(feasible);

                        bool feasible2 =
                            feasibleConstraintGraph.shortestPathSrc();
                        File() << "Original system feasible? " << feasible
                               << "\n";
                        assert(feasible2);

                        addToFeasibleVerify(v, u, c);

                        return true;
                    }

                    // queue.print();
                    errs() << "x: " << x << " old_dist: " << getD(newD, x)
                           << " new_dist: " << new_dist << "\n";

                    if (affected[x] > 100) {
                        negCycle = true;
                        File() << "Negative cycle detected\n";
                        // feasibleConstraintGraph.removeEdge(u, v, c);
                        // return false;
                        continue;
                    }

                    // assert(affected[x] < 100 && "Affected variable changed
                    // twice!\n");

                    // assert(0 && "Affected variable changed twice!\n");

                    // assert(0 && "Affected variable changed twice!\n");
                }
                affected[x]++;

                setD(newD, x, new_dist);

                std::map<int, float> &successors =
                    feasibleConstraintGraph.getSuccessors(x);
                for (std::map<int, float>::iterator si = successors.begin(),
                                                    ei = successors.end();
                     si != ei; ++si) {
                    int y = si->first;
                    float length = si->second;

                    assert(feasibleConstraintGraph.length(x, y) == length);

                    // scaling length is fixed for each x-> edge pair
                    float scaledLength = (getD(FeasibleSoln, x) + length -
                                          getD(FeasibleSoln, y));

                    // scaledLength is often negative does this break
                    // Dijkstra's algorithm?
                    // assert(scaledLength >= 0);

                    float scaledPathLength = dist_x + scaledLength;

                    // File() << "x: " << x << "\n";
                    // File() << "y: " << y << "\n";
                    // File() << "D(x): " << getD(FeasibleSoln, x) << "\n";
                    // File() << "dist_x: " << dist_x << "\n";
                    // File() << "length: " << length << "\n";
                    // File() << "D(y): " << getD(FeasibleSoln, y) << "\n";
                    // File() << "scaledPathLength: " << scaledPathLength <<
                    // "\n";
                    if (scaledPathLength < queue.hashKey(y)) {
                        queue.adjustHeap(y, scaledPathLength);
                    }
                }
            }
        }
    }

    // assert(!infeasible );

    assert(!negCycle);
    // File() << "Feasible system: update solution\n";

    // update Feasible solution with the new solution
    for (std::map<int, float>::iterator i = newD.begin(), e = newD.end();
         i != e; ++i) {
        int x = i->first;
        float d = i->second;
        setD(FeasibleSoln, x, d);
    }

    // File() << "Affected SDC vertices: " << affected.size() << "\n";

    addToFeasibleVerify(v, u, c);

    return true;
}

void SDCSolver::addToFeasibleVerify(int v, int u, float c) {

    // float distGvu = feasibleConstraintGraph.shortestPathBellmanFord(v, u);
    // assert(distGvu != -1e20);
    // bool infeasible = (distGvu + c < 0.0);
    // assert(!infeasible);

    // dummy node is 1...
    // scale the feasible solution such that the dummy node remains
    // equal to zero:
    //    v - u <= c
    // Given a feasible solution for v/u we can add any offset (x),
    // as long as we add it to every single variable in the feasible
    // solution:
    //    (v + x) - (u + x) <= c
    //    v + x - u - x <= c
    //    v - u <= c
    // Constraints are still satisfied.
    float dummyVal = getD(FeasibleSoln, 1);
    if (dummyVal != 0) {
        // File() << "Dummy node changed....\n";
        for (std::map<int, float>::iterator i = FeasibleSoln.begin(),
                                            e = FeasibleSoln.end();
             i != e; ++i) {
            int x = i->first;
            FeasibleSoln[x] = FeasibleSoln[x] - dummyVal;
        }
    }
    dummyVal = getD(FeasibleSoln, 1);
    assert(dummyVal == 0);

    // File() << "FeasibleSoln.size()" << FeasibleSoln.size() << "\n";
    // File() << "newD.size()" << newD.size() << "\n";
    printFeasibleSoln();

    // if (C) {
    //    assert(C->v == v);
    //    assert(C->u == u);
    //    assert(C->c == c);
    //} else {
    Constraint *C = new Constraint;
    C->v = v;
    C->u = u;
    C->c = c;

    processed.push_back(C);

    verifyFeasibleSoln();
}

// SDC constraints must be of the form:
//    v - u <= c
void SDCSolver::addConstraintSDCForm(lprec *lp, int v_index, int u_index,
                                     REAL rh, Constraints *constraints,
                                     Instruction *I) {
    int idx[2];
    REAL coeff[2];
    idx[0] = v_index; // variable indicies
    idx[1] = u_index;
    coeff[0] = 1.0; // variable coefficients
    coeff[1] = -1.0;

    if (SDCdebug)
        File() << "Add constraint (SDC form): var(" << v_index << ") - "
               << "var(" << u_index << ") <= " << ftostr(rh) << "\n";

    // if (feasible) {
    //    File() << "System is Feasible!\n";
    //} else {
    //    //isFeasible = false;
    //    File() << "System is NOT Feasible!\n";
    //}

    add_constraintex(lp, 2, coeff, idx, LE, rh);

    if (constraints) {
        assert(I);
        Constraint *C = addConstraint(I);
        C->v = v_index;
        C->u = u_index;
        C->c = rh;
        // C->constr_type = LE;
        // C->feasible = feasible;
        constraints->list.push_back(C);
        constraintsAll.push_back(C);
    } else {
        Constraint *C = new Constraint;
        C->v = v_index;
        C->u = u_index;
        C->c = rh;
        constraintsAll.push_back(C);
    }

    if (LEGUP_CONFIG->getParameterInt("INCREMENTAL_SDC")) {
        incrAddConstraint(v_index, u_index, rh);
        assert(feasibleConstraintGraph.getNumConstraints() ==
               (int)processed.size());
        assert(feasibleConstraintGraph.getNumConstraints() +
                   (int)unprocessed.size() ==
               (int)constraintsAll.size());
    }
}

void SDCSolver::addConstraintIncremental(lprec *lp, int count, REAL *row,
                                         int *colno, int constr_type, REAL rh,
                                         Constraints *constraints /* = NULL*/,
                                         Instruction *I /* = NULL*/) {

    // File() << "addConstraintIncremental: \n";

    int v_index, u_index;
    if (count == 1) {

        int dummyIndex = 1;

        // use the dummy node, which is equal to 0
        if (row[0] == 1.0) {
            v_index = colno[0];
            u_index = dummyIndex;
        } else {
            assert(row[0] == -1.0);
            v_index = dummyIndex;
            u_index = colno[0];
        }

        // File() << row[0] << "*var(" << colno[0] << ") " <<
        // lpConstraintStr(constr_type)
        //    << ftostr(rh) << "\n";
        // assert(constr_type == GE);

        // make sure dummy node is equal to 0...

        //    if (constraints) {
        //        Constraint *C = addConstraint(I);
        //        C->v = 0;
        //        C->u = 0;
        //        C->c = 0;
        //        C->constr_type = GE;
        //        constraints->list.push_back(C);
        //    }

        //    File() << "skipping..\n";
    } else {
        assert(count == 2);

        // File() << row[0] << "*var(" << colno[0] << ") - " << row[0] <<
        //    "*var(" << colno[1] << ") " << lpConstraintStr(constr_type) <<
        //    ftostr(rh) << "\n";

        // Need to convert into SDC form:
        // v - u <= c
        if (row[0] == 1.0) {
            assert(row[1] == -1.0);
            v_index = colno[0];
            u_index = colno[1];
        } else {
            assert(row[0] == -1.0);
            assert(row[1] == 1.0);
            v_index = colno[1];
            u_index = colno[0];
        }
    }

    switch (constr_type) {
    // already in expected form:
    // v - u <= c
    case (LE):
        addConstraintSDCForm(lp, v_index, u_index, rh, constraints, I);
        break;
    // An EQ constraint:
    //    v - u == c
    // Can be handled as two constraints:
    //    v - u <= c
    //    u - v <= -c  (equivalent to: v - u >= c)
    case (EQ):
        addConstraintSDCForm(lp, v_index, u_index, rh, constraints, I);
        addConstraintSDCForm(lp, u_index, v_index, -1.0 * rh, constraints, I);
        break;
    // A GE constraint:
    //    v - u >= c
    // Can be handled as:
    //    u - v <= -c
    case (GE):
        addConstraintSDCForm(lp, u_index, v_index, -1.0 * rh, constraints, I);
        break;
    default:
        assert(0);
    }
}

SDCSolver::Constraint *SDCSolver::addConstraint(Instruction *I) {
    Constraint *C = new Constraint;
    if (constraintRows.empty()) {
        initialRow = get_Nrows(lp);
    }
    constraintRows.push_back(C);
    instrConstraints[I].push_back(C);
    assert(getRowNum(C) == get_Nrows(lp));
    return C;
}

std::list<SDCSolver::Constraint *>::iterator
SDCSolver::findConstraint(std::list<Constraint *> &rows, Constraint *C) {
    std::list<Constraint *>::iterator i, e;
    for (i = rows.begin(), e = rows.end(); i != e; ++i) {
        Constraint *cur = *i;
        if (cur == C)
            break;
    }
    return i;
}

void SDCSolver::deleteConstraint(Constraint *C) {
    bool deleted = del_constraint(lp, getRowNum(C));
    assert(deleted);

    std::list<Constraint *>::iterator i = findConstraint(constraintRows, C);
    assert(i != constraintRows.end() && "Cannot find constraint");
    constraintRows.erase(i);

    std::list<Constraint *>::iterator j = findConstraint(constraintsAll, C);
    assert(j != constraintsAll.end() && "Cannot find constraint");
    constraintsAll.erase(j);

    incrRemoveConstraint(C->v, C->u, C->c);
}

void SDCSolver::deleteConstraints(Constraints *constraints) {
    for (std::vector<Constraint *>::iterator i = constraints->list.begin(),
                                             e = constraints->list.end();
         i != e; ++i) {
        Constraint *C = *i;

        deleteConstraint(C);
    }

    assert(initialRow + (int)constraintRows.size() - 1 == get_Nrows(lp));
}

void SDCSolver::deleteAllConstraints(ConstraintListTy &list) {
    for (std::list<Constraint *>::iterator i = list.begin(), e = list.end();
         i != e; ++i) {
        if (findConstraint(constraintRows, *i) != constraintRows.end()) {
            deleteConstraint(*i);
        }
    }
    assert(initialRow + (int)constraintRows.size() - 1 == get_Nrows(lp));
}

void SDCSolver::deleteAllInstrConstraints(Instruction *I) {

    File() << "Deleting all constraints for: " << *I << "\n";
    assert(instrConstraints.find(I) != instrConstraints.end());
    deleteAllConstraints(instrConstraints[I]);
    instrConstraints[I].clear();
}
