/*
 * DFGPattern.h
 *
 *  Created on: 2012-08-29
 *      Author: fire
 */

#ifndef DFGPATTERN_H_
#define DFGPATTERN_H_
//Data Container for DFG Patterns
class DFGPattern {
public:
	DFGPattern();
	int cap;
	int seed;
	int inputNum;
	int outputNum;
	float inDepthFactor;
	int ArrayOutput;
	int ArrayInput;
	int Pattern_pool_idx;

	int used_times;
	virtual ~DFGPattern();
};



#endif /* DFGPATTERN_H_ */
