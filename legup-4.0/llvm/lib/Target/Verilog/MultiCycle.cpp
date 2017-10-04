//===-- GenerateRTL.cpp -----------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements functions of the the SDCScheduler and GenerateRTL
// objects that are related to profiling-driven multi-cycling
// See "Profiling-Driven Multi-Cycling in FPGA High-Level Synthesis"
//
//===----------------------------------------------------------------------===//

#include "Allocation.h"
#include "GenerateRTL.h"
#include "BipartiteWeightedMatchingBinding.h"
#include "PatternBinding.h"
#include "SchedulerDAG.h"
#include "SDCScheduler.h"
#include "LegupPass.h"
#include "LegupConfig.h"
#include "Ram.h"
#include "utils.h"
#include "RTL.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Metadata.h"
#include <sstream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "llvm/Support/FileSystem.h"

#include "llvm/IR/LLVMContext.h"
#include "EdmondMatching.h"
#include <lp_lib.h>
#include <map>
#include <algorithm>

#define DEBUG_TYPE "LegUp:GenerateRTL"

using namespace llvm;
using namespace legup;

namespace legup {

// -----------------------------------------------------------------------------
// Functions in the SDCScheduler class
// -----------------------------------------------------------------------------

bool has_uses_in_other_BB(Instruction *I)
// Local helper function to check whether this instruction
// has uses in other basic blocks
{
    BasicBlock *b = I->getParent();
    for (Value::user_iterator u = I->user_begin(); u != I->user_end(); ++u) {
        Instruction *s = dyn_cast<Instruction>(*u);
        if (s && s->getParent() != b) {
            return true; // Successor in other BB
        }
    }
    return false; // All successors in this BB
}

bool is_inst_a_mc_path_src_or_dest(Instruction *i) {
    // This function determines whether the given instruction acts as the source
    // or destination of a multi-cycle path. Many instructions will simply be
    // chained as part of the MC path, but some instructions cannot be chained
    // and therefore will start or terminate the path ("source" or
    // "destination").
    // Destinations are AKA sinks or roots. These source or dest instructions
    // will
    // not have their output registers removed, but rather their registers will
    // stay
    // and act as the source/dest of the MC path. So these source/dest
    // instructions
    // are equivalently "registered instructions" (after all other instructions
    // have
    // had their registers removed to enable MC / de-pipelined paths).
    //
    // Note that in most cases there is no distinction between sources or
    // destinations of MC paths, because if an instruction terminated one
    // MC path then it is also the beginning of the next one (except e.g. for
    // store instructions which are only dests).
    //
    // The instructions which are src/dsts of MC paths, i.e. whose registers
    // persist
    // even after de-pipelining of data paths, are:
    //
    //  - Instructions used across BB, including PHIs (for now)
    //  - call instructions: there are registers before module input
    //    ports (so the register is "outside the function")
    //  - Return instructions
    //  - Loads (registers are either memory_controller_out_reg_a or b)
    //  - Stores do NOT have registers (no input reg to LegUp altsyncrams)
    //    but still terminate data paths
    //  - Instructions with latency > 0
    //
    // Handle any more types as they come up (e.g. other terminators)

    bool is_mc_src_or_dest = false;

    // Loads and stores
    if (isMem(i)) {
        is_mc_src_or_dest = true;
    }
    // Instructions with latency
    // Note: we need to be careful with instructions that are pipelined
    // because pipelined instructions can't be sources of MC paths (new value
    // each cycle) unless we guarantee that there won't be new data during the
    // multi-cycle duration. So for now I'll add instructions with latency > 0
    // but exclude dividers and remainders, since they cannot be MC sources
    // (although maybe their inputs can come from MC paths?)
    /*
      else if (Scheduler::getNumInstructionCycles(i) > 0 && !isDiv(i) &&
      !isRem(i)) {
        is_mc_src_or_dest = true;
      }
    */
    // PHI instructions
    else if (isa<PHINode>(*i)) {
        is_mc_src_or_dest = true;
    }
    // Instructions used across BB
    else if (has_uses_in_other_BB(i)) {
        is_mc_src_or_dest = true;
    }
    // Call instructions
    else if (isa<CallInst>(*i)) {
        is_mc_src_or_dest = true;
    }
    // Ret instructions
    else if (isa<ReturnInst>(*i)) {
        is_mc_src_or_dest = true;
    }

    return is_mc_src_or_dest;
}

bool mc_push_inst_to_later_state(Instruction *i) {
    // This function determines whether the given instruction can be pushed to a
    // later state. We do this in the multi-cycle flow, after the first
    // scheduling.
    // Data paths are de-pipelined and the remaining instructions with registers
    // are the sources and roots of multi-cycle paths. We can then extend the
    // latency of MC paths by pushing these MC path root / destination
    // instructions
    // to later states.
    //
    // Note: See comments below. Ideally, this function is not needed, and
    // "is_inst_a_mc_path_src_or_dest()" should be called instead. But there
    // are some exceptions where instructions that are destinations of MC paths
    // actually shouldn't have their latencies extended (e.g. PHIs).

    bool can_push_to_later_state = false;

    // Loads and stores
    //  if (isMem(i)) {
    if (isa<StoreInst>(*i)) {
        // Note that in general we want to push instructions to an exact later
        // state, e.g. their current state + 1. But for loads and stores we
        // need to relax the constraint to any state >= current state + 1,
        // due to resource constraints (only 2 scheduled per state.)
        // Also, it might seem that loads are only sources of MC data paths
        // and never dests, but the address port of a load can have some
        // logic (getelementptr) so they can be dests as well.
        // For now, don't push loads, although some paths may benefit from
        // pushing.
        can_push_to_later_state = true;
    }
    /*
      // Instructions with latency
      else if (Scheduler::getNumInstructionCycles(i) > 0) {
        // See comment in is_inst_a_mc_path_src_or_dest()
        if (!isRem(i) && !isDiv(i)) {
          can_push_to_later_state = true;
        }
      }
    */
    // PHI instructions
    else if (isa<PHINode>(*i)) {

        // PHIs often terminate MC paths but they are not re-scheduled
        // because they should always be at the start of their BB
        // (i.e. don't push PHIs to a later state)
        // In the future however PHIs don't need to have registers, they
        // can simply be muxes

    }
    // Instructions used across BB
    else if (has_uses_in_other_BB(i)) {
        can_push_to_later_state = true;
    }
    // Call instructions
    else if (isa<CallInst>(*i)) {
        can_push_to_later_state = true;
    }
    /*
      // Note: pushing branches/returns is how to increase the latency
      // of the BB. I used to do this for every infrequent BB but now
      // I only do it if the BB has a MC path, since some BB are short
      // (e.g. 1 cycle), so it doesn't make sense to extend it e.g. from
      // 1 to 2 cycles if there aren't even any MC paths in it.

      // Ret instructions
      else if (isa<ReturnInst>(*i)) {
        //can_push_to_later_state = true;
      }

      // Branch instructions
      else if (isa<BranchInst>(*i)) {
        //can_push_to_later_state = true;
      }
      else if (isa<SwitchInst>(*i)) {
        //can_push_to_later_state = true;
      }
    */

    return can_push_to_later_state;
}

void SDCScheduler::find_instructions_to_delay(
    Function *F, std::map<Instruction *, int> &mc_dest_instructions_to_state)
// This function is used when removing registers from data paths. It iterates
// over all the instructions in the function to find those instructions which:
//  (1) are destinations of MC paths, and
//  (2) should be delayed in the second scheduling (scheduled in a later state)
// Destinations of MC paths include store instructions, loads (address port),
// and instructions which still have registers even after de-pipelining.
// But of these, only some should be scheduled differently to have the paths
// which they terminate extended, and this function finds those instructions
// which we should delay and maps them to their new intended state (with
// respect to their BB).
{
    // for all BBs in F
    for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b) {
        // Call a helper to find the instructions requiring registers
        if (alloc->is_BB_executed_infrequently(b)) {
            find_instructions_to_delay_in_BB(b, mc_dest_instructions_to_state);
        }
    }
}

void SDCScheduler::save_instruction_states(Function *F)
// Stores the state # for each instruction (with respect to its own BB)
{
    REAL *variables = new REAL[numVars];
    get_variables(lp, variables);

    // for all BBs in F
    for (Function::iterator b = F->begin(), be = F->end(); b != be; ++b) {
        // for all I in BB
        mc_debug() << "\n\nInstruction to state mappings:\n";
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
            unsigned idx = startVariableIndex[dag->getInstructionNode(i)];
            unsigned state = (unsigned)variables[idx];
            // Store this instruction state in a map
            alloc->add_registered_instruction(i, state);
            mc_debug() << "\n    Instruction " << *i << " mapped to state "
                       << state;

            // Also store whether or not this instruction still has a register
            // once we multi-cycle all the datapaths
            if (is_inst_a_mc_path_src_or_dest(i)) {
                alloc->set_register_type(i, "registered");
                mc_debug() << " (registered)";
            } else {
                alloc->set_register_type(i, "unregistered");
                mc_debug() << " (NOT registered)";
            }
        }
        mc_debug() << "\n";

        // Store the number of states in each BB.
        // Note: add 1 since map->getNumStates returns the last
        // state #, and states start at 0
        alloc->set_num_states_in_BB(b, map->getNumStates(b) + 1);
    }

    delete[] variables;
}

void SDCScheduler::find_instructions_to_delay_in_BB(
    BasicBlock *b, std::map<Instruction *, int> &mc_dest_instructions_to_state)
// Helper to find_instructions_to_delay
// It iterates over all the instructions in the function to find
// those instructions which are destinations of MC paths and should
// be delayed. It then  maps each of these instructions to the state
// (within their BB) that they should be re-scheduled.
{
    mc_debug() << "\n\nInfrequent Basic Block:";

    REAL *variables = new REAL[numVars];
    get_variables(lp, variables);

    // We may also want to push the terminator (branch, ret, etc.) of
    // this basic block. But only do that if the BB has some other
    // multi-cycle path in it.
    bool bb_has_multicycle_path = false;

    // for all I in BB
    for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
        // The updated state of i is its originally scheduled state
        // plus whatever latency we want to add
        unsigned idx = startVariableIndex[dag->getInstructionNode(i)];
        unsigned current_state = (unsigned)variables[idx];

        // By default we add latency depending on LLVM_PROFILE_EXTRA_CYCLES
        // However this does not work well for BB with multiple consecutive
        // paths. An alternative is to use a multiplier based on the state.
        unsigned extended_state;
        if (LEGUP_CONFIG->getParameterInt("LLVM_PROFILE_STRETCH_BB")) {
            unsigned stretch_factor =
                LEGUP_CONFIG->getParameterInt("LLVM_PROFILE_EXTRA_CYCLES");
            extended_state = (unsigned)((float)current_state *
                                        (float)(stretch_factor + 0.5));
        } else {
            unsigned extra_latency =
                LEGUP_CONFIG->getParameterInt("LLVM_PROFILE_EXTRA_CYCLES");
            extended_state = current_state + extra_latency;
        }
        // Another idea: Can also extend more if very infrequent (e.g. 1%,
        // extend 2 cycles)

        // Determine whether we want to push this instruction to a later
        // state, i.e. if it terminates a MC path and is not a PHI or
        // something.
        if (mc_push_inst_to_later_state(i)) {
            // Push to a later cycle when we re-schedule.
            mc_debug() << "\nMulti-cycle paths: Pushing " << *i
                       << " to state >= " << extended_state;
            mc_dest_instructions_to_state[i] = extended_state;
            bb_has_multicycle_path = true;
        }
    }

    // If this BB had any multi-cycle paths, now extend the latency of
    // the entire BB, thereby dilating every path "exiting" this BB.
    if (bb_has_multicycle_path) {
        Instruction *t = b->getTerminator();
        mc_debug() << "\nBB has terminator " << *t;
        bool push_to_later_state = false;
        if (isa<ReturnInst>(*t)) {
            push_to_later_state = true;
        } else if (isa<BranchInst>(*t)) {
            push_to_later_state = true;
        }
        /*
            else if (isa<SwitchInst>(*t)) { // Not switches for now (change if
           needed)
              push_to_later_state = true;
            }
        */
        if (push_to_later_state) {
            unsigned idx = startVariableIndex[dag->getInstructionNode(t)];
            unsigned current_state = (unsigned)variables[idx];
            unsigned extended_state;
            if (LEGUP_CONFIG->getParameterInt("LLVM_PROFILE_STRETCH_BB")) {
                unsigned stretch_factor =
                    LEGUP_CONFIG->getParameterInt("LLVM_PROFILE_EXTRA_CYCLES");
                extended_state = (unsigned)((float)current_state *
                                            (float)(stretch_factor + 0.5));
            } else {
                unsigned extra_latency =
                    LEGUP_CONFIG->getParameterInt("LLVM_PROFILE_EXTRA_CYCLES");
                extended_state = current_state + extra_latency;
            }

            mc_debug() << "\nMulti-cycle paths: Pushing Terminator " << *t
                       << " to state >= " << extended_state;
            mc_dest_instructions_to_state[t] = extended_state;
        }
    }

    delete[] variables;
}

bool SDCScheduler::should_scheduling_be_repeated_to_add_latency() {
    bool repeat = true;
    if (!LEGUP_CONFIG->getParameterInt("LLVM_PROFILE")) {
        repeat = false;
    } else if (LEGUP_CONFIG->getParameterInt("LLVM_PROFILE_EXTRA_CYCLES") <=
               0) {
        repeat = false;
    }
    return repeat;
}

