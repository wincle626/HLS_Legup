/*
 * Generator.h
 *
 *  Created on: 2012-05-21
 *      Author: fire
 */

#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <iostream>
#include <fstream>
#include "DAGDFGNtk.h"
#include "AutoConfig.h"
#include "Node.h"
#include <time.h>
#include <queue>
#include <vector>
#include <list>
#include "NodePattern.h"


#define ASSERT(x) \
if (!(x)) \
{ \
cout << "ERROR!! Assert " << #x << " failed\n"; \
cout << " on line " << __LINE__  << "\n"; \
cout << " in file " << __FILE__ << "\n";  \
}

class DFGGenerator {
public:
	int NodeCap;
	int Seed;
	//larger the depthFactor, deeper the graph will be
	float depthFactor;
	AutoConfig *AC;
	DAGDFGNtk *Ntk;
	list<int> oplist;
public:
	DFGGenerator();
	DFGGenerator(int cap, int seed, int inputNum, int outputNum, float inDepthFactor, AutoConfig *inAC);
	DFGGenerator(int cap, int seed, int inputNum, int outputNum, float inDepthFactor, int ArrayOutput, int ArrayInput, AutoConfig *inAC);
	int DFG_StartGen();
	void initializeOpList();
	int DFG_StartGen_fake();
	virtual ~DFGGenerator();
	//void print_ntk_to_dot(char* fileName);
private:
	vector<NodePattern*> vPatterns;
	int AllInputConst(Node *inNode);
	Node *GenRandNode(char no_const);
	int addInput(Node *tarNode, Node *inNode);
	Node* rand_assign_node(DAGDFGNtk *Ntk, Node* tarNode);
	Node* rand_assign_const(DAGDFGNtk *Ntk);
	OperationType rand_assign_operation();
};

#endif /* GENERATOR_H_ */
