/*
 * CFGNtk.cpp
 *
 *  Created on: 2012-06-23
 *      Author: fire
 */

#include "CFGNtk.h"

CFGNtk::CFGNtk(int max_BBNum, int numIn, int numOut, int numBranchIn, AutoConfig * ac) {
	int i;
	Node *pNode;
	numInput = numIn;
	numOutput = numOut;
	//numBranchInput = numBranchIn;
	numBranchCreated = 0;
	cfgAC = ac;

	BB_cap = max_BBNum;
	Entry = new BBlock(0);
	Entry->BBidx = 0;
	ArrayIn = 0;
	ArrayOut = 0;
	//Creating input nodes for the Entry block
	//printf("Gets Here\n");
	Entry->DFG_NtkGenerator = new DFGGenerator();
	//printf("Gets Here 1\n");
	for(i=0;i<numIn;i++){
		pNode = new Node(2);
		Entry->DFG_NtkGenerator->Ntk->add_Pi(pNode);
		pNode = new Node(3);
		Entry->DFG_NtkGenerator->Ntk->add_Po(pNode);
	}

	Exit = new BBlock(1);
	Exit->BBidx = 1;

	//Creating output nodes for the Exit block
	Exit->DFG_NtkGenerator = new DFGGenerator();
	for(i=0;i<numOut;i++){
		pNode = new Node(2);
		Exit->DFG_NtkGenerator->Ntk->add_Pi(pNode);
		pNode = new Node(3);
		Exit->DFG_NtkGenerator->Ntk->add_Po(pNode);
	}
	vBBlock.push_back(Entry);
	vBBlock.push_back(Exit);
	numBB = 2;
	numLoop = 0;

}
CFGNtk::CFGNtk(int max_BBNum, int numIn, int numOut, int numBranchIn, int ArrayInput, int ArrayOutput, AutoConfig * ac){
	int i;
	Node *pNode;
	numInput = numIn;
	numOutput = numOut;
	//numBranchInput = numBranchIn;
	numBranchCreated = 0;

	BB_cap = max_BBNum;
	Entry = new BBlock(0);
	Entry->BBidx = 0;
	ArrayIn = ArrayInput;
	ArrayOut = ArrayOutput;
	cfgAC = ac;
	//Creating input nodes for the Entry block
	//Entry->DFG_NtkGenerator = new DFGGenerator();	//The parameters here doesn't take effect.
	Entry->DFG_NtkGenerator = new DFGGenerator(5, 1, numIn, numIn, 0.4, ArrayInput, ArrayOutput, cfgAC);	//The parameters here doesn't take effect.
	Entry->DFG_NtkGenerator->Ntk->isArrayInput = ArrayInput;
	Entry->DFG_NtkGenerator->Ntk->isArrayOutput = ArrayOutput;

	//printf("Gets Here 1\n");
	if(!ArrayInput){
		for(i=0;i<numIn;i++){
			pNode = new Node(2);
			Entry->DFG_NtkGenerator->Ntk->add_Pi(pNode);
			pNode = new Node(3);
			Entry->DFG_NtkGenerator->Ntk->add_Po(pNode);
		}
	}else{
		for(i=0;i<numIn;i++){
			pNode = new Node(2);
			Entry->DFG_NtkGenerator->Ntk->add_array_Pi(pNode);
			pNode = new Node(3);
			Entry->DFG_NtkGenerator->Ntk->add_Po(pNode);
		}
	}


	Exit = new BBlock(1);
	Exit->BBidx = 1;

	//Creating output nodes for the Exit block
	Exit->DFG_NtkGenerator = new DFGGenerator(5, 1, numOut, numOut, 0.4, cfgAC);
	//Exit->DFG_NtkGenerator = new DFGGenerator();
	for(i=0;i<numOut;i++){
		pNode = new Node(2);
		Exit->DFG_NtkGenerator->Ntk->add_Pi(pNode);
		pNode = new Node(3);
		Exit->DFG_NtkGenerator->Ntk->add_Po(pNode);
	}
	//printf("THE CFGNtk CREATOR vPis: %d, vPos: %d\n", Exit->DFG_NtkGenerator->Ntk->vPis.size(), Exit->DFG_NtkGenerator->Ntk->vPos.size());
	vBBlock.push_back(Entry);
	vBBlock.push_back(Exit);
	numBB = 2;
	numLoop = 0;
}
void CFGNtk::addNewBB(BBlock *BBin) {
	this->vBBlock.push_back(BBin);
	BBin->BBidx = this->numBB;
	this->numBB++;
}
void CFGNtk::print_cfg_to_dot(char* fileName) {
	ofstream mydotfile;
	BBlock *BB, *BBFanin;
	CFGLoop * Loop;
	int i;
	queue<BBlock*> qBB;
	NtkForEachBBlock(this, BB, i ) {
		BB->visited = 0;
	}
	mydotfile.open(fileName);
	mydotfile << "digraph G" << " {\n";
	mydotfile << "compound=true\n";
	NtkForEachBBlock(this, BB, i ) {
		if (BB->hasGraph && BB->use_CFG==0)
			BB->DFG_NtkGenerator->Ntk->print_ntk_to_subgraph(&mydotfile,
					BB->BBidx);
		else {
			mydotfile << "subgraph cluster" << BB->BBidx << " {\n";
			//mydotfile << "BB_inv_node_" << BB->BBidx << " [style=invisible];\n";
			mydotfile << "BB_inv_node_" << BB->BBidx << "\n";
			mydotfile << "}\n";
		}

	}

	qBB.push(this->Exit);
	while (qBB.size() > 0) {
		BB = qBB.front();
		qBB.pop();

		BBForEachFanin( BB, BBFanin, i ) {
			mydotfile << "BB_inv_node_" << BBFanin->BBidx << "->"
					<< "BB_inv_node_" << BB->BBidx << "[label=\"" << BBFanin->outputNodeNum << "_"<< BB->inputNodeNum << "\"]" <<" [ltail=cluster"
					<< BBFanin->BBidx << ",lhead=cluster" << BB->BBidx
					<< "];\n";
			//qBB.push(this->Entry);
			//printf("pushed %d\n", BB->BBidx);
			if (!BBFanin->visited) {
				qBB.push(BBFanin);

				BBFanin->visited = 1;
			}
		}
	}
	NtkForEachLoop(this, Loop, i ) {
		mydotfile << "BB_inv_node_" << Loop->BackEdgeHead->BBidx << "->"
				<< "BB_inv_node_" << Loop->BackEdgeTail->BBidx
				<< " [ltail=cluster" << Loop->BackEdgeHead->BBidx
				<< ",lhead=cluster" << Loop->BackEdgeTail->BBidx << "];\n";
	}
	mydotfile << "}\n";
}
Function* CFGNtk::makeLLVMModulefromSUBCFGNtk(Module* mod, CFGNtk* cfgGraph, int uniqueFuncID) {
	//For the test function
	Node *pNode, *Pi, *Const, *Po;
	BBlock *BB, *BBFanin, *BBFanout, *LoopTailBB, *LoopHeadBB;
	CFGLoop * cfgLoop;
	int i,j;
	//Creating the function arguments
	//printf("cfgGraph ID: %d Start\n", cfgGraph->cfgIdx);
	std::vector<Type*> testFuncTy_args;
	if(cfgGraph->ArrayIn){
		for(i=0;i<cfgGraph->ArrayIn;i++){
			PointerType* PointerTy_1 = PointerType::get(IntegerType::get(mod->getContext(), 32), 0);
			testFuncTy_args.push_back(PointerTy_1);
		}
	}else{
		NtkForEachPi(cfgGraph->Entry->DFG_NtkGenerator->Ntk, Pi, i ) {
			testFuncTy_args.push_back(IntegerType::get(mod->getContext(), 32));
		}
	}
	for(i=0; i<cfgGraph->numBranchInput;i++){
		testFuncTy_args.push_back(IntegerType::get(mod->getContext(), 32));
	}
	//Done with Creating the function arguments

	//Creating the function type
	FunctionType* testFuncTy_0;
	if(cfgGraph->numOutput==1){
		testFuncTy_0 = FunctionType::get(
			/*Result=*/IntegerType::get(mod->getContext(), 32),
			/*Params=*/testFuncTy_args,
			/*isVarArg=*/false);
	}else{
		PointerType* PointerTy_2 = PointerType::get(IntegerType::get(mod->getContext(), 32), 0);
		testFuncTy_0 = FunctionType::get(
			/*Result=*/PointerTy_2,
			/*Params=*/testFuncTy_args,
			/*isVarArg=*/false);
	}
	//Done with Creating the function type

	// Function Declaration
	char FuncName[20];
	sprintf(FuncName, "testFunc_%d_%d", cfgGraph->cfgIdx, uniqueFuncID);
	Function* func_test = mod->getFunction(FuncName);
	if (!func_test) {

		func_test = Function::Create(
		/*Type=*/testFuncTy_0,
		/*Linkage=*/GlobalValue::ExternalLinkage,
		/*Name=*/FuncName, mod);
		func_test->setCallingConv(CallingConv::C);
	}

	//Create constants
	ConstantInt* const_int32;
	//printf("Number of blocks in cfgGraph: %d\n", cfgGraph->vBBlock.size());
	NtkForEachBBlock(cfgGraph, BB, i ){
		//printf("Processing BB: %d\n", BB->BBidx);
		if(BB->use_CFG) continue;
		NtkForEachConst(BB->DFG_NtkGenerator->Ntk, Const, j ){
			char ConstVal[20];
			sprintf(ConstVal, "%d", Const->NodeValue);
			const_int32 = ConstantInt::get(mod->getContext(), APInt(32, StringRef(ConstVal), 10));
			Const->NodeLLVMValue = const_int32;
			Const->NodeLLVMValueCreated = 1;
			BB->vConstantInt_in_BB.push_back(const_int32);
		}
	}

	//Useful Constants
	cfgGraph->const_zero = ConstantInt::get(mod->getContext(), APInt(32, StringRef("0"), 10));
	cfgGraph->const_one = ConstantInt::get(mod->getContext(), APInt(32, StringRef("1"), 10));
	cfgGraph->const_one_64 = ConstantInt::get(mod->getContext(), APInt(64, StringRef("1"), 10));

	//Function: testFunc
	{
		//Create all the BasicBlocks
		NtkForEachBBlock(cfgGraph, BB, i ){
			char BB_name[20];
			sprintf(BB_name, "BB_%d_%d_%d", BB->BBidx, uniqueFuncID, cfgGraph->cfgIdx);
			//printf("Created Block: %s\n", BB_name);
			BB->LLVM_BB = BasicBlock::Create(mod->getContext(), BB_name, func_test,0);
		}
		queue<BBlock*> qBBlock;
		//Going though all the blocks in order
		qBBlock.push(cfgGraph->Entry);
		//printf("Entry block %d\n", cfgGraph->Entry->BBidx);
		int can_create;
		while (qBBlock.size()){
			//printf("in While\n");
			BB = qBBlock.front();
			qBBlock.pop();
			//printf("Popped block %d\n", BB->BBidx);
			can_create = 0;
			//printf("Checking for block %d, qBBlock size: %d\n", BB->BBidx, qBBlock.size());
			if(BB->allPredecessorCreated()){
				can_create = 1;
				//Create LLVM for the single Block BB
				if(BB->LLVM_created == 0){
					//printf("Creating LLVM for block %d\n", BB->BBidx);
					if(BB->use_CFG){
						//TODO

						CFGNtk* temp_CFG = (CFGNtk*)(BB->CFG_Ntk);
						BB->func_ptr = cfgGraph->makeLLVMModulefromSUBCFGNtk(mod, temp_CFG, BB->BBidx);

						assert(BB->InputBB.size()>0);
						if(BB->InputBB.size()==1){
							BBFanin = BB->InputBB.at(0);
							NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, i ) {
								Po = BBFanin->DFG_NtkGenerator->Ntk->vPos.at(i);
								Pi->NodeLLVMValue = Po->NodeLLVMValue;
								Pi->NodeLLVMValueCreated = 1;
							}
						}else if (BB->InputBB.size()>1){
							NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, i ) {
								PHINode* int32_p = PHINode::Create(
									IntegerType::get(mod->getContext(), 32), 2, "",
									BB->LLVM_BB);
								Pi->NodeLLVMValue = int32_p;
								Pi->NodeLLVMValueCreated = 1;
							}
							BBForEachFanin( BB, BBFanin, i ) {
								NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, j ) {
									Po = BBFanin->DFG_NtkGenerator->Ntk->vPos.at(j);
									PHINode* int32_phi = (PHINode*) Pi->NodeLLVMValue;
									assert(Po->NodeLLVMValue!=NULL);
									Type* OpType = Po->NodeLLVMValue->getType();
									Value* new_op_0;
									if(int32_phi->getType()!=OpType){
										new_op_0 = create_convert_instr(mod, Po->NodeLLVMValue, int32_phi->getType(), BBFanin->LLVM_BB);
									}else{
										new_op_0 = Po->NodeLLVMValue;
									}

									int32_phi->addIncoming(new_op_0, BBFanin->LLVM_BB);
								}
							}
						}
						//If this is a CFG instead, make the call to the funtion first.
						CFGNtk* current_cfgGraph = (CFGNtk*)BB->CFG_Ntk;
						std::vector<Value*> int32_params;

						for (i = 0; i < BB->inputNodeNum; i++) {
							Pi = BB->DFG_NtkGenerator->Ntk->vPis.at(i);
							Type* OpType = IntegerType::get(mod->getContext(), 32);
							Value* new_op;
							if(Pi->NodeLLVMValue->getType()!=OpType){
								new_op = create_convert_instr(mod, Pi->NodeLLVMValue, OpType, BB->LLVM_BB);
								//printf("Types not match!!\n");
							}else{
								new_op = Pi->NodeLLVMValue;
							}
							int32_params.push_back(new_op);
						}
						//printf("numBranchInput: %d\n", current_cfgGraph->numBranchInput);
						for (i = 0; i < current_cfgGraph->numBranchInput; i++) {
							char ConstVal[20];
							sprintf(ConstVal, "%d", rand() % 2);
							ConstantInt* const_input = ConstantInt::get(
									mod->getContext(),
									APInt(32, StringRef(ConstVal), 10));
							//printf("pushing branch arg: \n");
							int32_params.push_back(const_input);
						}
						CallInst* int32_26 = CallInst::Create(BB->func_ptr, int32_params, "", BB->LLVM_BB);
						//printf("CallInst Type %d, number of output is %d\n", int32_26->getType(), BB->outputNodeNum);
						//Then handle the returned values.
						if(BB->outputNodeNum==1){
							BB->DFG_NtkGenerator->Ntk->vPos.at(0)->NodeLLVMValue=int32_26;
						}else{
							for (i = 0; i < BB->outputNodeNum; i++) {
								//printf("creating output for idx %d\n", i);
								char ConstVal[20];
								//sprintf(ConstVal, "%d", BB->outputNodeNum*4);
								sprintf(ConstVal, "%d", i);
								//printf("creating output for idx %s\n", ConstVal);
								ConstantInt* const_idx = ConstantInt::get(mod->getContext(), APInt(32, StringRef(ConstVal), 10));
								GetElementPtrInst* ptr_mem = GetElementPtrInst::Create(int32_26, const_idx, "", BB->LLVM_BB);

								BB->DFG_NtkGenerator->Ntk->vPos.at(i)->NodeLLVMValue = new LoadInst(ptr_mem, "", false, BB->LLVM_BB);
							}
						}
						//printf("DONE calling makeLLVMModulefromSUBCFGNtk for Block: %d\n", BB->BBidx);
						BB->LLVM_created=1;
					}else{
						cfgGraph->makeLLVMModulefromBasicBlock(mod, func_test, BB);
					}

				}
				BBForEachFanout(BB, BBFanout, i ){
					if(BBFanout->LLVM_created == 0){
						//printf("Pushing1 back block %d\n", BBFanout->BBidx);
						qBBlock.push(BBFanout);
					}
				}
			}else{
				//printf("Pushing2 back block %d\n", BB->BBidx);
				qBBlock.push(BB);
			}
		}
		//printf("While Done\n");
		//printf("After While\n");
		NtkForEachLoop(cfgGraph, cfgLoop, i ){
			cfgGraph->makeLLVMForLoop(mod,  cfgLoop);
		}
		//printf("cfgGraph ID: %d\n");
		NtkForEachBBlock(cfgGraph, BB, i ){
			if(!BB->TerminatorCreated){
				cfgGraph->makeLLVMTerminatorBasicBlock(mod, func_test, BB);
			}
		}

	}
	//printf("cfgGraph ID: %d Done\n", cfgGraph->cfgIdx);
	return func_test;
}
Module* CFGNtk::makeLLVMModulefromCFGNtk() {
	Node *pNode, *Pi, *Const, *Po;
	BBlock *BB, *BBFanout, *BBFanin, *LoopTailBB, *LoopHeadBB;
	CFGLoop * cfgLoop;
	int i, j;
	queue<Node*> qNode;
	Module* mod = new Module("testGen", getGlobalContext());
	mod->setDataLayout(
			"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128");
	mod->setTargetTriple("i386-pc-linux-gnu");
	// Type Definitions

	//Used by printf
	ArrayType* ArrayTy_0 = ArrayType::get(
			IntegerType::get(mod->getContext(), 8), 15);
	PointerType* PointerTy_1 = PointerType::get(ArrayTy_0, 0);

	//For the test function
	std::vector<Type*> testFuncTy_args;
	if(this->ArrayIn){
		for(i=0;i<this->ArrayIn;i++){
			PointerType* PointerTy_1 = PointerType::get(IntegerType::get(mod->getContext(), 32), 0);
			testFuncTy_args.push_back(PointerTy_1);
		}
	}else{
		NtkForEachPi(this->Entry->DFG_NtkGenerator->Ntk, Pi, i ) {
			testFuncTy_args.push_back(IntegerType::get(mod->getContext(), 32));
		}
	}

	for(i=0; i<this->numBranchInput;i++){
		testFuncTy_args.push_back(IntegerType::get(mod->getContext(), 32));
	}
	FunctionType* testFuncTy_0 = FunctionType::get(
	/*Result=*/IntegerType::get(mod->getContext(), 32),
	/*Params=*/testFuncTy_args,
	/*isVarArg=*/false);

	//For the main function
	std::vector<Type*> mainFuncTy_args;
	FunctionType* mainFuncTy = FunctionType::get(
	/*Result=*/IntegerType::get(mod->getContext(), 32),
	/*Params=*/mainFuncTy_args,
	/*isVarArg=*/false);

	//For printf
	PointerType* PointerTy_printf = PointerType::get(
			IntegerType::get(mod->getContext(), 8), 0);
	std::vector<Type*> FuncTy_printf_args;
	FuncTy_printf_args.push_back(PointerTy_printf);
	FunctionType* FuncTy_printf = FunctionType::get(
	/*Result=*/IntegerType::get(mod->getContext(), 32),
	/*Params=*/FuncTy_printf_args,
	/*isVarArg=*/true);

	// Function Declarations

	//for test function
	Function* func_test = mod->getFunction("testFunc");
	if (!func_test) {
		func_test = Function::Create(
		/*Type=*/testFuncTy_0,
		/*Linkage=*/GlobalValue::ExternalLinkage,
		/*Name=*/"testFunc", mod);
		func_test->setCallingConv(CallingConv::C);
	}
	//for main
	Function* func_main = mod->getFunction("main");
	if (!func_main) {
		func_main = Function::Create(
		/*Type=*/mainFuncTy,
		/*Linkage=*/GlobalValue::ExternalLinkage,
		/*Name=*/"main", mod);
		func_main->setCallingConv(CallingConv::C);
	}
	Function* func_printf = mod->getFunction("printf");
	if (!func_printf) {
		func_printf = Function::Create(
		/*Type=*/FuncTy_printf,
		/*Linkage=*/GlobalValue::ExternalLinkage,
		/*Name=*/"printf", mod); // (external, no body)
		func_printf->setCallingConv(CallingConv::C);
	}
	AttrListPtr func_printf_PAL;
	func_printf->setAttributes(func_printf_PAL);
	//Global Variable Declarations

	//for printf
	GlobalVariable* gvar_array__str = new GlobalVariable(/*Module=*/*mod,
	/*Type=*/ArrayTy_0,
	/*isConstant=*/true,
	/*Linkage=*/GlobalValue::PrivateLinkage,
	/*Initializer=*/0, // has initializer, specified below
			/*Name=*/".str");



	gvar_array__str->setAlignment(1);
	ConstantInt* const_int32;
	NtkForEachBBlock(this, BB, i ){
		//printf("Processing BB: %d\n", BB->BBidx);
		if(BB->use_CFG) continue;
		NtkForEachConst(BB->DFG_NtkGenerator->Ntk, Const, j ){
			char ConstVal[20];
			sprintf(ConstVal, "%d", Const->NodeValue);
			const_int32 = ConstantInt::get(mod->getContext(), APInt(32, StringRef(ConstVal), 10));
			Const->NodeLLVMValue = const_int32;
			Const->NodeLLVMValueCreated = 1;
			BB->vConstantInt_in_BB.push_back(const_int32);
		}
		//printf("Processing BB: %d\n", BB->BBidx);
	}

	//Useful Constants
	const_zero = ConstantInt::get(mod->getContext(), APInt(32, StringRef("0"), 10));
	const_one = ConstantInt::get(mod->getContext(), APInt(32, StringRef("1"), 10));
	const_one_64 = ConstantInt::get(mod->getContext(), APInt(64, StringRef("1"), 10));

	//Constant for prinf
	Constant* const_for_printf = ConstantArray::get(mod->getContext(), "return_val:%u\x0A", true);
	ConstantInt* const_int32_11 = ConstantInt::get(mod->getContext(), APInt(32, StringRef("0"), 10));
	std::vector<Constant*> const_for_printf_indices;
	const_for_printf_indices.push_back(const_int32_11);
	const_for_printf_indices.push_back(const_int32_11);

	Constant* const_ptr_14 = ConstantExpr::getGetElementPtr(gvar_array__str, const_for_printf_indices);
	gvar_array__str->setInitializer(const_for_printf);

	//Constant for input vectors
	vector<ConstantInt*> vConstantInput_in_main;

	NtkForEachPi(this->Entry->DFG_NtkGenerator->Ntk, Pi, i ) {
		ConstantInt* const_input = ConstantInt::get(mod->getContext(), APInt(32, StringRef("20"), 10));
		vConstantInput_in_main.push_back(const_input);
	}
	//Function Definitions
	//Function: testFunc
	{


		//Create all the BasicBlocks
		NtkForEachBBlock(this, BB, i ){
			char BB_name[20];
			sprintf(BB_name, "BB_%d", BB->BBidx);
			//printf("Created Block: %s\n", BB_name);
			BB->LLVM_BB = BasicBlock::Create(mod->getContext(), BB_name, func_test,0);
		}
		queue<BBlock*> qBBlock;
		//Going though all the blocks in order
		qBBlock.push(this->Entry);
		int can_create;
		while (qBBlock.size()){

			BB = qBBlock.front();
			qBBlock.pop();
			can_create = 0;
			//printf("Left in the Queue");

			//printf("Checking for block %d, qBBlock size: %d\n", BB->BBidx, qBBlock.size());
			if(BB->allPredecessorCreated()){
				can_create = 1;
				//Create LLVM for the single Block BB
				if(BB->LLVM_created == 0){
					//printf("Creating LLVM for block %d\n", BB->BBidx);
					if(BB->use_CFG){
						assert(BB->InputBB.size()!=0);

						//printf("calling makeLLVMModulefromSUBCFGNtk for Block: %d\n", BB->BBidx);
						BB->func_ptr=makeLLVMModulefromSUBCFGNtk(mod, (CFGNtk*)BB->CFG_Ntk, BB->BBidx);
						//printf("DONE calling makeLLVMModulefromSUBCFGNtk for Block: %d\n", BB->BBidx);

						//If this is a CFG instead, make the call to the funtion first.
						CFGNtk* current_cfgGraph = (CFGNtk*)BB->CFG_Ntk;
						std::vector<Value*> int32_params;

						if(BB->InputBB.size()==1){
							BBFanin = BB->InputBB.at(0);
							NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, i ) {

								//printf("expected Pi size: %d, expected Po size: %d\n", BB->DFG_NtkGenerator->Ntk->NumPis, BBFanin->DFG_NtkGenerator->Ntk->NumPos);
								//printf("Pi size: %d, Po size: %d\n", BB->DFG_NtkGenerator->Ntk->vPis.size(), BBFanin->DFG_NtkGenerator->Ntk->vPos.size());
								Po = BBFanin->DFG_NtkGenerator->Ntk->vPos.at(i);
								//printf("Gets Here\n");
								qNode.push(Pi);
								Pi->NodeLLVMValue = Po->NodeLLVMValue;
								Pi->NodeLLVMValueCreated = 1;
								if (Pi->NodeLLVMValue == NULL) {
									printf(
											"Fanin Block %d's output is having problems on input %d!!\n",
											BBFanin->BBidx, Pi->NodeIdx);
									printf("Error!!\n");
								}
								Type *OpType32 = IntegerType::get(mod->getContext(), 32);
								//Type *OpType64 = IntegerType::get(mod->getContext(), 64);
								Value* new_op_0;
								if(Pi->NodeLLVMValue->getType()!=OpType32){
									new_op_0 = create_convert_instr(mod, Pi->NodeLLVMValue, OpType32, BB->LLVM_BB);
									//printf("Types not match!!\n");
								}else{
									new_op_0 = Pi->NodeLLVMValue;
								}
								//printf("pushing function arg1 type: %d\n", new_op_0->getType());
								int32_params.push_back(new_op_0);
							}
						}else{
							//printf("Creating a joint block for %d\n", BB->BBidx);
							NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, i ) {
								//printf("Processing input %d\n", i);
								PHINode* int32_p = PHINode::Create(
									IntegerType::get(mod->getContext(), 32), 2, "",
									BB->LLVM_BB);
									//printf("Processing input %d Done\n", i);
								Pi->NodeLLVMValue = int32_p;
								Pi->NodeLLVMValueCreated = 1;
								//printf("pushing function arg type: %d\n", int32_p->getType());
								int32_params.push_back(Pi->NodeLLVMValue);
							}
							BBForEachFanin( BB, BBFanin, i ) {
								//printf("Number of fanout from BB %d is %d\n", BBFanin->BBidx, BBFanin->DFG_NtkGenerator->Ntk->vPos.size());
								NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, j ) {
									Po = BBFanin->DFG_NtkGenerator->Ntk->vPos.at(j);
									PHINode* int32_phi = (PHINode*) Pi->NodeLLVMValue;
									assert(Po->NodeLLVMValue!=NULL);
									Type* OpType = Po->NodeLLVMValue->getType();
									Value* new_op_0;
									if(int32_phi->getType()!=OpType){
										new_op_0 = create_convert_instr(mod, Po->NodeLLVMValue, int32_phi->getType(), BBFanin->LLVM_BB);
									}else{
										new_op_0 = Po->NodeLLVMValue;
									}

									int32_phi->addIncoming(new_op_0, BBFanin->LLVM_BB);
								}
							}
						}

						for (i = 0; i < current_cfgGraph->numBranchInput; i++) {
							char ConstVal[20];
							sprintf(ConstVal, "%d", rand() % 2);
							ConstantInt* const_input = ConstantInt::get(
									mod->getContext(),
									APInt(32, StringRef(ConstVal), 10));
							//printf("pushing branch arg type: %d\n", const_input->getType());
							int32_params.push_back(const_input);
						}

						//printf("Argument size: %d\n", int32_params.size());
						FunctionType *FTy =
						    cast<FunctionType>(cast<PointerType>(BB->func_ptr->getType())->getElementType());
						//printf("Func Argument size: %d\n", FTy->getNumParams());
						for (i = 0; i != int32_params.size(); ++i){
							//cout << FTy->getParamType(i)<< " "<<  int32_params[i]->getType() <<endl;
							//printf("i: %d, FunctionParamType: %d, InputParamType: %d\n", i, FTy->getParamType(i), int32_params[i]->getType());
							assert(FTy->getParamType(i) == int32_params[i]->getType() &&
							           "Calling a function with a bad signature!");
						}
						CallInst* int32_26 = CallInst::Create(BB->func_ptr, int32_params, "", BB->LLVM_BB);
						//Then handle the returned values.
						//printf("number of output is: %d\n", BB->outputNodeNum);
						if(BB->outputNodeNum==1){
							BB->DFG_NtkGenerator->Ntk->vPos.at(0)->NodeLLVMValue=int32_26;
						}else{
							for (i = 0; i < BB->outputNodeNum; i++) {
								char ConstVal[20];
								sprintf(ConstVal, "%d", i);
								ConstantInt* const_idx = ConstantInt::get(mod->getContext(), APInt(32, StringRef(ConstVal), 10));
								//printf("creating output for idx %s\n", ConstVal);
								//printf("return function type: %d\n", int32_26->getType());
								//printf("const_idx type: %d\n", const_idx->getType());
								GetElementPtrInst* ptr_mem = GetElementPtrInst::Create(int32_26, const_idx, "", BB->LLVM_BB);
								//printf("Creating ouput LLVM at %d\n", i);
								BB->DFG_NtkGenerator->Ntk->vPos.at(i)->NodeLLVMValue = new LoadInst(ptr_mem, "", false, BB->LLVM_BB);
							}
						}

						//printf("DONE calling makeLLVMModulefromSUBCFGNtk for Block: %d\n", BB->BBidx);
						BB->LLVM_created = 1;
					}else{
						makeLLVMModulefromBasicBlock(mod, func_test, BB);
					}

				}
				BBForEachFanout(BB, BBFanout, i ){
					if(BBFanout->LLVM_created == 0){

						qBBlock.push(BBFanout);
					}
				}
			}else{

				qBBlock.push(BB);
			}
		}

		//printf("After While\n");
		NtkForEachLoop(this, cfgLoop, i ){
			makeLLVMForLoop(mod,  cfgLoop);
		}
		//printf("After NtkForEachLoop\n");
		//Creating Terminators for each block
		NtkForEachBBlock(this, BB, i ){
			if(!BB->TerminatorCreated)
				makeLLVMTerminatorBasicBlock(mod, func_test, BB);
		}
		//printf("After NtkForEachBBlock\n");
	}
	int genMain = 1;
	if(genMain){
		// Function: main (func_main)
		{
			BasicBlock* BB_label_main = BasicBlock::Create(mod->getContext(), "", func_main, 0);

			std::vector<Value*> int32_params;
			if(!this->ArrayIn){
				NtkForEachPi(this->Entry->DFG_NtkGenerator->Ntk, Pi, i ) {
					int32_params.push_back(vConstantInput_in_main.at(i));
				}
			}else{
				for (i = 0; i < this->ArrayIn; i++) {
					ArrayType* ArrayTy_main = ArrayType::get(IntegerType::get(mod->getContext(), 32), numInput);
					PointerType* PointerTy_main = PointerType::get(ArrayTy_main, 0);
					GlobalVariable* gvar_array_main_X = new GlobalVariable(
							/*Module=*/*mod,
							/*Type=*/ArrayTy_main,
							/*isConstant=*/true,
							/*Linkage=*/GlobalValue::PrivateLinkage,
							/*Initializer=*/0, // has initializer, specified below
							/*Name=*/"main.Array");
					gvar_array_main_X->setAlignment(16);
					std::vector<Constant*> const_main_array_elems;
					for(int k=0;k<numInput;k++){
						char ConstVal[20];
						sprintf(ConstVal, "%d", rand());
						ConstantInt* const_main_array_elem = ConstantInt::get(mod->getContext(), APInt(32, StringRef(ConstVal), 10));
						const_main_array_elems.push_back(const_main_array_elem);
					}
					Constant* const_main_array = ConstantArray::get(ArrayTy_main, const_main_array_elems);
					gvar_array_main_X->setInitializer(const_main_array);
					std::vector<Value*> ptr_89_indices;
					ptr_89_indices.push_back(const_zero);
					ptr_89_indices.push_back(const_zero);
					Instruction* Array_main_ptr = GetElementPtrInst::Create(gvar_array_main_X, ptr_89_indices, "", BB_label_main);
					int32_params.push_back(Array_main_ptr);

				}

			}

			for(i=0; i<this->numBranchInput;i++){
				char ConstVal[20];
				sprintf(ConstVal, "%d", rand()%2);
				ConstantInt* const_input = ConstantInt::get(mod->getContext(), APInt(32, StringRef(ConstVal), 10));
				int32_params.push_back(const_input);
			}

			CallInst* int32_26 = CallInst::Create(func_test, int32_params, "",
					BB_label_main);

			int32_26->setCallingConv(CallingConv::C);
			int32_26->setTailCall(false);
			AttrListPtr int32_26_PAL;
			int32_26->setAttributes(int32_26_PAL);
			//printf
			std::vector<Value*> int32_printf_params;
			int32_printf_params.push_back(const_ptr_14);
			int32_printf_params.push_back(int32_26);
			CallInst* int32_39 = CallInst::Create(func_printf, int32_printf_params, "",
					BB_label_main);
			int32_39->setCallingConv(CallingConv::C);
			int32_39->setTailCall(false);
			AttrListPtr int32_39_PAL;
			int32_39->setAttributes(int32_39_PAL);


			ReturnInst::Create(mod->getContext(), int32_26, BB_label_main);
		}
	}
	return mod;
}
void CFGNtk::makeLLVMForLoop(Module* mod, CFGLoop * cfgLoop){
	int i;
	BBlock *BB, *BBFanout, *LoopTailBB, *LoopHeadBB, *BBFanin;
	LoopTailBB = cfgLoop->BackEdgeTail;
	LoopHeadBB = cfgLoop->BackEdgeHead;
	//printf("Head of the loop is %d\n", LoopHeadBB->BBidx);
	//printf("Tail of the loop is %d\n", LoopTailBB->BBidx);

	Argument* fwdref_i = new Argument(IntegerType::get(mod->getContext(), 32));
	PHINode* int32_i = PHINode::Create(IntegerType::get(mod->getContext(), 32), 2, "i.01.i", LoopTailBB->LLVM_BB->begin());	//This is the forward reference i
	int32_i->addIncoming(fwdref_i, LoopHeadBB->LLVM_BB);
	BBForEachFanin( LoopTailBB, BBFanin, i ){
		//BBlock *faninBB = LoopTailBB->InputBB.at(0);
		//printf("Fanin is %d\n", BBFanin->BBidx);
		int32_i->addIncoming(const_zero, BBFanin->LLVM_BB);
	}

	//printf("Gets here - 1\n");
	BinaryOperator* int32_i_add = BinaryOperator::Create(Instruction::Add, int32_i, const_one, "", LoopHeadBB->LLVM_BB);
	char ConstVal[20];
	sprintf(ConstVal, "%d", cfgLoop->loop_iteration_num);
	//printf("Gets here - 2\n");
	ConstantInt* const_iterationNum = ConstantInt::get(mod->getContext(), APInt(32, StringRef(ConstVal), 10));
	ICmpInst* int1_exitcond_i = new ICmpInst(*LoopHeadBB->LLVM_BB, ICmpInst::ICMP_EQ, int32_i_add, const_iterationNum, "exitcond.i");
	//printf("Gets here - 3\n");
	BBlock *fanoutBB = LoopHeadBB->OutputBB.at(0);
	//printf("Gets here - 4\n");
	BranchInst::Create(fanoutBB->LLVM_BB, LoopTailBB->LLVM_BB, int1_exitcond_i, LoopHeadBB->LLVM_BB);

	fwdref_i->replaceAllUsesWith(int32_i_add);
	LoopHeadBB->TerminatorCreated = 1;

}
void CFGNtk::createReturnInstrForArrayBasicBlock(Module* mod, BBlock *BB){
	Node *pNode;
	DAGDFGNtk *Ntk = BB->DFG_NtkGenerator->Ntk;




	char output_array_result_name[30];
	sprintf(output_array_result_name, "output_array_result_%d_%d", BB->BBidx, cfgIdx);
	//printf("Creating Array %s\n", output_array_result_name);
	ArrayType* ArrayTy_OutputArray = ArrayType::get(IntegerType::get(mod->getContext(), 32), BB->outputNodeNum);
	PointerType* PointerTy_OutputArray = PointerType::get(ArrayTy_OutputArray, 0);
	GlobalVariable* output_array_result = new GlobalVariable(/*Module=*/*mod,
	 /*Type=*/ArrayTy_OutputArray,
	 /*isConstant=*/false,
	 /*Linkage=*/GlobalValue::CommonLinkage,
	 /*Initializer=*/0, // has initializer, specified below
	 /*Name=*/output_array_result_name);
	output_array_result->setAlignment(16);

	ConstantAggregateZero* const_array_9 = ConstantAggregateZero::get(ArrayTy_OutputArray);
	Value* ptr_ele;
	Value* ret_val;
	const_zero_64 = ConstantInt::get(mod->getContext(), APInt(64, StringRef("0"), 10));

	output_array_result->setInitializer(const_array_9);

	for(int i=0;i<BB->outputNodeNum;i++){
		//creating index for array access

		char ConstVal[20];
		sprintf(ConstVal, "%d", i);
		//printf("Creating %s\n", ConstVal);
		ConstantInt* const_idx = ConstantInt::get(mod->getContext(), APInt(64, StringRef(ConstVal), 10));

		pNode = Ntk->vPis.at(i);
		Value* ptr_val =  pNode->NodeLLVMValue;
		Value* ptr_val_new;
		Type *OpType = IntegerType::get(mod->getContext(), 32);
		if(ptr_val->getType()!=OpType){
			ptr_val_new = create_convert_instr(mod, ptr_val, OpType, BB->LLVM_BB);
		}else{
			ptr_val_new = ptr_val;
		}

		std::vector<Constant*> const_ptr_indices;
		const_ptr_indices.push_back(const_zero_64);
		const_ptr_indices.push_back(const_idx);
		Constant* const_ptr_13 = ConstantExpr::getGetElementPtr(output_array_result, const_ptr_indices);
		if(i==0) ret_val = const_ptr_13;
		StoreInst* void_store = new StoreInst(ptr_val_new, const_ptr_13, false, BB->LLVM_BB);
		void_store->setAlignment(16);
	}
	ReturnInst::Create(mod->getContext(), ret_val, BB->LLVM_BB);

}
void CFGNtk::makeLLVMTerminatorBasicBlock(Module* mod, Function* func_test, BBlock *BB){
	//Get where the fanout block is at
	DAGDFGNtk *Ntk = BB->DFG_NtkGenerator->Ntk;
	Node *pNode;
	BBlock* pBB;
	//Firstly assume there's only 1 fanout
	//printf("Creating terminator instruction for Block %d\n", BB->BBidx);

	if(BB->OutputBB.size()==1){
		//printf("Gets here %d\n",  BB->OutputBB.size());
		BBlock *fanoutBB = BB->OutputBB.at(0);
		BranchInst::Create(fanoutBB->LLVM_BB, BB->LLVM_BB);
		BB->TerminatorCreated = 1;
		//printf("Gets here\n");
	}else if (BB->OutputBB.size()==0){
		//This is an Exit Block
		assert(BB->use_CFG==0);
		if(BB->outputNodeNum==1){
			pNode = Ntk->vPis.at(0);
			Value* ptr_val =  pNode->NodeLLVMValue;
			Value* ptr_val_new;
			//printf("Creating return instruction for Block %d\n", BB->BBidx);
			Type *OpType = IntegerType::get(mod->getContext(), 32);
			if(ptr_val->getType()!=OpType){
				ptr_val_new = create_convert_instr(mod, ptr_val, OpType, BB->LLVM_BB);
			}else{
				ptr_val_new = ptr_val;
			}


			if(ptr_val_new == NULL) printf("E!!!\n");
			ReturnInst::Create(mod->getContext(), ptr_val_new, BB->LLVM_BB);
		}else{
			createReturnInstrForArrayBasicBlock(mod, BB);
		}

		BB->TerminatorCreated = 1;

	}else{
		//This is a branching block
		//printf("Creating switch instruction for Block %d\n", BB->BBidx);
		//printf("BranchControlInput size %d\n",  BranchControlInput.size());
		//printf("numBranchCreated size %d\n",  numBranchCreated);
		if(numBranchCreated>numBranchInput) printf("Error!\n");
		pNode = BranchControlInput.at(numBranchCreated);
		pBB = BB->OutputBB.at(0);
		//if(pNode->NodeLLVMValue==NULL)printf("Control node pNode is not created\n");

		//if(pBB->LLVM_BB==NULL)printf("pBB %d is not created\n", pBB->BBidx);
		SwitchInst* sw_inst = SwitchInst::Create(pNode->NodeLLVMValue, pBB->LLVM_BB, 2, BB->LLVM_BB);

		pBB = BB->OutputBB.at(1);

		//if(pBB->LLVM_BB==NULL)printf("pBB %d is not created\n", pBB->BBidx);
		//printf("pBB pNode %d \n", pBB->LLVM_created);
		sw_inst->addCase(const_one, pBB->LLVM_BB);
		numBranchCreated++;
		BB->TerminatorCreated = 1;
		//printf("Gets here\n");
	}

}
void CFGNtk::makeLLVMModulefromBasicBlock(Module* mod, Function* func_test, BBlock *BB){
	DAGDFGNtk *Ntk = BB->DFG_NtkGenerator->Ntk;
	Value* op_0;
	Value* op_1;
	Value* op_result;
	Node *pNode, *Pi, *Po, *Const, *pFanout;
	Node *op_0_node;
	Node *op_1_node;
	queue<Node*> qNode;
	int i, j;
	//if(BB->BBidx == 3) BB->DFG_NtkGenerator->Ntk->print_ntk_to_dot("Block_3.dot");
	Ntk->reset_visit_info();
	//printf("Creating BB: %d\n", BB->BBidx);
	if(!Ntk->isArrayInput){
		if(BB->InputBB.size()==0){
			//This is an Entry Block
			Value* inputValue;
			//Declair all the inputs
			Function::arg_iterator args = func_test->arg_begin();
			NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, i ) {
				inputValue = args++;
				std::stringstream inputValName;
				inputValName << "input_" << i;
				inputValue->setName(inputValName.str());
				Pi->NodeLLVMValue = inputValue;
				Pi->NodeLLVMValueCreated = 1;
				Pi->visited=1;
				//qNode.push(Pi);
			}
			//Creating the branch control inputs
			for(i=0; i<this->numBranchInput;i++){
				inputValue = args++;
				//std::stringstream inputValName;
				pNode = BranchControlInput.at(i);
				char inputValName[30];
				sprintf(inputValName, "input_branch_%u_%u", pNode->NodeIdx, BB->BBidx);
				//printf("%s\n", inputValName);
				//inputValName << "input_branch_" << pNode->NodeIdx;
				inputValue->setName(inputValName);
				pNode->NodeLLVMValue = inputValue;
				pNode->NodeLLVMValueCreated = 1;
				//pNode->visited=1;
			}
			//For Entry Block, I am considering Pi as Pos
			NtkForEachPo(BB->DFG_NtkGenerator->Ntk, Po, i ) {
				Pi = BB->DFG_NtkGenerator->Ntk->vPis.at(i);
				Po->NodeLLVMValue = Pi->NodeLLVMValue;
				Po->NodeLLVMValueCreated = 1;
			}
			//printf("Gets Here for LLVM_created\n");
			//cout << "Number of Instructions now " << BB->LLVM_BB->size() << "\n";
			BB->LLVM_created = 1;
			return;
		}else if(BB->InputBB.size()==1){
			//This is an regular block
			int vari_result=this->CFG_NtkVerification();


			//printf("Input size %d, expected: %d\n",BB->DFG_NtkGenerator->Ntk->vPis.size(), BB->inputNodeNum);

			assert(vari_result);
			//printf("Creating a regular block %d\n", BB->BBidx);
			BBlock *BBFanin = BB->InputBB.at(0);

			//printf("Output size %d, expected: %d\n",BBFanin->DFG_NtkGenerator->Ntk->vPos.size(), BBFanin->outputNodeNum);

			//printf("The fanin block is %d\n", BBFanin->BBidx);
			NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, i ) {

				//printf("expected Pi size: %d, expected Po size: %d\n", BB->DFG_NtkGenerator->Ntk->NumPis, BBFanin->DFG_NtkGenerator->Ntk->NumPos);
				//printf("Pi size: %d, Po size: %d\n", BB->DFG_NtkGenerator->Ntk->vPis.size(), BBFanin->DFG_NtkGenerator->Ntk->vPos.size());
				Po = BBFanin->DFG_NtkGenerator->Ntk->vPos.at(i);
				//printf("Gets Here\n");
				qNode.push(Pi);
				Pi->NodeLLVMValue = Po->NodeLLVMValue;
				Pi->NodeLLVMValueCreated = 1;
				if (Pi->NodeLLVMValue == NULL) {
					printf(
							"Fanin Block %d's output is having problems on input %d!!\n",
							BBFanin->BBidx, Pi->NodeIdx);
					printf("Error!!\n");
				}
				Pi->visited = 1;
			}
			//cout << "Number of Instructions now " << BB->LLVM_BB->size() << "\n";

		}else{
			//printf("Creating a joint block for %d\n", BB->BBidx);
			BBlock *BBFanin;
			//if(BB->DFG_NtkGenerator->Ntk == NULL) printf("DFG_NtkGenerator does not exist\n");
			//BB->DFG_NtkGenerator->Ntk->print_ntk_to_dot("Block_5.dot");
			NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, i ) {
				//printf("Processing input %d\n", i);
				PHINode* int32_p = PHINode::Create(
						IntegerType::get(mod->getContext(), 32), 2, "",
						BB->LLVM_BB);
				//printf("Processing input %d Done\n", i);
				Pi->NodeLLVMValue = int32_p;
				Pi->NodeLLVMValueCreated = 1;
				qNode.push(Pi);
				//cout << "Number of Instructions now " << BB->LLVM_BB->size() << "\n";
			}
			//printf("Number of fanin to BB %d is %d\n", BB->BBidx, BB->InputBB.size());
			//printf("Number of fanin to BB %d is %d\n", BB->BBidx, BB->DFG_NtkGenerator->Ntk->vPis.size());
			BBForEachFanin( BB, BBFanin, i ) {
				//printf("Number of fanout from BB %d is %d\n", BBFanin->BBidx, BBFanin->DFG_NtkGenerator->Ntk->vPos.size());
				NtkForEachPi(BB->DFG_NtkGenerator->Ntk, Pi, j ) {
					Po = BBFanin->DFG_NtkGenerator->Ntk->vPos.at(j);
					PHINode* int32_phi = (PHINode*) Pi->NodeLLVMValue;
					assert(Po->NodeLLVMValue!=NULL);
					Type* OpType = Po->NodeLLVMValue->getType();
					Value* new_op_0;
					if(int32_phi->getType()!=OpType){
						new_op_0 = create_convert_instr(mod, Po->NodeLLVMValue, int32_phi->getType(), BBFanin->LLVM_BB);
					}else{
						new_op_0 = Po->NodeLLVMValue;
					}

					int32_phi->addIncoming(new_op_0, BBFanin->LLVM_BB);
				}
			}
			//printf("Creating a joint block. DONE\n");
		}
	}else{
		//The input to this DFG is array.
		if(BB->InputBB.size()==0){
			//printf("Creating Entry BB: %d\n", BB->BBidx);
			//This is a entry block
			Value* inputValue;
			//Declair all the inputs
			Function::arg_iterator args = func_test->arg_begin();
			for(j=0;j<Ntk->isArrayInput;j++){
				//printf("Gets Here - 1\n");
				inputValue = args++;

				std::stringstream inputValName;
				inputValName << "input_array_" << j;
				inputValue->setName(inputValName.str());
				//printf("Gets Here - 2\n");
				Pi = Ntk->inputArray->ptrNode;
				//printf("Gets Here - 3\n");
				Pi->NodeLLVMValue = inputValue;
				Pi->NodeLLVMValueCreated = 1;
				Pi->visited=1;
				//printf("Gets Here - 4\n");
				for(i=0;i<Ntk->inputArray->ArrayNodes.size();i++){
					char inputIdxStr[20];
					sprintf(inputIdxStr, "%d", i);
					ConstantInt* array_idx = ConstantInt::get(mod->getContext(), APInt(32, StringRef(inputIdxStr), 10));
					//printf("Gets Here - 5\n");
					GetElementPtrInst* GEPInst = GetElementPtrInst::Create(inputValue, array_idx, "", BB->LLVM_BB);

					//printf("Gets Here - 6\n");
					Ntk->inputArray->ArrayNodes.at(i)->NodeLLVMValue = new LoadInst(GEPInst, "", false, BB->LLVM_BB);
				}
			}
			//Creating the branch control inputs
			for(i=0; i<this->numBranchInput;i++){
				//printf("Gets Here - 1 - %d\n", i);
				inputValue = args++;
				//std::stringstream inputValName;
				pNode = BranchControlInput.at(i);
				char inputValName[30];
				sprintf(inputValName, "input_branch_%u_%u", pNode->NodeIdx, BB->BBidx);
				//inputValName << "input_branch_" << pNode->NodeIdx;
				inputValue->setName(inputValName);
				pNode->NodeLLVMValue = inputValue;
				pNode->NodeLLVMValueCreated = 1;
				//pNode->visited=1;
			}

			//For Entry Block, I am considering Pi as Pos
			//printf("Currently, the size of the output is %d\n", Ntk->vPos.size());
			//printf("Currently, the size of the input is %d\n", Ntk->inputArray->ArrayNodes.size());
			NtkForEachPo(BB->DFG_NtkGenerator->Ntk, Po, i ) {
				//printf("Gets Here - 5 - %d\n", i);
				Pi = Ntk->inputArray->ArrayNodes.at(i);
				Po->NodeLLVMValue = Pi->NodeLLVMValue;
				Po->NodeLLVMValueCreated = 1;
			}
			//printf("Gets Here for LLVM_created\n");
			//cout << "Number of Instructions now " << BB->LLVM_BB->size() << "\n";
			BB->LLVM_created = 1;
			//printf("Gets Here - Done\n");
			return;
		}
	}


	NtkForEachConst(Ntk, Const, i ){
		qNode.push(Const);
	}

	int can_create;
	//printf("Gets Here before while, size of the queue is %d\n", qNode.size());
	if(BB->BBType!=0 || BB->BBType!=1){
		while (!qNode.empty()) {

			pNode = qNode.front();
			qNode.pop();
			can_create = 1;

			//cout << "popoed node " << pNode->NodeIdx << "\n";
			//If this is a Po
			if(pNode->isPo){
				assert(pNode->isPattern==0);
				op_0_node = pNode->InputNode.at(0);
				if(op_0_node->NodeLLVMValueCreated==1){
					can_create = 1;
					op_0 = op_0_node->NodeLLVMValue;
					pNode->NodeLLVMValue = op_0;
					pNode->NodeLLVMValueCreated = 1;
				}
			}else if(!pNode->isPi && !pNode->isPo && !pNode->isConst()){
				can_create = 0;
				if(pNode->isPattern==0){
					op_0_node = pNode->InputNode.at(0);
					op_1_node = pNode->InputNode.at(1);

					if(op_0_node->NodeLLVMValueCreated==1 && op_1_node->NodeLLVMValueCreated==1 ){
						can_create = 1;
						op_0 = op_0_node->NodeLLVMValue;
						op_1 = op_1_node->NodeLLVMValue;
						//printf("Connecting %d and %d node\n", op_0_node->NodeIdx, op_1_node->NodeIdx);
						//cout << "node " << op_0_node->NodeIdx<< " type " << op_0->getType() << "\n";
						//cout << "node " << op_1_node->NodeIdx<< " type " << op_1->getType() << "\n";
						//cout << "node " << pNode->NodeIdx << " type: " << pNode->NodeOperation << endl;

						if(op_0==NULL || op_1==NULL) printf("Input Operation is null\n");
						Value* result_val = createInstforNode(mod, pNode, op_0, op_1, BB->LLVM_BB);

						pNode->NodeLLVMValue = result_val;
						pNode->NodeLLVMValueCreated = 1;
						//cout << "node "<< pNode->NodeIdx << " type "<< pNode->NodeLLVMValue->getType() << " (created)\n";

						//can_create = 1;
					}else{
						//cout << "pushed node " << pNode->NodeIdx << "\n";
						qNode.push(pNode);
					}

				}else{
					can_create = 1;
					for(i=0;i<pNode->InputNode.size();i++){
						if(pNode->InputNode.at(i)->NodeLLVMValueCreated==0){
							can_create = 0;
							break;
						}
					}
					if(can_create){
						makeLLVMModulefromPatern(mod, pNode, BB);
						pNode->NodeLLVMValueCreated = 1;
					}else{
						qNode.push(pNode);
					}
				}
			}
			if(can_create){
				NodeForEachFanout( pNode, pFanout, i ){
					if(!pFanout->visited){
						pFanout->visited = 1;
						//cout << "pushed node " << pFanout->NodeIdx << "\n";
						qNode.push(pFanout);
					}

				}
			}

		}

	}
	BB->LLVM_created = 1;
}
void CFGNtk::makeLLVMModulefromPatern(Module* mod, Node* PatternNode, BBlock *BB){
	int i;
	Node *pNode, *pFanin, *pFanout;
	queue<Node*> qNode;

	NodePattern* Pattern = (NodePattern*)PatternNode;

	//printf("creating LLVM for pattern %d\n", Pattern->NodeIdx);
	//printf("Pattern: %d, Pattern->FakeInputNode.size(): %d, PatternNode->InputNode.size(): %d\n", Pattern->NodeIdx, Pattern->FakeInputNode.size(), PatternNode->InputNode.size());
	assert(Pattern->FakeInputNode.size()==PatternNode->InputNode.size());

	for(i=0;i<Pattern->vNodes.size();i++){
		pNode = Pattern->vNodes.at(i);
		pNode->visited=0;
	}

	for(i=0; i<Pattern->FakeInputNode.size(); i++){
		pNode = Pattern->FakeInputNode.at(i);
		pNode->NodeLLVMValue=PatternNode->InputNode.at(i)->NodeLLVMValue;
		qNode.push(pNode);
		pNode->NodeLLVMValueCreated = 1;
		pNode->visited = 1;
	}
	int can_create;
	while (!qNode.empty()) {
		pNode = qNode.front();
		qNode.pop();
		can_create = 1;
		if(pNode->isPo){
			Node* output_node = pNode->InputNode.at(0);
			if(output_node->NodeLLVMValueCreated==1){
				can_create = 1;
				PatternNode->NodeLLVMValue = output_node->NodeLLVMValue;
				pNode->NodeLLVMValueCreated = 1;
			}
		}else if(!pNode->isPi && !pNode->isPo && !pNode->isConst()){
			can_create = 0;
			//printf("Creating node %d for pattern %d with operation %d\n", pNode->NodeIdx, PatternNode->NodeIdx, pNode->NodeOperation);
			Node *op_0_node = pNode->InputNode.at(0);
			Node *op_1_node = pNode->InputNode.at(1);
			if(op_0_node->NodeLLVMValueCreated==1 && op_1_node->NodeLLVMValueCreated==1 ){
				can_create = 1;
				Value *op_0 = op_0_node->NodeLLVMValue;
				Value *op_1 = op_1_node->NodeLLVMValue;
				if(op_0==NULL || op_1==NULL) printf("Input Operation is null\n");

				Value* result_val = createInstforNode(mod, pNode, op_0, op_1, BB->LLVM_BB);
				pNode->NodeLLVMValue = result_val;
				pNode->NodeLLVMValueCreated = 1;
			}else{
				//cout << "pushed node " << pNode->NodeIdx << "\n";
				qNode.push(pNode);
			}


		}
		if(can_create){
			NodeForEachFanout( pNode, pFanout, i ){
				if(!pFanout->visited){
					pFanout->visited = 1;
					//cout << "pushed node " << pFanout->NodeIdx << "\n";
					qNode.push(pFanout);
				}
			}
		}
	}
}
BinaryOperator* CFGNtk::createInstforNode(Module* mod, Node* pNode, Value* op_0, Value* op_1, BasicBlock* BB){


	BinaryOperator* result_val;
	Type *OpType;
	Value* new_op_0, *new_op_1;
	//cout << "Creating Opt for Node: " << pNode->NodeOperation << "\n";
	//cout << "op_0 type " << op_0->getType() << "\n";
	//cout << "op_1 type " << op_1->getType() << "\n";
	OpType = IntegerType::get(mod->getContext(), 32);
	if((pNode->NodeOperation>=0 && pNode->NodeOperation<4)||(pNode->NodeOperation>=8 && pNode->NodeOperation<=10)){
		//This is a 32-bit int operation
		OpType = IntegerType::get(mod->getContext(), 32);
		if(op_0->getType()!=OpType){
			new_op_0 = create_convert_instr(mod, op_0, OpType, BB);
			//printf("Types not match!!\n");
		}else{
			new_op_0 = op_0;
		}
		if(op_1->getType()!=OpType){
			new_op_1 = create_convert_instr(mod, op_1, OpType, BB);
			//printf("Types not match!!\n");
		}else{
			new_op_1 = op_1;
		}
	}else if((pNode->NodeOperation>=4 && pNode->NodeOperation<8)||(pNode->NodeOperation>=11 && pNode->NodeOperation<=13)){
		//This is a 64-bit int operation
		//printf("Gets Here\n");
		OpType = IntegerType::get(mod->getContext(), 64);
		if(op_0->getType()!=OpType){
			new_op_0 = create_convert_instr(mod, op_0, OpType, BB);
		}else{
			new_op_0 = op_0;
		}
		if(op_1->getType()!=OpType){
			new_op_1 = create_convert_instr(mod, op_1, OpType, BB);
		}else{
			new_op_1 = op_1;
		}
	}else if((pNode->NodeOperation>=14 && pNode->NodeOperation<18)){
		//This is a 32-bit float operation
		OpType = Type::getFloatTy(mod->getContext());
		if(op_0->getType()!=OpType){
			new_op_0 = create_convert_instr(mod, op_0, OpType, BB);
		}else{
			new_op_0 = op_0;
		}
		if(op_1->getType()!=OpType){
			new_op_1 = create_convert_instr(mod, op_1, OpType, BB);
		}else{
			new_op_1 = op_1;
		}
	}else if((pNode->NodeOperation>=18 && pNode->NodeOperation<22)){
		//This is a 64-bit double operation
		OpType = Type::getDoubleTy(mod->getContext());
		if(op_0->getType()!=OpType){
			new_op_0 = create_convert_instr(mod, op_0, OpType, BB);
		}else{
			new_op_0 = op_0;
		}
		if(op_1->getType()!=OpType){
			new_op_1 = create_convert_instr(mod, op_1, OpType, BB);
		}else{
			new_op_1 = op_1;
		}
	}else if((pNode->NodeOperation>=22 && pNode->NodeOperation<26)){
		//This is a 8-bit integer operation
		OpType = IntegerType::get(mod->getContext(), 8);
		if(op_0->getType()!=OpType){
			new_op_0 = create_convert_instr(mod, op_0, OpType, BB);
		}else{
			new_op_0 = op_0;
		}
		if(op_1->getType()!=OpType){
			new_op_1 = create_convert_instr(mod, op_1, OpType, BB);
		}else{
			new_op_1 = op_1;
		}

	}else{
		new_op_0 = op_0;
		new_op_1 = op_1;
	}
	//printf("pNode->NodeType: %d\n", pNode->NodeType);
	assert(new_op_0!=NULL);
	assert(new_op_1!=NULL);
	assert(new_op_0->getType()==new_op_1->getType());
	switch(pNode->NodeOperation)
	{
		case 0:
		case 4:{	//ADD
			result_val = BinaryOperator::Create(Instruction::Add, new_op_0, new_op_1, "", BB);
			break;
		}
		case 1:
		case 5:{	//SUB>
			result_val = BinaryOperator::Create(Instruction::Sub, new_op_0, new_op_1, "", BB);
			break;
		}
		case 2:
		case 6:{	//MULT
			result_val = BinaryOperator::Create(Instruction::Mul, new_op_0, new_op_1, "", BB);
			break;
		}
		case 3:{
			//This is to make sure it's not having the divided by zero problem
			if(cfgAC->no_zero_avoidance){
				result_val = BinaryOperator::Create(Instruction::SDiv, new_op_0, new_op_1, "", BB);
			}else{
				Value* op_1_altered = BinaryOperator::Create(Instruction::Or, new_op_1, const_one, "", BB);
				result_val = BinaryOperator::Create(Instruction::SDiv, new_op_0, op_1_altered, "", BB);
			}

			break;

		}
		case 7:{	//DIV
			if(cfgAC->no_zero_avoidance){
				result_val = BinaryOperator::Create(Instruction::SDiv, new_op_0, new_op_1, "", BB);
			}else{
				Value* op_1_altered = BinaryOperator::Create(Instruction::Or, new_op_1, const_one_64, "", BB);
				result_val = BinaryOperator::Create(Instruction::SDiv, new_op_0, op_1_altered, "", BB);
			}
			break;
		}
		case 8:		//SHL
		case 11:{	//LSHL
			result_val = BinaryOperator::Create(Instruction::Shl, new_op_0, new_op_1, "", BB);
			break;
		}
		case 9:		//LSHR
		case 12:{	//LLSHR
			result_val = BinaryOperator::Create(Instruction::LShr, new_op_0, new_op_1, "", BB);
			break;
		}
		case 10:	//ASHR
		case 13:{	//LASHR
			result_val = BinaryOperator::Create(Instruction::AShr, new_op_0, new_op_1, "", BB);
			break;
		}
		case 14:	//FADD
		case 18:{	//DADD
			result_val = BinaryOperator::Create(Instruction::FAdd, new_op_0, new_op_1, "", BB);
			break;
		}
		case 15:	//FSub
		case 19:{	//DSub
			result_val = BinaryOperator::Create(Instruction::FSub, new_op_0, new_op_1, "", BB);
			break;
		}
		case 16:	//FMul
		case 20:{	//DMul
			result_val = BinaryOperator::Create(Instruction::FMul, new_op_0, new_op_1, "", BB);
			break;
		}
		case 17:{	//FDiv
			if(cfgAC->no_zero_avoidance){
				result_val = BinaryOperator::Create(Instruction::FDiv, new_op_0, new_op_1, "", BB);
			}else{
				CastInst* int32_op_1 = new FPToSIInst(new_op_1, IntegerType::get(mod->getContext(), 32), "", BB);
				Value* op_1_altered = BinaryOperator::Create(Instruction::Or, int32_op_1, const_one, "", BB);
				CastInst* float_op_1 = new SIToFPInst(op_1_altered, Type::getFloatTy(mod->getContext()), "", BB);
				result_val = BinaryOperator::Create(Instruction::FDiv, new_op_0, float_op_1, "", BB);
			}

			break;
		}
		case 21:{	//DDiv
			if(cfgAC->no_zero_avoidance){
				result_val = BinaryOperator::Create(Instruction::FDiv, new_op_0, new_op_1, "", BB);
			}else{
				CastInst* int32_op_1 = new FPToSIInst(new_op_1, IntegerType::get(mod->getContext(), 64), "", BB);
				Value* op_1_altered = BinaryOperator::Create(Instruction::Or, int32_op_1, const_one_64, "", BB);
				CastInst* float_op_1 = new SIToFPInst(op_1_altered, Type::getDoubleTy(mod->getContext()), "", BB);
				result_val = BinaryOperator::Create(Instruction::FDiv, new_op_0, float_op_1, "", BB);
			}
			break;
		}
		case 22:{	//ADD8
			result_val = BinaryOperator::Create(Instruction::Add, new_op_0, new_op_1, "", BB);
			break;
		}
		case 23:{	//SUB8
			result_val = BinaryOperator::Create(Instruction::Sub, new_op_0, new_op_1, "", BB);
			break;
		}
		case 24:{	//MULT8
			result_val = BinaryOperator::Create(Instruction::Mul, new_op_0, new_op_1, "", BB);
			break;
		}
		case 25:{	//DIV8
			//This is to make sure it's not having the divided by zero problem
			if(cfgAC->no_zero_avoidance){
				result_val = BinaryOperator::Create(Instruction::SDiv, new_op_0, new_op_1, "", BB);
			}else{
				Value* op_1_altered = BinaryOperator::Create(Instruction::Or, new_op_1, const_one, "", BB);
				result_val = BinaryOperator::Create(Instruction::SDiv, new_op_0, op_1_altered, "", BB);
			}
			break;
		}
		default:{
			break;
		}
	}
	return result_val;
}

