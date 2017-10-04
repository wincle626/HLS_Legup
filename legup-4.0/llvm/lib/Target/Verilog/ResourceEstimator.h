//===-- ResourceEstimator.h -------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// ResourceEstimator estimates the number of resources required by the
// synthesized circuit
//
//===----------------------------------------------------------------------===//
#ifndef LEGUP_RESOURCE_ESTIMATOR_H
#define LEGUP_RESOURCE_ESTIMATOR_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include <map>
#include <string>

using namespace llvm;

namespace legup {

class Allocation;

/// ResourceEstimator - Estimates the FPGA resources used by
/// the design.
/// @brief ResourceEstimator class
class ResourceEstimator {
public:
    ResourceEstimator(Allocation *alloc) : alloc(alloc), EstimatedFmax(1000),
    EstimatedCombinational(0), EstimatedRegisters(0), EstimatedVariables(0),
    EstimatedLEs(0), EstimatedDSPs(0) {}

    /// prints early resource estimation
    void print(raw_ostream &Out);

private:
    /// getEstimates - finds area and speed estimates
    void getEstimates();
    void printResourceEstimate(raw_ostream &Out);
    void getDSPEstimates();
    std::string getNoDSPMult(std::string OpName);

    Allocation *alloc;
    std::map <std::string, int> OperationUsage;
    std::string CritOpName;
    float EstimatedFmax;
    int EstimatedCombinational;
    int EstimatedRegisters;
    int EstimatedVariables;
    int EstimatedLEs;
    int EstimatedDSPs;
};

} // End legup namespace

#endif
