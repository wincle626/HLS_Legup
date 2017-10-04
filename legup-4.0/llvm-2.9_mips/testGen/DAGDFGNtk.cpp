/*
 * DAGNtk.cpp
 *
 *  Created on: 2012-05-16
 *      Author: fire
 */

#include "DAGDFGNtk.h"
#include <typeinfo>

DAGDFGNtk::DAGDFGNtk(int inputNum, int outputNum) {
	NumNodes = 0;
	NumPis = inputNum;
	NumPos = outputNum;
	isArrayInput = 0;
	isArrayOutput = 0;
}

DAGDFGNtk::DAGDFGNtk(int ArrayInput, int ArrayOutput, int InputSize, int OutputSize){
	isArrayInput = ArrayInput;
	isArrayOutput = ArrayOutput;
	NumNodes = 0;
	NumPis = InputSize;
	NumPos = OutputSize;
	if(isArrayInput){
		inputArray = new ArrayVar(0, InputSize);
	}
	if(isArrayOutput){
		outputArray = new ArrayVar(1, OutputSize);
	}

}

void DAGDFGNtk::add_node(Node *inNode){
	inNode->NodeIdx = vNodes.size();
	vNodes.push_back(inNode);
	if(inNode->isPattern){
		NodePattern* newPattern = (NodePattern*)inNode;
		NumNodes+=newPattern->patternSize;
	}else
		NumNodes++;

}
void DAGDFGNtk::add_Pi(Node *inNode){
	//add_node(inNode);
	inNode->isPi = 1;
	vPis.push_back(inNode);
}
void DAGDFGNtk::add_Po(Node *inNode){
	//add_node(inNode);
	inNode->isPo = 1;
	vPos.push_back(inNode);
}
void DAGDFGNtk::add_array_Po(Node *inNode){
	//add_node(inNode);
	inNode->isPo = 1;
	if(isArrayOutput)
		outputArray->ArrayNodes.push_back(inNode);
}
void DAGDFGNtk::add_array_Pi(Node *inNode){
	//add_node(inNode);
	inNode->isPi = 1;
	if(isArrayInput)
		inputArray->ArrayNodes.push_back(inNode);
}

void DAGDFGNtk::add_Const(Node *inNode){
	vConsts.push_back(inNode);
}

