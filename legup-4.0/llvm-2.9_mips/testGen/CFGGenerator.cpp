/*
 * CFGGenerator.cpp
 *
 *  Created on: 2012-06-19
 *      Author: fire
 */

#include "CFGGenerator.h"
#include "AutoConfig.h"
static int get_nodeNum(int PiNum, int PoNum, int PatternSize, float PatternRatio, int factor);

CFGGenerator::CFGGenerator(int max_BBNum, int inseed, int numIn, int numOut, int branchNum, float DFactor, int ArrayInput, int EnableSubFunc, AutoConfig * ac) {
	//CFG_Ntk = new CFGNtk(max_BBNum, numIn, numOut, branchNum);
	CFG_Ntk = new CFGNtk(max_BBNum, numIn, numOut, branchNum, ArrayInput, 0, ac);
	//cout << "Generating new CFG AAA\n";
	CFG_Ntk->cfgIdx=ac->CFG_idx;
	ac->CFG_idx=ac->CFG_idx++;
	seed = inseed;
	AC = ac;
	graph_level = 0;
	this->depthFactor = DFactor;
	enableSubFunc = EnableSubFunc;
	replicatedBBs = AC->numreplicatedBBs;
}
void CFGGenerator:: CFG_StartGenRand(){
	//StartGen();
	//return;
	queue<BBlock*> qBBlock;
	BBlock *pBBlock, *preExitBB, *newBB, *BB, *BBFanin, *BBFanout;
	int branchStackCounter = 0;
	int i, j;
	srand(seed);
	preExitBB = new BBlock(2);
	CFG_Ntk->addNewBB(preExitBB);

	qBBlock.push(CFG_Ntk->Entry);
	//printf("CFG_Ntk->numBB: %d\n", CFG_Ntk->numBB);
	srand(seed);
	while (!qBBlock.empty()) {
		pBBlock = qBBlock.front();
		qBBlock.pop();
		if (CFG_Ntk->numBB <= CFG_Ntk->BB_cap) {
			int fanout_num = genFanoutNum(branchStackCounter);
			if(pBBlock->BBType==0)fanout_num = 1;
			//cout << "fanout_num: " << fanout_num << "\n";
			//cout << "pBBlock: " << pBBlock->BBidx << "\n";
			pBBlock->expectedFanoutNumber = fanout_num;
			if(fanout_num>1) branchStackCounter++;
			for (int i = 0; i < fanout_num; i++) {
				float randFVal = ((float)(rand()%100))/100.;
				int randVal = (randFVal < depthFactor)? 1 : 0;	//depthFactor
				if (i == 0 || randVal) {
					//cout << "Generating new node CFG_idx " << AC->CFG_idx <<" maxCFGnum "<< AC->maxCFGnum << " graph_level: "<< graph_level << "\n";
					//if((rand() % (graph_level+2) == 1) && (AC->CFG_idx < AC->maxCFGnum)){
					//	newBB = new BBlock(2, enableSubFunc);
					//}else
					newBB = new BBlock(2, 0);

					CFG_Ntk->addNewBB(newBB);
					addInput(newBB, pBBlock);
					qBBlock.push(newBB);
				} else {
					//cout << "Not Generating new node\n";
				}
			}
		}else{
			addInput(preExitBB, pBBlock);
		}

	}
	//printf("Exit ID: %d\n", CFG_Ntk->Exit->BBidx);
	//printf("1- THE CFGNtk CFG_StartGenRand vPis: %d, vPos: %d\n", CFG_Ntk->Exit->DFG_NtkGenerator->Ntk->vPis.size(), CFG_Ntk->Exit->DFG_NtkGenerator->Ntk->vPos.size());
	addInput(CFG_Ntk->Exit, preExitBB);

	CFG_Ntk->update_level();

	/*
	NtkForEachBBlock(CFG_Ntk, BB, i ){
		printf("Block %d at level %d\n", BB->BBidx, BB->level);
	}
	*/

	NtkForEachBBlock(CFG_Ntk, BB, i ){
		if(BB->expectedFanoutNumber>BB->OutputBB.size()){
			//printf("Need one more output edge for block %d\n", BB->BBidx);
			randAssignFanout(BB);
		}
	}
	//printf("2- THE CFGNtk CFG_StartGenRand vPis: %d, vPos: %d\n", CFG_Ntk->Exit->DFG_NtkGenerator->Ntk->NumPis, CFG_Ntk->Exit->DFG_NtkGenerator->Ntk->NumPos);

	CFG_NtkAnalysis();
	//printf("3- THE CFGNtk CFG_StartGenRand vPis: %d, vPos: %d\n", CFG_Ntk->Exit->inputNodeNum, CFG_Ntk->Exit->outputNodeNum);

	for(int i=0; i<CFG_Ntk->numBranchInput;i++){
		Node *pNode = new Node(2);
		CFG_Ntk->BranchControlInput.push_back(pNode);
	}
	char BB_DOTA[30];
	sprintf(BB_DOTA, "cfg_a_%d.dot", CFG_Ntk->cfgIdx);
	CFG_Ntk->print_cfg_to_dot(BB_DOTA);

	CFG_FindAllDoms();

	int loop_enabled = AC->enable_loop;
	if(CFG_GraphVerification()){

		if(loop_enabled){
			vector<CFGLoop*> qLoopList;
			NtkForEachBBlock(CFG_Ntk, BB, i ){
				if(BB->BBType != 2) continue;

				for(j=0;j<BB->bvDominatorsBB->getSize();j++){
					//BB->bvDominatorsBB->printBV();
					if(BB->bvDominatorsBB->getBit(j)==1){
						pBBlock = CFG_Ntk->vBBlock.at(j);
						//printf("Checking %d and %d\n", BB->BBidx, pBBlock->BBidx);
						if(BB->outputNodeNum == pBBlock->inputNodeNum && pBBlock->InputBB.size()==1 && BB->OutputBB.size()==1 && pBBlock->BBType!=0){
							//printf("Block %d can jump back to Block %d\n", BB->BBidx, pBBlock->BBidx);
							CFGLoop *newLoop = new CFGLoop(BB, pBBlock, (rand()%100)+10);
							qLoopList.push_back(newLoop);
						}
					}
				}
			}

			if(qLoopList.size()){
				int loop_choice = rand()%qLoopList.size();
				CFGLoop *newLoop = qLoopList.at(loop_choice);
				CFG_Ntk->vCFGLoop.push_back(newLoop);
				CFG_Ntk->numLoop++;
			}

		}
		NtkForEachBBlock(CFG_Ntk, BB, i ){
			if(BB->BBType == 2){
				//printf("Generating block %d\n", BB->BBidx);
				int inputSize  = BB->inputNodeNum;
				int outputSize = BB->outputNodeNum;
				int BBsize;
				//printf("AC->block_size_factor: %d\n", AC->block_size_factor);
				//printf("AC->fixed_block_size: %d\n", AC->fixed_block_size);
				if(AC->fixed_block_size){
					BBsize = get_nodeNum(CFG_Ntk->numInput, CFG_Ntk->numOutput, AC->patternSize, AC->pattern_ratio, AC->block_size_factor);
					printf("BBsize1: %d\n", BBsize);
				}else{
					BBsize = get_nodeNum(inputSize, outputSize, AC->patternSize, AC->pattern_ratio, AC->block_size_factor);
					//printf("BBsize2: %d\n", BBsize);
				}
				//printf("previous BBsize: %d\n", (inputSize+outputSize)*2);

				assert(BBsize>0);
				//cout << "Generating new node CFG_idx " << AC->CFG_idx <<" maxCFGnum "<< AC->maxCFGnum << " graph_level: "<< graph_level << "\n";
				int randFactor = (CFG_Ntk->vBBlock.size()/((AC->maxCFGnum/AC->maxSubFuncLevel)+graph_level+1))+1;
				if(AC->enable_sub_funtion && (rand() % randFactor == 1) && (AC->CFG_idx < AC->maxCFGnum && (this->graph_level<=AC->maxSubFuncLevel))){
					BB->use_CFG = 1;
					//printf("Block %d is using CFG\n", BB->BBidx);
					//printf("inputSize: %d, outputSize: %d\n", inputSize, outputSize);
					BB->DFG_NtkGenerator = new DFGGenerator(BBsize, seed+BB->BBidx, inputSize, outputSize, depthFactor, 0, 0, AC);
					//printf("inputSize: %d, outputSize: %d\n", BB->DFG_NtkGenerator->Ntk->NumPis, BB->DFG_NtkGenerator->Ntk->NumPos);
					BB->DFG_NtkGenerator->DFG_StartGen_fake();
					CFGGenerator *temp_cfg_generator;
					//printf("graph_level: %d, maxSubFuncLevel: %d\n", graph_level, AC->maxSubFuncLevel);
					if(this->graph_level<AC->maxSubFuncLevel){
						temp_cfg_generator = new CFGGenerator(2*(inputSize+outputSize), seed+BB->BBidx, inputSize, outputSize, 0, depthFactor, 0, 1, AC);
					}else{
						temp_cfg_generator = new CFGGenerator(2*(inputSize+outputSize), seed+BB->BBidx, inputSize, outputSize, 0, depthFactor, 0, 0, AC);
					}
					temp_cfg_generator->graph_level=this->graph_level+1;
					if(temp_cfg_generator->graph_level > AC->CFG_DEPTH) AC->CFG_DEPTH = temp_cfg_generator->graph_level;
					//printf("Creating CFG for block: %d at level %d, idx %d \n", BB->BBidx, temp_cfg_generator->graph_level, temp_cfg_generator->CFG_Ntk->cfgIdx);
					BB->CFG_NtkGenerator = temp_cfg_generator;

					((CFGGenerator*)BB->CFG_NtkGenerator)->CFG_StartGenRand();
					BB->CFG_Ntk = ((CFGGenerator*)BB->CFG_NtkGenerator)->CFG_Ntk;
					//printf("CFG for this Block %d has %d blocks\n", BB->BBidx, ((CFGNtk*)(BB->CFG_Ntk))->vBBlock.size());
				}else{
					//printf("replicatedBBs: %d\n", replicatedBBs);
					if(replicatedBBs==0){
						BB->use_CFG = 0;
						BB->DFG_NtkGenerator = new DFGGenerator(BBsize, seed+BB->BBidx, inputSize, outputSize, this->depthFactor, AC);
						AC->addDFGPattern(2*(inputSize+outputSize), seed+BB->BBidx, inputSize, outputSize, this->depthFactor);
						BB->DFG_NtkGenerator->DFG_StartGen();

					}else{
						//printf("Replicating BB %d\n", BB->BBidx);
						BB->use_CFG = 0;
						BB->DFG_NtkGenerator = new DFGGenerator(BBsize, seed, inputSize, outputSize, this->depthFactor, AC);
						AC->addDFGPattern(2*(inputSize+outputSize), seed, inputSize, outputSize, this->depthFactor);
						BB->DFG_NtkGenerator->DFG_StartGen();
						replicatedBBs--;
					}
					//BB->use_CFG = 0;
					//BB->DFG_NtkGenerator = new DFGGenerator(BBsize, seed+BB->BBidx, inputSize, outputSize, this->depthFactor, AC);
					//AC->addDFGPattern(2*(inputSize+outputSize), seed+BB->BBidx, inputSize, outputSize, this->depthFactor);
					//BB->DFG_NtkGenerator->DFG_StartGen();
					//printf("Block %d Input size %d, expected: %d\n", BB->BBidx, BB->DFG_NtkGenerator->Ntk->vPis.size(), BB->inputNodeNum);
					//printf("Block %d Output size %d, expected: %d\n", BB->BBidx, BB->DFG_NtkGenerator->Ntk->vPos.size(), BB->outputNodeNum);
				}

				BB->hasGraph = 1;
				//printf("Generating block %d Done\n", BB->BBidx);
			}
		}
	}


	assert(CFG_GraphVerification()==1);
	assert(CFG_Ntk->CFG_NtkVerification()==1);
	statsAnalysis();

}
static int get_nodeNum(int PiNum, int PoNum, int PatternSize, float PatternRatio, int factor){
	//printf("PiNum: %d PoNum: %d PatternSize: %d PatternRatio: %f factor: %d\n", PiNum, PoNum, PatternSize, PatternRatio, factor);
	int A = factor*(PiNum+PoNum)*PatternSize;
	float B = PatternRatio+PatternSize-PatternRatio*PatternSize;
	int result = A/B;
	return result;
}
int CFGGenerator:: genFanoutNum(int branchStackCounter){
	if(rand()%(branchStackCounter+1) == 0){
		return 2;
	}else{
		return 1;
	}
}
void CFGGenerator:: CFG_NtkAnalysis(){
	BBlock *pBBlock, *BBFanin, *BB;
	int i;
	queue<BBlock*> qBBlock;
	CFG_Ntk->Exit->inputNodeNum = CFG_Ntk->numOutput;
	CFG_Ntk->Exit->outputNodeNum = CFG_Ntk->numOutput;
	CFG_Ntk->Entry->inputNodeNum = CFG_Ntk->numInput;
	CFG_Ntk->Entry->outputNodeNum = CFG_Ntk->numInput;

	CFG_Ntk->reset_visit();
	qBBlock.push(CFG_Ntk->Exit);
	CFG_Ntk->Exit->visited = 1;
	while (!qBBlock.empty()) {
		pBBlock = qBBlock.front();
		qBBlock.pop();
		if(pBBlock->inputNodeNum>0){
			BBForEachFanin( pBBlock, BBFanin, i ){
				BBFanin->outputNodeNum = pBBlock->inputNodeNum;
				if(!BBFanin->visited){
					qBBlock.push(BBFanin);
					BBFanin->visited = 1;
				}
			}
		}else{
			assignInputNodeNum(pBBlock);
			BBForEachFanin( pBBlock, BBFanin, i ){
				if(!BBFanin->visited){
					qBBlock.push(BBFanin);
					BBFanin->visited = 1;
				}
			}
		}
	}
	int branchNum = 0;
	NtkForEachBBlock(CFG_Ntk, BB, i ){
		if(BB->OutputBB.size()>1){
			branchNum++;
		}
	}
	CFG_Ntk->numBranchInput = branchNum;
}

