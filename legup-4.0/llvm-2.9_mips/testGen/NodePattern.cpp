/*
 * NodePattern.cpp
 *
 *  Created on: 2012-08-29
 *      Author: fire
 */

#include "NodePattern.h"
#include "Node.h"
NodePattern::NodePattern(int PatternSize, int inputNum, int outputNum) : Node(){
	patternSize = PatternSize;
	patternRealSize = PatternSize+inputNum+outputNum;
	PatternInputSize = inputNum;
	PatternOutputSize = outputNum;

	isPattern = 1;
	NumInput = 0;
	NumOutput = 0;
	NodeType = 4;
	visited = 0;
	isPo = 0;
	isPi = 0;
	NodeLLVMValueCreated = 0;
	isFake = 0;
	numberNode = 0;
	usedCopyPattern = 1;
	Pattern_pool_idx = 0;
	Seed = 0;
}
/*
NodePattern::NodePattern(int PatternSize, int inputNum, int outputNum, int inDepthFactor, int seed, AutoConfig *inAC) : Node(){
	Node* pNode;
	patternSize = PatternSize;
	patternRealSize = PatternSize+inputNum+outputNum;
	Seed = seed;
	AC = inAC;
	depthFactor = inDepthFactor;

	isPattern = 1;
	PatternInputSize = inputNum;
	PatternOutputSize = outputNum;

	NumInput = 0;
	NumOutput = 0;
	NodeType = 4;
	visited = 0;
	isPo = 0;
	isPi = 0;
	NodeLLVMValueCreated = 0;
	isFake = 0;
	numberNode = 0;
	usedCopyPattern = 0;
	//Currently, only consider pattern with single outputs
	assert(outputNum==1);

	for(int i=0; i<outputNum; i++){
		pNode = new Node(3);
		pNode->isPo = 1;
		pNode->isFake = 1;
		pNode->NodeIdx=numberNode;
		vNodes.push_back(pNode);

		FakeOutputNode.push_back(pNode);
		numberNode++;
	}
	for(int i=0; i<inputNum; i++){
		pNode = new Node(2);
		pNode->isPi = 1;
		pNode->isFake = 1;
		pNode->NodeIdx=numberNode;
		vNodes.push_back(pNode);

		FakeInputNode.push_back(pNode);
		numberNode++;
	}
}
*/
void NodePattern::Pattern_StartGen(float inDepthFactor, int seed, AutoConfig *inAC){
	int i;
	Node *pNode, *NewpNode;
	queue<Node*> qNode;

	Seed = seed;
	AC = inAC;
	depthFactor = inDepthFactor;

	for(int i=0; i<PatternOutputSize; i++){
		pNode = new Node(3);
		pNode->isPo = 1;
		pNode->isFake = 1;
		pNode->NodeIdx=numberNode;
		vNodes.push_back(pNode);

		FakeOutputNode.push_back(pNode);
		numberNode++;
	}
	for(int i=0; i<PatternInputSize; i++){
		pNode = new Node(2);
		pNode->isPi = 1;
		pNode->isFake = 1;
		pNode->NodeIdx=numberNode;
		vNodes.push_back(pNode);

		FakeInputNode.push_back(pNode);
		numberNode++;
	}

	for(i=0; i<PatternOutputSize; i++){
		pNode = FakeOutputNode.at(i);
		qNode.push(pNode);
	}

	//printf("SEED: %d\n", this->Seed);
	//srand(this->Seed);

	//printf("\n");
	//printf("Generating Pattern for %d\n", NodeIdx);
	while (!qNode.empty()) {
		pNode = qNode.front();
		qNode.pop();
		if (vNodes.size() <= patternRealSize) {
			int ExpectedNumInput = pNode->getExpectedNumInput();
			for (i = 0; i < ExpectedNumInput; i++) {
				if (vNodes.size() >= patternRealSize)
					break;
				float randFVal = ((float)(rand()%100))/100.;
				int randVal = (randFVal < depthFactor)? 1 : 0;	//depthFactor
				//if(pNode->InputNode.size()==0) randVal =1;
				if (i == 0 || randVal) {
					NewpNode = new Node(1);
					NewpNode->NodeOperation = this->rand_assign_operation();

					NewpNode->NodeIdx=numberNode;
					//printf("Assigning operation %d to node %d\n", NewpNode->NodeOperation, NewpNode->NodeIdx);
					//printf("Pushng %d to vNodes\n", NewpNode->NodeIdx);
					vNodes.push_back(NewpNode);

					numberNode++;
					//printf("Node: %d is connecting to Node: %d\n", NewpNode->NodeIdx, pNode->NodeIdx);
					addInput(pNode, NewpNode);
					qNode.push(NewpNode);
				}
			}
		}
	}
	//printf("vNodes size: %d, patternSize: %d, numberNode: %d\n", vNodes.size(), patternSize, numberNode);
	assert(vNodes.size() == patternRealSize);
	this->update_level();
	//For each node in this pattern
	for(i=0;i<vNodes.size();i++){
		pNode = vNodes.at(i);
		int ExpectedNumInput = pNode->getExpectedNumInput();
		if(!pNode->isPi){
			//printf("ExpectedNumInput: %d, NumInput: %d\n", ExpectedNumInput, pNode->NumInput);
			while(pNode->NumInput<ExpectedNumInput){
				Node *assignedNode;
				assignedNode = this->rand_assign_node(pNode);
				if(assignedNode!=NULL){
					//printf("Adding input from %d to %d\n", assignedNode->NodeIdx, pNode->NumInput);
					//printf("2 Node: %d is connecting to Node: %d\n", assignedNode->NodeIdx, pNode->NodeIdx);
					addInput(pNode, assignedNode);
				}
			}
			//printf("After ExpectedNumInput: %d, NumInput: %d\n", ExpectedNumInput, pNode->NumInput);
		}
	}
	for(i=0;i<vNodes.size();i++){
		pNode = vNodes.at(i);
		assert(pNode->InputNode.size()<3);
	}


}
OperationType NodePattern::rand_assign_operation(){
	int i;
	OperationInfo *oi;
	int randNum=rand()%AC->possibility_resolution;
	AFForEachOperation( AC, oi, i ){
		if(randNum<=oi->maxIdx && randNum>=oi->minIdx){
			return (OperationType)oi->opIdx;
		}
	}
	return (OperationType)-1;
}
int NodePattern::addInput(Node* tarNode, Node* inNode){

	if(tarNode->NumInput<tarNode->getExpectedNumInput()){
		tarNode->InputNode.push_back(inNode);
		tarNode->NumInput++;
		inNode->OutputNode.push_back(tarNode);
		inNode->NumOutput++;
		return 1;
	}else{
		return 0;
	}
}
void NodePattern::update_level(){
	int i, assigning_level;
	Node *pNode, *pFanin;
	queue<Node*> qNode;

	for(i=0;i<vNodes.size();i++){
		pNode = vNodes.at(i);
		pNode->level=0;
	}
	int changed = 1;
	while(changed){
		changed = 0;
		for (i = 0; i < PatternOutputSize; i++) {
			pNode = FakeOutputNode.at(i);
			qNode.push(pNode);
		}
		while(qNode.size()>0){
			pNode = qNode.front();
			qNode.pop();
			NodeForEachFanin( pNode, pFanin, i ){
				assigning_level = pFanin->assign_level();
				if(pFanin->level != assigning_level){
					changed = 1;
					pFanin->level = assigning_level;
				}
				qNode.push(pFanin);
			}
		}
	}
}
bool close_level_cmp_function_pattern (Node * i,Node * j) { return (i->level_diff < j->level_diff); }
bool fanoutNumber_cmp_function_pattern (Node * i,Node * j) { return (i->OutputNode.size() < j->OutputNode.size()); }

