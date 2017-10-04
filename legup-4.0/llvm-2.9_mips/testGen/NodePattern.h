/*
 * NodePattern.h
 *
 *  Created on: 2012-08-29
 *      Author: fire
 */

#ifndef NODEPATTERN_H_
#define NODEPATTERN_H_

#include "Node.h"
#include <queue>
#include "AutoConfig.h"
class NodePattern: public Node {
public:

	int patternSize;
	float depthFactor;
	int PatternInputSize;
	int PatternOutputSize;
	AutoConfig *AC;
	int numberNode;
	int usedCopyPattern;
	int Pattern_pool_idx;
	vector<Node*> FakeInputNode;
	vector<Node*> FakeOutputNode;
	vector<Node*> vNodes;
	//NodePattern(int PatternSize, int inputNum, int outputNum, int inDepthFactor, int seed, AutoConfig *inAC) ;
	NodePattern(int PatternSize, int inputNum, int outputNum);
	void Pattern_StartGen(float inDepthFactor, int seed, AutoConfig *inAC);
	void copy_pattern(NodePattern* sourcePattern);
	Node* get_node_by_idx(int idx);
	virtual ~NodePattern();
private:
	int patternRealSize;
	int Seed;
	OperationType rand_assign_operation();
	int addInput(Node* tarNode, Node* inNode);
	void update_level();
	Node* rand_assign_node(Node* tarNode);
};

#endif /* NODEPATTERN_H_ */
