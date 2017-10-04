//===-- ResourceEstimator.cpp -----------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Implements ResourceEstimator class
//
//===----------------------------------------------------------------------===//
#include "ResourceEstimator.h"
#include "LegupConfig.h"
#include "Allocation.h"

using namespace llvm;
using namespace legup;


namespace legup {

std::string ResourceEstimator::getNoDSPMult(std::string OpName) {
    if (OpName == "signed_multiply_8")
        OpName = "signed_multiply_nodsp_8";
    else if (OpName == "signed_multiply_16")
        OpName = "signed_multiply_nodsp_16";
    else if (OpName == "signed_multiply_32")
        OpName = "signed_multiply_nodsp_32";
    else if (OpName == "signed_multiply_64")
        OpName = "signed_multiply_nodsp_64";
    return OpName;
}

// Count DSPs. We need to account for the case where we use more DSPs than
// available
void ResourceEstimator::getDSPEstimates() {
    EstimatedDSPs = 0;
    for(std::map<std::string, int>::iterator i = OperationUsage.begin(), e =
            OperationUsage.end(); i != e; ++i) {
        std::string opName = i->first;
        if(opName == "") continue;
        Operation *Op = LEGUP_CONFIG->getOperationRef(opName);
        int numOps = i->second;
        int DSPs = Op->getDSPElements()*numOps;
        // ignore operations that don't use DSPs
        if (!DSPs) continue;

        int DSPsLeft = LEGUP_CONFIG->getMaxDSPs() - EstimatedDSPs;

        if (DSPs > DSPsLeft) {
            // we've used up all the DSPs
            std::string noDSPEquivalentOp = getNoDSPMult(opName);
            int available = DSPsLeft/Op->getDSPElements();
            OperationUsage[noDSPEquivalentOp] += numOps - available;
            OperationUsage[opName] = available;
            EstimatedDSPs = available * Op->getDSPElements();
        } else {
            EstimatedDSPs += DSPs;
        }
    }
}

void ResourceEstimator::getEstimates() {
    getDSPEstimates();
    EstimatedCombinational = 0;
    EstimatedLEs = 0;
    for(std::map<std::string, int>::iterator i = OperationUsage.begin(), e =
            OperationUsage.end(); i != e; ++i) {
        std::string OpName = i->first;
        int Usage = i->second;
        if(OpName == "") continue;
        Operation *Op = LEGUP_CONFIG->getOperationRef(OpName);
        assert(Op);
        float Freq = Op->getFmax();
        if (Freq < EstimatedFmax && Freq != 0) {
            EstimatedFmax = Freq;
            CritOpName = OpName;
        }
        EstimatedCombinational += (Op->getLUTs())*Usage;
    }
    EstimatedLEs = int((EstimatedCombinational + EstimatedRegisters)*0.7);
}

void ResourceEstimator::print(raw_ostream &Out) {

    // updates OperationUsage
    for (Allocation::hw_iterator i = alloc->hw_begin(), e = alloc->hw_end();
            i != e; ++i) {
        GenerateRTL *HW = *i;
        HW->updateOperationUsage(OperationUsage);
        EstimatedRegisters += alloc->getRegCount(HW);
        EstimatedVariables += alloc->getVarCount(HW);
    }
    getEstimates();


    Out << "-------------------------- Datapath Operation Usage --------------------------\n\n";
    for(std::map<std::string, int>::iterator i = 
                OperationUsage.begin(), e = OperationUsage.end(); i != e; ++i) {
        std::string OpName = i->first;
        if (OpName == "")
            Out << "Operation \"" << "unknown" << "\" x " << i->second << " \n";
        else 
            Out << "Operation \"" << OpName << "\" x " << i->second << " \n";                   
    }
    Out << "\n----------------------------- General Information -----------------------------\n\n"
        << "Number of declared variables in main: " << EstimatedVariables << "\n"
        << "Critical path contains operation: " << CritOpName << "\n"
        << "\n------------------------------ Resource Estimate ------------------------------\n\n"
        << "Fmax: " << (int)EstimatedFmax << " MHz\n"
        << "Note: Assuming only " << LEGUP_CONFIG->getMaxDSPs() << " DSPs available\n"
        << "Warning: These estimates don't account for binding yet!\n"
        << "Logic Elements: " << EstimatedLEs << "\n"
        << "    Combinational: " << EstimatedCombinational << "\n"
        << "    Registers: " << EstimatedRegisters << "\n"
        << "DSP Elements: " << EstimatedDSPs << "\n";
}


} // End legup namespace

