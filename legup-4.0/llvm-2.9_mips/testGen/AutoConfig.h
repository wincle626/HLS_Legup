/*
 * AutoConfig.h
 *
 *  Created on: 2012-08-13
 *      Author: fire
 */

#ifndef AUTOCONFIG_H_
#define AUTOCONFIG_H_
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <map>
#include <vector>
#include "OperationInfo.h"
#include "DFGPattern.h"

using namespace std;
//32-bits Integer Operations
static const char addOp[] = "ADD";		//0
static const char subOp[] = "SUB";		//1
static const char divOp[] = "DIV";		//2
static const char mulOp[] = "MULT";		//3
//64-bits Integer Operations
static const char laddOp[] = "LADD";	//4
static const char lsubOp[] = "LSUB";	//5
static const char ldivOp[] = "LDIV";	//6
static const char lmulOp[] = "LMULT";	//7

//32-bits floating point Operations
static const char faddOp[] = "FADD";	//8
static const char fsubOp[] = "FSUB";	//9
static const char fdivOp[] = "FDIV";	//10
static const char fmulOp[] = "FMULT";	//11
//64-bits floating point Operations
static const char daddOp[] = "DADD";	//12
static const char dsubOp[] = "DSUB";	//13
static const char ddivOp[] = "DDIV";	//14
static const char dmulOp[] = "DMULT";	//15

//32-bits Shifting Operations
static const char shlOp[] = "SHL";		//16
static const char lshrOp[] = "LSHR";	//17
static const char ashrOp[] = "ASHR";	//18
//64-bits Shifting Operations
static const char lshlOp[] = "LSHL";	//19
static const char llshrOp[] = "LLSHR";	//20
static const char lashrOp[] = "LASHR";	//21


//32-bits Integer Operations
static const char add8Op[] = "ADD8";		//22
static const char sub8Op[] = "SUB8";		//23
static const char div8Op[] = "DIV8";		//24
static const char mul8Op[] = "MULT8";	//25

//Generator param
static const char BBnum_para[] = "BB_NUM";
static const char Input_num[]  = "INPUT_NUM";
static const char Output_num[] = "OUTPUT_NUM";
static const char Depth_Factor[] = "DEPTH_FACTOR";
static const char Const_Density[] = "CONST_DENSITY";
static const char Enable_Loop[] = "ENABLE_LOOP";
static const char Enable_Sub_Function[] = "ENABLE_SUB_FUNC";
static const char Array_Input[] = "ARRAY_INPUT";
static const char In_Seed[] = "SEED";
static const char Max_CFG_Num[] = "MAX_CFG_NUM";
static const char Max_SUB_FUNC_LVL[] = "MAX_SUB_FUNC_LVL";

static const char Fix_Block_Size[] = "FIX_BLOCK_SIZE";
static const char Block_Size_Factor[] = "BLOCK_SIZE_FACTOR";

static const char No_Zero_Avoidance[] = "NO_ZERO_AVOIDANCE";

static const char True_Value[] = "True";
static const char False_Value[] = "False";

//params for pattern sharing
static const char Enable_Pattern[] = "ENABLE_PATTERN";
static const char Pattern_Ratio[] = "PATTERN_RATIO";
static const char Pattern_Pool_Ratio[] = "PATTERN_POOL_RATIO";
static const char Pattern_Size[] = "PATTERN_SIZE";
static const char Pattern_Input_Size[] = "PATTERN_INPUT_SIZE";
static const char Number_Replicated_Blocks[] = "NUM_REPLICATED_BBS";

class AutoConfig {
public:
	AutoConfig(string fileName);
	void addDFGPattern(int cap, int seed, int inputNum, int outputNum, float inDepthFactor);
	vector<OperationInfo*> vOperationInfo;
	int BBNum;
	int InputNum;
	int OutputNum;
	float DepthFactor;
	int ConstantDensity;
	int has_seed;
	int seed;
	int enable_loop;
	int useArrayInput;
	int enable_sub_funtion;
	int possibility_resolution;
	int maxSubFuncLevel;
	int maxCFGnum;
	int input_connectivity;
	int fixed_block_size;
	int block_size_factor;
	int no_zero_avoidance;

	float pattern_ratio;
	float pattern_pool_ratio;
	int enable_bb_patterns;
	int enable_patterns;
	int patternSize;
	int patternInputSize;
	int numreplicatedBBs;

	int CFG_idx;
	int CFG_DEPTH;
	vector<DFGPattern*> vDFGPatterns;

private:
	void addOI(string intype, float inRatio, int tyIdx);

};
#define AFForEachOperation( AutoConfig, OperationInfo, i )                                                           \
    for ( i = 0; (i < AutoConfig->vOperationInfo.size()) && (((OperationInfo) = AutoConfig->vOperationInfo.at(i)), 1); i++ )
#endif /* AUTOCONFIG_H_ */