void SDCScheduler::multicycle_and_modify_schedule(Function *F)
// Check whether registers should be removed from paths, and if so remove the
// registers here. This is done by re-scheduling all instructions, with
// dependency constraints but without timing constraints, forcing all
// instructions to be chained (i.e. no pipeline registers for timing)
//
// Also, if software profiling was performed for this compilation,
// extend infrequent BB by a certain number of cycles by performing
// a second scheduling phase.
{
    // MULTICYCLE_TODO: For now disable multi-cycle paths in cases where
    // resources are pipelined (includes loop pipelining). Also only
    // enable for the pure-HW flow
    if (!LEGUP_CONFIG->does_flow_support_multicycle_paths()) {
        return;
    }

    // Extend the latency of paths in infrequently executed BB
    bool re_schedule_to_add_latency =
        should_scheduling_be_repeated_to_add_latency();
    if (re_schedule_to_add_latency) {
        // 1. Find instructions in all basic blocks whose state should be
        //    delayed in the second scheduling.
        std::map<Instruction *, int> mc_dest_instructions_to_state;
        find_instructions_to_delay(F, mc_dest_instructions_to_state);

        // 2. Delete the current LP and create a new one, adding
        //    only dependency constraints
        delete_lp(lp);
        numVars = 0;
        numInst = 0;
        createLPVariables(F);
        addMulticycleConstraints(F);
        addDependencyConstraints(F);

        // Note: For now I'll also re-add timing constraints
        addTimingConstraints(F);

        // 3. Add LP constraints for instructions whose state will persist
        //    in the second scheduling (i.e. MC destination instructions)
        std::map<Instruction *, int>::const_iterator inst_it =
            mc_dest_instructions_to_state.begin();
        for (; inst_it != mc_dest_instructions_to_state.end(); ++inst_it) {
            // Constraints are of the form:   1.0 * VAR_INDEX >= STATE
            REAL var_coefficient = 1.0;
            Instruction *persistent_instruction = inst_it->first;
            int var_index = 1 + startVariableIndex[dag->getInstructionNode(
                                    persistent_instruction)];
            REAL state = REAL(inst_it->second);
            // Force it to the same state
            add_constraintex(lp, 1, &var_coefficient, &var_index, GE, state);
        }

        // 4. Schedule ASAP
        scheduleASAP();

        // Resource constraints are added here and scheduling is done again
        do_post_scheduling_steps(F);

        // 5. Update the states for each instruction, as well as the number
        //    of instructions in each BB in case extending path latencies
        //    increased any BB latencies.
        depositSchedule(F);
    }

    // Store the states of all instructions. This is used in GenerateRTL
    // when printing multi-cycle timing constraints.
    save_instruction_states(F);
}

// -----------------------------------------------------------------------------
// Functions in the GenerateRTL class
// -----------------------------------------------------------------------------

// This function returns all instructions that are part of the conditional
// assignment of a particular signal. For example if signal S is connected
// to driver D when signal C==5, then this function returns the instruction
// corresponding with signal C (and all other such "conditional instructions").
//
// Conditions are structured as follows:
//
//          RTLSignal C     Constant 5
//                   \       /
//                    \     /
//                   RTLOp (EQ)     <-- "S->getCondition(0)" returns this RTLOp
//                       |
//                       |
//                    Signal S (AKA "curr" in this function)
//
// Except this may be deeper (i.e. multiple levels of operations).
// This function returns all conditional instructions, e.g. the one associated
// with C in the diagram aboce.
// This function is similar to the recursive "printConditions" in VerilogWriter.
void recursive_get_all_conditional_instructions(RTLSignal *curr,
                                                std::vector<Value *> &preds) {
    // Base case -- this condition is not an RTLOp
    if (!curr->isOp()) {
        // This condition could be cur_state, a memory controller signal,
        // a parameter (e.g. state #5), or an instruction. Only if it
        // is an instruction, add it to the predecessor queue
        Instruction *inst = curr->getInstPtr(0);
        if (inst) {
            preds.push_back((Value *)inst);
        }
    }
    // Otherwise continue the recursion
    else {
        RTLOp *op = (RTLOp *)curr;
        for (unsigned i = 0; i < op->getNumOperands(); ++i) {
            recursive_get_all_conditional_instructions(op->getOperand(i),
                                                       preds);
        }
    }
}

// Helper to remove_conditional_register
//
// MULTICYCLE_TODO: This does NOT remove all the registers for some reason...
// For example, if I have
//
//        if ((Flush_Buffer_0_5_reg == 1'd1))
//                next_state = LEGUP_F_Flush_Buffer_BB_6_5;
//        else if ((Flush_Buffer_0_5_reg == 1'd0))
//                next_state = LEGUP_F_Flush_Buffer_BB_30_31;
//
// I would like to replace Flush_Buffer_0_5_reg with Flush_Buffer_0_5.
// However, the code below only does this for Flush_Buffer_0_5_reg == 1'd1
// (makes it Flush_Buffer_0_5 == 1'd1)... but the == 1'd0 still reads the reg.
// This is true even though the code below does not stop after finding the
// first occurrence of the register to remove.
// For now I added "mc_remove_reg_from_icmp_instructions"
bool recursive_remove_conditional_register(
    RTLSignal *curr, RTLSignal *src_wire, RTLSignal *src_reg,
    bool &found_signal // Only need to find once
    ) {
    // Base case -- this condition is not an RTLOp
    if (!curr->isOp()) {
        // If this signal is the one we want, return true
        // and end the recursion
        if (curr == src_wire || curr == src_reg) {
            return true;
        }
    }
    // Otherwise continue the recursion
    else {
        bool found;
        RTLOp *op = (RTLOp *)curr;
        for (unsigned i = 0; i < op->getNumOperands(); ++i) {
            // If we find the required register, replace it with the
            // wire and end the recursion
            found = recursive_remove_conditional_register(
                op->getOperand(i), src_wire, src_reg, found_signal);
            if (found) {
                op->setOperand(i, src_wire);
                found_signal = true;
            }
        }
    }
    return false;
}

// Like recursive_get_all_conditional_instructions but instead of returning
// all conditional instructions, just remove the specified one.
bool remove_conditional_register(RTLSignal *dst, RTLSignal *src_wire,
                                 RTLSignal *src_reg) {
    bool found = false;
    for (unsigned i = 0; i < dst->getNumConditions(); ++i) {
        RTLSignal *cond = dst->getCondition(i);
        recursive_remove_conditional_register(cond, src_wire, src_reg, found);
        if (found) {
            return true;
        }
    }
    return false;
}

// Helper to get the number of drivers for a signal,
// whether or not the signal is an RTLOp
unsigned get_num_signal_drivers_helper(RTLSignal *s) {
    unsigned num_drivers = 0;
    if (s->isOp()) {
        num_drivers = ((RTLOp *)s)->getNumOperands();
    } else {
        num_drivers = s->getNumDrivers();
    }
    return num_drivers;
}

// Helper to get a drivers of a signal,
// whether or not the signal is an RTLOp
RTLSignal *get_signal_driver_helper(RTLSignal *s, unsigned i) {
    RTLSignal *driver = NULL;
    if (s->isOp()) {
        driver = ((RTLOp *)s)->getOperand(i);
    } else {
        driver = s->getDriver(i);
    }
    return driver;
}

// Helper to set a driver for a signal,
// whether or not the signal is an RTLOp
void set_signal_operand_helper(RTLSignal *s, unsigned i, RTLSignal *w) {
    if (s->isOp()) {
        ((RTLOp *)s)->setOperand(i, w);
    } else {
        s->setDriver(i, w);
    }
}

// Often the RTLOp is driven directly by the source register and we want to
// remove that register (connect wire directly). However sometimes the RTLOp
// is not driven directly by the register we need to remove, e.g. for
// getelementptr, and we need to do a "deeper search" of the drivers.
// This function handles all these cases (i.e. most of the time only 1 level
// of operands is searched, but more levels are searched if needed).
bool GenerateRTL::remove_intermediate_register_helper(RTLOp *rtl_op_driver,
                                                      RTLSignal *src_sig_w,
                                                      RTLSignal *src_sig_r) {
    // Set up BFS
    std::queue<RTLSignal *> frontier;
    std::set<RTLSignal *> visited;
    frontier.push(rtl_op_driver);
    visited.insert(rtl_op_driver);

    // Do BFS
    while (!frontier.empty()) {
        // Pop next
        RTLSignal *current = frontier.front();
        frontier.pop();

        // Check each driver
        unsigned num_drivers = get_num_signal_drivers_helper(current);
        // Loop over drivers of current
        for (unsigned i = 0; i < num_drivers; ++i) {
            RTLSignal *driver = get_signal_driver_helper(current, i);
            if (visited.find(driver) != visited.end()) {
                continue;
            }
            visited.insert(driver);
            frontier.push(driver);

            if (driver == src_sig_w || driver == src_sig_r) {
                // If the src is a register, connect to the wire instead
                if (driver == src_sig_r) {
                    set_signal_operand_helper(current, i, src_sig_w);
                }
                // Found the correct source
                return true;
            }
        }
    }
    return false;
}

// Given a store instruction and its input instruction, return the
// corresponding input port name
std::string GenerateRTL::get_store_port_name(Instruction *store,
                                             Instruction *input) {
    // Figure out if this path goes through the address or data port
    std::string port_type;
    if (input == store->getOperand(0)) {
        // Port 0 -- Data
        port_type = "_in_";
    } else if (input == store->getOperand(1)) {
        // Port 1 -- Address
        port_type = "_address_";
    } else {
        assert(0 && "Store with illegal operand");
    }

    // Check whether this store belongs to a local RAM
    std::string ram_name;
    StoreInst *SI = dyn_cast<StoreInst>(store);
    Value *addr = SI->getPointerOperand();
    RAM *ram = NULL;
    if (LEGUP_CONFIG->getParameterInt("LOCAL_RAMS")) {
        ram = alloc->getLocalRamFromValue(addr);
    }
    if (ram && alloc->isRAMLocaltoFct(Fp, ram)) {
        // The RAM is local to the function
        ram_name = ram->getName();
    } else {
        // The RAM belongs to the memory controller
        ram_name = "memory_controller";
    }

    // Port A or B?
    std::string port = connectedToPortB(store) ? "b" : "a";
    std::string full_port_name = ram_name + port_type + port;
    return full_port_name;
}

// Given a load instruction return its address port name
std::string GenerateRTL::get_load_port_name(Instruction *load) {
    // Check whether this load belongs to a local RAM
    std::string ram_name;
    LoadInst *LI = dyn_cast<LoadInst>(load);
    Value *addr = LI->getPointerOperand();
    RAM *ram = NULL;
    if (LEGUP_CONFIG->getParameterInt("LOCAL_RAMS")) {
        ram = alloc->getLocalRamFromValue(addr);
    }
    if (ram && alloc->isRAMLocaltoFct(Fp, ram)) {
        // The RAM is local to the function
        ram_name = ram->getName();
    } else {
        // The RAM belongs to the memory controller
        ram_name = "memory_controller";
    }
    std::string a_or_b = connectedToPortB(load) ? "b" : "a";
    std::string full_port_name = ram_name + "_address_" + a_or_b;
    return full_port_name;
}

// This function removes the register between dst and its
// operand (src), if a register exists.
void GenerateRTL::remove_intermediate_register(Instruction *src,
                                               Instruction *dst) {
    /*
     * Note: an alternative way to remove registers would be to modify
     * getOpNonConstant to return the wire instead of the register when
     * initially constructing the module. However, some instructions have
     * both register and wire uses. For example, an instruction may
     * have uses in the same state (chain directly) and also in another BB
     * (connected to its register).
     *
     * An alternative approach (this function) is to post-process the RTL
     * module to remove connections to registers and replace them with
     * direct connections to the wire.
     */

    /* The LLVM DFG and RTL Data structure graphs are different.
       For example consider the instruction %1 = add %2, %3 :

      LLVM Instructions           RTL Signals

                                %2 wire   %3 wire
                                   |       |
          %2    %3              %2 reg   %3 reg
            \  /                    \     /
             \/                      \   /
             %1                       \ /
                                  "Add" RTLOp
                                       |
                                       |
                                    %1 wire
                                       |
                                    %1 reg

        The inputs to this function are src and dst instructions, where
        dst is "%1 = add %2, %3" and src is one of %2 or %3

        For example if src is %2 and dst is %1, this function aims to
        modify the RTL above to directly connect the op to the wire src:

                                %2 wire   %3 wire
                                 / |       |
                           %2 reg  |      %3 reg
                                    \     /
                                     \   /
                                      \ /
                                  "Add" RTLOp
                                       |
                                       |
                                    %1 wire
                                       |
                                    %1 reg

    Then if needed we call it again later for src = %3.
    */

    // Get the RTL signal of dst (wire) and src (wire and reg)
    RTLSignal *src_sig_w = rtl->findExists(verilogName(src));
    RTLSignal *src_sig_r = rtl->findExists(verilogName(src) + "_reg");
    RTLSignal *dst_sig_w;
    if (isa<StoreInst>(*dst)) {
        dst_sig_w = rtl->findExists(get_store_port_name(dst, src));
    } else if (isa<LoadInst>(*dst)) {
        dst_sig_w = rtl->findExists(get_load_port_name(dst));
    } else {
        dst_sig_w = rtl->findExists(verilogName(dst));
    }

    if (!dst_sig_w) {
        mc_debug() << "\n    No signal, dst = " << *dst << ", "
                   << verilogName(dst) << "\n";
        return;
    }
    if (!src_sig_w) {
        mc_debug() << "\n    No signal, src = " << *src << ", "
                   << verilogName(src) << "\n";
        return;
    }
    if (!src_sig_r) {
        return; // Src has no register signal, so there is nothing to remove
    }
    if (!src_sig_r->isReg()) {
        return;
    }

    mc_debug() << "\nRemoving reg from " << src_sig_w->getName() << " to "
               << dst_sig_w->getName();

    bool found_correct_driver_signal = remove_intermediate_register_helper(
        (RTLOp *)dst_sig_w, src_sig_w, src_sig_r);
    if (!found_correct_driver_signal && isa<PHINode>(*dst)) {
        found_correct_driver_signal =
            remove_conditional_register(dst_sig_w, src_sig_w, src_sig_r);
        assert(found_correct_driver_signal);
    }
}

