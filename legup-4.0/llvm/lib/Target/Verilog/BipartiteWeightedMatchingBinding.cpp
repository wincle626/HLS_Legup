//===-- BipartiteWeightedMatchingBinding.cpp - Binding ----------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// LegUp uses a weighted bipartite matching heuristic to solve the
// binding problem. See paper: Huang et al, Data Path Allocation Based on
// Bipartite Weighted Matching, DAC'90
//
// The binding problem is represented using a bipartite graph with two vertex
// sets. The first vertex set corresponds to the operations being bound (i.e.
// LLVM instructions). The second vertex set corresponds to the available
// functional units.  A weighted edge is introduced from a vertex in the first
// set to a vertex in the second set if the corresponding operation is a
// candidate to be bound to the corresponding functional unit. We set the cost
// (edge weight) of assigning an operation to a functional unit using the
// cost function described in the comments of constructWeights().
//
// Weighted bipartite matching can be solved optimally in polynomial time using
// the Hungarian method [Kuhn 2010]. We formulate and solve the
// matching problem one clock cycle at a time until the operations in all clock
// cycles (states) have been bound.
//
//===----------------------------------------------------------------------===//

#include "BipartiteWeightedMatchingBinding.h"
#include "Allocation.h"
#include "LegupPass.h"
#include "Hungarian.h"
#include "GlobalNames.h"
#include "VerilogWriter.h"
#include "LegupConfig.h"
#include "MinimizeBitwidth.h"
#include "FiniteStateMachine.h"
#include "LVA.h"
#include "RTL.h"
#include "utils.h"
#include <iostream>
#include <iomanip>

using namespace llvm;
using namespace legup;

