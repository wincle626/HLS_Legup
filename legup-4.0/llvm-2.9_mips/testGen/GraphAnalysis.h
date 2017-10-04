/*
 * GraphAnalysis.h
 *
 *  Created on: 2012-09-14
 *      Author: fire
 */

#ifndef GRAPHANALYSIS_H_
#define GRAPHANALYSIS_H_

#include "AutoConfig.h"
#include "DFGGenerator.h"
#include "CFGGenerator.h"
#include <map>

namespace llvm {

class GraphAnalysis {
public:
	AutoConfig* AC;
	map<string, int> OperationCounter;

	GraphAnalysis(AutoConfig* ACin);
	void StartAnalysis(CFGNtk *CFG_Ntk);
	void DFGAnalysis(DAGDFGNtk *DFG_Ntk);
	void PrintAnalysisReuslt();
	virtual ~GraphAnalysis();
};

} /* namespace llvm */
#endif /* GRAPHANALYSIS_H_ */