// Like remove_intermediate_register but the destination is a signal
// instead of an instruction
void
GenerateRTL::remove_intermediate_register_signal_dst(Instruction *src,
                                                     RTLSignal *dst_sig_w) {
    // MULTICYCLE_TODO: This is copied from remove_intermediate_register,
    // can refactor into a function which takes 2 signals

    // Get the RTL signal of dst (wire) and src (wire and reg)
    RTLSignal *src_sig_w = rtl->findExists(verilogName(src));
    RTLSignal *src_sig_r = rtl->findExists(verilogName(src) + "_reg");
    assert(dst_sig_w);

    if (!src_sig_w) {
        mc_debug() << "\n    No signal, src = " << *src << ", "
                   << verilogName(src) << "\n";
        return;
    }
    if (!src_sig_r) {
        return; // Src has no register signal, so there is nothing to remove
    }
    if (!src_sig_r->isReg()) {
        return;
    }

    mc_debug() << "\nRemoving reg (sig dst) from " << src_sig_w->getName()
               << " to " << dst_sig_w->getName();

    bool found_correct_driver_signal = remove_intermediate_register_helper(
        (RTLOp *)dst_sig_w, src_sig_w, src_sig_r);
    if (!found_correct_driver_signal) {
        found_correct_driver_signal =
            remove_conditional_register(dst_sig_w, src_sig_w, src_sig_r);
    }
    assert(found_correct_driver_signal);
}

// De-pipeline things like mults or shifts, but not dividers
bool is_instruction_pipelined_after_multicycling(Instruction *I) {
    bool pipelined = false;

    if (Scheduler::getNumInstructionCycles(I) > 0) {
        if (isDiv(I) || isRem(I) || isFPArith(I) || isFPCmp(I) || isFPCast(I)) {
            pipelined = true;
        }
    }

    return pipelined;
}

// Determine whether the combinational data path would still be
// functionally correct given this new predecessor
bool GenerateRTL::can_path_still_be_combinational(Instruction *pred) {
    bool can_remove_registers = true;

    // -------------------------------------------------------------------------
    // Case 1 -- pred is a load instruction
    // -------------------------------------------------------------------------
    if (isa<LoadInst>(*pred)) {
        // If we are duplicating memory controller output registers
        // so that each load is stored in a unique register, no need to
        // return false here
        if (!LEGUP_CONFIG->duplicate_load_reg()) {
            can_remove_registers = false;
        }
    }

    // -------------------------------------------------------------------------
    // Case 2 -- pred is function call
    // -------------------------------------------------------------------------
    // Outputs of a function are stored in a shared register
    else if (isa<CallInst>(*pred)) {
        mc_debug() << "Pred is call: " << *pred;
        can_remove_registers = false;
    }

    // -------------------------------------------------------------------------
    // Case 3 -- pred has pipeline registers
    // -------------------------------------------------------------------------
    else if (is_instruction_pipelined_after_multicycling(pred)) {
        can_remove_registers = false;
    }

    // -------------------------------------------------------------------------
    // Handle additional cases here if any are discovered
    // -------------------------------------------------------------------------

    return can_remove_registers;
}

// Write contraints from the vector to a file
void
print_multicycle_constraints_to_file(std::vector<std::string> sdc_constraints,
                                     std::vector<std::string> qsf_constraints) {
    std::ofstream sdc, qsf;

    // Don't print the same constraint multiple times
    bool print_qsf =
        LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_DISABLE_REG_MERGING");

    sdc.open("llvm_prof_multicycle_constraints.sdc", std::fstream::app);
    if (print_qsf) {
        qsf.open("llvm_prof_multicycle_constraints.qsf", std::fstream::app);
    }

    for (std::vector<std::string>::const_iterator sdc_it =
             sdc_constraints.begin();
         sdc_it != sdc_constraints.end(); ++sdc_it) {
        sdc << *sdc_it;
    }

    // Check if we should also print .qsf constraints to disable registers
    // with timing constraints to be merged
    if (print_qsf) {
        for (std::vector<std::string>::const_iterator qsf_it =
                 qsf_constraints.begin();
             qsf_it != qsf_constraints.end(); ++qsf_it) {
            qsf << *qsf_it;
        }
    }

    sdc.close();
    if (print_qsf) {
        qsf.close();
    }
}

// Is this an instruction which must have a register?
bool instruction_must_have_register(Instruction *i) {

    bool must_have_register = false;

    // If the predecessor is a load, then its value is stored in either
    // memory_controller_out_reg_a or b.
    if (isa<LoadInst>(*i)) {
        mc_debug() << "\n    Load -> " << *i << " has reg";
        must_have_register = true;
    }
    // PHI nodes also have registers
    else if (isa<PHINode>(*i)) {
        mc_debug() << "\n    PHI -> " << *i << " has reg";
        must_have_register = true;
    }

    return must_have_register;
}

// Determine whether there is a register between the predecessor instruction
// and the current instruction. In normal flows this defaults to checking
// whether they are in different states but in flows where registers are
// removed from data paths we need to do additional checks.
bool does_pred_have_register(Instruction *pred, Instruction *current,
                             bool remove_registers_from_data_paths,
                             FiniteStateMachine *fsm) {

    bool does_pred_have_register = false;

    // If we are removing registers from compare instructions then
    // return false right away
    if (isa<ICmpInst>(*pred) &&
        LEGUP_CONFIG->mc_remove_reg_from_icmp_instructions()) {
        return false;
    }

    // Check instructions which always have registers
    if (instruction_must_have_register(pred)) {
        does_pred_have_register = true;
    }
    // Check if the predecessor is in a different basic block
    else if (pred->getParent() != current->getParent()) {
        mc_debug() << "\n   Other BB from " << *current;

        // Special case of PHI
        if (!isa<PHINode>(*current)) {
            does_pred_have_register = true;
        }
    }
    // Check if the predecessor has latency (shift register)
    // But not things like mults etc. because we de-pipeline those
    // when mc paths are used
    else if (is_instruction_pipelined_after_multicycling(pred)) {
        mc_debug() << "\n    Latency > 0 -> " << *pred << " has reg";
        does_pred_have_register = true;
    }
    // Final case:
    // Only for flows where we are NOT removing registers from data
    // paths, there is also a register if pred is in a different state.
    else if (!remove_registers_from_data_paths) {
        if (fsm->getEndState(pred) != fsm->getEndState(current)) {
            does_pred_have_register = true;
        }
    }
    return does_pred_have_register;
}

// Usually this returns all the operands of the instruction current
// However, if current is a PHI also return any signals in the "if".
// This is required because the PHI is a Mux and the select (if
// conditions) can be part of combinational paths
std::vector<Value *> GenerateRTL::get_all_operands(Instruction *current) {
    std::vector<Value *> preds;

    // Return all operands
    for (unsigned i = 0; i < current->getNumOperands(); ++i) {
        preds.push_back(current->getOperand(i));
    }

    // Special case for PHI instructions only
    if (isa<PHINode>(current)) {
        RTLSignal *cur_sig = rtl->findExists(verilogName(current));
        assert(cur_sig);
        // errs() << "\nFound PHI with " << cur_sig->getNumConditions() << "
        // conditions: " << *current << "\n";
        for (unsigned i = 0; i < cur_sig->getNumConditions(); ++i) {
            recursive_get_all_conditional_instructions(cur_sig->getCondition(i),
                                                       preds);
        }
    }

    return preds;
}

// Helper function that takes a list of instruction pairs
// (Use STL map for simplicity) and removes intermediate
// registers between each pair
void GenerateRTL::remove_intermediate_registers(
    const std::multimap<Instruction *, Instruction *> &remove_reg_pairs) {
    std::multimap<Instruction *, Instruction *>::const_iterator i, e;
    i = remove_reg_pairs.begin(), e = remove_reg_pairs.end();
    for (; i != e; ++i) {
        remove_intermediate_register(i->first, i->second);
    }
}

// Like remove_intermediate_registers but removes registers
// from a single root signal to a set of sources
void GenerateRTL::remove_driver_registers_for_root_signal(
    RTLSignal *root, std::set<Instruction *> srcs) {
    std::set<Instruction *>::const_iterator i;
    for (i = srcs.begin(); i != srcs.end(); ++i) {
        remove_intermediate_register_signal_dst(*i, root);
    }
}

// Debugging only -- helper function that prints a instruction path
void print_path_helper(std::vector<Instruction *> v) {
    if (LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_DEBUG")) {
        errs() << "\nPath:\n";
        for (std::vector<Instruction *>::reverse_iterator vi = v.rbegin();
             vi != v.rend(); ++vi) {
            errs() << "    " << **vi << "\n";
        }
        errs() << "End of Path\n";
    }
}

// Helper function, takes a vector of instructions (a path), finds all
// adjacent pairs in this path, and removes the pairs from a multimap
// Borrowed from http://stackoverflow.com/questions/3952476/
void remove_all_pairs_in_path_from_multimap(
    std::vector<Instruction *> cur_path,
    std::multimap<Instruction *, Instruction *> &remove_reg_pairs,
    std::set<std::pair<Instruction *, Instruction *>> &keep_reg_pairs) {
    typedef std::multimap<Instruction *, Instruction *>::iterator mmap_it;
    std::vector<Instruction *>::const_iterator it = cur_path.begin(),
                                               end = cur_path.end();
    Instruction *previous = *it;
    ++it;

    for (; it != end; ++it) {

        // Get all of the pairs in the multimap with key = previous
        std::pair<mmap_it, mmap_it> iterpair =
            remove_reg_pairs.equal_range(*it);

        // Erase the pair (previous, *it)
        // Note that the vector is in the order root --> src
        mmap_it it2 = iterpair.first;
        for (; it2 != iterpair.second; ++it2) {
            if (it2->second == previous) {
                remove_reg_pairs.erase(it2);
                break;
            }
        }

        // Don't remove reg from this pair later
        keep_reg_pairs.insert(std::make_pair(*it, previous));

        // And update previous for next iteration
        previous = *it;
    }
}

// Helper function for dfs
void GenerateRTL::root_dfs_visit_preds_of_current(
    Instruction *current, std::stack<Instruction *> &frontier,
    std::vector<Value *> &preds, std::vector<std::vector<Instruction *>> &paths,
    std::vector<Instruction *> &cur_path,
    std::multimap<std::vector<Instruction *>, Value *> &paths_with_arg_srcs,
    std::set<Instruction *> &visited,
    std::multimap<Instruction *, Instruction *> &remove_reg_pairs,
    std::set<std::pair<Instruction *, Instruction *>> &keep_reg_pairs,
    std::map<Instruction *, int> &num_unfinished_preds) {
    std::vector<Value *>::const_iterator pred_i = preds.begin();
    for (; pred_i != preds.end(); ++pred_i) {
        Instruction *pred = dyn_cast<Instruction>(*pred_i);

        // Ignore non-instructions and predecessors already visited
        if (!pred) {
            // Note: Usually we ignore predecessors which are not instructions.
            // However, if the predecessor is an input to this function, then
            // we can use its register as a multi-cycle source
            if (dyn_cast<Argument>(*pred_i)) {
                mc_debug() << "\n    Found input arg: "
                           << verilogName(dyn_cast<Argument>(*pred_i));
                paths.push_back(cur_path);
                paths_with_arg_srcs.insert(std::make_pair(cur_path, *pred_i));
            }
            // Could also be a constant, etc., so just continue
            continue;
        }

        bool revisiting_pred = false;
        if (visited.find(pred) != visited.end()) {
            // Note we don't just continue if a pred is being revisited
            // because the path is different (different "current" inst.)
            // so we might still have to remove the reg.
            revisiting_pred = true;
        } else {
            visited.insert(pred);
        }

        // Check whether removing the register between current and pred
        // would be functionally correct. If not, then for now return an
        // empty list of predecessors.
        // Another solution would be to break up the path into smaller
        // paths by keeping some intermediate registers to
        // maintain correctness.
        mc_debug() << "\n    " << *pred;

        if (!can_path_still_be_combinational(pred)) {
            mc_debug() << "\n    Illegal, aborting current path\n";

            // Stop traversing this path in the DFS
            // Also if any instruction in this path was going to have its
            // register removed, undo this here
            remove_all_pairs_in_path_from_multimap(cur_path, remove_reg_pairs,
                                                   keep_reg_pairs);
            continue;
        }

        // Determine whether or not this predecessor has a register
        bool register_between_current_and_pred =
            does_pred_have_register(pred, current, true, sched->getFSM(Fp));

        // If there is a register between current and pred, add pred to
        // the vector. Otherwise push it onto the queue to examine
        // its predecessors.
        if (register_between_current_and_pred) {
            mc_debug() << "\n    Registered";
            cur_path.push_back(pred);
            print_path_helper(cur_path);
            paths.push_back(cur_path);
            cur_path.pop_back();
        } else {
            // No need to push pred on the frontier again
            if (!revisiting_pred) {
                frontier.push(pred);
                mc_debug() << "\n    Pushing onto queue";
                num_unfinished_preds[current]++;
            }

            // If this is a flow where registers are being removed from
            // data paths in the RTL, do the removal here.
            //
            // Don't remove registers between this pair of instructions if
            // we previously identified this as a pair which should have a
            // register in between
            if (keep_reg_pairs.find(std::make_pair(pred, current)) ==
                keep_reg_pairs.end()) {
                remove_reg_pairs.insert(
                    std::pair<Instruction *, Instruction *>(pred, current));
            }
        }
    }
    mc_debug() << "\n    Finished all preds of current = " << *current;
}

