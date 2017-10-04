/*
 * Node.cpp
 *
 *  Created on: 2012-05-21
 *      Author: fire
 */
#include "Node.h"
#include "NodePattern.h"

using namespace std;
Node::Node(){

}
Node::Node(int Type) {
	isPattern = 0;
	nodeSize = 1;
	NumInput = 0;
	NumOutput = 0;
	NodeType = Type;
	visited = 0;
	isPo = 0;
	isPi = 0;
	NodeLLVMValueCreated = 0;
	isFake = 0;
	//printf("Gets Here 2\n");
}
int Node:: isConst(){
	return (NodeType==0);
}
int Node:: getExpectedNumInput(){
	int Num;
	if(this->isPattern){
		NodePattern* Pattern = (NodePattern*)this;
		return Pattern->PatternInputSize;
	}
	if(isConst()){
		return 0;
	}
	if(isPo){
		return 1;
	}
	switch(NodeOperation)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:{
			Num = 2;
			break;
		}
		default:{
			assert(1);
			Num = 1;
			break;
		}
	}

	return Num;
}
int Node:: assign_level(){
	int max_level = 0;
	int i;
	Node *pFanout;
	NodeForEachFanout( this, pFanout, i ){
		if(pFanout->level>max_level)
			max_level = pFanout->level;
	}
	return max_level+1;
}
int Node:: assign_level_reverse(){
	int max_level = 0;
	int i;
	Node *pFanin;
	NodeForEachFanin( this, pFanin, i ){
		if(pFanin->level>max_level)
			max_level = pFanin->level;
	}
	return max_level+1;
}