namespace legup {

// top level function in binding. Called by RTLGenerator
// modifies BindingInstrFU member variable
void BipartiteWeightedMatchingBinding::operatorAssignment() {

    // numFuncUnitsMap maps:
    // functional unit type -> number of functional units available
    std::map <std::string, int> &numFuncUnitsMap =
        this->alloc->getNumFuncUnits(this->Fp);

    // find the set of all instructions that will be shared
    std::set<Instruction*> Instructions;
    for (FiniteStateMachine::iterator state = this->fsm->begin(), se =
            this->fsm->end(); state != se; ++state) {

        // for each functional unit type
        for (std::map<std::string, int>::iterator f = numFuncUnitsMap.begin(),
                fe = numFuncUnitsMap.end(); f != fe; ++f) {
            std::string funcUnitType = f->first;

            for (State::iterator instr = state->begin(), ie = state->end();
                    instr != ie; ++instr) {
                if (shareInstructionWithFU(*instr, funcUnitType)) {
                    Instructions.insert(*instr);
                }
            }
        }
    }

    // Use this information to find every independent instruction
    // Fills the IndependentInstructions map:
    //      instruction -> set of all independent instructions
    AssignmentInfo assigned;
    FindIndependentInstructions(Instructions,
            assigned.IndependentInstructions, this->LVA, this->fsm);

    formatted_raw_ostream out(this->alloc->getBindingFile());

    out << "Running Bipartite Weighted Matching on function: " <<
        this->Fp->getName() << "\n";

    // assign operations to functional units one state at a time
    for (FiniteStateMachine::iterator state = this->fsm->begin(), se =
            this->fsm->end(); state != se; ++state) {

        // for each functional unit type
        for (std::map<std::string, int>::iterator f = numFuncUnitsMap.begin(),
                fe = numFuncUnitsMap.end(); f != fe; ++f) {
            std::string funcUnitType = f->first;
            int numFuncUnitsAvail = f->second;

            bindFunctUnitInState(out, state, funcUnitType, numFuncUnitsAvail,
                    assigned);
        }
    }
    out << "\n";
}

bool share_dsp (Instruction *I, Allocation *alloc) {
    assert(isMul(I));
    return (
        alloc->useExplicitDSP(I) && (
            LEGUP_CONFIG->getParameterInt("RESTRICT_TO_MAXDSP") ||
            LEGUP_CONFIG->getParameterInt("MULTIPUMPING")
        )
    );
}

// is the instruction a candidate to be shared?
bool BipartiteWeightedMatchingBinding::isInstructionSharable(Instruction *I,
        Allocation *alloc) {

    std::string FuName = LEGUP_CONFIG->getOpNameFromInst(I, alloc);
    bool sharing;
    if (!LEGUP_CONFIG->getOperationSharingEnabled(FuName, &sharing)
            && isMul(I)) {
        // handle multipliers specially
        return share_dsp(I, alloc);
    }

    // note: default is to share
    return sharing;
}


// shareInstructionWithFU() determines if an instruction is both:
// 1) a candidate operation to be shared
// 2) could be performed on this particular function unit type
bool BipartiteWeightedMatchingBinding::shareInstructionWithFU(Instruction *I,
        std::string funcUnitType) {

    if ( isInstructionSharable(I, this->alloc) ) {

        std::string opName = LEGUP_CONFIG->getOpNameFromInst(I, this->alloc);

        // the instruction doesn't have an associated functional unit type
        if (opName.empty()) return false;

        // the functional unit type doesn't match this operation
        if (funcUnitType != opName) return false;

        // found a candidate operation for sharing on this functional unit
        return true;
    }

    return false;
}


int** BipartiteWeightedMatchingBinding::vector_to_matrix(Table &v, int rows,
        int cols) {
    int i, j;
    int** m;
    // allocate an array of pointers (arrays)
    m = (int**)calloc(rows,sizeof(int*));
    assert(m);
    for(i = 0; i < rows; i++) {
        // allocate an array of integers
        m[i] = (int*)calloc(cols,sizeof(int));
        assert(m[i]);
        for(j = 0; j < cols; j++) {
            m[i][j] = v[i][j];
        }
    }
    return m;
}


void BipartiteWeightedMatchingBinding::solveBipartiteWeightedMatching(Table
        &weights, Table &assignments) {
    hungarian_problem_t p;

    int rows = weights.size();
    assert(rows > 0);
    int cols = weights[0].size();
    assert(cols > 0);

    // not necessary but good to enforce
    assert(rows == cols);

    int** m = vector_to_matrix(weights, rows, cols);

    /* initialize the hungarian_problem using the cost matrix*/
    hungarian_init(&p, m, rows, cols,
            HUNGARIAN_MODE_MINIMIZE_COST);

    /* solve the assignement problem */
    hungarian_solve(&p);

    /*
    fprintf(stderr, "assignment matrix: %d rows and %d columns.\n\n",
            matrix_size, matrix_size);

    fprintf(stderr, "cost-matrix:");
    hungarian_print_costmatrix(&p);

    fprintf(stderr, "assignment:");
    hungarian_print_assignment(&p);
    */

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            assignments[i][j] = p.assignment[i][j];
        }
    }

    /* free used memory */
    hungarian_free(&p);
    for(int i = 0; i < rows; i++) {
        free(m[i]);
    }
    free(m);
}

// We have three goals when binding operations to shared functional units.
// First, we would like to balance the sizes of the multiplexers across
// functional units to keep circuit performance high. Multiplexers with
// more inputs have higher delay, so it is desirable to avoid having a
// functional unit with a disproportionately large multiplexer on its input.
// Second, we want to recognize cases where we have shared inputs between
// operations, letting us save a multiplexer if the operations are assigned
// to the same functional unit. Lastly, during binding if we can assign two
// operations that have non-overlapping livetime intervals to the same
// functional unit, we can use a single output register for both operations.
// In this case we save a register, without needing a multiplexer. We use
// the LLVM live variable analysis pass to check for the livetime intervals.
// To account for these goals we use the following cost function to measure the
// benefit of assigning operation op to function unit fu:
// Cost(op, f u) = existingMuxInputsFactor * existingMuxInputs(f u) +
//                 newMuxInputsFactor * newMuxInputs(op, f u) -
//                 outputRegisterSharableFactor * outputRegisterSharable(op, f u)
// where existingMuxInputsFactor = 1, newMuxInputsFactor = 10, and
// outputRegisterSharableFactor = 5 to give priority to saving new
// multiplexer inputs, then output registers, and finally balancing the
// multiplexers.  Notice that sharing the output register reduces the cost,
// while the other factors increase it.