// Perform a DFS of the predecessors of root, within the same BB,
// and print multi-cycle constraints from all source registers.
void GenerateRTL::create_multicycle_paths_from_root(Instruction *root) {
    // Set up DFS
    std::stack<Instruction *> frontier;
    std::set<Instruction *> visited;
    frontier.push(root);
    visited.insert(root);

    // Keep track of the current path
    std::vector<Instruction *> cur_path;
    std::vector<std::vector<Instruction *>> paths;
    std::vector<std::string> sdc_constraints;
    std::vector<std::string> qsf_constraints;
    std::map<Instruction *, int> num_unfinished_preds;

    // List of all pairs of instructions to remove reg for.
    // Multimap since multiple paths with registers can exist
    // for the same instruction
    // (e.g. dst is a phi, with multiple sources from same reg)
    std::multimap<Instruction *, Instruction *> remove_reg_pairs;

    // Some paths have sources which are function arguments
    std::multimap<std::vector<Instruction *>, Value *> paths_with_arg_srcs;

    // Edit: We also need a set of pairs NOT to remove registers from...
    std::set<std::pair<Instruction *, Instruction *>> keep_reg_pairs;

    mc_debug() << "\n\nStarting a new DFS for " << *root << "\n";

    // Do DFS
    while (!frontier.empty()) {
        // Pop next
        Instruction *current = frontier.top();
        frontier.pop();
        cur_path.push_back(current);
        num_unfinished_preds[current] = 0;
        mc_debug() << "\n" << *current;
        std::vector<Value *> preds = get_all_operands(current);

        // Check each predecessor of current to continue the DFS
        root_dfs_visit_preds_of_current(
            current, frontier, preds, paths, cur_path, paths_with_arg_srcs,
            visited, remove_reg_pairs, keep_reg_pairs, num_unfinished_preds);

        // Backtrack in the DFS if we've finished all the predecessors
        // of the final instruction
        while (cur_path.back() != root &&
               num_unfinished_preds[cur_path.back()] == 0) {
            cur_path.pop_back();
            num_unfinished_preds[cur_path.back()]--;
            assert(num_unfinished_preds[cur_path.back()] >= 0);
        }
    }
    mc_debug() << "\nCompleted dst successfully\n";

    // This path wasn't illegal, so we can now finally remove
    // all the registers and print all the constraints
    remove_intermediate_registers(remove_reg_pairs);

    // Now print the .sdc and .qsf constraints for all the paths we found
    std::vector<std::vector<Instruction *>>::const_iterator path_it;
    for (path_it = paths.begin(); path_it != paths.end(); ++path_it) {
        add_multicycle_constraint_for_path(*path_it, sdc_constraints,
                                           qsf_constraints, keep_reg_pairs,
                                           paths_with_arg_srcs);
    }
    print_multicycle_constraints_to_file(sdc_constraints, qsf_constraints);
}

// Helper for create_multicycle_paths_to_signal DFS
void GenerateRTL::signal_dfs_visit_pred(
    Instruction *current, Instruction *pred,
    std::vector<Instruction *> &cur_path,
    std::vector<std::vector<Instruction *>> &paths, bool revisiting_pred,
    std::stack<Instruction *> &frontier, std::set<Instruction *> &visited,
    std::map<Instruction *, int> &num_unfinished_preds,
    std::set<std::pair<Instruction *, Instruction *>> &keep_reg_pairs,
    std::multimap<Instruction *, Instruction *> &remove_reg_pairs,
    std::set<Instruction *> &drivers_with_reg_to_remove) {

    bool register_between_current_and_pred = false;
    if (current) {
        register_between_current_and_pred =
            does_pred_have_register(pred, current, true, sched->getFSM(Fp));
    } else {
        // For the root signal, remove the intermediate register unless
        // it is a load or PHI
        if (isa<LoadInst>(*pred) || isa<PHINode>(*pred)) {
            // We need the register between the root signal and its
            // predecessor. Because this cannot be a multi-cycle path,
            // we can terminate the DFS here for this predecessor.
            return;
        }
    }

    // If there is a register between current and pred, add pred to
    // the vector. Otherwise push it onto the queue to examine
    // its predecessors.
    if (register_between_current_and_pred) {
        mc_debug() << "\n    Registered";
        cur_path.push_back(pred);
        print_path_helper(cur_path);
        paths.push_back(cur_path);
        cur_path.pop_back();
    } else if (current) {
        // If this instruction has already been pushed on the frontier,
        // don't push it again.
        if (!revisiting_pred) {
            frontier.push(pred);
            visited.insert(pred);
            mc_debug() << "\n    Pushing onto queue";
            num_unfinished_preds[current]++;
        }

        // If this is a flow where registers are being removed from
        // data paths in the RTL, do the removal here.
        //
        // Don't remove registers between this pair of instructions if
        // we previously identified this as a pair which should have a
        // register in between
        if (keep_reg_pairs.find(std::make_pair(pred, current)) ==
            keep_reg_pairs.end()) {
            remove_reg_pairs.insert(
                std::pair<Instruction *, Instruction *>(pred, current));
        }
    } else {
        if (!revisiting_pred) {
            frontier.push(pred);
            visited.insert(pred);
            drivers_with_reg_to_remove.insert(pred);
        }
    }
}

// Helper function for dfs
void GenerateRTL::signal_dfs_visit_preds_of_current(
    Instruction *current, std::stack<Instruction *> &frontier,
    std::vector<Value *> &preds, std::vector<std::vector<Instruction *>> &paths,
    std::vector<Instruction *> &cur_path,
    std::multimap<std::vector<Instruction *>, Value *> &paths_with_arg_srcs,
    std::set<Instruction *> &visited,
    std::multimap<Instruction *, Instruction *> &remove_reg_pairs,
    std::set<std::pair<Instruction *, Instruction *>> &keep_reg_pairs,
    std::set<Instruction *> &drivers_with_reg_to_remove,
    std::map<Instruction *, int> &num_unfinished_preds) {
    std::vector<Value *>::const_iterator pred_i = preds.begin();
    for (; pred_i != preds.end(); ++pred_i) {
        Instruction *pred = dyn_cast<Instruction>(*pred_i);

        // Ignore non-instructions and predecessors already visited
        if (!pred) {
            // Note: Usually we ignore predecessors which are not instructions.
            // However, if the predecessor is an input to this function, then
            // we can use its register as a multi-cycle source
            if (dyn_cast<Argument>(*pred_i)) {
                mc_debug() << "\n    Found input arg: "
                           << verilogName(dyn_cast<Argument>(*pred_i));
                paths.push_back(cur_path);
                paths_with_arg_srcs.insert(std::make_pair(cur_path, *pred_i));
            }
            // Could also be a constant, etc., so just continue
            continue;
        }

        bool revisiting_pred = false;
        if (visited.find(pred) != visited.end()) {
            // Note we don't just continue if a pred is being revisited
            // because the path is different (different "current" inst.)
            // so we might still have to remove the reg.
            revisiting_pred = true;
        }
        // Else: will add to visited later (if actually pushed on frontier)

        // Check whether removing the register between current and pred
        // would be functionally correct. If not, then for now return an
        // empty list of predecessors.
        // Another solution would be to break up the path into smaller
        // paths by keeping some intermediate registers to
        // maintain correctness.
        mc_debug() << "\n    " << *pred;

        if (!can_path_still_be_combinational(pred)) {
            mc_debug() << "\n    Illegal, aborting current path\n";

            // Stop traversing this path in the DFS
            // Also if any instruction in this path was going to have its
            // register removed, undo this here
            remove_all_pairs_in_path_from_multimap(cur_path, remove_reg_pairs,
                                                   keep_reg_pairs);

            // Also add back the register if it was removed from the root signal
            drivers_with_reg_to_remove.erase(cur_path.front());
            continue;
        }

        // Determine whether or not this predecessor has a register
        // If so, modify data structures to remove it later
        signal_dfs_visit_pred(current, pred, cur_path, paths, revisiting_pred,
                              frontier, visited, num_unfinished_preds,
                              keep_reg_pairs, remove_reg_pairs,
                              drivers_with_reg_to_remove);
    }

    if (current) {
        mc_debug() << "\n    Finished all preds of current = " << *current;
    }
}

// Like create_multicycle_paths_from_root but takes a signal as input rather
// than an instruction. Use this for signals which do not have a corresponding
// instruction, such as function arguments or the FSM state register (cur_state)
void GenerateRTL::create_multicycle_paths_to_signal(RTLSignal *root) {
    // Set up DFS
    std::stack<Instruction *> frontier;
    std::set<Instruction *> visited;

    // Keep track of the current path
    std::vector<Instruction *> cur_path;
    std::vector<std::vector<Instruction *>> paths;
    std::vector<std::string> sdc_constraints;
    std::vector<std::string> qsf_constraints;
    std::map<Instruction *, int> num_unfinished_preds;

    // List of all pairs of instructions to remove reg for.
    // Multimap since multiple paths with registers can exist
    // for the same instruction
    // (e.g. dst is a phi, with multiple sources from same reg)
    std::multimap<Instruction *, Instruction *> remove_reg_pairs;
    std::set<Instruction *> drivers_with_reg_to_remove;

    // Some paths have sources which are function arguments
    std::multimap<std::vector<Instruction *>, Value *> paths_with_arg_srcs;

    // Edit: We also need a set of pairs NOT to remove registers from...
    std::set<std::pair<Instruction *, Instruction *>> keep_reg_pairs;

    mc_debug() << "\n\nStarting a custom DFS for signal " << root->getName();

    // Do DFS
    bool first_iteration = true;
    while (first_iteration || !frontier.empty()) {
        Instruction *current = NULL;
        std::vector<Value *> preds;

        if (first_iteration) {
            first_iteration = false;

            // Get the drivers for the root signal. These consist of:
            //  1) Signals which directly drive this signal, and
            //  2) All signals in the conditional of an "if" statement (mux
            //  select)
            //     where this signal is assigned

            // 1. Direct drivers -- MULTICYCLE_TODO
            // For now only #2 is supported, which is required for cur_state
            // In the future, should support any signal (e.g. function args)
            assert(root->getName() == "cur_state");

            // 2. Conditional signals
            for (unsigned i = 0; i < root->getNumConditions(); ++i) {
                recursive_get_all_conditional_instructions(
                    root->getCondition(i), preds);
            }
        } else {
            // For every iteration but the 1st, the DFS continues normally
            current = frontier.top();
            frontier.pop();
            cur_path.push_back(current);
            num_unfinished_preds[current] = 0;
            mc_debug() << "\n" << *current;
            preds = get_all_operands(current);
        }

        // Check each predecessor of current to continue the DFS
        signal_dfs_visit_preds_of_current(
            current, frontier, preds, paths, cur_path, paths_with_arg_srcs,
            visited, remove_reg_pairs, keep_reg_pairs,
            drivers_with_reg_to_remove, num_unfinished_preds);

        // Backtrack in the DFS if we've finished all the predecessors
        // of the final instruction
        while (!cur_path.empty() &&
               num_unfinished_preds[cur_path.back()] == 0) {
            cur_path.pop_back();
            if (!cur_path.empty()) {
                num_unfinished_preds[cur_path.back()]--;
                assert(num_unfinished_preds[cur_path.back()] >= 0);
            }
        }
    }
    mc_debug() << "\nCompleted dst successfully\n";

    // This path wasn't illegal, so we can now finally remove
    // all the registers and print all the constraints
    remove_intermediate_registers(remove_reg_pairs);
    remove_driver_registers_for_root_signal(root, drivers_with_reg_to_remove);

    // Now print the .sdc and .qsf constraints for all the paths we found
    std::vector<std::vector<Instruction *>>::const_iterator path_it;
    for (path_it = paths.begin(); path_it != paths.end(); ++path_it) {
        add_multicycle_constraint_for_path_with_custom_root(
            *path_it, sdc_constraints, qsf_constraints, keep_reg_pairs,
            drivers_with_reg_to_remove, paths_with_arg_srcs, root);
    }
    print_multicycle_constraints_to_file(sdc_constraints, qsf_constraints);
}

// Get the source register name associated with this instruction
// MULTICYCLE_TODO: Right now this function just matches strings, which
// worked until pipeline instructions (e.g. mults) were added. Now, the
// names can be more tricky, e.g. _stage0_reg. This function should instead
// get the signal for the source Instruction, then look at signals it drives
// until it finds one for which isReg() is true, then return that signal name.
std::string GenerateRTL::get_source_register_name(Instruction *source) {
    std::string src_name = verilogName(source) + "_reg";

    // Special cases: Modify name

    if (!LEGUP_CONFIG->duplicate_load_reg()) {
        // Load
        if (isMem(source)) {
            assert(isa<LoadInst>(*source));
            assert(binding->existsBindingInstrFU(source));
            int connected_to_port_b = connectedToPortB(source);
            assert(connected_to_port_b != -1);
            if (connected_to_port_b) {
                src_name = "memory_controller_out_reg_b";
            } else {
                src_name = "memory_controller_out_reg_a";
            }
        }
    }
#if 0 // This is no longer needed since we de-pipeline multipliers
	if ( isMul(source) && Scheduler::getNumInstructionCycles(source) > 0) {
		// MULTICYCLE_TODO:
		// Mults with latency have output registers, and sometimes these
		// output registers can be combined into the same DSP as the mult.
		// Doing this will change the output register name to something like
		// altmult_add:Add58_rtl_0|mult_add_n5n3:auto_generated|mac_out5~DATAOUT10
		// Since we don't know if this will be the case, we can just print *mac*
		// as one of the src registers in the MC constraint. A more proper solution
		// Would be to remove the pipeline registers when multi-cycling.
		// Or instead of using strings like this I think it would be better to
		// get the signal from the instruction then look at the drivers until I
		// find one which is a reg (see comment at top of this function).
		//
		// Edit: Actually the easiest thing would probably be to just avoid
		// printing the pipeling registers... keep the latency the same when
		// scheduling, but just don't make the hardware pipelined.
		src_name = verilogName(source) + "* *mac";
		sdc_name_to_qsf_name[src_name] = verilogName(source);
	}
#endif

    return src_name;
}