//This method creates a converting instruction to TarType
Value* CFGNtk::create_convert_instr(Module* mod, Value* val, Type *TarType, BasicBlock* BB){
	Value* result_val;
	Type * curType=val->getType();
	assert(curType!=TarType);
	if(curType->isIntegerTy(64)){
		if(TarType->isIntegerTy(8)){
			return result_val = new TruncInst(val, IntegerType::get(mod->getContext(), 8), "", BB);
		}else if(TarType->isIntegerTy(32)){
			return result_val = new TruncInst(val, IntegerType::get(mod->getContext(), 32), "", BB);
		}else if(TarType->isFloatTy()){
			return result_val = new SIToFPInst(val, Type::getFloatTy(mod->getContext()), "", BB);
		}else if(curType->isDoubleTy()){
			return result_val = new SIToFPInst(val, Type::getDoubleTy(mod->getContext()), "", BB);
		}
	}else if(curType->isIntegerTy(32)){
		if(TarType->isIntegerTy(64)){
			return result_val = new SExtInst(val, IntegerType::get(mod->getContext(), 64), "", BB);
		}else if(TarType->isIntegerTy(8)){
			return result_val = new TruncInst(val, IntegerType::get(mod->getContext(), 8), "", BB);
		}else if(TarType->isFloatTy()){
			return result_val = new SIToFPInst(val, Type::getFloatTy(mod->getContext()), "", BB);
		}else if(curType->isDoubleTy()){
			return result_val = new SIToFPInst(val, Type::getDoubleTy(mod->getContext()), "", BB);
		}
	}else if(curType->isFloatTy()){
		if(TarType->isDoubleTy()){
			return result_val = new FPExtInst(val, Type::getDoubleTy(mod->getContext()), "", BB);
		}else if(TarType->isIntegerTy(32)){
			return result_val = new FPToSIInst(val, IntegerType::get(mod->getContext(), 32), "", BB);
		}else if(TarType->isIntegerTy(64)){
			return result_val = new FPToSIInst(val, IntegerType::get(mod->getContext(), 64), "", BB);
		}else if(TarType->isIntegerTy(8)){
			return result_val = new FPToSIInst(val, IntegerType::get(mod->getContext(), 8), "", BB);
		}
	}else if(curType->isDoubleTy()){
		if(TarType->isFloatTy()){
			return result_val = new FPTruncInst(val, Type::getFloatTy(mod->getContext()), "", BB);
		}else if(TarType->isIntegerTy(32)){
			return result_val = new FPToSIInst(val, IntegerType::get(mod->getContext(), 32), "", BB);
		}else if(TarType->isIntegerTy(64)){
			return result_val = new FPToSIInst(val, IntegerType::get(mod->getContext(), 64), "", BB);
		}else if(TarType->isIntegerTy(8)){
			return result_val = new FPToSIInst(val, IntegerType::get(mod->getContext(), 8), "", BB);
		}
	}else if(curType->isIntegerTy(8)){
		if(TarType->isIntegerTy(32)){
			return result_val = new SExtInst(val, IntegerType::get(mod->getContext(), 32), "", BB);
		}else if(TarType->isIntegerTy(64)){
			return result_val = new SExtInst(val, IntegerType::get(mod->getContext(), 64), "", BB);
		}else if(TarType->isFloatTy()){
			return result_val = new SIToFPInst(val, Type::getFloatTy(mod->getContext()), "", BB);
		}else if(curType->isDoubleTy()){
			return result_val = new SIToFPInst(val, Type::getDoubleTy(mod->getContext()), "", BB);
		}
	}

	return NULL;
}
void CFGNtk::update_level(){
	int i;
	BBlock *pBB, *BBFanin;
	queue<BBlock*> qBB;
	//assign the output nodes as level 1
	qBB.push(this->Exit);

	while(qBB.size()>0){
		pBB = qBB.front();
		qBB.pop();

		BBForEachFanin( pBB, BBFanin, i ){
			BBFanin->level = BBFanin->assign_level();
			qBB.push(BBFanin);
		}
	}
}
void CFGNtk::reset_visit(){
	BBlock *BB;
	int i;
	NtkForEachBBlock(this, BB, i ){
		BB->visited = 0;
	}
}
int CFGNtk:: CFG_NtkVerification(){
	int i, j;
	BBlock *BB, *BBFanin, *BBFanout;
	NtkForEachBBlock(this, BB, i ){
		int inNodeNum = BB->DFG_NtkGenerator->Ntk->NumPis;
		BBForEachFanin( BB, BBFanin, j ){
			if(BBFanin->DFG_NtkGenerator->Ntk->NumPos!=inNodeNum) {
				printf("[Ntk ERROR]Block %d's input node number %d does not match output number %d at block %d\n", BB->BBidx, inNodeNum, BBFanin->DFG_NtkGenerator->Ntk->vPos.size(), BBFanin->BBidx);
				//assert(BBFanin->outputNodeNum ==inNodeNum);
				return 0;
			}
		}
		int outNodeNum = BB->DFG_NtkGenerator->Ntk->NumPos;
		BBForEachFanout( BB, BBFanout, j ){
			if(BBFanout->DFG_NtkGenerator->Ntk->NumPis!=outNodeNum) {
				printf("[Ntk ERROR]Block %d's output node number does not match input number at block %d\n", BB->BBidx, BBFanin->BBidx);
				//assert(BBFanout->inputNodeNum==outNodeNum);
				return 0;
			}
		}
	}

	return 1;
}