void BipartiteWeightedMatchingBinding::constructWeights(raw_ostream &out,
        Instruction *I, int operationIdx, std::string funcUnitType, int
        numFuncUnitsAvail, AssignmentInfo &assigned, Table &weights) {

    int existingMuxInputsFactor = 1;
    int newMuxInputsFactor = 10;
    // note: this is negative, a shared output register reduces the cost
    int outputRegisterSharableFactor = -5;


    for (int fu = 0; fu < numFuncUnitsAvail; fu++) {
        int weight = 0;
        std::string fuId = this->alloc->verilogNameFunction(this->Fp, this->Fp)
            + "_" + funcUnitType + "_" + utostr(fu);

        // check both operands
        for (User::op_iterator i = I->op_begin(), e =
                I->op_end(); i != e; ++i) {
            Instruction *operand = dyn_cast<Instruction>(*i);
            if (!operand) continue;
            if (assigned.existingOperands[fuId].find(operand) ==
                    assigned.existingOperands[fuId].end()) {
                weight += newMuxInputsFactor;
            } else {
                std::string instStr = getValueStr(I);
                limitString(instStr, 30);
                out << instStr << " can share an input with another operation "
                    "already assigned to " << fuId << "\n";
            }

        }

        bool outputRegSharable = false;
        for (set<Instruction*>::iterator i =
                assigned.existingInstructions[fuId].begin(), e =
                assigned.existingInstructions[fuId].end(); i
                != e; ++i) {
            Instruction *shared = *i;

            // check for shared output register
            if (assigned.IndependentInstructions[I].find(shared) !=
                    assigned.IndependentInstructions[I].end() ) {
                //errs() << "Shared output: " << *shared << "\n";
                outputRegSharable = true;
                break;
            }
        }

        if (outputRegSharable) {
            weight += outputRegisterSharableFactor;
            std::string instStr = getValueStr(I);
            limitString(instStr, 30);
            out << instStr << " can share an output register with another "
                "operation already assigned to " << fuId << "\n";
        }

        weight += existingMuxInputsFactor * assigned.muxInputs[fuId];

        weights[fu][operationIdx] = weight;

        //errs() << "weight " << fu << " " << operationIdx << " = " <<
        //    weights[fu][operationIdx] << "\n";
    }
}


void BipartiteWeightedMatchingBinding::UpdateAssignments(raw_ostream &out, int
        numOperationsToShare, std::string funcUnitType, int numFuncUnitsAvail,
        AssignmentInfo &assigned, Table &assignments) {
    std::string s;
    std::stringstream ss;

    // Note that assignments (and weights) are square matrices,
    // numFuncUnitsAvail x numFuncUnitsAvail
    // However, numOperationsToShare <= numFuncUnitsAvail so the iteration
    // below is within bounds
    assert(numOperationsToShare <= numFuncUnitsAvail);
    for (int fu = 0; fu < numFuncUnitsAvail; fu++) {
        for (int o = 0; o < numOperationsToShare; o++) {
            if (assignments[fu][o]) {
                Instruction *I = opInstr[o];
                std::string fuId = this->alloc->verilogNameFunction(this->Fp,
                        this->Fp) + "_" + funcUnitType + "_" + utostr(fu);
                this->setBindingInstrFU(I, fuId);

                int numOperands = 0;
                for (User::op_iterator i = I->op_begin(), e =
                        I->op_end(); i != e; ++i) {
                    Instruction *operand = dyn_cast<Instruction>(*i);
                    numOperands++;
                    if (!operand) continue;
                    if (assigned.existingOperands[fuId].find(operand) ==
                            assigned.existingOperands[fuId].end()) {
                        assigned.existingOperands[fuId].insert(operand);
                        assigned.muxInputs[fuId]++;
                    }
                }

                std::string instStr = getValueStr(opInstr[o]);
                limitString(instStr, 30);
                ss << instStr << " (idx: " << o << ") -> " << fuId <<
                    " (mux inputs: " << assigned.muxInputs[fuId] << ")\n";

                // number of operands for mem_dual_port is not 2
                //assert(numOperands == 2);
                assigned.existingInstructions[fuId].insert(I);
            }
        }
    }

    ss.flush();
    out << ss.str();
}