// Get the destination register name associated with this instruction
// MULTICYCLE_TODO: See comment at the top of get_dest_register_name
std::string GenerateRTL::get_dest_register_name(Instruction *dest) {
    std::string dst_name = verilogName(dest) + "_reg";

    if (isa<ReturnInst>(*dest)) {
        // Dest name is e.g. main:main_inst|return_val
        // Note: might be safer to use wildcards (*return_val*)
        dst_name = verilogName(Fp) + ":" + verilogName(Fp) + "_inst:return_val";
    }

    // MULTICYCLE_TODO: If the destination is a memory instruction, then
    // adjust dst_name to be the RAM input port. There is no explicit
    // register because altsyncrams do not have registered inputs.
    // Currently I just use memory_controller but it would be better to
    // use a specific port name.

    // Load
    if (isa<LoadInst>(*dest)) {
        // dst_name = get_load_port_name(dest);
        dst_name = "ram_dual_port* *rom_dual_port* *memory_controller";
        sdc_name_to_qsf_name[dst_name] = "";
    }

    // Store
    if (isa<StoreInst>(*dest)) {
        // Instruction *second_last_instruction = *(++path.begin());
        // dst_name = get_store_port_name(dest, second_last_instruction);
        dst_name = "ram_dual_port* *rom_dual_port* *memory_controller";
        sdc_name_to_qsf_name[dst_name] = "";
    }

    // Call
    if (isa<CallInst>(*dest)) {
        dst_name = "arg";
        sdc_name_to_qsf_name[dst_name] = "";
    }

#if 0 // This is no longer needed since we de-pipeline multipliers
	// If this is a mult with latency
	if ( isMul(dest) && Scheduler::getNumInstructionCycles(dest) > 0) {
		// See comment in get_source_register_name: and sometimes the
		// pipeline register is combined into the dsp
		dst_name = verilogName(dest) + "_stage* *mac";
		sdc_name_to_qsf_name[dst_name] = verilogName(dest);
	}
#endif

    return dst_name;
}

// Return true if src is a direct pred of dst (BBs)
bool is_direct_predecessor(BasicBlock *src, BasicBlock *dst) {
    for (pred_iterator PI = pred_begin(dst), E = pred_end(dst); PI != E; ++PI) {
        if (*PI == src) {
            return true;
        }
    }
    return false;
}

// Return the minimum number of FSM states betweeen these two basic
// blocks (from the end of src to the start of dst)
// If src is a direct predecessor of pred, return 0
int GenerateRTL::min_state_difference(BasicBlock *src, BasicBlock *dst) {
    if (is_direct_predecessor(src, dst)) {
        return 0;
    }

    // MULTICYCLE_TODO:
    // Corner case -- if src and dst are the same, that means we called
    // this function to find the shortest loop from the BB back to itself.
    // However currently the function would return 0 for that case. Since
    // we know that this is not a direct loop back (or else above we'd have
    // returned 0), for now return 1.
    if (src == dst) {
        return 1;
    }

    // Algorithm: Uniform Cost Search (like Dijkstra for 1 path)
    // Start at dst and find shortest path up to src

    // Two data structures, a priority queue of BB to search,
    // sorted by key (distance), and a reverse mapping (BB->dist)
    std::set<std::pair<int, BasicBlock *>> pq;
    std::map<BasicBlock *, int> BB_to_dist;

    // Initialize distance to dst to be 0
    pq.insert(std::pair<int, BasicBlock *>(0, dst));
    BB_to_dist[dst] = 0;

    // Outer loop: pop BB with lowest dist and iterate over neighbors
    while (!pq.empty()) {
        // Get the next lowest distance and its BB, and remove it from the pq
        int d = pq.begin()->first; // Guaranteed to be the lowest dist
        BasicBlock *curr =
            pq.begin()->second; // An arbitrary BB with this min dist
        pq.erase(pq.begin());

        // Stop if we've reached src
        if (curr == src) {
            break;
        }

        // Inner loop: update dist for all preds
        for (pred_iterator PI = pred_begin(curr), E = pred_end(curr); PI != E;
             ++PI) {
            BasicBlock *pred = *PI;
            int new_dist = d + alloc->get_num_states_in_BB(pred);

            // If we've never seen this pred before, add it
            if (BB_to_dist.find(pred) == BB_to_dist.end()) {
                pq.insert(std::pair<int, BasicBlock *>(new_dist, pred));
                BB_to_dist[pred] = new_dist;
                continue;
            }

            // Otherwise, check if we've found a shorter path
            int old_dist = BB_to_dist[pred];
            if (new_dist < old_dist) {
                pq.erase(std::pair<int, BasicBlock *>(old_dist, pred));
                pq.insert(std::pair<int, BasicBlock *>(new_dist, pred));
                BB_to_dist[pred] = new_dist;
            }
        }
    }
    // Return the distance from dst --> src, however ignore the
    // length of src itself
    int total_dist = BB_to_dist[src] - alloc->get_num_states_in_BB(src);

    // Corner case: In case the dist is ever 0 return 1, because we know
    // that src and dst have at least 1 BB between them
    total_dist = std::max(1, total_dist);

    return total_dist;
}

// Return the maximum number of FSM states betweeen these two basic
// blocks (from the end of src to the start of dst)
int GenerateRTL::max_state_difference(BasicBlock *src, BasicBlock *dst) {
    // Algorithm: Breadth-First Search with cycle checking
    // Start at dst and find all paths up to src

    // Set up BFS
    std::queue<BasicBlock *> frontier;
    frontier.push(dst);

    // Map each BB to its max distance so far
    // This map is also used to track already visited BB
    std::map<BasicBlock *, int> BB_to_dist;
    BB_to_dist[dst] = 0;

    // Do BFS
    while (!frontier.empty()) {
        // Pop next
        BasicBlock *curr = frontier.front();
        frontier.pop();

        // Loop over pred BBs of curr
        for (pred_iterator PI = pred_begin(curr), E = pred_end(curr); PI != E;
             ++PI) {
            BasicBlock *pred = *PI;

            // Get the distance from the start to pred along this path
            int new_dist = BB_to_dist[curr] + alloc->get_num_states_in_BB(pred);

            // If we've never seen this pred before, add its distance
            // and also add it to the queue
            if (BB_to_dist.find(pred) == BB_to_dist.end()) {
                BB_to_dist[pred] = new_dist;
                frontier.push(pred);
            }
            // Otherwise, check if we've found a longer path
            else if (new_dist > BB_to_dist[pred]) {
                BB_to_dist[pred] = new_dist;
            }
        }
    }
    // Return the distance from dst --> src, however ignore the
    // length of src itself
    int total_dist = BB_to_dist[src] - alloc->get_num_states_in_BB(src);

    // Corner case: In case the dist is ever 0 return 1, because we know
    // that src and dst have at least 1 BB between them
    total_dist = std::max(1, total_dist);

    return total_dist;
}

// Make a .sdc multi-cycle constraint for this path
std::string GenerateRTL::make_multicycle_constraint(
    std::string src, std::string dst, int multicycle_multiplier,
    std::string type,
    std::vector<Instruction *> intermediate_path // Excludes src/dst
    ) {
    std::string sdc_constraint = "set_multicycle_path -from [get_registers {*" +
                                 src + "*}] -to [get_registers {*" + dst +
                                 "*}] -" + type + " -end " +
                                 utostr(multicycle_multiplier);

    // Print though constraints for intermediate nodes in this path
    // However, all of these intermediate signals can be optimized away (this
    // happens very often, only reg / internally created logic cells are kept).
    // So, we will add synthesis keep on these paths later as well. However,
    // doing that (1) introduces extra lcell delays, and (2) causes compile
    // times to take 1 day +. To fix this, we'll only add synthesis keep for
    // the minimum number of required signals.
    if (!intermediate_path.empty() &&
        LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS")) {
        // Adding a tab here so we can parse it later.
        sdc_constraint += "\t-through [get_nets { *";
        sdc_constraint += verilogName(intermediate_path.front());

        // Print -through constraints
        std::vector<Instruction *>::const_iterator vi =
            intermediate_path.begin();
        ++vi; // Start at the second one
        for (; vi != intermediate_path.end(); ++vi) {
            sdc_constraint += ("* *" + verilogName(*vi));
        }
        sdc_constraint += "* }]";
    }

    sdc_constraint += "\n";
    return sdc_constraint;
}

// Helper for write_multicycle_constraint_to_vectors
// Print qsf constraints instructing Quartus synthesis not to
// merge registers that have multi-cycle constraints
void print_qsf_constraints_to_disable_register_merging(
    std::string src_name, std::string dst_name,
    std::set<std::string> &reg_with_qsf_constraints,
    std::vector<std::string> &qsf_constraints,
    std::map<std::string, std::string> &sdc_name_to_qsf_name) {

    bool print_qsf =
        LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_DISABLE_REG_MERGING");
    if (!print_qsf) {
        return;
    }

    // First check if these registers have different names in the qsf
    if (sdc_name_to_qsf_name.find(src_name) != sdc_name_to_qsf_name.end()) {
        src_name = sdc_name_to_qsf_name[src_name];
    }
    if (sdc_name_to_qsf_name.find(dst_name) != sdc_name_to_qsf_name.end()) {
        dst_name = sdc_name_to_qsf_name[dst_name];
    }

    // Add the qsf constraints
    if (reg_with_qsf_constraints.find(src_name) ==
            reg_with_qsf_constraints.end() &&
        !src_name.empty()) {
        qsf_constraints.push_back(
            "set_instance_assignment -name dont_merge_register on -to *" +
            src_name + "*\n");
        reg_with_qsf_constraints.insert(src_name);
    }
    if (reg_with_qsf_constraints.find(dst_name) ==
            reg_with_qsf_constraints.end() &&
        !dst_name.empty()) {
        qsf_constraints.push_back(
            "set_instance_assignment -name dont_merge_register on -to *" +
            dst_name + "*\n");
        reg_with_qsf_constraints.insert(dst_name);
    }
}

// Helper to write_multicycle_constraint_to_vectors. Takes a vector of
// .sdc contraints (strings), as well as a new constraint to add. The
// new constraint may replace an existing constraint, so check if that
// existing constraint exists already or not.
void replace_sdc_constraint(std::vector<std::string> &sdc_constraints,
                            std::string previous_constraint,
                            std::string new_constraint) {

    std::vector<std::string>::iterator vec_it;
    vec_it = std::find(sdc_constraints.begin(), sdc_constraints.end(),
                       previous_constraint);
    if (vec_it == sdc_constraints.end()) {
        sdc_constraints.push_back(new_constraint);
    } else {
        mc_debug() << "\nReplacing " << *vec_it << " with " << new_constraint
                   << "\n";
        *vec_it = new_constraint;
    }
}

