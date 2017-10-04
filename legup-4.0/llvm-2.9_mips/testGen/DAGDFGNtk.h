/*
 * DAGNtk.h
 *
 *  Created on: 2012-05-16
 *      Author: fire
 */

#ifndef DAGNTK_H_
#define DAGNTK_H_
#include "Node.h"
#include "ArrayVar.h"
#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>
#include "NodePattern.h"

using namespace std;

class DAGDFGNtk {
public:
	vector<Node*> vNodes;
	vector<Node*> vConsts;
	int isArrayInput;
	vector<Node*> vPis;
	ArrayVar *inputArray;
	int isArrayOutput;
	vector<Node*> vPos;
	ArrayVar *outputArray;

	int NumNodes;

	int NumPis;
	int NumPos;
public:
	DAGDFGNtk(int inputNum, int outputNum);
	DAGDFGNtk(int ArrayInput, int ArrayOutput, int InputSize, int OutputSize);
	virtual ~DAGDFGNtk();
	void add_node(Node *inNode);
	void add_Pi(Node *inNode);
	void add_array_Po(Node *inNode);
	void add_Po(Node *inNode);
	void add_array_Pi(Node *inNode);
	void add_Const(Node *inNode);

	void update_level();
	void update_level_reverse();
	void print_ntk_to_dot(char* fileName);
	void print_ntk_to_subgraph(ofstream* mydotfile, int idx);
	void print_pattern_to_subgraph(ofstream* mydotfile, NodePattern* Pattern);
	void reset_visit_info();
	void sort_fanout_by_level();
	void sort_by_level_closest();
};

#define NtkForEachNode(Ntk, pNode, i )                                                           \
    for ( i = 0; (i < Ntk->vNodes.size()) && (((pNode) = Ntk->vNodes.at(i)), 1); i++ )
#define NtkForEachPi(Ntk, Pi, i )                                                           \
    for ( i = 0; (i < Ntk->vPis.size()) && (((Pi) = Ntk->vPis.at(i)), 1); i++ )
#define NtkForEachPo(Ntk, Po, i )                                                           \
    for ( i = 0; (i < Ntk->vPos.size()) && (((Po) = Ntk->vPos.at(i)), 1); i++ )
#define NtkForEachConst(Ntk, Const, i )                                                           \
    for ( i = 0; (i < Ntk->vConsts.size()) && (((Const) = Ntk->vConsts.at(i)), 1); i++ )
#define NtkForEachArrayPi(Ntk, Pi, i )                                                           \
    for ( i = 0; (i < Ntk->inputArray->ArrayNodes.size()) && (((Pi) = Ntk->inputArray->ArrayNodes.at(i)), 1); i++ )
#define NtkForEachArrayPo(Ntk, Po, i )                                                           \
    for ( i = 0; (i < Ntk->outputArray->ArrayNodes.size()) && (((Pi) = Ntk->outputArray->ArrayNodes.at(i)), 1); i++ )
#endif /* DAGNTK_H_ */
