//===------ ModuloScheduling.cpp ---------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Holds the datastructure for modulo scheduling
//      Modulo Reservation Table
// Also has helper functions that would be used by all loop pipelining
// algorithms
//
//===----------------------------------------------------------------------===//

#ifndef MODULOSCHEDULER_H
#define MODULOSCHEDULER_H

#include "SchedulerDAG.h"
#include "ElementaryCycles.h"
#include "llvm/Support/FileSystem.h"

using namespace llvm;

namespace legup {

class Allocation;

class ModuloScheduler {

  public:
    Function *F;
    Module *M;
    BasicBlock *BB;
    bool forceNoChain;
    std::set<std::string> constrainedFuNames;
    std::string loopLabel;
    unsigned tripCount;
    SchedulerDAG *dag;
    Instruction *getLateDefiner(Instruction *I);
    Value *getEarlyDefiner(Instruction *I);
    void getRestructuredOpsForAddSubLateOp0(
        Instruction *curOp, Instruction *late, Instruction *lateParent,
        Instruction::BinaryOps &newCurOpType,
        Instruction::BinaryOps &newEarlyOpType, Instruction::BinaryOps addType,
        Instruction::BinaryOps subType);
    void getRestructuredOpsForAddSubLateOp1(
        Instruction *curOp, Instruction *late, Instruction *lateParent,
        Instruction::BinaryOps &newCurOpType,
        Instruction::BinaryOps &newEarlyOpType, Instruction::BinaryOps addType,
        Instruction::BinaryOps subType);
    bool getRestructuredOpsForAddSub(Instruction *curOp, Instruction *late,
                                     Instruction *lateParent,
                                     Instruction::BinaryOps &newCurOpType,
                                     Instruction::BinaryOps &newEarlyOpType);
    bool getRestructuredOps(Instruction *curOp, Instruction *late,
                            Instruction *lateParent,
                            Instruction::BinaryOps &newCurOpType,
                            Instruction::BinaryOps &newEarlyOpType);
    bool restructureBinaryOperation(Instruction *curOp, Instruction *early,
                                    Instruction *late);
    void simpleDCE();
    void restructureDFG();

    void printLineBreak() {
        File() << "------------------------------------------------------------"
                  "--------------------\n";
    }

    bool isResourceConstrained(Instruction *I) {
        std::string FuName = LEGUP_CONFIG->getOpNameFromInst(I, alloc);
        return constrainedFuNames.find(FuName) != constrainedFuNames.end();
    }

    void printDFGFile(const char *fileName);

    Allocation *alloc;
    raw_fd_ostream *file;

    LegupConfig::LOOP_PIPELINE PipelineTclInfo;

    ModuloScheduler() : alloc(0), file(NULL) {

        ranAlready = false;
        totalLoopsPipelined = 0;
        if (!file) {
            std::string FileError;
            file = new raw_fd_ostream("pipelining.legup.rpt", FileError,
                                      llvm::sys::fs::F_None);
            assert(FileError.empty() && "Error opening log files");
        }
    }

    ~ModuloScheduler() {
        if (file) {
            // need to close the file and flush any remaining output
            delete file;
        }
    }

    raw_fd_ostream &File() { return *file; }

    // member variables
    // IRBuilder<> *Builder;
    PHINode *inductionVar;
    BasicBlock *loopPreheader;
    // basic block of the kernel
    Loop *loop;
    int II;
    bool ranAlready;
    int totalLoopsPipelined;
    std::set<std::string> loopsPipelined;
    std::map<Instruction *, int> schedTime;
    std::map<Instruction *, int> schedStage;

    // modulo reservation table.
    // The table has II instruction entries in each row (II cycles per stage).
    // the key for the map is the FU name for the operation type
    // the first index of the array is the issue slot, for instance,
    // memory usually has 2 ports so there are 2 issue slots
    // don't access map directly, use helper functions below
    // each local RAM has a separate reservation table entry (distinct FuName)
    std::map<std::string, std::vector<std::vector<Instruction *>>>
        reservationTable;

    // given a function unit (FU) name, issue slot (i.e. memory port), and time
    // in the pipeline return the instruction in the modulo reservation table
    // return NULL if slot is empty
    Instruction *getReservationTable(std::string FuName, int issueSlot,
                                     int time) {
        return reservationTable[FuName].at(issueSlot).at(time);
    }

    void setReservationTable(std::string FuName, int issueSlot, int time,
                             Instruction *I) {
        reservationTable[FuName].at(issueSlot).at(time) = I;
    }

    void initReservationTable();
    int delay(Instruction *I);

    bool check_for_legup_label(BasicBlock *bb);
    std::string get_legup_label(BasicBlock *bb);
    bool find_legup_label(Module *M, std::string label);
    void verify_can_find_all_loop_labels();