// Print the constraint for this source and dst string (setup and hold)
// to a set of .sdc constraints
void GenerateRTL::write_multicycle_constraint_to_vectors(
    std::string src_name, std::string dst_name, unsigned multicycle_multiplier,
    std::vector<std::string> &sdc_constraints,
    std::vector<std::string> &qsf_constraints,
    std::vector<Instruction *> intermediate_path) {

    // Don't print the same constraints multiple times
    static std::set<std::string> reg_with_qsf_constraints;

    // Generate the sdc constraints
    // Note that for a multi-cycle path with slack N, the setup constraint
    // is N and the hold constraint is N-1. See the DATE 2015 paper for details.
    std::string setup_constraint = make_multicycle_constraint(
        src_name, dst_name, multicycle_multiplier, "setup", intermediate_path);
    std::string hold_constraint = make_multicycle_constraint(
        src_name, dst_name, multicycle_multiplier - 1, "hold",
        intermediate_path);

    // If -through constraints are being printed, add the constraints here.
    // Otherwise, they will be added below.
    if (LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS")) {
        sdc_constraints.push_back(setup_constraint);
        sdc_constraints.push_back(hold_constraint);
    }

    // How the constraints are printed depends on whether
    // MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS is set.

    // If -through constraints will be printed, then at this point we've printed
    // the
    // entire path, including all -through constraints. In order for these
    // through
    // constraints to not be ignored, the intermediate wires need to not be
    // synthesized away.
    //
    // However, adding synthesis keep to every wire causes large delays and
    // large
    // increases in compile time. We can fix this by only adding synthesis keep
    // to
    // paths which require though constraints, and even in those paths only
    // adding
    // synthesis keep to the minimum number of signals required.

    // Check if the current pair of src/dst registers already has a multi-cycle
    // constraint
    bool pair_found_for_first_time = false;
    unsigned previous_multiplier = 0;
    src_dst_pair_t src_dst_pair(src_name, dst_name);
    if (src_dst_pair_to_path_map.find(src_dst_pair) !=
        src_dst_pair_to_path_map.end()) {
        duplicate_src_dst_pairs.insert(src_dst_pair);
        if (src_dst_pair_to_min_slack[src_dst_pair] > multicycle_multiplier) {
            previous_multiplier = src_dst_pair_to_min_slack[src_dst_pair];
            src_dst_pair_to_min_slack[src_dst_pair] = multicycle_multiplier;
        }
        // Also store the max slack
        if (src_dst_pair_to_max_slack[src_dst_pair] < multicycle_multiplier) {
            src_dst_pair_to_max_slack[src_dst_pair] = multicycle_multiplier;
        }
    }
    // Otherwise this is the first time we're seeing this src/dst pair
    else {
        src_dst_pair_to_min_slack[src_dst_pair] = multicycle_multiplier;
        src_dst_pair_to_max_slack[src_dst_pair] = multicycle_multiplier;
        pair_found_for_first_time = true;
    }

    // Add the current pair->path to our map
    src_dst_pair_to_path_map.insert(
        std::make_pair(src_dst_pair, std::make_pair(intermediate_path,
                                                    multicycle_multiplier)));

    // Later, this map/set of all duplicates will be used to obtain a set of
    // instructions
    // to not synthesize away.

    // However, if -through constraints are NOT printed in this flow, then we
    // need
    // to instead print the constraint here only if it is the minimum cycle
    // constraint for this src/dst pair
    if (!LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS")) {
        // If this constraint has never been added, add it here with
        // the current multiplier
        if (pair_found_for_first_time) {
            sdc_constraints.push_back(setup_constraint);
            sdc_constraints.push_back(hold_constraint);
        }
        // Otherwise the constraint has been added. Replace it with this
        // constraint
        // if the multiplier is smaller
        else {
            // This constraint was previously added with a larger multiplier
            // Replace that constraint with this one
            if (multicycle_multiplier < previous_multiplier) {
                // Also replace in the vector
                std::string previous_setup = make_multicycle_constraint(
                    src_name, dst_name, previous_multiplier, "setup",
                    intermediate_path);
                std::string previous_hold = make_multicycle_constraint(
                    src_name, dst_name, previous_multiplier - 1, "hold",
                    intermediate_path);

                // Replace the setup constraint with this new one
                replace_sdc_constraint(sdc_constraints, previous_setup,
                                       setup_constraint);
                // Replace the hold constraint too
                replace_sdc_constraint(sdc_constraints, previous_hold,
                                       hold_constraint);
            }
        }
    }

    // Check if we should also print .qsf constraints to disable registers
    // with timing constraints to be merged
    print_qsf_constraints_to_disable_register_merging(
        src_name, dst_name, reg_with_qsf_constraints, qsf_constraints,
        sdc_name_to_qsf_name);
}

// Convert the vector of instructions to a vector of just the
// containing BB. Helper to num_cycles_in_path.
void GenerateRTL::ipath_to_bbpath(std::vector<Instruction *> &path,
                                  std::vector<BasicBlock *> &bb_path) {

    BasicBlock *prev_BB = NULL;
    int prev_instruction_end_state = -1;
    // We are traversing the path in reverse because it is root->src
    // but we want BBs from src->root
    for (std::vector<Instruction *>::reverse_iterator pi = path.rbegin();
         pi != path.rend(); ++pi) {
        Instruction *I = *pi;
        int start_state = alloc->get_registered_instruction_state(I);
        // If different BB, push the new BB onto the path
        if (I->getParent() != prev_BB) {
            prev_BB = I->getParent();
            bb_path.push_back(I->getParent());
        } else {
            // Otherwise, the previous instruction and this instruction
            // are in the same BB. However, the BB may have "looped around"
            // to itself, which happens in dfdiv.
            assert(prev_instruction_end_state >= 0);
            // MULTICYCLE_TODO: The BB can also loop back to itself if this
            // instruction is in the SAME state as the previous one (i.e.
            // not just in an earlier one), if the previous instruction is
            // not a direct pred of the current instruction. For now this
            // case is ignored as it seems very rare.
            if (start_state < prev_instruction_end_state) {
                bb_path.push_back(I->getParent());
            }
        }
        int end_state = start_state;
        if (Scheduler::getNumInstructionCycles(I) > 1) {
            end_state += Scheduler::getNumInstructionCycles(I);
        }
        prev_instruction_end_state = end_state;
    }
}

// Get the state # of an instruction given that it is the destination
// of an MC path
int GenerateRTL::get_dest_state(Instruction *dest) {

    int dest_state = alloc->get_registered_instruction_state(dest);

    // If the dest instruction is a branch or switch then its state
    // is the last state of the Basic Block
    if (isa<BranchInst>(dest) || isa<SwitchInst>(dest)) {
        dest_state = alloc->get_num_states_in_BB(dest->getParent()) - 1;
    }

    return dest_state;
}

// Get the state # of an instruction given that it is the source
// of an MC path
int GenerateRTL::get_source_state(Instruction *source) {

    int source_state = alloc->get_registered_instruction_state(source);

    // If the source instruction is a PHI then its state is actually "-1"
    // in its Basic Block
    if (isa<PHINode>(source)) {
        source_state -= 1;
    }

    // If this source has latency > 0, then only the
    // first state was stored, so adjust source_state.
    //
    // Note: we subtract 1 from the # instruction cycles below
    // because the code assumes that an instruction scheduled in
    // state X has its value stored in a register at the end of
    // state X, and that this register can be read starting from
    // state X+1. E.g. if we have an adder in state 9, then this
    // means the adder's logic is in state 9 and so is its output
    // register. The register can be read starting from state 10.
    // Now, consider the example of a load, which returns 2 for
    // getNumInstructionCycles. Loads scheduled in state 9 will
    // be "ready to use" in state 11. Using the analogy of the
    // adder, it is as if the "load's logic" is in state 10 and the
    // load's register therefore is in state 10 as well, so the load
    // can be used starting state 11. Therefore we really only want
    // to extend the source state of the load by 1, not 2.
    if (Scheduler::getNumInstructionCycles(source) > 1) {
        source_state += (Scheduler::getNumInstructionCycles(source) - 1);
    }

    return source_state;
}

// Given a vector of instructions describing a path (root->src),
// return the number of FSM states (clock cycles) between the
// instructions
int GenerateRTL::num_cycles_in_path(std::vector<Instruction *> path) {

    // Convert the vector of instructions to a vector of just the BB
    std::vector<BasicBlock *> bb_path;
    ipath_to_bbpath(path, bb_path);

    // Get the state # of the dest instruction
    Instruction *dest = path.front();
    int dest_state = get_dest_state(dest);
    assert(bb_path.back() == dest->getParent());

    // Get the state # of the source instruction
    Instruction *source = path.back();
    int source_state = get_source_state(source);
    assert(bb_path.front() == source->getParent());

    // Calculate the multi-cycle multiplier
    int total_num_states = dest_state - source_state;

    // Case 1. If the source and dst are in the same BB,
    // with no intermediate BB
    if (bb_path.size() == 1) {
        BasicBlock *common_parent = bb_path.front();
        mc_debug() << "\nSame BB, dst = " << dest_state
                   << ", src = " << source_state;

        // Corner case: If src is actually before dst in the same BB,
        // this means that the BB loops back to itself. This would have
        // been caught in the checks above.
        // However this also happens if the src and dest are both PHIs in
        // the same basic block (since all other reg have been removed).
        // In this PHI case dst will not be before src, they will be in the
        // same state, but the BB must still loop around to itself.
        //
        // Note that there is still the corner case where source and dest are
        // in the same state but source does not drive dest directly, in which
        // case the path would also loop around the BB (see note above).
        if (isa<PHINode>(source) && isa<PHINode>(dest)) {
            total_num_states += alloc->get_num_states_in_BB(common_parent);
            mc_debug() << " + BB length = "
                       << alloc->get_num_states_in_BB(common_parent) << "\n";
        } else {
            assert(dest_state >= source_state);
        }
    }
    // Case 2. If the source and dst are in different BBs or have
    // BBs between them
    else {
        mc_debug() << "\ndst state = " << dest_state
                   << ", src state = " << source_state << "\n";
        std::vector<BasicBlock *>::const_iterator bi = bb_path.begin();
        std::vector<BasicBlock *>::const_iterator be = bb_path.end();

        // Ignore the first and last Basic Blocks, since we already
        // accounted for those with source_state and dest_state
        BasicBlock *prev_BB = *bi;
        ++bi;
        --be;

        total_num_states += alloc->get_num_states_in_BB(source->getParent());
        mc_debug()
            << "\nDiff BB, so add back states in source BB: total_num_states = "
            << total_num_states;
        for (; bi != be; ++bi) {
            total_num_states += alloc->get_num_states_in_BB(*bi);
            total_num_states += min_state_difference(prev_BB, *bi);
            mc_debug() << "\nIntermediate BB, total_num_states = "
                       << total_num_states;
            prev_BB = *bi;
        }
        // One more for the difference to the last BB
        total_num_states += min_state_difference(prev_BB, *bi);
        mc_debug() << "\nFinal intermediate BB, total_num_states = "
                   << total_num_states;
    }

    assert(total_num_states >= 0);
    return total_num_states;
}

// Given a vector of instructions describing a path (root->src),
// where the source of the path is an input argument, return the number
// of FSM states (clock cycles) between the instructions
// Like num_cycles_in_path but has a source that is a function input arg,
// can combine them into 1 function.
int
GenerateRTL::num_cycles_in_path_with_src_arg(std::vector<Instruction *> path) {
    // Convert the vector of instructions to a vector of containing BB
    // We are traversing the path in reverse because it is root->src
    // but we want BBs from src->root
    std::vector<BasicBlock *> bb_path;
    BasicBlock *entry_BB = &(Fp->getEntryBlock());
    bb_path.push_back(entry_BB);
    BasicBlock *prev_BB = entry_BB;
    int prev_instruction_end_state = 0;
    for (std::vector<Instruction *>::reverse_iterator pi = path.rbegin();
         pi != path.rend(); ++pi) {
        Instruction *I = *pi;
        int start_state = alloc->get_registered_instruction_state(I);
        // If different BB, push the new BB onto the path
        if (I->getParent() != prev_BB) {
            prev_BB = I->getParent();
            bb_path.push_back(I->getParent());
        } else {
            // Otherwise, the previous instruction and this instruction
            // are in the same BB. However, the BB may have "looped around"
            // to itself, which happens in dfdiv.
            assert(prev_instruction_end_state >= 0);
            if (start_state < prev_instruction_end_state) {
                bb_path.push_back(I->getParent());
            }
        }
        int end_state = start_state;
        if (Scheduler::getNumInstructionCycles(I) > 1) {
            end_state += Scheduler::getNumInstructionCycles(I);
        }
        prev_instruction_end_state = end_state;
    }

    // Get the state # of the dest instruction
    Instruction *dest = path.front();
    int dest_state = alloc->get_registered_instruction_state(dest);
    assert(bb_path.back() == dest->getParent());

    // If the dest instruction is a branch or switch then its state
    // is the last state of the Basic Block
    if (isa<BranchInst>(dest) || isa<SwitchInst>(dest)) {
        dest_state = alloc->get_num_states_in_BB(dest->getParent()) - 1;
    }

    // The state # of the arg source is always -1 (like PHIs)
    int source_state = -1;

    // Calculate the multi-cycle multiplier
    int total_num_states = dest_state - source_state;

    // Case 1. If the source and dst are in the same BB,
    // with no intermediate BB
    if (bb_path.size() == 1) {
        mc_debug() << "\nSame BB, dst = " << dest_state
                   << ", src = " << source_state;
    }
    // Case 2. If the source and dst are in different BBs or have
    // BBs between them
    else {
        mc_debug() << "\ndst state = " << dest_state
                   << ", src state = " << source_state << "\n";
        std::vector<BasicBlock *>::const_iterator bi = bb_path.begin();
        std::vector<BasicBlock *>::const_iterator be = bb_path.end();

        // Ignore the first and last Basic Blocks, since we already
        // accounted for those with source_state and dest_state
        BasicBlock *prev_BB = *bi;
        ++bi;
        --be;

        total_num_states += alloc->get_num_states_in_BB(entry_BB);
        mc_debug()
            << "\nDiff BB, so add back states in source BB: total_num_states = "
            << total_num_states;
        for (; bi != be; ++bi) {
            total_num_states += alloc->get_num_states_in_BB(*bi);
            total_num_states += min_state_difference(prev_BB, *bi);
            mc_debug() << "\nIntermediate BB, total_num_states = "
                       << total_num_states;
            prev_BB = *bi;
        }
        // One more for the difference to the last BB
        total_num_states += min_state_difference(prev_BB, *bi);
        mc_debug() << "\nFinal intermediate BB, total_num_states = "
                   << total_num_states;
    }

    assert(total_num_states >= 0);
    return total_num_states;
}

// Helper function. Takes a path (vector of instructions) and a
// set of instruction pairs whose intermediate register is required,
// and truncates the path at the first pair along the path in the set.
void truncate_path_at_first_register_pair(
    std::vector<Instruction *> &path, // order is root -> src
    std::set<std::pair<Instruction *, Instruction *>> keep_reg_pairs) {
    // Iterate in reverse, i.e. src -> root
    // Should use reverse iterators but then can't vector.erase...
    std::vector<Instruction *>::iterator it = path.end(), end = path.begin(),
                                         previous = path.end();
    --it;
    --it;
    --previous;
    --end;

    for (; it != end; --it, --previous) {
        // Check if keep_reg_pairs contains (*previous, *it)
        if (keep_reg_pairs.find(std::make_pair(*previous, *it)) !=
            keep_reg_pairs.end()) {
            // Truncate path here (from the start, i.e. replace the root)
            // vector.erase removes [first,last)
            path.erase(path.begin(), previous);
            break;
        }
    }
}