//This method finds all the dominators of the all the BBs.

void CFGGenerator::CFG_FindAllDoms(){
	int i, j;
	BBlock *BB, *BBFanin;
	NtkForEachBBlock(CFG_Ntk, BB, i ){
		BB->bvDominatorsBB = new Bitvector(CFG_Ntk->numBB);
		//printf("Creating DON BV for Block %d, with the size %d\n", BB->BBidx, BB->bvDominatorsBB->getSize());
		if(BB->BBType==0){
			BB->bvDominatorsBB->setBit(BB->BBidx, 1);
		}else{
			BB->bvDominatorsBB->setAllOnes();
		}

	}
	int changed = 1;
	//printf("Gets Here\n");
	while(changed){
		changed = 0;
		NtkForEachBBlock(CFG_Ntk, BB, i ){
			//printf("Gets Here - %d\n", changed);
			if(BB->BBType>1){
				Bitvector* newDOM = new Bitvector(CFG_Ntk->numBB);
				newDOM->setAllOnes();
				BBForEachFanin( BB, BBFanin, j ){
					//printf("newDOM size: %d, FaninDom size: %d\n", newDOM->getSize(), BBFanin->bvDominatorsBB->getSize());
					//printf("Fanin Type: %d, ID: %d\n", BBFanin->BBType, BBFanin->BBidx);
					newDOM = newDOM->BVand(newDOM, BBFanin->bvDominatorsBB);
				}
				newDOM->setBit(BB->BBidx, 1);
				int same = BB->bvDominatorsBB->BVsame(newDOM);
				if(!same){
					BB->bvDominatorsBB->BVcopy(newDOM);
					changed = 1;
				}
			}
		}

	}
	/*
	NtkForEachBBlock(CFG_Ntk, BB, i ){
		//BB->bvDominatorsBB->printBV();
		printf("Block %d's DOMs: ", BB->BBidx);

		for(j=0;j<BB->bvDominatorsBB->getSize(); j++){
			if(BB->bvDominatorsBB->getBit(j)==1){
				printf("%d ", j);
			}
		}
		printf("\n");
	}

	*/
}