void DAGDFGNtk::print_ntk_to_dot(char* fileName){
	ofstream mydotfile;
	Node *pNode, *newNode, *pFanin, *pFanout, *pFaninReal, *pFanoutReal;
	int i, j;
	queue<Node*> qNode;
	NtkForEachNode(this, pNode, i ){
		pNode->visited = 0;
	}
	mydotfile.open(fileName);
	for (i = 0; i < NumPos; i++) {
		pNode = vPos.at(i);
		qNode.push(pNode);
		pNode->visited = 1;
	}
	mydotfile << "digraph graphname {\n";
	NtkForEachNode(this, pNode, i ){
		//NodePattern* my_Pattern = dynamic_cast<NodePattern*>(pNode);
		if (pNode->isPattern==0){
			if(pNode->NodeType==0)
				mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->NodeIdx << "(const=" << pNode->NodeValue <<")" << "\"]" << ";\n";
			else if(pNode->NodeType==2)
				mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->NodeIdx << "(input)" << "\"]" << ";\n";
			else if(pNode->NodeType==3)
				mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->NodeIdx << "(output)" << "\"]" << ";\n";
			else{

				switch(pNode->NodeOperation){
				case 0:{
					mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->NodeIdx << "(ADD)" << "\"]" << ";\n";
					break;
				}
				case 1:{
					mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->NodeIdx << "(SUB)" << "\"]" << ";\n";
					break;
				}
				case 2:{
					mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->NodeIdx << "(MULT)" << "\"]" << ";\n";
					break;
				}
				case 3:{
					mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->NodeIdx << "(DIV)" << "\"]" << ";\n";
					break;
				}
				default:{
					mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->NodeIdx << "(" << pNode->NodeOperation << ")" << "\"]" << ";\n";
					break;
				}
				}
			}
		}else{
			//printf("Printing for Pattern\n");
			NodePattern* newPattern = (NodePattern*)pNode;
			Node *Pattern_pNode;
			//printf("parrtern size: %d\n", newPattern->vNodes.size());

			char Pattern_Colour[30];
			switch(newPattern->Pattern_pool_idx%10){
			case 0:{
				sprintf(Pattern_Colour, "blue");
				break;
			}
			case 1:{
				sprintf(Pattern_Colour, "brown1");
				break;
			}
			case 2:{
				sprintf(Pattern_Colour, "cadetblue1");
				break;
			}
			case 3:{
				sprintf(Pattern_Colour, "chartreuse");
				break;
			}
			case 4:{
				sprintf(Pattern_Colour, "chocolate");
				break;
			}
			case 5:{
				sprintf(Pattern_Colour, "crimson");
				break;
			}
			case 6:{
				sprintf(Pattern_Colour, "gold");
				break;
			}
			case 7:{
				sprintf(Pattern_Colour, "green");
				break;
			}
			case 8:{
				sprintf(Pattern_Colour, "hotpink");
				break;
			}
			case 9:{
				sprintf(Pattern_Colour, "orange");
				break;
			}
			}

			for(int j=0;j<newPattern->vNodes.size();j++){
				Pattern_pNode = newPattern->vNodes.at(j);
				if(Pattern_pNode->isPi==0 && Pattern_pNode->isPo==0){
					//printf("Not Fake Node\n");
					switch(Pattern_pNode->NodeOperation){
					case 0:{
						mydotfile << "A" << pNode->NodeIdx << Pattern_pNode->NodeIdx << "[label=\"" << Pattern_pNode->NodeIdx << "@"<< pNode->NodeIdx << "(ADD)" << "\" style=filled color=" << Pattern_Colour<<  "]" << ";\n";
						break;
					}
					case 1:{
						mydotfile << "A" << pNode->NodeIdx << Pattern_pNode->NodeIdx << "[label=\"" << Pattern_pNode->NodeIdx << "@"<< pNode->NodeIdx << "(SUB)" << "\" style=filled color=" << Pattern_Colour<<  "]" << ";\n";
						break;
					}
					case 2:{
						mydotfile << "A" << pNode->NodeIdx << Pattern_pNode->NodeIdx << "[label=\"" << Pattern_pNode->NodeIdx << "@"<< pNode->NodeIdx << "(MULT)" << "\" style=filled color=" << Pattern_Colour<<  "]" << ";\n";
						break;
					}
					case 3:{
						mydotfile << "A" << pNode->NodeIdx << Pattern_pNode->NodeIdx << "[label=\"" << Pattern_pNode->NodeIdx << "@"<< pNode->NodeIdx << "(DIV)" << "\" style=filled color=" << Pattern_Colour<<  "]" << ";\n";
						break;
					}
					default:{
						mydotfile << "A" << pNode->NodeIdx << Pattern_pNode->NodeIdx << "[label=\"" << Pattern_pNode->NodeIdx << "@"<< pNode->NodeIdx << "(" << Pattern_pNode->NodeOperation << ")" << "\" style=filled color=" << Pattern_Colour<<  "]" << ";\n";
						break;
					}
					}
				}else{
					//mydotfile << "A" << pNode->NodeIdx << Pattern_pNode->NodeIdx << "[label=\"" << Pattern_pNode->NodeIdx << "@"<< pNode->NodeIdx << "(" << Pattern_pNode->NodeOperation << ")" << "\" style=filled color=black]" << ";\n";
				}
			}
			//mydotfile << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << " PATTERN\"]" << ";\n";
		}


	}
	while(qNode.size()>0){
		pNode = qNode.front();
		qNode.pop();
		//printf("pNode is Fake? %d\n", pNode->isFake);

		NodeForEachFanin( pNode, pFanin, i ){
			if(pNode->isPattern==0){
				assert(pNode->InputNode.size()<3);
				if(pFanin->isPattern==0){
					mydotfile << pFanin->NodeIdx << "->" << pNode->NodeIdx << ";\n";
				}else{
					NodePattern* FaninPattern = (NodePattern*)pFanin;
					//print_pattern_to_subgraph(&mydotfile, FaninPattern);
					pFaninReal = FaninPattern->FakeOutputNode.at(0)->InputNode.at(0);
					mydotfile << "A" << pFanin->NodeIdx << pFaninReal->NodeIdx << "->" << pNode->NodeIdx << ";\n";
				}
			}else{
				if(pFanin->isPattern==0){
					NodePattern* FanoutPattern = (NodePattern*)pNode;
					pFanoutReal = FanoutPattern->FakeInputNode.at(i);
					NodeForEachFanout(pFanoutReal, pFanout, j){
						mydotfile << pFanin->NodeIdx <<  "->" << "A" << pNode->NodeIdx << pFanout->NodeIdx <<";\n";
					}

				}else{
					NodePattern* FanoutPattern = (NodePattern*)pNode;
					pFanoutReal = FanoutPattern->FakeInputNode.at(i);

					NodePattern* FaninPattern = (NodePattern*)pFanin;

					pFaninReal = FaninPattern->FakeOutputNode.at(0)->InputNode.at(0);;
					NodeForEachFanout(pFanoutReal, pFanout, j){
						mydotfile << "A" << pFanin->NodeIdx << pFaninReal->NodeIdx  <<  "->" << "A" << pNode->NodeIdx << pFanoutReal->NodeIdx <<";\n";
					}

				}
			}

			if(!pFanin->visited){
				if(pFanin->isPattern==1){
					NodePattern* FaninPattern = (NodePattern*)pFanin;
					print_pattern_to_subgraph(&mydotfile, FaninPattern);
				}
				qNode.push(pFanin);
				pFanin->visited=1;
			}

		}

	}
	mydotfile << "}";

	mydotfile.close();
}
void DAGDFGNtk::print_pattern_to_subgraph(ofstream* mydotfile, NodePattern* Pattern){
	int i;
	Node *pNode, *pFanin;
	queue<Node*> qNode;
	//printf("Printing Pattern: %d\n", Pattern->NodeIdx);
	for(i=0;i<Pattern->vNodes.size();i++){
		pNode = Pattern->vNodes.at(i);
		pNode->visited=0;
	}
	for(i=0; i<Pattern->PatternOutputSize; i++){
		pNode = Pattern->FakeOutputNode.at(i);
		qNode.push(pNode);
		pNode->visited = 1;
	}
	while(qNode.size()>0){
		pNode = qNode.front();
		qNode.pop();
		//printf("pNode %d has %d of outputs\n", pNode->NodeIdx, pNode->NumInput);
		NodeForEachFanin( pNode, pFanin, i ){
			if(pFanin->isFake){
				//*mydotfile << "A" << Pattern->NodeIdx <<pFanin->InputNode.at(0)->NodeIdx << "->" << "A" << Pattern->NodeIdx <<pNode->NodeIdx << ";\n";
			}else{
				*mydotfile << "A" << Pattern->NodeIdx <<pFanin->NodeIdx << "->" << "A" << Pattern->NodeIdx <<pNode->NodeIdx << ";\n";
			}

			if(!pFanin->visited){
				qNode.push(pFanin);
				pFanin->visited=1;
			}
		}
	}
}
void DAGDFGNtk::print_ntk_to_subgraph(ofstream* mydotfile, int idx){

	Node *pNode, *newNode, *pFanin;
	int i;
	queue<Node*> qNode;

	NtkForEachNode(this, pNode, i ){
		pNode->visited = 0;
	}
	for (i = 0; i < NumPos; i++) {
		pNode = vPos.at(i);
		qNode.push(pNode);
		pNode->visited = 1;
	}
	*mydotfile << "subgraph cluster" << idx << " {\n";
	*mydotfile << "BB_inv_node_"<< idx << " [style=invisible];\n";
	NtkForEachNode(this, pNode, i ){
		if(pNode->NodeType==0)
			*mydotfile << "BB_"<< idx << "_" << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->level << "(const=" << pNode->NodeValue <<")" << "\"]\n";
		else if(pNode->NodeType==2)
			*mydotfile << "BB_"<< idx << "_" << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->level << "(input)" << "\"]\n";
		else if(pNode->NodeType==3)
			*mydotfile << "BB_"<< idx << "_" << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->level << "(output)" << "\"]\n";
		else{

			switch(pNode->NodeOperation){
			case 0:{
				*mydotfile << "BB_"<< idx << "_" << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->level << "(ADD)" << "\"]\n";
				break;
			}
			case 1:{
				*mydotfile << "BB_"<< idx << "_" << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->level << "(SUB)" << "\"]\n";
				break;
			}
			case 2:{
				*mydotfile << "BB_"<< idx << "_" << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->level << "(MULT)" << "\"]\n";
				break;
			}
			case 3:{
				*mydotfile << "BB_"<< idx << "_" << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->level << "(DIV)" << "\"]\n";
				break;
			}
			default:{
				*mydotfile << "BB_"<< idx << "_" << pNode->NodeIdx << "[label=\"" << pNode->NodeIdx << "@"<< pNode->level << "(" << pNode->NodeOperation << ")" << "\"]\n";
				break;
			}
			}
		}

	}
	while(qNode.size()>0){
		pNode = qNode.front();
		qNode.pop();
		NodeForEachFanin( pNode, pFanin, i ){
			*mydotfile << "BB_"<< idx << "_" << pFanin->NodeIdx << "->" << "BB_"<< idx << "_" << pNode->NodeIdx << ";\n";
			if(!pFanin->visited){
				qNode.push(pFanin);
				pFanin->visited=1;
			}
		}

	}
	*mydotfile << "}";

	//mydotfile.close();
}
void DAGDFGNtk::update_level(){
	int i, assigning_level;
	Node *pNode, *pFanin;
	queue<Node*> qNode;
	//assign the output nodes as level 1

	NtkForEachNode(this, pNode, i ){
		pNode->level=0;
	}
	int changed = 1;
	while(changed){
		changed = 0;
		for (i = 0; i < NumPos; i++) {
			pNode = vPos.at(i);
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
void DAGDFGNtk::update_level_reverse(){
	int i, changed, assigning_level;
	Node *pNode, *pFanout, *Pi;
	queue<Node*> qNode;

	//Resetting all the level information
	NtkForEachNode(this, pNode, i ){
		pNode->level=0;
	}
	changed = 1;
	while(changed){
		changed = 0;
		NtkForEachPi(this, Pi, i ){
			qNode.push(Pi);
		}

		while(qNode.size()>0){
			pNode = qNode.front();
			qNode.pop();

			NodeForEachFanout( pNode, pFanout, i ){
				assigning_level = pFanout->assign_level_reverse();
				if(pFanout->level != assigning_level){
					changed = 1;
					pFanout->level = assigning_level;
				}
				qNode.push(pFanout);
			}
		}
	}
	sort_fanout_by_level();

}
void DAGDFGNtk::reset_visit_info(){
	int i;
	Node *pNode;
	NtkForEachNode(this, pNode, i ){
		pNode->visited = 0;
	}
}

bool fanout_level_cmp_function (Node * i,Node * j) { return (i->level < j->level); }

void DAGDFGNtk::sort_fanout_by_level(){
	int i;
	Node *pNode;
	NtkForEachNode(this, pNode, i ){
		sort (pNode->OutputNode.begin(), pNode->OutputNode.end(), fanout_level_cmp_function);
	}
}
void DAGDFGNtk::sort_by_level_closest(){
	int i;
	Node *pNode;
	NtkForEachNode(this, pNode, i ){
		sort (pNode->OutputNode.begin(), pNode->OutputNode.end(), fanout_level_cmp_function);
	}
}
DAGDFGNtk::~DAGDFGNtk() {
	// TODO Auto-generated destructor stub
}