// This helper function prints a .sdc multi-cycle constraint for the
// given path (vector of instruction)
void GenerateRTL::add_multicycle_constraint_for_path(
    std::vector<Instruction *> path, // order is root -> src
    std::vector<string> &sdc_constraints, std::vector<string> &qsf_constraints,
    std::set<std::pair<Instruction *, Instruction *>> keep_reg_pairs,
    std::multimap<std::vector<Instruction *>, Value *> &paths_with_arg_srcs) {
    // -------------------------------------------------------------------------
    // 0. EDIT: Sometimes we don't remove all registers from a path. The pairs
    // of instructions whose intermediate registers cannot be removed (e.g.
    // because of pipelined instructions) are stored in keep_reg_pairs.
    // So, find the first instruction pair in the path within keep_reg_pairs
    // and truncate the path there.
    // -------------------------------------------------------------------------
    truncate_path_at_first_register_pair(path, keep_reg_pairs);

    // -------------------------------------------------------------------------
    // 1. Get the destination register name
    // -------------------------------------------------------------------------
    Instruction *dest = path.front();
    std::string dst_name = get_dest_register_name(dest);
    mc_debug() << "\nPrinting constraints now for dst = " << dst_name << "   ("
               << *dest << ")";

    // -------------------------------------------------------------------------
    // 2. Get the source register name
    // -------------------------------------------------------------------------
    std::string src_name;

    // Usually the source is the first instruction. However, the source may also
    // be an input to this function. Check if this path has an arg as a source.
    Value *src_arg = NULL;
    std::multimap<std::vector<Instruction *>, Value *>::iterator it =
        paths_with_arg_srcs.find(path);
    if (it != paths_with_arg_srcs.end()) {
        src_arg = it->second;
        paths_with_arg_srcs.erase(it);
        src_name = verilogName(Fp) + "_" + verilogName(src_arg);
        mc_debug() << "\n    src = " << src_name << "    (Input arg)";
    } else {
        Instruction *source = path.back();
        src_name = get_source_register_name(source);
        mc_debug() << "\n    src = " << src_name << "    (" << *source << ")";
    }

    // -------------------------------------------------------------------------
    // 3. Get the multiplier for this constraint
    // -------------------------------------------------------------------------
    int multicycle_multiplier;
    if (src_arg) {
        multicycle_multiplier = num_cycles_in_path_with_src_arg(path);
    } else {
        multicycle_multiplier = num_cycles_in_path(path);
    }

    // -------------------------------------------------------------------------
    // 4. Print the constraint
    // -------------------------------------------------------------------------
    // Ignore corner cases and cases where the multiplier is only 1
    if (multicycle_multiplier > 1) {
        // Remove the first and last instructions from path, since the
        // function below takes only the intermediate signals (nets)
        std::vector<Instruction *> intermediate_path = path;
        intermediate_path.erase(intermediate_path.begin());
        if (!src_arg) {
            if (!intermediate_path.empty()) {
                intermediate_path.pop_back();
            }
        }
        write_multicycle_constraint_to_vectors(
            src_name, dst_name, multicycle_multiplier, sdc_constraints,
            qsf_constraints, intermediate_path);
    }
    mc_debug() << "\n\n";
}

// Helper for add_multicycle_constraint_for_path_with_custom_root, which
// checks whether an MC path was truncated by a register which couldn't
// be removed.
bool check_path_truncation(std::vector<Instruction *> &path, Instruction *root,
                           std::set<Instruction *> drivers_with_reg_to_remove) {

    bool truncated = false;

    // If no truncation was done, also truncate the path at cur_state if we are
    // not removing that register
    if (path.front() == root) {
        if (drivers_with_reg_to_remove.find(path.front()) ==
            drivers_with_reg_to_remove.end()) {
            // The 1st instruction of this path is NOT going to
            // have its register removed, so truncate there
            path.erase(path.begin());
            truncated = true;
        }
    } else {
        truncated = true;
    }
    return truncated;
}

// Helper to add_multicycle_constraint_for_path_with_custom_root. Certain
// signals (so far only cur_state) have additional multi-cycle slack.
void GenerateRTL::adjust_multicycle_multiplier_for_specific_signals(
    std::string dst_name, int &multicycle_multiplier,
    std::vector<Instruction *> path) {

    // If the destination is cur_state, then its driver is in an if condition.
    // Determine if the driver was used in the same state that it was
    // calculated,
    // or if there was cycle slack.
    //
    // The following example illustrates this case:
    //
    // In a BB, I have some register %a. The value of %a is used in a
    // series of computations (a path) and then the final output of
    // the path is wire %b.
    //
    // The wire %b is then used in a check like this:
    //
    // always @(posedge clk)
    //     if (cur_state == LEGUP_X & b == 1'b1)
    //         cur_state <= LEGUP_Y;
    //
    // The IR would look something like this:
    //
    //   %a = xor i32 %foo, %bar
    //   %b = add i32 %a, 4
    //   %cmp = icmp eq i32 %b, 1
    //   br i1 %cmp, label %Y, label %Z
    //
    // There is now a path between reg %a and reg cur_state, through the
    // wire %b. Note that this case only happens when transitioning
    // between BB (within a BB the states progress in a known order so
    // the "if" condition only involves the cur_state and memory controller
    // wait requests, not outside signals. Only for branches across BB do
    // we have these additional checks).
    //
    // Because this check runs every cycle (@posedge clk), it seems we
    // cannot multi-cycle the path from %a->cur_state.
    //
    // However, say we know from the scheduling that within the BB
    // where %b is calculated, %a is clocked in state 2, %b is calculated
    // in state 3, and %b is used in a branch in state 4 (e.g. the icmp
    // instruction b==1 is scheduled in state 4).
    //
    // Then, we know that between the state where %a is clocked and when
    // icmp b==1 is used, there is a certain cycle slack.
    //
    // Specifically, the code below does the following:
    //
    // For a register (e.g. %a), if %a or its descendant is used in a
    // branch or switch (or perhaps multiple), add cycle slack from %a
    // to cur_state equal to the min state difference between %a and
    // any downstream branch or switch (the first one found).
    //
    // Note that the MC paths always terminates at the FSM register. I never
    // handled the case of MC paths originating at the FSM registers
    // (I don't think I ever observed that, or if it can happen. It happens
    // in the case of terminating because of conditional branches).
    if (dst_name == "cur_state") {
        // path.front() is used in a conditional statement for cur_state
        Instruction *cond = path.front();

        int min_cycle_slack = 100;
        bool found_branch_use = false;
        // Check all the uses of this compare instruction.
        // Keep the minimum slack from the compare to a use branch.
        for (Value::user_iterator u = cond->user_begin(); u != cond->user_end();
             ++u) {
            Instruction *use = dyn_cast<Instruction>(*u);
            if (!use)
                continue;
            if (isa<BranchInst>(*use) || isa<SwitchInst>(*use)) {
                found_branch_use = true;
                // Get the cycle slack from cond to use.
                // Note: The convention num_cycles_in_path follows is root->src
                std::vector<Instruction *> tmp_path;
                tmp_path.push_back(use);
                tmp_path.push_back(cond);
                int state_difference = num_cycles_in_path(tmp_path);
                assert(state_difference >= 0);
                min_cycle_slack = std::min(min_cycle_slack, state_difference);
            }
        }
        assert(found_branch_use);
        multicycle_multiplier += min_cycle_slack;
    }
}

// Like add_multicycle_constraint_for_path but for roots which are not
// instructions (e.g. the cur_state register)
//
// MULTICYCLE_TODO: This can be combined with add_multicycle_constraint_for_path
// because a lot of the code is the same (just check if root_signal == NULL)
void GenerateRTL::add_multicycle_constraint_for_path_with_custom_root(
    std::vector<Instruction *> path, // order is root -> src
    std::vector<string> &sdc_constraints, std::vector<string> &qsf_constraints,
    std::set<std::pair<Instruction *, Instruction *>> keep_reg_pairs,
    std::set<Instruction *> drivers_with_reg_to_remove,
    std::multimap<std::vector<Instruction *>, Value *> &paths_with_arg_srcs,
    RTLSignal *root_signal) {
    // -------------------------------------------------------------------------
    // 0. EDIT: Sometimes we don't remove all registers from a path. The pairs
    // of instructions whose intermediate registers cannot be removed (e.g.
    // because of pipelined instructions) are stored in keep_reg_pairs.
    // So, find the first instruction pair in the path within keep_reg_pairs
    // and truncate the path there.
    // -------------------------------------------------------------------------
    Instruction *root = path.front();
    truncate_path_at_first_register_pair(path, keep_reg_pairs);
    bool truncated =
        check_path_truncation(path, root, drivers_with_reg_to_remove);

    // -------------------------------------------------------------------------
    // 1. Get the destination register name
    // -------------------------------------------------------------------------
    std::string dst_name;
    if (truncated) {
        Instruction *dest = path.front();
        dst_name = get_dest_register_name(dest);
        mc_debug() << "\nPrinting constraints now for dst = " << dst_name
                   << "   (" << *dest << ")";
    } else {
        dst_name = root_signal->getName();
        mc_debug() << "\nPrinting constraints now for dst = " << dst_name;
    }

    // -------------------------------------------------------------------------
    // 2. Get the source register name
    // -------------------------------------------------------------------------
    std::string src_name;

    // Usually the source is the first instruction. However, the source may also
    // be an input to this function. Check if this path has an arg as a source.
    Value *src_arg = NULL;
    std::multimap<std::vector<Instruction *>, Value *>::iterator it =
        paths_with_arg_srcs.find(path);
    if (it != paths_with_arg_srcs.end()) {
        src_arg = it->second;
        paths_with_arg_srcs.erase(it);
        src_name = verilogName(Fp) + "_" + verilogName(src_arg);
        mc_debug() << "\n    src = " << src_name << "    (Input arg)";
    } else {
        Instruction *source = path.back();
        src_name = get_source_register_name(source);
        mc_debug() << "\n    src = " << src_name << "    (" << *source << ")";
    }

    // -------------------------------------------------------------------------
    // 3. Get the multiplier for this constraint
    // -------------------------------------------------------------------------
    // Note: Same if dst is cur_state, arg, etc
    int multicycle_multiplier;
    if (src_arg) {
        multicycle_multiplier = num_cycles_in_path_with_src_arg(path);
    } else {
        multicycle_multiplier = num_cycles_in_path(path);
    }

    // Adjust for special-case signals
    adjust_multicycle_multiplier_for_specific_signals(
        dst_name, multicycle_multiplier, path);

    // -------------------------------------------------------------------------
    // 4. Print the constraint
    // -------------------------------------------------------------------------
    // Ignore corner cases and cases where the multiplier is only 1
    if (multicycle_multiplier > 1) {
        // Remove the first and last instructions from path, since the
        // function below takes only the intermediate signals (nets)
        std::vector<Instruction *> intermediate_path = path;
        if (truncated) {
            if (!intermediate_path.empty()) {
                intermediate_path.erase(intermediate_path.begin());
            }
        }
        if (!src_arg) {
            if (!intermediate_path.empty()) {
                intermediate_path.pop_back();
            }
        }
        write_multicycle_constraint_to_vectors(
            src_name, dst_name, multicycle_multiplier, sdc_constraints,
            qsf_constraints, intermediate_path);
    }
    mc_debug() << "\n\n";
}

// This function is called to add .sdc multi-cycle constraints for
// dividers which are not pipelined
void GenerateRTL::print_lpm_div_multicycle_constraints() {
#if 0
	// Print constraints for all dividers
	std::set<Instruction*> dividers = alloc->get_multicycled_dividers();
	for (std::set<Instruction*>::const_iterator div_it = dividers.begin();
			div_it != dividers.end(); ++div_it)
	{
		Instruction *i = *div_it;

		// Note: Rather than print multi-cycle constraints here for the
		// dividers, it makes more sense to print them during the main DFS in
		// create_multicycle_paths_from_root. So if an instruction found in that
		// DFS is a divider, and it is in alloc->get_multicycled_dividers(),
		// then rather than stop the DFS (i.e. rather than set the divider to be
		// the source of a multi-cycle path), the traversal should just continue
		// past it.
	}
#endif
}

// Helper to find_signals_to_synth_keep
Instruction *
GenerateRTL::find_instruction_unique_to_path(unsigned i,
                                             const std::vector<path_t> &paths) {
    path_t path = paths[i];
    path_t::const_iterator inst_it = path.begin();
    path_t::const_iterator inst_e = path.end();

    // For all instructions in this path
    for (; inst_it != inst_e; ++inst_it) {
        // Check if it is in any of the other paths
        Instruction *inst = *inst_it;
        bool is_in_any_other_path = false;

        // For all other paths
        for (unsigned j = 0; j < paths.size(); ++j) {
            if (i == j) {
                continue;
            }

            path_t other_path = paths[j];

            // If inst is in other_path
            if (std::find(other_path.begin(), other_path.end(), inst) !=
                other_path.end()) {
                is_in_any_other_path = true;
                break;
            }
        }

        // If this is not in any of the other paths
        if (!is_in_any_other_path) {
            return inst;
        }
    }
    return NULL;
}

