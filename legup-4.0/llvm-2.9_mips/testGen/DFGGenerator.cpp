/*
 * Generator.cpp
 *
 *  Created on: 2012-05-21
 *      Author: fire
 */

#include "DFGGenerator.h"

DFGGenerator::DFGGenerator() {
	Ntk = new DAGDFGNtk(1, 1);
}
DFGGenerator::DFGGenerator(int cap, int seed, int inputNum, int outputNum, float inDepthFactor, AutoConfig *inAC) {

	NodeCap = cap;
	Seed = seed;
	depthFactor = inDepthFactor;
	Ntk = new DAGDFGNtk(inputNum, outputNum);
	AC = inAC;
	//printf("seed %d, input %d, output %d, Depth Factor %d, \n", seed, inputNum, outputNum, depthFactor);
}

//Constructor for DFG with array input/outputs
DFGGenerator::DFGGenerator(int cap, int seed, int inputNum, int outputNum, float inDepthFactor, int ArrayInput, int ArrayOutput, AutoConfig *inAC) {

	NodeCap = cap;
	Seed = seed;
	depthFactor = inDepthFactor;

	Ntk = new DAGDFGNtk(ArrayInput, ArrayOutput, inputNum, outputNum);
	AC = inAC;

}
int DFGGenerator::DFG_StartGen_fake(){
	Node *pNode;
	int i;
	//Start from output nodes.
	//printf("IN FAKE, NumPis: %d NumPos: %d\n", Ntk->NumPis, Ntk->NumPos);
	if(!Ntk->isArrayOutput){
		for(i=0; i<Ntk->NumPos; i++){
			pNode = new Node(3);
			Ntk->add_node(pNode);
			Ntk->add_Po(pNode);
		}
	}else{
		for(i=0; i<Ntk->NumPos; i++){
			pNode = new Node(3);
			Ntk->add_node(pNode);
			Ntk->add_array_Po(pNode);
		}
	}


	//Now, let's deal with the inputs....
	if(!Ntk->isArrayInput){
		for(i=0; i<Ntk->NumPis; i++){
			pNode = new Node(2);
			Ntk->add_node(pNode);
			Ntk->add_Pi(pNode);

		}
	}else{
		for(i=0; i<Ntk->NumPis; i++){
			pNode = new Node(2);
			Ntk->add_node(pNode);
			Ntk->add_array_Pi(pNode);

		}
	}
}
void DFGGenerator::initializeOpList(){
	assert(AC);
	int j;
	OperationInfo *oi;
	int total=0;
	//printf("initializeOpList!!!!\n");
	AFForEachOperation( AC, oi, j ){
		//printf("processing op: %d\n", oi->opIdx);
		int occTime ;
		if(j==AC->vOperationInfo.size()-1){
			occTime = NodeCap-total;
		}else{
			occTime = NodeCap*oi->ratio;
			total += occTime;
		}
		//printf("occTime: %d\n", occTime);
		if(occTime>0){
			for(int i=0; i<occTime; i++){
				oplist.insert(oplist.end(), oi->opIdx);
			}
		}
	}
	//list<int>::iterator ii;
	//for (ii =  oplist.begin(); ii != oplist.end(); ++ii){
	//	cout << *ii << endl;
	//}
}
int DFGGenerator::DFG_StartGen(){
	int generatedNumNode=0;
	int i;
	int ExpectedNumInput;
	int randVal;
	Node *pNode, *newNode, * assignedNode;
	queue<Node*> qNode;
	vector<Node*> vNode;
	int numOfPatterns = 0;
	//printf("DFG Generation gets called once\n");
	//Start from output nodes.
	//printf("NodeCap: %d\n", NodeCap);
	initializeOpList();
	if(!Ntk->isArrayOutput){
		for(i=0; i<Ntk->NumPos; i++){
			pNode = new Node(3);
			qNode.push(pNode);
			Ntk->add_node(pNode);
			Ntk->add_Po(pNode);
		}
	}else{
		for(i=0; i<Ntk->NumPos; i++){
			pNode = new Node(3);
			qNode.push(pNode);
			Ntk->add_node(pNode);
			Ntk->add_array_Po(pNode);
		}
	}
	srand(Seed);
	//printf("Seed: %d\n", Seed);
	int newCreatedNode = 0;
	while (!qNode.empty()) {
		pNode = qNode.front();
		qNode.pop();
		//cout << "Generated " << Ntk->NumNodes << " Nodes\n";
		//if (Ntk->NumNodes <= NodeCap) {
		if (newCreatedNode <= NodeCap) {
			ExpectedNumInput = pNode->getExpectedNumInput();

			for (i = 0; i < ExpectedNumInput; i++) {
				if (newCreatedNode >= NodeCap)
					break;

				float randFVal = ((float)(rand()%100))/100.;
				//cout << "randFVal: " << randFVal << " " <<"\n";
				//printf("randFVal: %f\n", randFVal);
				randVal = (randFVal < depthFactor)? 1 : 0;	//depthFactor
				//cout << "randVal: " << randVal << " " <<"\n";

				//randVal = rand() % 100;
				//randVal = rand() % depthFactor;
				//cout << "Generated " << randVal << " " <<"\n";
				if (i == 0 || randVal) {

					//cout << "Generating new node\n";
					//if(1)

					randVal = rand()%NodeCap;
					//cout << "randVal: " << randVal << endl;
					int node_left = NodeCap-Ntk->NumNodes;
					if(AC->enable_patterns==0){
						//newNode = GenRandNode(AllInputConst(pNode));
						newNode = GenRandNode(1);
						newNode->isPattern=0;
					}else{
						if((randVal)<(NodeCap*AC->pattern_ratio) && node_left>AC->patternSize){
							NodePattern* newPattern = new NodePattern(AC->patternSize, AC->patternInputSize, 1);
							newNode = (Node*)newPattern;
							numOfPatterns++;
						}else{
							//newNode = GenRandNode(AllInputConst(pNode));
							newNode = GenRandNode(1);
							newNode->isPattern=0;
						}
					}
					//printf("newNode: %d\n", newNode->NodeType);
					Ntk->add_node(newNode);
					newCreatedNode++;
					addInput(pNode, newNode);
					qNode.push(newNode);
				} else {
					//cout << "Not Generating new node\n";
				}

			}
		}
	}
	int DFGopNum=0;
	NtkForEachNode(Ntk, pNode, i ){
		if(pNode->NodeType==1){
			DFGopNum++;
		}
	}
	//printf("DFGopNum: %d\n", DFGopNum);
	//ASSERT(Ntk->vNodes.size() == NodeCap);
	//printf("newCreatedNode: %d, NodeCap: %d\n", newCreatedNode, NodeCap);
	//ASSERT(Ntk->NumNodes == NodeCap);
	ASSERT(newCreatedNode == NodeCap);
	Ntk->update_level();
	//Now, let's deal with the inputs....
	if(!Ntk->isArrayInput){
		for(i=0; i<Ntk->NumPis; i++){
			pNode = new Node(2);
			Ntk->add_node(pNode);
			Ntk->add_Pi(pNode);

		}
	}else{
		for(i=0; i<Ntk->NumPis; i++){
			pNode = new Node(2);
			Ntk->add_node(pNode);
			Ntk->add_array_Pi(pNode);

		}
	}


	NtkForEachNode(Ntk, pNode, i ){
		ExpectedNumInput = pNode->getExpectedNumInput();
		if(!pNode->isPi){
			while(pNode->NumInput<ExpectedNumInput){
				assignedNode = rand_assign_node(Ntk, pNode);
				if(assignedNode!=NULL){
					//cout << "Assigning node "<< assignedNode->NodeIdx << " to " << pNode->NodeIdx << "\n";
					addInput(pNode, assignedNode);
					//cout << "node has "<< assignedNode->NumOutput << " outputs\n";
				}
			}
		}
	}

	int patternPoolSize = numOfPatterns*AC->pattern_pool_ratio;

	if(patternPoolSize<1) patternPoolSize = 1;

	//printf("numOfPatterns: %d\n", numOfPatterns);
	//printf("patternPoolSize: %d\n", patternPoolSize);
	NtkForEachNode(Ntk, pNode, i ){
		if(pNode->isPattern){
			NodePattern* newPattern = (NodePattern*)pNode;
			if(vPatterns.size()<patternPoolSize){
				newPattern->Pattern_StartGen(depthFactor, Seed, AC);
				newPattern->Pattern_pool_idx=vPatterns.size();
				vPatterns.push_back(newPattern);
			}else{
				assert(vPatterns.size()>0);
				int patternIdx = rand()%vPatterns.size();
				NodePattern* Pattern = vPatterns.at(patternIdx);
				newPattern->copy_pattern(Pattern);
			}
		}
	}
	char BB_DOT[30];
	sprintf(BB_DOT, "dfg_aa.dot");
	Ntk->print_ntk_to_dot(BB_DOT);

	NtkForEachPi(Ntk, pNode, i ){
		assert(pNode->OutputNode.size()>0);
		//cout << "node "<< pNode->NodeIdx << " output number " << pNode->OutputNode.size() << "\n";
	}

	return 1;
}
Node *DFGGenerator::GenRandNode(char no_const){
	Node *NewpNode;
	int randType, randOp;
	//srand((unsigned)time(0));
	if(no_const){
		randType = 1;
	}else{
		randType = rand()%3;
	}
	if(randType==0){
		NewpNode = new Node(randType);
	}
	if(randType==1){
		NewpNode = new Node(randType);
		//randOp = rand()%4;
		NewpNode->NodeOperation = rand_assign_operation();
	}else if (randType==2){

	}

	return NewpNode;
}
int DFGGenerator::AllInputConst(Node *inNode){
	int i;
	int allConst=1;
	for(i=0;i<inNode->NumInput;i++){
		if(inNode->InputNode.at(i)->isConst()){
			allConst = 0;
			break;
		}
	}

	return allConst;
}
//Assign inNode as fanin to tarNode as well as updating NumInput
int DFGGenerator::addInput(Node* tarNode, Node* inNode){
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
bool close_level_cmp_function (Node * i,Node * j) { return (i->level_diff < j->level_diff); }

Node* DFGGenerator::rand_assign_node(DAGDFGNtk *Ntk, Node* tarNode){
	int node_level = tarNode->level;
	int i;
	int randIdx;
	Node *pNode, *Pi;
	vector<Node*> vNode_Inputs;
	vector<Node*> vNode_Op;
	//Checks if there's any Pi not being assigned, if so, assign it
	NtkForEachPi(Ntk, Pi, i ){
		if(Pi->NumOutput==0)
			vNode_Inputs.push_back(Pi);
	}
	//printf("vNode_Inputs.size(): %d\n", vNode_Inputs.size());
	if(vNode_Inputs.size()>0){
		randIdx = rand()%vNode_Inputs.size();
		return vNode_Inputs.at(randIdx);
	}else{
		if(rand()%AC->ConstantDensity == 0){
			pNode = rand_assign_const(Ntk);
			return pNode;
		}
		//printf("Random assignment\n");
		Ntk->update_level();

		NtkForEachPi(Ntk, Pi, i ){
			vNode_Inputs.push_back(Pi);
		}
		NtkForEachNode(Ntk, pNode, i ){
			if(pNode->NodeType!=2 && pNode->level>node_level){
				vNode_Op.push_back(pNode);
			}
		}

		if(rand()%AC->input_connectivity==0 || vNode_Op.size()==0){
			if(vNode_Inputs.size()==0){
				return NULL;
			}else{
				randIdx = rand()%vNode_Inputs.size();
				//cout << "randIdx: "<< randIdx << "\n";
				//cout << "vNode.at(randIdx): "<< vNode.at(randIdx)->NodeIdx << "\n";
				//printf("Random assigned to an Input %d\n", vNode_Inputs.at(randIdx)->NodeIdx);
				return vNode_Inputs.at(randIdx);
			}
		}else{

			//printf("node_level: %d\n", node_level);
			for(i=0;i<vNode_Op.size();i++){
				//printf("Node %d's is at level: %d, Type: %d\n", vNode_Op.at(i)->NodeIdx, vNode_Op.at(i)->level, vNode_Op.at(i)->NodeType);
				if(node_level>vNode_Op.at(i)->level)
					vNode_Op.at(i)->level_diff = node_level-vNode_Op.at(i)->level;
				else
					vNode_Op.at(i)->level_diff = vNode_Op.at(i)->level-node_level;
			}
			sort (vNode_Op.begin(), vNode_Op.end(), close_level_cmp_function);
			for(i=0;i<vNode_Op.size();i++){
				//printf("Node %d's level_diff: %d\n", vNode_Op.at(i)->NodeIdx, vNode_Op.at(i)->level_diff);
			}
			if(vNode_Op.size()==0){
				return NULL;
			}else{
				randIdx = rand()%vNode_Op.size();
				//cout << "randIdx: "<< randIdx << "\n";
				//cout << "vNode.at(randIdx): "<< vNode.at(randIdx)->NodeIdx << "\n";
				//printf("Random assigned to an Op %d\n", vNode_Op.at(randIdx)->NodeIdx);
				//return vNode_Op.at(0);
				return vNode_Op.at(randIdx);
			}
		}
	}



}
Node* DFGGenerator::rand_assign_const(DAGDFGNtk *Ntk){
	Node *pNode = new Node(0);
	pNode->NodeValue = (rand()%1000)+1;
	Ntk->add_node(pNode);
	Ntk->add_Const(pNode);
	return pNode;
}
OperationType DFGGenerator::rand_assign_operation(){
	int i;

	if(1){
		int numOpLeft = oplist.size();
		int randNum=rand()%numOpLeft;
		list<int>::iterator ii;
		int idx=0;
		//printf("randNum : %d\n", randNum);
		for (ii =  oplist.begin(); ii != oplist.end(); ii++){
			//cout << *ii << endl;
			if(idx==randNum) break;
			idx++;
		}

		int operationAssign = *ii;
		//printf("Hit operation: %d, list size: %d\n", operationAssign, oplist.size());
		oplist.erase (ii);
		//printf("Deleted operation: %d\n", operationAssign);
		return (OperationType)operationAssign;
	}
	OperationInfo *oi;
	int randNum=rand()%AC->possibility_resolution;
	AFForEachOperation( AC, oi, i ){
		if(randNum<=oi->maxIdx && randNum>=oi->minIdx){
			return (OperationType)oi->opIdx;
		}
	}
	return (OperationType)-1;
}


DFGGenerator::~DFGGenerator() {
	// TODO Auto-generated destructor stub
}

