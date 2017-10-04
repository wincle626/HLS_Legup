#ifndef EDMOND_MATCHING_H
#define EDMOND_MATCHING_H

#include "LegupConfig.h"

#define MAX 100

using namespace llvm;

namespace legup {

class EdmondMatching {

public:

    EdmondMatching() : graphSize(0) {}
    typedef std::pair<unsigned, unsigned> EdgeType;
    typedef std::vector<EdgeType> MatchesType;
    typedef MatchesType::iterator iterator;
    typedef MatchesType::const_iterator const_iterator;

    inline iterator       begin()       { return Matches.begin(); }
    inline const_iterator begin() const { return Matches.begin(); }
    inline iterator       end  ()       { return Matches.end();   }
    inline const_iterator end  () const { return Matches.end();   }

    void solveEdmondMatching();
    void addEdge(unsigned, unsigned);

private:
    MatchesType Edges, Matches;
    unsigned graphSize;
                        //Labels are the key to this implementation of the algorithm.
    struct Label {      //An even label in a vertex means there's an alternating path
           int even;    //of even length starting from the root node that ends on the
           int odd[2];  //vertex. To find this path, the backtrace() function is called,
    };                  //constructing the path by following the content of the labels.
                        //Odd labels are similar, the only difference is that base nodes
                        //of blossoms inside other blossoms may have two. More on this later.


    struct elem {            //This is the element of the queue of labels to be analyzed by
           int vertex,type;  //the augmentMatching() procedure. Each element contains the vertex
    };                       //where the label is and its type, odd or even.


    int g[MAX][MAX];         //The graph, as an adjacency matrix.

                             //blossom[i] contains the base node of the blossom the vertex i
    int blossom[MAX];        //is in. This, together with labels eliminates the need to
                             //contract the graph.

                                  //The path arrays are where the backtrace() routine will
    int path[2][MAX],endPath[2];  //store the paths it finds. Only two paths need to be
                                  //stored. endPath[p] denotes the end of path[p].

    bool match[MAX];  //An array of flags. match[i] stores if vertex i is in the matching.

                      //label[i] contains the label assigned to vertex i. It may be undefined,
    Label label[MAX]; //empty (meaning the node is a root) and a node might have even and odd
                      //labels at the same time, which is the case for nonbase nodes of blossoms

    elem queue[2*MAX];         //The queue is necessary for efficiently scanning all labels.
    int queueFront,queueBack;  //A label is enqueued when assigned and dequeued after scanned.

    void initGraph(int n);
    int readGraph();
    void initAlg(int n);
    void backtrace (int vert, int pathNum, int stop, int parity, int direction);
    void enqueue (int vert, int t);
    void newBlossom (int a, int b);
    void augmentPath ();
    bool augmentMatching (int n);
    void findMaximumMatching (int n);
    int main();



};

} // End legup namespace

#endif