// Helper to find_signals_to_synth_keep
void GenerateRTL::prune_duplicate_src_dst_pairs_based_on_path_latency() {
    duplicate_src_dst_pairs_t src_dst_pairs_to_throw_away;

    // For all src/dst pairs
    duplicate_src_dst_pairs_t::const_iterator pair_it =
        duplicate_src_dst_pairs.begin();
    duplicate_src_dst_pairs_t::const_iterator pair_e =
        duplicate_src_dst_pairs.end();
    for (; pair_it != pair_e; ++pair_it) {
        bool prune_this_pair = true;

        // For all paths with this src/dst
        src_dst_pair_t src_dst_pair = *pair_it;

        // Each pair has multiple paths associated with it.
        // Get the latencies of each of these paths.
        src_dst_pair_to_path_map_t::const_iterator mm_it_start =
            src_dst_pair_to_path_map.lower_bound(src_dst_pair);
        src_dst_pair_to_path_map_t::const_iterator mm_it_end =
            src_dst_pair_to_path_map.upper_bound(src_dst_pair);

        bool first = true;
        unsigned latency = 0;
        while (mm_it_start != mm_it_end) {
            if (first) {
                latency = mm_it_start->second.second;
                first = false;
            } else if (latency != mm_it_start->second.second) {
                prune_this_pair = false;
                break;
            }
            ++mm_it_start;
        }
        assert(latency > 0);

        if (prune_this_pair) {
            src_dst_pairs_to_throw_away.insert(src_dst_pair);
        }
    }

    // Now remove each pair we want to erase
    pair_it = src_dst_pairs_to_throw_away.begin();
    pair_e = src_dst_pairs_to_throw_away.end();
    for (; pair_it != pair_e; ++pair_it) {
        duplicate_src_dst_pairs.erase(*pair_it);
        src_dst_pair_to_path_map.erase(*pair_it);
        src_dst_pair_to_min_slack.erase(*pair_it);
    }
}

// This function takes a multimap of all src/dst pairs for each mc path in the
// module
// and also a set of the duplicate src/dst pairs (redundant given the multimap).
// It then generates a minimal (minimum?) set of signals, signals_to_keep, which
// must
// be kept by synthesis in order for all these paths to be differentiated.
void GenerateRTL::find_signals_to_synth_keep() {
    if (!LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS")) {
        return;
    }

    // First, there is no need to add -through constraints for multiple paths
    // sharing a src/dst register if the cycle slacks of all those paths are
    // the same. We can remove pairs for duplicate_src_dst_pairs in which all
    // of the path latencies are the same.
    prune_duplicate_src_dst_pairs_based_on_path_latency();

    // We'll write duplicate_src_dst_pairs to a file, so open it here
    // We'll also write pairs to a file which should have all their -through
    // constraints removed
    std::ofstream pairs_with_through_constraints_file;
    std::ofstream pairs_whose_through_constraints_must_be_removed_file;
    pairs_with_through_constraints_file.open(
        "src_dst_pairs_with_through_constraints.txt", std::fstream::app);
    pairs_whose_through_constraints_must_be_removed_file.open(
        "pairs_whose_through_constraints_must_be_removed.txt");

    // Handle one set of duplicates at a time
    duplicate_src_dst_pairs_t::const_iterator pair_it =
        duplicate_src_dst_pairs.begin();
    duplicate_src_dst_pairs_t::const_iterator pair_e =
        duplicate_src_dst_pairs.end();
    for (; pair_it != pair_e; ++pair_it) {
        // Get the pair
        src_dst_pair_t src_dst_pair = *pair_it;
        std::string src = src_dst_pair.first;
        std::string dst = src_dst_pair.second;

        // Each pair has multiple paths associated with it.
        // Get this list of paths
        std::vector<path_t> paths;
        src_dst_pair_to_path_map_t::const_iterator mm_it_start =
            src_dst_pair_to_path_map.lower_bound(src_dst_pair);
        src_dst_pair_to_path_map_t::const_iterator mm_it_end =
            src_dst_pair_to_path_map.upper_bound(src_dst_pair);

        while (mm_it_start != mm_it_end) {
            paths.push_back(mm_it_start->second.first);
            ++mm_it_start;
        }

        // paths now contains all the paths associated with this pair.
        // Now we have to do some processing to get the signals which, if kept,
        // will
        // make all these paths unique.

        // Recall why this is being done: In some circuits (e.g. gsm) there are
        // multiple register-to-register paths with different latencies, because
        // the
        // paths are multi-cycled and span basic blocks. For example:
        //
        //       SRC REGISTER
        //            |
        //           / \ .
        //          /   \ .
        //         BB1  BB2
        //          |   |
        //           \ /
        //            |
        //       DEST REGISTER
        //
        // Note this is not possible for 2 such paths in the same BB (since
        // src/dst will
        // always be in the same state regardless of intermediate wires). But
        // across BB
        // this happens a lot when dst is a PHI. BB1 can have latency 2, and BB2
        // can have
        // latency 20 (this also happens sometimes). If we multi-cycle both
        // these paths, then
        // we can set a multi-cycle constraint of 2 on the left path, and 20 on
        // the right.
        // We can set these different multi-cycle constraints by adding a
        // -through argument
        // to the .sdc constraint we print. Otherwise, if we don't use a
        // -through constraint,
        // then the two paths will have the same multi-cycle slack, which will
        // have to be
        // the minimum of the 2 slacks for functional correctness. This leads to
        // a large
        // critical path for some circuits. Using the minimum slack is the
        // default behavior
        // if MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS is not set. If it is, then
        // -through
        // constraints are printed.
        //
        // But there's a problem: Quartus synthesis does not guarantee that the
        // intermediate
        // signals will be kept. Often all intermediate signals are synthesized
        // away, and
        // replaced with internal logiccells (e.g. Add3~53|cout,
        // LessThan2~9|combout). One
        // solution is to add synthesis keep to all these signals, but this adds
        // a lot of
        // delay and also takes very long in fit/STA (up to and including
        // Quartus 13.0sp1)
        //
        // Other than -through constraints + Synthesis keep, another solution
        // would be to
        // duplicate the registers. In the example above, we can make src
        // register 1 and
        // src register 2:
        //
        //      SRC 1   SRC 2
        //          |   |
        //          |   |
        //          |   |
        //         BB1  BB2
        //          |   |
        //           \ /
        //            |
        //       DEST REGISTER
        //
        // Now separate constraints can be applied to these 2 paths (no -through
        // constraints needed)
        // But this becomes really complicated if we have something simple like:
        //
        //       SRC REGISTER
        //            |
        //           BB1
        //            |
        //           / \ .
        //          /   \ .
        //         BB2  BB3
        //          |   |
        //           \ /
        //            |
        //       DEST REGISTER
        //
        // This is because now duplicating the register created 4 paths, instead
        // of 2.
        // I think therefore the best solution is to use -through constraints
        // and synthesis
        // keep, but only apply synthesis keep to some wires (as few as
        // possible).
        //
        // So say we have this:
        //
        //           SRC
        //            |
        //            W1 (e.g. inside BB1)
        //            |
        //           /|\ .
        //          / | \ .
        //         /  |  \ .
        //        |   |   |
        //        W2  W3  W4
        //        |   |   |
        //         \  |  /
        //          \ | /
        //           \|/
        //            |
        //            W5
        //            |
        //           DST
        //
        // Here, I just need to keep wires 2, 3, and 4. Wires {1,5} can be
        // synthesized away
        // because only {2,3,4} are needed to uniquely identify the paths.
        //
        // A more complicated example is this:
        //
        //           SRC
        //            |
        //            W1
        //           /|
        //         W2 |
        //           \|
        //            W3
        //            |
        //           DST
        //
        // This might never happen so for cases like this (when there's no
        // unique
        // signal per path) I'll just print the minimum of all the paths.
        //
        // Note: a lot of this seems unnecessary, but it raises the Fmax of
        // blowfish,
        // gsm, and other circuits.

        std::vector<Instruction *> unique_signal_per_path;
        bool found_unique_inst_for_all_paths = true;
        for (unsigned path_index = 0; path_index < paths.size(); ++path_index) {
            Instruction *inst_unique_to_this_path =
                find_instruction_unique_to_path(path_index, paths);

            if (!inst_unique_to_this_path) {
                // Found a path without a unique instruction
                found_unique_inst_for_all_paths = false;
                break;
            } else {
                unique_signal_per_path.push_back(inst_unique_to_this_path);
            }
        }

        if (found_unique_inst_for_all_paths) {
            // Add all signals to the synth_keep set
            std::vector<Instruction *>::const_iterator inst_it =
                unique_signal_per_path.begin();
            std::vector<Instruction *>::const_iterator inst_e =
                unique_signal_per_path.end();
            for (; inst_it != inst_e; ++inst_it) {
                signals_to_keep.insert(verilogName(*inst_it));
            }
            // Also write this pair to a file
            pairs_with_through_constraints_file << src << "\t" << dst
                                                << std::endl;
        } else {
            // Default to the min constraint of all the paths
            // We'll do this using a separate post-processing script, which
            // modifies the .sdc file.
            // We also need to write the min constraint
            pairs_whose_through_constraints_must_be_removed_file
                << src << "\t" << dst << "\t"
                << src_dst_pair_to_min_slack[src_dst_pair] << std::endl;
        }
    }

    pairs_with_through_constraints_file.close();
    pairs_whose_through_constraints_must_be_removed_file.close();
}

bool GenerateRTL::multi_cycle_force_wire_operand(Instruction *I) {
    if (isa<LoadInst>(*I) || isa<PHINode>(*I) || isa<CallInst>(*I) ||
        is_instruction_pipelined_after_multicycling(I)) {
        return false;
    }
    return mc_force_wire_operand;
}

void GenerateRTL::abort_if_poor_multicycling()
// If MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS is disabled, then this function
// checks whether we have any very unbalanced paths which would cause
// low fmax. If this is the case, exit and warn the user.
{
    if (LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS")) {
        return;
    }
    if (!LEGUP_CONFIG->getParameterInt("MULTI_CYCLE_ABORT")) {
        return;
    }

    const int threshold = 40;
    bool abort = false;

    // Use src_dst_pair_to_min_slack and src_dst_pair_to_max_slack
    src_dst_pair_to_min_slack_t::const_iterator i =
        src_dst_pair_to_min_slack.begin();
    src_dst_pair_to_min_slack_t::const_iterator e =
        src_dst_pair_to_min_slack.end();
    for (; i != e; ++i) {
        src_dst_pair_t pair = i->first;
        int min = i->second;
        assert(src_dst_pair_to_max_slack.find(pair) !=
               src_dst_pair_to_max_slack.end());
        int max = src_dst_pair_to_max_slack[pair];

        if (((max - min) > threshold) && (min < 5)) {
            errs() << "Found " << pair.first << " to " << pair.second
                   << " with " << min << " and " << max << "!\n\n";
            abort = true;
            break;
        }
    }

    if (!abort) {
        return;
    }

    errs() << "*******************************  EXIT  "
              "*******************************\n";
    errs() << "LegUp has detected that multi-cycling will result in a low "
              "clock      \n";
    errs() << "frequency and exited early. To disable this check, unset the "
              "option   \n";
    errs() << "MULTI_CYCLE_ABORT in legup.tcl. You can also turn on the "
              "setting      \n";
    errs() << "MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS, which will cause LegUp to "
              "attempt\n";
    errs() << "to fix the low clock frequency by applying extra timing "
              "constraints.  \n";
    errs() << "****************************************************************"
              "******\n";
    exit(0);
}

void GenerateRTL::printSDCMultiCycleConstraints()
// This function runs in flows where registers are removed from data paths
// and replaced with multicycle .sdc constraints.
{
    // Now that binding is finished, print constraints for dividers
    print_lpm_div_multicycle_constraints();

    // For now multi-cycle paths are disabled in cases where resources
    // are pipelined (includes loop pipelining).
    // Also only enable for the pure-HW flow.
    if (!LEGUP_CONFIG->does_flow_support_multicycle_paths()) {
        return;
    }

    // For all basic blocks, iterate over all instructions and print the
    // required multicycle constraints
    for (Function::iterator b = Fp->begin(), be = Fp->end(); b != be; ++b) {
        // Print multi-cycle constraints for all instructions in this BB
        //
        // Algorithm: For every instruction I in this BB,
        //   - If this instruction does not haave a register, continue
        //   - Else perform a DFS of the predecessors P of I (within the BB):
        //      - If P has a register, set a multi-cycle constraint from P->I
        //      - Else continue the DFS on P
        //
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
            if (!alloc->is_registered_instruction(i)) {
                continue;
            }

            // Instead of continuing for call instructions, the arguments of
            // the call can be dests of MC paths, since args are stored in
            // registers outside the instantiated module.
            // Use arg_operands() to iterate over all args of this call, and
            // for each arg signal call create_multicycle_paths_to_signal.
            //
            // We can also just do the traversal normally using
            // create_multicycle_paths_from_root, but then reg must be removed
            // from signals driving each arg register.
            // if (isa<CallInst>(*i)) { continue; }

            create_multicycle_paths_from_root(i);
        }
    }

    // The iteration above was for all paths terminating at registers that
    // correspond to instructions. However, we also need to print multi-cycle
    // constraints for paths which are part of next-state logic and terminate
    // at the cur_state register.

    // MULTICYCLE_TODO
    // Update: Actually cur_state should just correspond to Branch and Switch
    // instructions. So rather than call create_multicycle_paths_to_signal
    // we could just include Branches and Switches in SDCScheduler as registered
    // instructions, then print "cur_state" for them as MC path roots.
    // Then they would be handled in create_multicycle_paths_from_root() and
    // no special call to create_multicycle_paths_to_signal is needed.
    RTLSignal *cur_sig = rtl->findExists("cur_state");
    assert(cur_sig);
    create_multicycle_paths_to_signal(cur_sig);

    // Final step: Now src_dst_pair_to_path_map and duplicate_src_dst_pairs
    // have been filled. The final step is to take these two and determine
    // a set of intermediate signals to add synthesis keep to.
    find_signals_to_synth_keep();
    rtl->add_signals_to_synth_keep(signals_to_keep);

    abort_if_poor_multicycling();
}

} // End legup namespace