    int numIssueSlots(std::string FuName);
    bool resourceConflict(Instruction *I, int timeSlot);
    int findTimeSlot(Instruction *I, int minTime, int maxTime);
    bool MRTSlotEmpty(int slot, std::string FuName);
    void printModuloReservationTable();
    // is there a memory dependency from i -> j?
    bool memory_dependence(Instruction *i, Instruction *j);
    // is there a data dependence from i to j
    // ie. does j depend on i
    bool dependent(Instruction *i, Instruction *j);
    int getLocalMemDistance(Instruction *i, Instruction *j);
    // distance(i, j) is the number of iterations separating dependent
    // operations
    // i and j
    int distance(Instruction *i, Instruction *j);
    int maxStage;
    int totalTime;
    void gather_pipeline_stats();
    void pipeline_table_headers(std::stringstream &stage_header,
                                std::stringstream &time_header,
                                std::stringstream &ii_header);
    void print_pipeline_table();
    void set_llvm_metadata();
    BasicBlock *find_pipelined_bb();
    void sanityCheckII(int II);

    void addEdge(formatted_raw_ostream &out, dotGraph<InstructionNode> &graph,
                 InstructionNode *start, InstructionNode *end, int delay,
                 int distance, std::string color);
    // print a dot graph representing the dependency information (both normal
    // and
    // memory) for the loop
    void printDependencyDot(formatted_raw_ostream &out);
    int onCriticalPath(Instruction *I);
    FindElementaryCycles EC;
    // maps every instruction in a recurrence to the maximum recMII of all
    // recurrences the instruction is part of
    std::map<Instruction *, int> recurrenceMaxRecMII;
    void initElementaryCycles();
    // given a cycle of instructions through the DFG find the recurrence minimum
    // initiation interval
    int getCycleRecMII(std::list<Instruction *> &path);
    int findLoopRecurrences();
    // todo: move to SchedulerDAG?

    struct MEM_ACCESS {
        // I is a load/store
        Instruction *I;
        RAM *ram;
        MEM_ACCESS() : I(0), ram(0), offset(0), ptr(0) {}
        enum TYPE { InductionOffset, Address } type;
        // type: InductionOffset. The memory was accessed as:
        // ram[i + offset]
        int offset;
        // type: Address. The memory was accessed as:
        // ram[GEP]
        Value *ptr;
    };
    // takes an offset (%offset) from the induction variable (%ind):
    //    %ind = phi i32 [ 0, %0 ], [ %tmp29, %1 ],
    //    %offset = add i32 %ind, 1
    // returns true if the offset is valid and updates the arg: indexOffset
    bool findInductionOffset(Value *offset, RAM *ram, PHINode *induction,
                             std::string memtype, int *indexOffsetPtr);
    std::map<Instruction *, std::map<Instruction *, int>> localMemDistances;
    // This function populates two datastructures:
    // memAccessMap: maps a load/store instruction to information about the
    //               memory access (RAM, offset, etc)
    // memoryAccesses: maps a RAM to all memory accesses (load/stores) to/from
    //                 that RAM
    void findLoopCarriedMemoryAccesses(
        RAM *globalRAM, std::map<Instruction *, MEM_ACCESS> &memAccessMap,
        std::map<RAM *, std::vector<MEM_ACCESS>> &memoryAccesses);
    // look for very simple loop-carried dependencies for local arrays...
    // something like:
    //    a[i+1] = a[i] + val;
    // the LLVM IR would look something like:
    //    %ind = phi i32 [ 0, %0 ], [ %tmp29, %1 ],
    //    %i.plus.1 = add i32 %ind, 1
    //    %a.i.ptr = getelementptr [100 x i32]* @a, i32 0, i32 %ind
    //    %a.i.plus.1.ptr = getelementptr [100 x i32]* @a, i32 0, i32 %tmp29
    //    %a.i = volatile load i32* %a.i.ptr, align 4
    //    %add = add i32 %val, %a.i
    //    volatile store i32 %add, i32* %a.i.plus.1.ptr, align 4,
    //
    // tip: make sure the induction variable is a i32 not a i64
    void addLocalMemConstraints();
    // get the dependency distance between a load and store to the same RAM
    // For instance:
    //    loop i: 1 to N
    //      a[i+5] = ...
    //      ...      = a[i-3]
    // The distance would be: 5 - (-3) = 8
    // returns -1 when we can't figure out the distance
    int getDistance(MEM_ACCESS storeAccess, MEM_ACCESS loadAccess);
    // when we find a dependency we update two datastructures:
    //    1) localMemDistances[StoreInst][LoadInst] = distance;
    //    2) update the dag to include memory dependencies for the load/store
    void foundLoopCarriedDependency(int distance, MEM_ACCESS storeAccess,
                                    MEM_ACCESS loadAccess);
    // given a store to a particular local RAM, can we find a load to that
    // same RAM that creates a loop carried RAW dependency?
    // TODO: a store could also alias
    void findMemoryAliasingWithStore(MEM_ACCESS storeAccess,
                                     std::vector<MEM_ACCESS> &aliasedAccesses);
};

} // End legup namespace

#endif