Node* NodePattern::rand_assign_node(Node* tarNode){
	int i;
	int randIdx;
	int node_level = tarNode->level;
	Node *pNode, *Pi;
	vector<Node*> vNode_Inputs;
	vector<Node*> vNode_Op;
	//For Each Pi
	for(i=0;i<FakeInputNode.size();i++){
		Pi = FakeInputNode.at(i);
		if(Pi->NumOutput==0)
			vNode_Inputs.push_back(Pi);
	}
	//printf("vNode_Inputs.size(): %d\n", vNode_Inputs.size());
	if(vNode_Inputs.size()>0){
		randIdx = rand()%vNode_Inputs.size();
		//printf("Returning an input\n");
		return vNode_Inputs.at(randIdx);
	}else{
		this->update_level();
		for(i=0;i<FakeInputNode.size();i++){
			Pi = FakeInputNode.at(i);
			vNode_Inputs.push_back(Pi);
		}
		for(i=0;i<vNodes.size();i++){
			pNode = vNodes.at(i);
			if(pNode->NodeType!=2 && pNode->level>node_level){
				vNode_Op.push_back(pNode);
			}
		}
		if(rand()%AC->input_connectivity==0 || vNode_Op.size()==0){
			if(vNode_Inputs.size()==0){
				//printf("Returning NULL\n");
				return NULL;
			}else{
				sort (vNode_Inputs.begin(), vNode_Inputs.end(), fanoutNumber_cmp_function_pattern);
				int randIdx = rand()%vNode_Inputs.size();
				return vNode_Inputs.at(randIdx);
			}
		}else{
			for(i=0;i<vNode_Op.size();i++){
				if(node_level>vNode_Op.at(i)->level)
					vNode_Op.at(i)->level_diff = node_level-vNode_Op.at(i)->level;
				else
					vNode_Op.at(i)->level_diff = vNode_Op.at(i)->level-node_level;
			}
			sort (vNode_Op.begin(), vNode_Op.end(), close_level_cmp_function_pattern);
			if(vNode_Op.size()==0){
				//printf("Returning NULL\n");
				return NULL;
			}else{
				randIdx = rand()%vNode_Op.size();
				//printf("Returning a node\n");
				return vNode_Op.at(randIdx);
			}
		}
	}
}
void NodePattern::copy_pattern(NodePattern* sourcePattern){

	assert(this->patternSize ==sourcePattern->patternSize);
	assert(this->PatternInputSize == sourcePattern->PatternInputSize);
	assert(this->PatternOutputSize == sourcePattern->PatternOutputSize);
	assert(this->patternRealSize == sourcePattern->patternRealSize);


	this->depthFactor = sourcePattern->depthFactor;
	this->Seed = sourcePattern->Seed;
	this->AC = sourcePattern->AC;
	this->Pattern_pool_idx=sourcePattern->Pattern_pool_idx;

	//printf("Copying Pattern for %d\n", this->NodeIdx);
	//printf("PatternInputSize %d\n", PatternInputSize);
	assert(this->PatternOutputSize==1);

	Node* pNodeSource, *pNodeNew, *pFaninSource, *pFaninNew, *pFanoutSource, *pFanoutNew;
	int j;
	for(int i=0;i<sourcePattern->vNodes.size();i++){
		pNodeSource = sourcePattern->vNodes.at(i);
		pNodeNew = new Node(pNodeSource->NodeType);
		pNodeNew->isFake = pNodeSource->isFake;
		pNodeNew->NodeIdx = pNodeSource->NodeIdx;


		if(pNodeSource->isPi){
			pNodeNew->isPi=pNodeSource->isPi;
			FakeInputNode.push_back(pNodeNew);
		}else if(pNodeSource->isPo){
			pNodeNew->isPo=pNodeSource->isPo;
			FakeOutputNode.push_back(pNodeNew);
		}else{
			pNodeNew->NodeOperation = pNodeSource->NodeOperation;
			//printf("Copy operation %d to node %d in pattern %d\n", pNodeSource->NodeOperation, pNodeNew->NodeIdx, this->NodeIdx);
		}
		numberNode++;
		this->vNodes.push_back(pNodeNew);
	}
	for(int i=0;i<sourcePattern->vNodes.size();i++){
		pNodeSource = sourcePattern->vNodes.at(i);
		pNodeNew = get_node_by_idx(pNodeSource->NodeIdx);
		NodeForEachFanin( pNodeSource, pFaninSource, j ){
			pFaninNew = get_node_by_idx(pFaninSource->NodeIdx);
			addInput(pNodeNew, pFaninNew);
		}
		NodeForEachFanout( pNodeSource, pFanoutSource, j ){
			pFanoutNew = get_node_by_idx(pFanoutSource->NodeIdx);
			addInput(pFanoutNew, pNodeNew);
		}
	}
}
Node* NodePattern::get_node_by_idx(int idx){
	Node* pNode;
	for(int i=0;i<this->vNodes.size();i++){
		pNode = vNodes.at(i);
		if(pNode->NodeIdx==idx){
			return pNode;
		}
	}
	return NULL;
}
NodePattern::~NodePattern() {
	// TODO Auto-generated destructor stub
}