void BipartiteWeightedMatchingBinding::CheckAllWereAssigned(int
        numOperationsToShare, int numFuncUnitsAvail, Table &assignments) {

    int numOperationsWithAssignment = 0;
    for (int fu = 0; fu < numFuncUnitsAvail; fu++) {
        for (int o = 0; o < numOperationsToShare; o++) {
            if (assignments[fu][o]) {
                numOperationsWithAssignment++;
            }
        }
    }
    if (numOperationsWithAssignment != numOperationsToShare) {
        errs() << "numOperationsWithAssignment: " <<
            numOperationsWithAssignment << "\n";
        errs() << "numOperationsToShare: " << numOperationsToShare << "\n";
        assert(0 && "All operations weren't assigned!");
    }
}


void BipartiteWeightedMatchingBinding::printTable(raw_ostream &out, std::string
        funcUnitType, int numOperationsToShare, int numFuncUnitsAvail, Table
        &weights) {

    std::string s;
    std::stringstream ss;

    const int firstColWidth = 50;
    ss << setw(firstColWidth) << " ";
    for (int fu = 0; fu < numFuncUnitsAvail; fu++) {
        std::string fuId = this->alloc->verilogNameFunction(this->Fp, this->Fp)
            + "_" + funcUnitType + "_" + utostr(fu);
        ss << left << setw(30) << fuId;
    }
    ss << "\n";

    for (int o = 0; o < numOperationsToShare; o++) {
        std::string instStr = getValueStr(opInstr[o]);
        limitString(instStr, 30);
        std::string opStr = instStr + " (idx: " + utostr(o) + ")";
        ss << left << setw(firstColWidth) << opStr;
        for (int fu = 0; fu < numFuncUnitsAvail; fu++) {
            ss << left << setw(30) << weights[fu][o];
        }
        ss << "\n";
    }

    ss.flush();
    out << ss.str();
}

void BipartiteWeightedMatchingBinding::bindFunctUnitInState(raw_ostream &out,
        State* state, std::string funcUnitType, int numFuncUnitsAvail,
        AssignmentInfo &assigned) {
    // create a numFuncUnitsAvail x numFuncUnitsAvail matrix of integer weights
    // and assignments for solving the bipartite weighted matching problem
    Table weights(numFuncUnitsAvail, std::vector<int>(numFuncUnitsAvail));
    Table assignments(numFuncUnitsAvail, std::vector<int>(numFuncUnitsAvail));

    std::string tmp;
    raw_string_ostream weights_stream(tmp);

    // loop over all operations in this state
    int operationIdx = 0;
    for (State::iterator instr = state->begin(), ie = state->end();
            instr != ie; ++instr) {

        Instruction *I = *instr;
        if (shareInstructionWithFU(I, funcUnitType)) {
            constructWeights(weights_stream, I, operationIdx, funcUnitType,
                    numFuncUnitsAvail, assigned, weights);
            opInstr[operationIdx] = I;
            operationIdx++;
        }
    }

    // only share if there is more than one operation using
    // this functional unit
    int numOperationsToShare = operationIdx;
    if (numOperationsToShare >= 1) {
        out << "State: " << state->getName() << "\n";
        out << "Binding functional unit type: " << funcUnitType << "\n";

        assert(numOperationsToShare <= numFuncUnitsAvail && "Invalid schedule!");

        out << "Weight matrix for operation/function unit matching:\n";
        weights_stream.flush();
        out << weights_stream.str();
        printTable(out, funcUnitType, numOperationsToShare, numFuncUnitsAvail,
                weights);

        out << "Solving Bipartite Weighted Matching (minimize weights)...\n";
        solveBipartiteWeightedMatching(weights, assignments);
        out << "Assignment matrix after operation/function unit matching:\n";
        printTable(out, funcUnitType, numOperationsToShare, numFuncUnitsAvail,
                assignments);

        out << "Checking that every operator was assigned to a functional unit...";
        CheckAllWereAssigned(numOperationsToShare, numFuncUnitsAvail,
                assignments);
        out << "yes\n";

        out << "Binding operator -> functional unit assignments:\n";
        UpdateAssignments(out, numOperationsToShare, funcUnitType,
                numFuncUnitsAvail, assigned, assignments);
    }
}

} // End legup namespace
