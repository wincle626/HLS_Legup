/*
 * AutoConfig.cpp
 *
 *  Created on: 2012-08-13
 *      Author: fire
 */

#include "AutoConfig.h"


using namespace std;
static char *StringToCharArray(string inString);
static int compare_float(float f1, float f2);


AutoConfig::AutoConfig(string fileName) {
	char *Filename = StringToCharArray(fileName);
	int j;
	OperationInfo *oi;
	ifstream myconfigfile (Filename);
	BBNum = 0;
	InputNum = 0;
	OutputNum = 0;
	DepthFactor = 0;
	ConstantDensity = 3;
	has_seed = 0;
	enable_loop = 0;
	enable_sub_funtion = 0;
	possibility_resolution = 10000;
	maxSubFuncLevel=1;
	maxCFGnum = 1;
	input_connectivity = 4;
	no_zero_avoidance = 0;

	enable_patterns = 0;
	patternSize = -1;
	pattern_ratio = -1;
	patternInputSize = -1;
	pattern_pool_ratio = -1;
	numreplicatedBBs = 0;

	fixed_block_size = 0;
	block_size_factor = 2;

	if (myconfigfile.is_open()){
		string line;
		while ( myconfigfile.good() ){
			getline (myconfigfile,line);
			char *strLine = StringToCharArray(line);
			if(strLine[0]=='#') continue;
			char * pch;
			pch = strtok (strLine," ");
			if (pch != NULL){
				string OpName;
				float OpRatio;
				if( strcmp(pch, addOp) == 0){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 0);
				}else if(strcmp(pch, subOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 1);
				}else if(strcmp(pch, mulOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 2);
				}else if(strcmp(pch, divOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 3);
				}else if(strcmp(pch, laddOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 4);
				}else if(strcmp(pch, lsubOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 5);
				}else if(strcmp(pch, lmulOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 6);
				}else if(strcmp(pch, ldivOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 7);
				}else if(strcmp(pch, shlOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 8);
				}else if(strcmp(pch, lshrOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 9);
				}else if(strcmp(pch, ashrOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 10);
				}else if(strcmp(pch, lshlOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 11);
				}else if(strcmp(pch, llshrOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 12);
				}else if(strcmp(pch, lashrOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 13);
				}else if(strcmp(pch, faddOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 14);
				}else if(strcmp(pch, fsubOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 15);
				}else if(strcmp(pch, fmulOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 16);
				}else if(strcmp(pch, fdivOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 17);
				}else if(strcmp(pch, daddOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 18);
				}else if(strcmp(pch, dsubOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 19);
				}else if(strcmp(pch, dmulOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 20);
				}else if(strcmp(pch, ddivOp) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 21);

				}else if(strcmp(pch, add8Op) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 22);

				}else if(strcmp(pch, sub8Op) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 23);

				}else if(strcmp(pch, div8Op) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 24);

				}else if(strcmp(pch, mul8Op) == 0 ){
					OpName = string(pch);
					OpRatio = atof(strtok (NULL, " "));
					assert(OpRatio>0);
					addOI(OpName, OpRatio, 25);

				}else if(strcmp(pch, BBnum_para) == 0){
					BBNum = atoi(strtok (NULL, " "));
				}else if(strcmp(pch, Input_num) == 0){
					InputNum = atoi(strtok (NULL, " "));
				}else if(strcmp(pch, Output_num) == 0){
					OutputNum = atoi(strtok (NULL, " "));
				}else if(strcmp(pch, Depth_Factor) == 0){
					DepthFactor = atof(strtok (NULL, " "));
					assert(DepthFactor>=0);
					assert(DepthFactor<=1);

				}else if(strcmp(pch, Const_Density) == 0){
					ConstantDensity = atoi(strtok (NULL, " "));
				}else if(strcmp(pch, In_Seed) == 0){
					seed = atoi(strtok (NULL, " "));
					has_seed = 1;
				}else if(strcmp(pch, Max_CFG_Num) == 0){
					maxCFGnum = atoi(strtok (NULL, " "));
				}else if(strcmp(pch, Max_SUB_FUNC_LVL) == 0){
					maxSubFuncLevel = atoi(strtok (NULL, " "));
				}else if(strcmp(pch, Enable_Loop) == 0){
					enable_loop = atoi(strtok (NULL, " "));
					if(enable_loop>1) enable_loop=1;
				}else if(strcmp(pch, Array_Input) == 0){
					//useArrayInput = atoi(strtok (NULL, " "));
					char* useArrayInput_Val = strtok (NULL, " ");
					assert(strcmp(useArrayInput_Val, True_Value) == 0 || strcmp(useArrayInput_Val, False_Value) == 0);
					if(strcmp(useArrayInput_Val, True_Value) == 0)useArrayInput=1;
				}else if(strcmp(pch, Enable_Sub_Function) == 0){
					enable_sub_funtion = atoi(strtok (NULL, " "));
					if(enable_sub_funtion>1) enable_sub_funtion=1;
				}else if(strcmp(pch, Enable_Pattern) == 0){
					enable_patterns = atoi(strtok (NULL, " "));
					if(enable_patterns>1) enable_patterns=1;
				}else if(strcmp(pch, Pattern_Ratio) == 0){
					pattern_ratio = atof(strtok (NULL, " "));
					if(pattern_ratio>1)pattern_ratio=1;
					assert(pattern_ratio>0);
				}else if(strcmp(pch, Pattern_Pool_Ratio) == 0){
					pattern_pool_ratio = atof(strtok (NULL, " "));
					if(pattern_pool_ratio>1)pattern_pool_ratio=1;
					assert(pattern_pool_ratio>0);
				}else if(strcmp(pch, Pattern_Size) == 0){
					patternSize = atoi(strtok (NULL, " "));
					assert(pattern_ratio>0);
				}else if(strcmp(pch, Pattern_Input_Size) == 0){
					patternInputSize = atoi(strtok (NULL, " "));
					assert(patternInputSize>1);
				}else if(strcmp(pch, Fix_Block_Size) == 0){
					//fixed_block_size = atoi(strtok (NULL, " "));
					char* fixed_block_size_val = strtok (NULL, " ");
					//if(fixed_block_size>1)fixed_block_size=1;
					assert(strcmp(fixed_block_size_val, True_Value) == 0 || strcmp(fixed_block_size_val, False_Value) == 0);
					if(strcmp(fixed_block_size_val, True_Value) == 0)fixed_block_size=1;
				}else if(strcmp(pch, Block_Size_Factor) == 0){
					block_size_factor = atoi(strtok (NULL, " "));
					//assert(block_size_factor>2);
				}else if(strcmp(pch, No_Zero_Avoidance) == 0){
					no_zero_avoidance = atoi(strtok (NULL, " "));
				}else if(strcmp(pch, Number_Replicated_Blocks) == 0){
					numreplicatedBBs = atoi(strtok (NULL, " "));
				}
			}
		}
	}
	float sum=0;
	int counter=0;
	AFForEachOperation( this, oi, j ){
		sum+=oi->ratio;
		oi->minIdx=counter;
		oi->maxIdx=counter+oi->ratio*possibility_resolution;
		counter=oi->maxIdx;
		//cout << oi->optype << ": " << oi->ratio << endl;
		//cout << oi->optype << " between: " << oi->minIdx << " and " << oi->maxIdx << endl;
	}
	float aa=1.0;
	cout << "Total weight of all the operations is " <<sum << endl;
	assert(compare_float(sum, 1.0));
	CFG_idx = 0;
	CFG_DEPTH = 0;

	if(enable_patterns){
		assert(patternSize>0);
		assert(patternInputSize>0);
		assert(pattern_ratio>0);
		assert(pattern_pool_ratio>0);
	}
	if(numreplicatedBBs>0){
		enable_bb_patterns = 1;
	}
	//patternNumCap = BBNum*pattern_ratio;
	//if(patternNumCap==0) patternNumCap = 1;

	cout << "BBNum: " << BBNum << endl;
	cout << "InputNum: " << InputNum << endl;
	cout << "OutputNum: " << OutputNum << endl;
	cout << "DepthFactor: " << DepthFactor << endl;


}

void AutoConfig::addDFGPattern(int cap, int seed, int inputNum, int outputNum, float inDepthFactor){
	DFGPattern * Pattern = new DFGPattern();
	Pattern->cap=cap;
	Pattern->seed=seed;
	Pattern->inputNum=inputNum;
	Pattern->outputNum=outputNum;
	Pattern->inDepthFactor=inDepthFactor;

	Pattern->used_times = 0;

	this->vDFGPatterns.push_back(Pattern);
}
static int compare_float(float f1, float f2) {
	float precision = 0.00001;
	if (((f1 - precision) < f2) && ((f1 + precision) > f2)) {
		return 1;
	} else {
		return 0;
	}
}

static char *StringToCharArray(string inString){
	int TempNumOne=inString.size();
	char *Filename = new char[TempNumOne+1];
	for (int a=0;a<=TempNumOne;a++){
		Filename[a]=inString[a];
	}
	return Filename;
}
void AutoConfig::addOI(string intype, float inRatio, int tyIdx){
	OperationInfo* oi = new OperationInfo(intype, inRatio);
	oi->opIdx = tyIdx;
	//cout << oi->optype << ": " << oi->ratio << endl;
	vOperationInfo.push_back(oi);
}
