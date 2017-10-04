/*
 * OperationInfo.h
 *
 *  Created on: 2012-08-14
 *      Author: fire
 */

#ifndef OPERATIONINFO_H_
#define OPERATIONINFO_H_
#include <string.h>
#include <fstream>

using namespace std;

class OperationInfo {
public:
	OperationInfo(string intype, float inRatio);
	string optype;
	int opIdx;
	float ratio;
	int minIdx;
	int maxIdx;
};

#endif /* OPERATIONINFO_H_ */
