/*
 * GraphAnalysis.cpp
 *
 *  Created on: 2012-09-14
 *      Author: fire
 */

#include "GraphAnalysis.h"

namespace llvm {
string getOperationName(Node* pNode);

GraphAnalysis::GraphAnalysis(AutoConfig* ACin) {
	int j;
	OperationInfo *oi;
	AC = ACin;

	AFForEachOperation( ACin, oi, j ){
		OperationCounter[oi->optype]=0;
	}
	//printf("in total, there are %d different operations used \n", OperationCounter.size());

}
void GraphAnalysis::StartAnalysis(CFGNtk *CFG_Ntk){
	int i;
	BBlock *BB;
	NtkForEachBBlock(CFG_Ntk, BB, i ){
		if(BB->use_CFG==0){
			DAGDFGNtk *DFG_Ntk = BB->DFG_NtkGenerator->Ntk;
			DFGAnalysis(DFG_Ntk);
		}else{

		}
	}
}
void GraphAnalysis::DFGAnalysis(DAGDFGNtk *DFG_Ntk){
	queue<Node*> qNode;
	Node* Pi, *pNode, *pFanout;
	int i;
	//DFG_Ntk->reset_visit_info();
	int opNum=0;
	NtkForEachNode(DFG_Ntk, pNode, i ){
		if(pNode->NodeType==1){
			opNum++;
			string OpName = getOperationName(pNode);
			OperationCounter[OpName]++;
		}
	}
	//printf("opNum: %d\n", opNum);
	/*
	NtkForEachPi(DFG_Ntk, Pi, i ){
		qNode.push(Pi);
		Pi->visited = 1;
	}
	int opNum=0;
	while(qNode.size()){
		pNode = qNode.front();
		qNode.pop();
		NodeForEachFanout( pNode, pFanout, i ){
			if(pFanout->visited==0){
				pFanout->visited = 1;
				if(pFanout->NodeType==1){
					opNum++;
					//this is a operation
					string OpName = getOperationName(pNode);
					OperationCounter[OpName]++;
				}
				qNode.push(pFanout);
			}
		}
	}
	//printf("opNum: %d\n", opNum);

	*/
}
void GraphAnalysis::PrintAnalysisReuslt(){
	for( map<string,int>::iterator ii=OperationCounter.begin(); ii!=OperationCounter.end(); ++ii){
		cout << (*ii).first << ": " << (*ii).second << endl;
	}
}
string getOperationName(Node* pNode){
	string name;
	switch(pNode->NodeOperation){
	case 0:{
		name = "ADD";
		break;
	}
	case 1:{
		name = "SUB";
		break;
	}
	case 2:{
		name = "DIV";
		break;
	}
	case 3:{
		name = "MULT";
		break;
	}
	case 4:{
		name = "LADD";
		break;
	}
	case 5:{
		name = "LSUB";
		break;
	}
	case 6:{
		name = "LDIV";
		break;
	}
	case 7:{
		name = "LMULT";
		break;
	}
	case 8:{
		name = "FADD";
		break;
	}
	case 9:{
		name = "FSUB";
		break;
	}
	case 10:{
		name = "FDIV";
		break;
	}
	case 11:{
		name = "LMULT";
		break;
	}
	case 12:{
		name = "DADD";
		break;
	}
	case 13:{
		name = "DSUB";
		break;
	}
	case 14:{
		name = "DDIV";
		break;
	}
	case 15:{
		name = "DMULT";
		break;
	}
	case 16:{
		name = "SHL";
		break;
	}
	case 17:{
		name = "LSHR";
		break;
	}
	case 18:{
		name = "ASHR";
		break;
	}
	case 19:{
		name = "LSHL";
		break;
	}
	case 20:{
		name = "LLSHR";
		break;
	}
	case 21:{
		name = "LASHR";
		break;
	}
	default:{
		printf("invalid operation: %d\n", pNode->NodeOperation);
		assert(1);
		break;
	}
	}
	return name;
}
GraphAnalysis::~GraphAnalysis() {
	// TODO Auto-generated destructor stub
}

} /* namespace llvm */