int CFGGenerator:: CFG_GraphVerification(){
	int i, j;
	BBlock *BB, *BBFanin, *BBFanout;
	NtkForEachBBlock(CFG_Ntk, BB, i ){
		int inNodeNum = BB->inputNodeNum;
		BBForEachFanin( BB, BBFanin, j ){
			if(BBFanin->outputNodeNum!=inNodeNum) {
				printf("[GRAPH ERROR]Block %d's input node number does not match output number at block %d\n", BB->BBidx, BBFanin->BBidx);
				//assert(BBFanin->outputNodeNum ==inNodeNum);
				return 0;
			}
		}
		int outNodeNum = BB->outputNodeNum;
		BBForEachFanout( BB, BBFanout, j ){
			if(BBFanout->inputNodeNum!=outNodeNum) {
				printf("[GRAPH ERROR]Block %d's output node number does not match input number at block %d\n", BB->BBidx, BBFanin->BBidx);
				//assert(BBFanout->inputNodeNum==outNodeNum);
				return 0;
			}
		}
	}

	return 1;
}
void CFGGenerator:: assignInputNodeNum(BBlock *BBin){
	BBlock *BBFanin, *BBFanout, *BB;
	int i, j;
	int changed;
	queue<BBlock*> qInputBBlock;
	queue<BBlock*> qOutputBBlock;
	int fixed_at = -1;
	BBForEachFanin( BBin, BBFanin, i ){
		if(BBFanin->outputNodeNum>0){
			fixed_at = BBFanin->outputNodeNum;
			break;
		}
	}
	if(fixed_at<0){
		if(AC->enable_bb_patterns)
			fixed_at = this->CFG_Ntk->numInput;
		else
			fixed_at = (rand()%CFG_Ntk->numInput)+1;
	}
	changed = 1;
	qOutputBBlock.push(BBin);
	while(changed){
		changed = 0;
		while(qOutputBBlock.size()){
			BB = qOutputBBlock.front();
			qOutputBBlock.pop();
			BBForEachFanin( BB, BBFanin, i ){
				if(BBFanin->outputNodeNum != fixed_at){
					changed = 1;
					BBFanin->outputNodeNum = fixed_at;
				}
				qInputBBlock.push(BBFanin);
			}
		}
		while(qInputBBlock.size()){
			BB = qInputBBlock.front();
			qInputBBlock.pop();
			BBForEachFanout( BB, BBFanout, i ){
				if(BBFanout->inputNodeNum != fixed_at){
					changed = 1;
					BBFanout->inputNodeNum = fixed_at;
				}
				qOutputBBlock.push(BBFanout);
			}
		}

	}

	/*
	printf("Assign %d to Block %d input\n", fixed_at, BBin->BBidx);
	BBin->inputNodeNum = fixed_at;
	BBForEachFanin( BBin, BBFanin, i ){
		printf("Assign %d to Block %d output\n", fixed_at, BBFanin->BBidx);
		BBFanin->outputNodeNum = fixed_at;
		BBForEachFanout( BBFanin, BBFanout, j ){
			printf("Assign %d to Block %d input\n", fixed_at, BBFanout->BBidx);
			BBFanout->inputNodeNum = fixed_at;
		}
	}
	*/
}
void CFGGenerator:: randAssignFanout(BBlock *BBin){
	int i;
	BBlock *BB, *tarBB;;
	vector<BBlock*> AvailableBBList;
	//srand(seed);
	NtkForEachBBlock(CFG_Ntk, BB, i ){
		if(BB->level<(BBin->level-1) && BB->level!=0){
			AvailableBBList.push_back(BB);
		}
	}

	if(AvailableBBList.size()>0){
		int idx = rand() % AvailableBBList.size();
		tarBB = AvailableBBList.at(idx);
		//printf("Assigning BB %d to BB %d\n", BBin->BBidx, tarBB->BBidx);
		addInput(tarBB, BBin);
	}else{
		//printf("No Available BB for BB %d\n", BBin->BBidx);
	}
}
void CFGGenerator:: StartGen(){
	printf("Start Generating\n");
	BBlock *BB0 = new BBlock(2);
	BB0->DFG_NtkGenerator = new DFGGenerator(5, seed, 2, 2, 0.4, AC);
	BB0->DFG_NtkGenerator->DFG_StartGen();
	BB0->hasGraph = 1;

	BBlock *BB1 = new BBlock(2);
	BB1->DFG_NtkGenerator = new DFGGenerator(5, seed+1, 2, 2, 0.4, AC);
	BB1->DFG_NtkGenerator->DFG_StartGen();
	BB1->hasGraph = 1;

	BBlock *BB2 = new BBlock(2);
	BB2->DFG_NtkGenerator = new DFGGenerator(5, seed+1, 2, 2, 0.4, AC);
	BB2->DFG_NtkGenerator->DFG_StartGen();
	BB2->hasGraph = 1;

	BBlock *BB3 = new BBlock(2);
	BB3->DFG_NtkGenerator = new DFGGenerator(5, seed+1, 2, 1, 0.4, AC);
	BB3->DFG_NtkGenerator->DFG_StartGen();
	BB3->hasGraph = 1;


	CFG_Ntk->addNewBB(BB0);
	CFG_Ntk->addNewBB(BB1);
	CFG_Ntk->addNewBB(BB2);
	CFG_Ntk->addNewBB(BB3);

	BBlock *EntryBB = CFG_Ntk->Entry;
	BBlock *ExitBB = CFG_Ntk->Exit;
	this->addInput(BB0, EntryBB);
	this->addInput(BB1, BB0);
	this->addInput(BB2, BB0);
	this->addInput(BB3, BB2);
	this->addInput(BB3, BB1);
	this->addInput(ExitBB, BB3);
	//this->addInput(BB0, BB1);
	//Add the Loop to the CFG
	CFGLoop *newLoop = new CFGLoop(BB2, BB0, 10);
	CFG_Ntk->vCFGLoop.push_back(newLoop);
	CFG_Ntk->numLoop++;

	//Creating the nodes for breanch control inputs
	for(int i=0; i<CFG_Ntk->numBranchInput;i++){
		Node *pNode = new Node(2);
		CFG_Ntk->BranchControlInput.push_back(pNode);
	}

	printf("Done\n");
}

//Assign inNode as fanin to tarNode as well as updating NumInput
int CFGGenerator::addInput(BBlock* tarBB, BBlock* inBB) {
	tarBB->InputBB.push_back(inBB);
	inBB->OutputBB.push_back(tarBB);
	return 1;
}

void CFGGenerator::statsAnalysis(){
	int i, j;
	BBlock *BB;
	Node *pNode;
	map<int, int> counter;

	NtkForEachBBlock(CFG_Ntk, BB, i ){
		if(BB->use_CFG) continue;
		if(BB->BBType==2){
			assert(BB->hasGraph);
			NtkForEachNode(BB->DFG_NtkGenerator->Ntk, pNode, j ){
				if(pNode->NodeType==1){
					//This node is an operation
					counter[pNode->NodeOperation]++;
				}
			}
		}
	}
	//cout << "size: " << counter.size() <<endl;
	/*
	for( map<int,int>::iterator ii=counter.begin(); ii!=counter.end(); ++ii){
	       cout << (*ii).first << ": " << (*ii).second << endl;
	}
	*/
}
