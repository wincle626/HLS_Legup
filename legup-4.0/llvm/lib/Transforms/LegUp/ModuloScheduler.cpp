//===------ ModuloScheduling.cpp ---------------------------------===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// See header
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/LLVMContext.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "ModuloScheduler.h"
#include "Scheduler.h"
#include "Ram.h"
#include "utils.h"
#include <iomanip>

using namespace legup;

void ModuloScheduler::initReservationTable() {
    reservationTable.clear();
    constrainedFuNames.clear();

    for (BasicBlock::iterator instr = BB->begin(), ie = BB->end(); instr != ie;
         ++instr) {

        std::string FuName =
            LEGUP_CONFIG->getOpNameFromInst(instr, this->alloc);

        // skip unconstrained instructions
        if (numIssueSlots(FuName) == 0)
            continue;

        if (reservationTable.find(FuName) != reservationTable.end())
            continue;

        constrainedFuNames.insert(FuName);

        for (int i = 0; i < numIssueSlots(FuName); i++) {
            std::vector<Instruction *> timeSlots;
            for (int j = 0; j < this->II; j++) {
                timeSlots.push_back(NULL);
            }
            reservationTable[FuName].push_back(timeSlots);
        }
    }
}

void ModuloScheduler::verify_can_find_all_loop_labels() {
    // LI = &getAnalysis<LoopInfo>();
    loopPreheader = loop->getLoopPreheader();
    assert(loopPreheader);
    F = loopPreheader->getParent();
    M = F->getParent();
    // make sure we can find each loop label in the tcl file
    std::map<std::string, LegupConfig::LOOP_PIPELINE> &loop_pipelines =
        LEGUP_CONFIG->getAllLoopPipelines();

    for (std::map<std::string, LegupConfig::LOOP_PIPELINE>::iterator
             i = loop_pipelines.begin(),
             ie = loop_pipelines.end();
         i != ie; ++i) {
        std::string label = i->first;
        if (!find_legup_label(M, label)) {
            errs() << "Error: Couldn't find any basic block with label: "
                   << label << "\n";
        }
    }
}

// see if we can find label in any basic blocks of this program
bool ModuloScheduler::find_legup_label(Module *M, std::string label) {
    for (Module::iterator F = M->begin(), FE = M->end(); F != FE; ++F) {
        for (Function::iterator BB = F->begin(), EE = F->end(); BB != EE;
             ++BB) {
            if (get_legup_label(BB) == label) {
                // errs() << "Found label: " << label << " in BB: " <<
                // getLabel(BB) << "\n";
                return true;
            }
        }
    }
    return false;
}

std::string ModuloScheduler::get_legup_label(BasicBlock *bb) {
    std::string label = "";
    for (BasicBlock::iterator instr = bb->begin(), ie = bb->end(); instr != ie;
         ++instr) {
        const CallInst *ci = dyn_cast<CallInst>(instr);
        if (!ci)
            continue;

        Function *calledFunc = ci->getCalledFunction();
        // ignore indirect function invocations
        if (!calledFunc)
            continue;

        if (calledFunc->getName() == "__legup_label") {

            // errs() << "Found label: " << *ci << "\n";
            Value *str = *ci->op_begin();
            // handle getelementptr
            if (const User *U = dyn_cast<User>(str)) {
                if (U->getNumOperands() > 1) {
                    str = U->getOperand(0);
                }
            }
            GlobalVariable *G = dyn_cast<GlobalVariable>(str);
            assert(G);
            Constant *C = G->getInitializer();
            assert(C);
            // TODO: LLVM 3.4 update
            // ConstantArray *CA = dyn_cast<ConstantArray>(C);
            if (ConstantArray *CA = dyn_cast<ConstantArray>(C)) {
                assert(CA);
                label = arrayToString(CA);
            } else if (ConstantDataArray *CA = dyn_cast<ConstantDataArray>(C)) {
                assert(CA);
                label = CA->getAsCString();
            }
            return label;
        }
    }
    return label;
}

// check for a call to __legup_label(label) in this basic block
// with a label that matches a loop pipeline label
bool ModuloScheduler::check_for_legup_label(BasicBlock *bb) {
    if (LEGUP_CONFIG->getParameterInt("PIPELINE_ALL")) {
        PipelineTclInfo.user_II = false;
        PipelineTclInfo.II = 0;
        PipelineTclInfo.ignoreMemDeps = false;
        PipelineTclInfo.label = "nolabel";
        return true;
    }

    loopLabel = get_legup_label(bb);

    if (LEGUP_CONFIG->isLoopPipelined(loopLabel)) {
        PipelineTclInfo = LEGUP_CONFIG->getLoopPipeline(loopLabel);
        return true;
    }
    return false;
}

// TODO: fix these numbers to account for chaining
// the delay of an operation is not actually known
// but determined by the schedule and depends on chaining
// delay(I): is the latency of instruction I
int ModuloScheduler::delay(Instruction *I) {

    if (isa<PHINode>(I)) {
        return 0;
    }

    if (isa<BitCastInst>(I)) {
        return 0;
    }

    /*
    if (PHINode *phi = dyn_cast<PHINode>(I)) {
        int index = phi->getBasicBlockIndex(BB);
        if (index == -1) return 0;
        Value *v = phi->getIncomingValue(index);
        if (PHINode *p = dyn_cast<PHINode>(v)) {
            if (p->getParent() == BB) {
                // push to next cycle
                return 1;
            }
        }
        return 0;
    }
    */

    int latency = Scheduler::getNumInstructionCycles(I);

    if (latency > 0) {
        return latency;
    }

    switch (I->getOpcode()) {
    case (Instruction::SExt):
    case (Instruction::ZExt):
    case (Instruction::Trunc):
        if (LEGUP_CONFIG->getParameterInt("LOOP_PIPELINE_CHAIN_EXT")) {
            // chain these simple operations
            return 0;
        }
    default: {

        if (LEGUP_CONFIG->getParameterInt("LOOP_PIPELINE_CHAIN_BINARY_NOOP")) {
            if (LEGUP_CONFIG->isBinaryOperatorNoOp(I)) {
                return 0;
            }
        }
        if (LEGUP_CONFIG->getParameterInt("PIPELINE_NO_CHAIN")) {
            // assume no chaining
            return 1;
        }
        if (forceNoChain) {
            return 1;
        }

        if (LEGUP_CONFIG->getParameterInt("SDC_ONLY_CHAIN_CRITICAL")) {
            if (onCriticalPath(I)) {
                // errs() << "critical: " << *I << "\n";
                return 0;
            } else {
                // errs() << "non-critical: " << *I << "\n";
                return 1;
            }
        }

        // TODO: read this delay in from SDC schedule, an operation
        // might not be chained if there are a few operations in the
        // same clock cycle
        return 0;
    }
    }
}

int ModuloScheduler::numIssueSlots(std::string FuName) {

    int constraint;
    if (LEGUP_CONFIG->getNumberOfFUsAllocated(FuName, &constraint)) {
        return constraint;
    } else {
        // no user-specified constraint
        return 0;
    }
}

bool ModuloScheduler::resourceConflict(Instruction *I, int timeSlot) {
    int moduloTimeSlot = timeSlot % this->II;

    if (isResourceConstrained(I)) {
        std::string FuName = LEGUP_CONFIG->getOpNameFromInst(I, this->alloc);
        for (int i = 0; i < numIssueSlots(FuName); i++) {
            if (getReservationTable(FuName, i, moduloTimeSlot) == NULL) {
                // found a free slot
                return false;
            }
        }
        return true;
    }

    return false;
}

bool ModuloScheduler::MRTSlotEmpty(int slot, std::string FuName) {
    for (int j = 0; j < numIssueSlots(FuName); j++) {
        if (getReservationTable(FuName, j, slot)) {
            return false;
        }
    }
    return true;
}

void ModuloScheduler::printModuloReservationTable() {

    for (std::set<std::string>::iterator i = constrainedFuNames.begin(),
                                         e = constrainedFuNames.end();
         i != e; ++i) {
        std::string FuName = *i;
        File() << "FuName: " << FuName << "\n";

        for (int i = 0; i < this->II; i++) {
            File() << "time slot: " << i;
            if (MRTSlotEmpty(i, FuName)) {
                File() << " empty\n";
                continue;
            }
            File() << "\n";
            for (int j = 0; j < numIssueSlots(FuName); j++) {
                // TODO: LLVM 3.5 update: cannot print value if it is NULL so
                // check it here
                if (getReservationTable(FuName, j, i) == NULL) {
                    File() << "   issue slot: " << j << " instr: "
                           << "printing a <null> value"
                           << "\n";
                } else {
                    File() << "   issue slot: " << j
                           << " instr: " << *getReservationTable(FuName, j, i)
                           << "\n";
                }
            }
        }
    }
}

bool ModuloScheduler::memory_dependence(Instruction *i, Instruction *j) {
    InstructionNode *iNode = dag->getInstructionNode(i);
    for (InstructionNode::iterator use = iNode->mem_use_begin(),
                                   e = iNode->mem_use_end();
         use != e; ++use) {
        // is there a memory dependency from i -> j?
        if ((*use)->getInst() == j) {
            return true;
        }
    }
    return false;
}

bool ModuloScheduler::dependent(Instruction *i, Instruction *j) {
    bool debug = LEGUP_CONFIG->getParameterInt("DEBUG_MODULO_DEPENDENT");
    if (debug)
        errs() << "dependent(" << *i << ", " << *j << ") = ";

    // check for cross-iteration memory dependencies
    // iter 0: load -> store
    // iter 1: load -> store
    // you will have a dependency between the store -> load with dist=1
    if (getLocalMemDistance(i, j)) {
        if (debug)
            errs() << "true (cross-iteration)\n";
        return true;
    }

    // can't use SchedulerDAG here because it doesn't include phi nodes
    for (User::op_iterator dep = j->op_begin(), e = j->op_end(); dep != e;
         ++dep) {
        Instruction *pred = dyn_cast<Instruction>(dep);
        if (!pred)
            continue;
        if (pred == i) {
            if (debug)
                errs() << "true\n";
            return true;
        }
    }

    if (memory_dependence(i, j)) {
        if (debug)
            errs() << "true (memory)\n";
        return true;
    }

    if (debug)
        errs() << "false\n";
    return false;
}

int ModuloScheduler::getLocalMemDistance(Instruction *i, Instruction *j) {
    if (localMemDistances.find(i) != localMemDistances.end()) {
        int distance = localMemDistances[i][j];
        if (distance) {
            return distance;
        }
    }
    return 0;
}

int ModuloScheduler::distance(Instruction *i, Instruction *j) {
    assert(dependent(i, j));

    if (int dist = getLocalMemDistance(i, j)) {

        if (PipelineTclInfo.ignoreMemDeps) {
            return 0;
        }

        // errs() << "distance " << dist << " from: " << *i << " to " <<
        //    *j << "\n";
        return dist;
    }

    // if j is a phi that has an incoming value of i for the loop's basic
    // block then we have a distance of 1
    if (PHINode *phi = dyn_cast<PHINode>(j)) {
        int index = phi->getBasicBlockIndex(BB);
        if (index == -1)
            return 0;
        Value *v = phi->getIncomingValue(index);
        if (v == i) {
            File() << "distance 1 from: " << *i << " to " << *phi << "\n";
            return 1;
        }
    }

    // see explanation in dependent() for handling memory operations that
    // span across iterations
    if (isMem(i) && isMem(j)) {

        if (PipelineTclInfo.ignoreMemDeps) {
            return 0;
        }

        if (memory_dependence(j, i)) {
            // memory dependency from j -> i (cross-iteration)
            File() << "distance 1 from memory: " << *j << " to " << *i << "\n";
            return 1;
        }
    }

    return 0;
}

void ModuloScheduler::gather_pipeline_stats() {

    this->maxStage = 0;
    this->totalTime = 0;

    std::map<int, std::vector<Instruction *>> instrSchedAtTime;

    for (BasicBlock::iterator I = BB->begin(), ie = BB->end(); I != ie; ++I) {

        assert(schedTime.find(I) != schedTime.end());

        int start_time = schedTime[I];
        int avail_time = start_time + delay(I);
        schedStage[I] = start_time / this->II;
        this->totalTime = std::max(this->totalTime, avail_time);
        this->maxStage = std::max(this->maxStage, schedStage[I]);
        instrSchedAtTime[start_time].push_back(I);
    }

    int numStages = this->maxStage + 1;
    File() << "Final Pipeline Schedule:\n";
    File() << "Total pipeline stages: " << numStages << "\n";
    for (int stage = 0; stage < numStages; stage++) {
        File() << "Stage: " << stage << "\n";
        for (int ii = 0; ii < this->II; ii++) {
            int time = this->II * stage + ii;
            for (std::vector<Instruction *>::iterator
                     i = instrSchedAtTime[time].begin(),
                     e = instrSchedAtTime[time].end();
                 i != e; ++i) {
                Instruction *I = *i;

                File() << "Time: " << schedTime[I]
                       << " Stage: " << schedStage[I] << " instr: " << *I
                       << "\n";
            }
        }
    }
    File() << "\n\n";
}

void ModuloScheduler::pipeline_table_headers(std::stringstream &stage_header,
                                             std::stringstream &time_header,
                                             std::stringstream &ii_header) {

    int colwidth = 15;
    int stagewidth = colwidth * this->II;
    int numStages = this->maxStage + 1;
    for (int stage = 0; stage < numStages; stage++) {
        if (stage == 0) {
            stage_header << "Stage: " << std::setw(stagewidth - stagewidth / 2)
                         << stage;
        } else {
            stage_header << std::setw(stagewidth) << stage;
        }
        for (int ii = 0; ii < this->II; ii++) {
            int time = this->II * stage + ii;
            if (ii == 0 && stage == 0) {
                ii_header << std::setw(colwidth / 2)
                          << "II: " << std::setw(colwidth - colwidth / 2) << ii;
                time_header << std::setw(colwidth / 2)
                            << "Time: " << std::setw(colwidth - colwidth / 2)
                            << time;
            } else if (ii == 0 && stage > 0) {
                ii_header << std::setw(colwidth / 2) << " | "
                          << std::setw(colwidth - colwidth / 2) << ii;
                time_header << std::setw(colwidth / 2) << " | "
                            << std::setw(colwidth - colwidth / 2) << time;
            } else {
                time_header << std::setw(colwidth) << time;
                ii_header << std::setw(colwidth) << ii;
            }
        }
    }
}

void ModuloScheduler::print_pipeline_table() {
    std::stringstream table;
    std::stringstream stage_header;
    std::stringstream time_header;
    std::stringstream ii_header;

    pipeline_table_headers(stage_header, time_header, ii_header);

    int colwidth = 15;
    int numStages = this->maxStage + 1;

    table << "Pipeline Table:\n";
    table << "Total pipeline stages: " << numStages << "\n";
    stage_header.flush();
    time_header.flush();
    ii_header.flush();
    table << stage_header.str() << "\n";
    table << ii_header.str() << "\n";
    table << time_header.str() << "\n";
    for (BasicBlock::iterator I = BB->begin(), ie = BB->end(); I != ie; ++I) {
        int start_time = schedTime[I];
        int mod_time = schedTime[I] % II;
        for (int stage = 0; stage < numStages; stage++) {
            for (int ii = 0; ii < this->II; ii++) {
                int time = this->II * stage + ii;
                if ((time % II) == mod_time && time >= start_time) {
                    std::string label = getLabel(I);
                    limitString(label, colwidth - 3);
                    table << std::setw(colwidth) << label;
                } else {
                    table << std::setw(colwidth) << "-";
                }
            }
        }
        table << "\n";
    }
    table << "\n";

    table.flush();
    File() << table.str();
}

void ModuloScheduler::set_llvm_metadata() {

    // time is indexed at 0
    this->totalTime++;

    // mark original/prologue/epilogue basic blocks as "loop.pipelined"
    setMetadataInt(BB->getTerminator(), "legup.pipelined", 1);
    setMetadataInt(BB->getTerminator(), "legup.II", this->II);
    setMetadataInt(BB->getTerminator(), "legup.totalTime", this->totalTime);
    setMetadataInt(BB->getTerminator(), "legup.maxStage", this->maxStage);
    setMetadataInt(BB->getTerminator(), "legup.tripCount", this->tripCount);
    setMetadataStr(BB->getTerminator(), "legup.label",
                   this->PipelineTclInfo.label);

    for (BasicBlock::iterator I = BB->begin(), ie = BB->end(); I != ie; ++I) {
        assert(schedTime.find(I) != schedTime.end());
        assert(schedStage.find(I) != schedStage.end());
        setMetadataInt(I, "legup.pipeline.start_time", schedTime[I]);
        setMetadataInt(I, "legup.pipeline.avail_time", schedTime[I] + delay(I));
        setMetadataInt(I, "legup.pipeline.stage", schedStage[I]);
    }
}

BasicBlock *ModuloScheduler::find_pipelined_bb() {

    // skip pipelining for now
    if (LEGUP_CONFIG->getParameterInt("NO_LOOP_PIPELINING")) {
        return NULL;
    }

    if (loop->getBlocks().size() > 1) {
        // ignore loops with more than one basic block for now
        return NULL;
    }

    BasicBlock *BB = NULL;
    for (Loop::block_iterator bb = loop->block_begin(), e = loop->block_end();
         bb != e; ++bb) {
        assert(!BB);

        if (check_for_legup_label(*bb)) {
            BB = *bb;
            break;
        }
    }
    return BB;
}

// sanity check - make sure the II is less than the maximum possible II
void ModuloScheduler::sanityCheckII(int II) {
    int largestPossibleII = 1;

    // assume every instruction is dependent and executed sequentially
    for (BasicBlock::iterator instr = BB->begin(), ie = BB->end(); instr != ie;
         ++instr) {
        largestPossibleII += 1; // delay(instr);
    }
    File() << "largestPossibleII: " << largestPossibleII << "\n";

    assert(II <= largestPossibleII && "Error: II should never be this large");
}

void ModuloScheduler::addEdge(formatted_raw_ostream &out,
                              dotGraph<InstructionNode> &graph,
                              InstructionNode *start, InstructionNode *end,
                              int delay, int distance, std::string color) {
    std::string label = "label=\"" + utostr(delay);
    if (distance) {
        label += "[" + utostr(distance) + "]";
    }
    label += "\",color=blue";
    if (distance) {
        InstructionNode *tmp;
        tmp = start;
        start = end;
        end = tmp;
        label += ",dir=back";
    }
    graph.connectDot(out, start, end, label);
}

static void printNodeLabel(raw_ostream &out, InstructionNode *I) {
    out << *I->getInst();
}

void ModuloScheduler::printDependencyDot(formatted_raw_ostream &out) {

    dotGraph<InstructionNode> graph(out, printNodeLabel);
    graph.setLabelLimit(40);

    bool ignoreDummyCalls =
        !LEGUP_CONFIG->getParameterInt("DFG_SHOW_DUMMY_CALLS");

    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            if (ignoreDummyCalls && isaDummyCall(i))
                continue;
            if (ignoreDummyCalls && isaDummyCall(j))
                continue;

            if (dependent(i, j)) {
                addEdge(out, graph, dag->getInstructionNode(i),
                        dag->getInstructionNode(j), delay(i), distance(i, j),
                        "blue");
            }
        }
    }

    /*
    bool ignoreDummyCalls = true;
    for (BasicBlock::iterator I = BB->begin(), ie = BB->end(); I != ie; ++I) {
        InstructionNode *op = dag->getInstructionNode(I);
        if (ignoreDummyCalls && isaDummyCall(I)) continue;

        for (Value::use_iterator use = I->use_begin(), e = I->use_end(); use !=
    e;
                ++use) {
            if (Instruction *child = dyn_cast<Instruction>(*use)) {
                if (ignoreDummyCalls && isaDummyCall(child)) continue;

                addEdge(out, graph, op, dag->getInstructionNode(child),
                        delay(I), distance(I, child), "blue");
            }
        }

        for (InstructionNode::iterator use = op->mem_use_begin(),
                e = op->mem_use_end(); use != e; ++use) {
            Instruction *child = (*use)->getInst();
            if (ignoreDummyCalls && isaDummyCall(child)) continue;

            addEdge(out, graph, op, *use,
                    delay(I), distance(I, child), "red");
        }

    }
    */
}

int ModuloScheduler::onCriticalPath(Instruction *I) {
    /*

    //return (minDist[I][I] == 0);
    if (minDistCopy.find(I) == minDistCopy.end()) {
        //errs() << "mindist not found 1. onCriticalPath=true\n";
        //return false;
        return true;
    }
    if (minDistCopy[I].find(I) == minDistCopy[I].end()) {
        //errs() << "mindist not found 2. onCriticalPath=true\n";
        //return false;
        return true;
    }
    int distFromOrigRecMII = II - origRecMII;
    //errs() << "II: " << II << "\n";
    //errs() << "mindist: " << minDistCopy[I][I] << " == " <<
    -distFromOrigRecMII << "\n";
    //errs() << "onCriticalPath: mindist" << minDistCopy[I][I] << "==0\n";
    //return (minDist[I][I] == 0);
    //
    return (minDistCopy[I][I] == -distFromOrigRecMII);
    */
    /*
    for (FindElementaryCycles::CycleListTy::iterator i = EC.cycles.begin(), e
            = EC.cycles.end(); i != e; ++i) {
        std::list< Instruction* > &path = *i;
        for (std::list< Instruction* >::iterator pi = path.begin(), pe =
                path.end(); pi != pe; ++pi) {
            if (I == *pi) return true;
        }
    }
    */
    return recurrenceMaxRecMII[I];
}

void ModuloScheduler::printDFGFile(const char *fileName) {
    if (LEGUP_CONFIG->getParameterInt("NO_DFG_DOT_FILES")) {
        return;
    }

    { // Need to go out of scope to write to file properly
        std::string FileError;
        // print out the dependence graph
        raw_fd_ostream pipelineDFG(fileName, FileError, llvm::sys::fs::F_None);
        assert(FileError.empty() && "Error opening log files");
        formatted_raw_ostream out(pipelineDFG);
        printDependencyDot(out);
    }
}

void ModuloScheduler::initElementaryCycles() {
    EC.clear();

    bool ignoreDummyCalls = true;
    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        for (BasicBlock::iterator j = BB->begin(), je = BB->end(); j != je;
             ++j) {
            if (ignoreDummyCalls && isaDummyCall(i))
                continue;
            if (ignoreDummyCalls && isaDummyCall(j))
                continue;
            if (isMem(i) & isMem(j) & PipelineTclInfo.ignoreMemDeps)
                continue;

            if (dependent(i, j)) {
                EC.addEdge(i, j);
            }
        }
    }
}

int ModuloScheduler::getCycleRecMII(std::list<Instruction *> &path) {

    int cycleDelay = 0;
    int cycleDistance = 0;
    Instruction *first = NULL;
    Instruction *prev = NULL;
    int edgeDistance = 0;
    bool multipleBackEdges = false;
    for (std::list<Instruction *>::iterator pi = path.begin(), pe = path.end();
         pi != pe; ++pi) {
        Instruction *I = *pi;
        if (!prev) {
            first = I;
        } else {
            edgeDistance = distance(prev, I);
            if (edgeDistance) {
                if (cycleDistance) {
                    multipleBackEdges = true;
                }
                cycleDistance = edgeDistance;
            }
        }
        prev = I;
        cycleDelay += delay(I);
        // do I need this?
        // well if we have a recurrence path where the latency of all
        // operations is 0 then the recMII for that path will be 0...
        if (isa<PHINode>(I)) {
            cycleDelay++;
        }
        File() << "delay: " << cycleDelay << " I: " << *I << "\n";
    }
    assert(prev);
    assert(first);
    edgeDistance = distance(prev, first);
    if (edgeDistance) {
        if (cycleDistance) {
            multipleBackEdges = true;
        }
        cycleDistance = edgeDistance;
    }
    if (multipleBackEdges) {
        File() << "More than one back edge -- ignoring\n";
        return 0;
    }
    assert(cycleDistance);
    File() << "Total cycle delay: " << cycleDelay << "\n";
    File() << "Total cycle dependency distance: " << cycleDistance << "\n";
    int cycleRecMII = ceil((float)cycleDelay / (float)cycleDistance);
    File() << "recMII = ceil(delay/distance) = " << cycleRecMII << "\n";
    File() << "\n";

    return cycleRecMII;
}

int ModuloScheduler::findLoopRecurrences() {
    printLineBreak();
    File() << "Finding all Loop Recurrences\n";

    if (LEGUP_CONFIG->getParameterInt("SKIP_ELEM_CYCLES")) {
        return 1;
    }

    initElementaryCycles();
    EC.solve();
    File() << "Found " << EC.cycles.size() << " elementary recurrence cycles\n";

    int overallRecMII = 0;
    int num = 1;
    for (FindElementaryCycles::CycleListTy::iterator path = EC.cycles.begin(),
                                                     e = EC.cycles.end();
         path != e; ++path) {

        File() << "Recurrence " << num++ << ":\n";

        int cycleRecMII = getCycleRecMII(*path);

        for (std::list<Instruction *>::iterator pi = path->begin(),
                                                pe = path->end();
             pi != pe; ++pi) {
            recurrenceMaxRecMII[*pi] =
                std::max(cycleRecMII, recurrenceMaxRecMII[*pi]);
        }

        overallRecMII = std::max(overallRecMII, cycleRecMII);
    }
    File() << "Overall recMII (max of recMII from all elementary recurrence "
              "cycles): " << overallRecMII << "\n";
    return overallRecMII;
    printLineBreak();
}

bool ModuloScheduler::findInductionOffset(Value *offset, RAM *ram,
                                          PHINode *induction,
                                          std::string memtype,
                                          int *indexOffsetPtr) {

    int indexOffset = 0;
    if (offset == induction) {
        indexOffset = 0;
        File() << "Found " << memtype << " to ram " << ram->getName()
               << " at index i\n";
    } else {
        // looking for something like:
        //      i + <num>
        //      i - <num>
        // where i is the induction variable

        Instruction *offsetInst = dyn_cast<Instruction>(offset);
        if (!offsetInst)
            return false;

        if (!isIAdd(offsetInst) && !isISub(offsetInst))
            return false;

        Value *op0 = offsetInst->getOperand(0);
        Value *op1 = offsetInst->getOperand(1);
        Value *num = NULL;
        if (op0 == loop->getCanonicalInductionVariable()) {
            num = dyn_cast<ConstantInt>(op1);
        } else if (op1 == loop->getCanonicalInductionVariable()) {
            num = dyn_cast<ConstantInt>(op0);
        }
        if (!num)
            return false;
        ConstantInt *ci = dyn_cast<ConstantInt>(num);
        if (!ci)
            return false;
        indexOffset = ci->getZExtValue();

        if (isIAdd(offsetInst)) {
            indexOffset = indexOffset;
        } else if (isISub(offsetInst)) {
            indexOffset = -indexOffset;
        } else {
            // not an add or subtract
            return false;
        }

        File() << "Found " << memtype << " to ram " << ram->getName()
               << " at index i + " << indexOffset << "\n";
    }
    *indexOffsetPtr = indexOffset;
    return true;
}

void ModuloScheduler::findLoopCarriedMemoryAccesses(
    RAM *globalRAM, std::map<Instruction *, MEM_ACCESS> &memAccessMap,
    std::map<RAM *, std::vector<MEM_ACCESS>> &memoryAccesses) {

    assert(alloc);
    // add additional memory constraints for local memory read/writes
    for (BasicBlock::iterator I = BB->begin(), ie = BB->end(); I != ie; I++) {
        Value *addr = NULL;
        std::string memtype;
        if (LoadInst *L = dyn_cast<LoadInst>(I)) {
            addr = L->getPointerOperand();
            memtype = "load";
        } else if (StoreInst *S = dyn_cast<StoreInst>(I)) {
            addr = S->getPointerOperand();
            memtype = "store";
        } else {
            continue;
        }

        RAM *ram;
        if (LEGUP_CONFIG->getParameterInt("LOCAL_RAMS")) {
            ram = alloc->getLocalRamFromValue(addr);
        } else {
            ram = globalRAM;
        }
        if (!ram)
            continue;

        GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(addr);
        if (!GEP)
            continue;

        Value *offset = GEP->getOperand(2);

        MEM_ACCESS access;
        access.I = I;
        access.ram = ram;

        int indexOffset = 0;
        if (findInductionOffset(offset, ram,
                                loop->getCanonicalInductionVariable(), memtype,
                                &indexOffset)) {
            // found an offset to the induction variable
            access.type = MEM_ACCESS::InductionOffset;
            access.offset = indexOffset;
        } else {
            access.type = MEM_ACCESS::Address;
            access.ptr = GEP;
        }

        memoryAccesses[ram].push_back(access);
        memAccessMap[I] = access;
    }
}

void ModuloScheduler::addLocalMemConstraints() {

    RAM *globalRAM = NULL;
    if (!LEGUP_CONFIG->getParameterInt("LOCAL_RAMS")) {
        // create a "dummy" global RAM so the code below works
        globalRAM = createEmptyRAM(M, alloc, "Global Shared RAM");
    }

    std::map<Instruction *, MEM_ACCESS> memAccessMap;
    std::map<RAM *, std::vector<MEM_ACCESS>> memoryAccesses;

    findLoopCarriedMemoryAccesses(globalRAM, memAccessMap, memoryAccesses);

    for (BasicBlock::iterator S = BB->begin(), ie = BB->end(); S != ie; S++) {
        // only looking for store instructions
        if (!isa<StoreInst>(S))
            continue;
        RAM *ram;
        if (LEGUP_CONFIG->getParameterInt("LOCAL_RAMS")) {
            ram = alloc->getLocalRamFromInst(S);
        } else {
            ram = globalRAM;
        }
        if (!ram)
            continue;
        if (memAccessMap.find(S) == memAccessMap.end())
            continue;

        findMemoryAliasingWithStore(memAccessMap[S], memoryAccesses[ram]);
    }
}

int ModuloScheduler::getDistance(MEM_ACCESS storeAccess,
                                 MEM_ACCESS loadAccess) {

    int distance = -1;

    if (storeAccess.type == MEM_ACCESS::InductionOffset &&
        loadAccess.type == MEM_ACCESS::InductionOffset) {
        distance = storeAccess.offset - loadAccess.offset;

    } else if (storeAccess.type == MEM_ACCESS::Address &&
               loadAccess.type == MEM_ACCESS::Address) {
        if (storeAccess.ptr == loadAccess.ptr) {

            if (memory_dependence(storeAccess.I, loadAccess.I)) {
                // The distance is 0 for the following case (just a
                // normal RAW memory dependency):
                //   a[const] = ...
                //   ...      = a[const]
                distance = 0;
            } else {
                // For example (distance = 1):
                //   ...  = a[const]
                //   a[const] = ...
                distance = 1;
            }
        }
    }

    return distance;
}

void ModuloScheduler::foundLoopCarriedDependency(int distance,
                                                 MEM_ACCESS storeAccess,
                                                 MEM_ACCESS loadAccess) {

    assert(distance > 0);

    Instruction *L = loadAccess.I;
    Instruction *S = storeAccess.I;

    File() << "Found a loop carried dependency:\n";
    File() << "Distance: " << distance << "\n";
    if (storeAccess.type == MEM_ACCESS::InductionOffset &&
        loadAccess.type == MEM_ACCESS::InductionOffset) {
        File() << "Type: Induction offset\n";
        File() << "Store to ram: " << storeAccess.ram->getName() << " at i + "
               << storeAccess.offset << " I: " << *S << "\n";
        File() << "Load to ram: " << loadAccess.ram->getName() << " at i + "
               << loadAccess.offset << " I: " << *L << "\n";
    } else {
        assert(storeAccess.type == MEM_ACCESS::Address &&
               loadAccess.type == MEM_ACCESS::Address);
        File() << "Type: Same address\n";
        File() << "Store to ram: " << storeAccess.ram->getName()
               << " at GEP: " << getLabel(storeAccess.ptr) << " I: " << *S
               << "\n";
        File() << "Load to ram: " << loadAccess.ram->getName()
               << " at GEP: " << getLabel(loadAccess.ptr) << " I: " << *L
               << "\n";
    }
    localMemDistances[S][L] = distance;

    InstructionNode *storeNode = dag->getInstructionNode(S);
    InstructionNode *loadNode = dag->getInstructionNode(L);
    storeNode->addMemDepInst(loadNode);
    loadNode->addMemUseInst(storeNode);
}

void ModuloScheduler::findMemoryAliasingWithStore(
    MEM_ACCESS storeAccess, std::vector<MEM_ACCESS> &aliasedAccesses) {

    for (std::vector<MEM_ACCESS>::iterator i = aliasedAccesses.begin(),
                                           e = aliasedAccesses.end();
         i != e; ++i) {
        MEM_ACCESS loadAccess = *i;

        if (!isa<LoadInst>(loadAccess.I))
            continue;

        int distance = getDistance(storeAccess, loadAccess);

        // if the distance is negative that means the stores in
        // this iteration will never be able to affect the loads
        // for instance:
        //           ... = d[i+8]
        //        d[i+1] = ...
        // Here the distance is 1-8 = -7 so there is no carried dependency
        // And example of a loop carried dependency is:
        //           ... = d[i+3]
        //        d[i+5] = ...
        // the distance here is 5-3 = 2 so there is a carried dependency
        if (distance > 0) {
            foundLoopCarriedDependency(distance, storeAccess, loadAccess);
        }
    }
}

// given a binary operation
// returns the operand that is more critical (latest arriving)
Instruction *ModuloScheduler::getLateDefiner(Instruction *I) {
    assert(isa<BinaryOperator>(I));

    // todo: account for critical path
    Instruction *first = dyn_cast<Instruction>(I->getOperand(0));
    Instruction *second = dyn_cast<Instruction>(I->getOperand(1));

    // make sure early is on the critical path
    // if (onCriticalPath(first)) {
    if (onCriticalPath(first) >= onCriticalPath(second)) {
        return first;
    }
    return second;
}

// given a binary operation
// returns the non-critical operand that has some slack (earliest arriving)
// TODO: LLVM 3.4 update: if the argument is a constant, it causes a problem:
// casting a constant to an instruction returns NULL, use Value instead and
// cast to Instruction when you call getEarlyDefiner() when necessary.
Value *ModuloScheduler::getEarlyDefiner(Instruction *I) {
    assert(isa<BinaryOperator>(I));

    Instruction *late = getLateDefiner(I);
    int num = 0;
    if (I->getOperand(num) == late) {
        num = 1;
    }

    return dyn_cast<Value>(I->getOperand(num));
}

void ModuloScheduler::getRestructuredOpsForAddSubLateOp0(
    Instruction *curOp, Instruction *late, Instruction *lateParent,
    Instruction::BinaryOps &newCurOpType,
    Instruction::BinaryOps &newEarlyOpType, Instruction::BinaryOps addType,
    Instruction::BinaryOps subType) {
    // legend:
    // a = lateParent
    // b = earlyParent
    // brackets surround curOp
    // c = early
    assert(lateParent == late->getOperand(0));
    if (isSub(curOp) && isSub(late)) {
        // (a - b) - c
        // a - (b + c)
        newCurOpType = subType;
        newEarlyOpType = addType;
    } else if (isAdd(curOp) && isSub(late)) {
        // (a - b) + c
        // a - (b - c)
        newCurOpType = subType;
        newEarlyOpType = subType;
    } else if (isSub(curOp) && isAdd(late)) {
        // (a + b) - c
        // a + (b - c)
        newCurOpType = addType;
        newEarlyOpType = subType;
    } else {
        // (a + b) + c
        // a + (b + c)
        newCurOpType = addType;
        newEarlyOpType = addType;
    }
}

void ModuloScheduler::getRestructuredOpsForAddSubLateOp1(
    Instruction *curOp, Instruction *late, Instruction *lateParent,
    Instruction::BinaryOps &newCurOpType,
    Instruction::BinaryOps &newEarlyOpType, Instruction::BinaryOps addType,
    Instruction::BinaryOps subType) {

    // legend:
    // a = lateParent
    // b = earlyParent
    // brackets surround curOp
    // c = early
    assert(lateParent == late->getOperand(1));
    if (isSub(curOp) && isSub(late)) {
        // (b - a) - c
        // (b - c) - a
        newCurOpType = subType;
        newEarlyOpType = subType;
    } else if (isAdd(curOp) && isSub(late)) {
        // (b - a) + c
        // (b + c) - a
        newCurOpType = subType;
        newEarlyOpType = addType;
    } else if (isSub(curOp) && isAdd(late)) {
        // (b + a) - c
        // (b - c) + a
        newCurOpType = addType;
        newEarlyOpType = subType;
    } else {
        // (b + a) + c
        // (b + c) + a
        newCurOpType = addType;
        newEarlyOpType = addType;
    }
}

bool ModuloScheduler::getRestructuredOpsForAddSub(
    Instruction *curOp, Instruction *late, Instruction *lateParent,
    Instruction::BinaryOps &newCurOpType,
    Instruction::BinaryOps &newEarlyOpType) {

    Instruction::BinaryOps addType, subType;

    if (isFAddSub(curOp)) {
        // float type
        if (!isFAddSub(late))
            return false;
        addType = Instruction::FAdd;
        subType = Instruction::FSub;
    } else {
        // integer type
        if (isFAddSub(late))
            return false;
        addType = Instruction::Add;
        subType = Instruction::Sub;
    }

    if (lateParent == late->getOperand(0)) {
        getRestructuredOpsForAddSubLateOp0(curOp, late, lateParent,
                                           newCurOpType, newEarlyOpType,
                                           addType, subType);
    } else {
        getRestructuredOpsForAddSubLateOp1(curOp, late, lateParent,
                                           newCurOpType, newEarlyOpType,
                                           addType, subType);
    }

    return true;
}

bool
ModuloScheduler::getRestructuredOps(Instruction *curOp, Instruction *late,
                                    Instruction *lateParent,
                                    Instruction::BinaryOps &newCurOpType,
                                    Instruction::BinaryOps &newEarlyOpType) {

    if (isMul(curOp)) {
        if (!isMul(late))
            return false;
        newCurOpType = Instruction::Mul;
        newEarlyOpType = Instruction::Mul;

    } else if (isAdd(curOp) || isSub(curOp)) {

        // todo: fix this, assumes that late operand is always
        // the minuend (difference = minuend - subtrahend)
        if (isSub(curOp)) {
            assert(curOp->getOperand(0) == late);
        }

        return getRestructuredOpsForAddSub(curOp, late, lateParent,
                                           newCurOpType, newEarlyOpType);
    }
    return true;
}

// restructure:
//    curOp = late <+/-/*> early
//
// if we assume additions only, given:
// late = lateParent + earlyParent
// curOp = late + early
// the exanded curOp expression is:
// curOp = (lateParent + earlyParent) + early
// we would like to use the associative property transform into:
// curOp = lateParent + (earlyParent + early)
//       = lateParent + newEarly
// this reduces the number of cycles on the critical recurrence
// path (from lateParent)
//
// to test this look at:
//    make pipelineDFG.ps
// versus:
//    make pipelineDFG.after.ps
// returns true if the IR was changed
bool ModuloScheduler::restructureBinaryOperation(Instruction *curOp,
                                                 Instruction *early,
                                                 Instruction *late) {
    assert(onCriticalPath(late));
    assert(isAdd(late) || isSub(late) || isMul(late));

    // returns true if the instruction is: add, mul, and, or, xor
    // doesn't work for sub
    // assert(curOp->isAssociative());

    Value *earlyParent = getEarlyDefiner(late);
    Instruction *lateParent = getLateDefiner(late);
    assert(early && late);

    // swap the operands
    // bool cannotReverse = curOp->swapOperands();
    // assert(!cannotReverse);

    Instruction::BinaryOps newCurOpType, newEarlyOpType;

    if (!getRestructuredOps(curOp, late, lateParent, newCurOpType,
                            newEarlyOpType)) {
        return false;
    }

    Instruction *newEarly =
        BinaryOperator::Create(newEarlyOpType, earlyParent, early,
                               "newEarly." + getLabelStripped(curOp), curOp);

    // Instruction *newCurOp = curOp->clone();
    // newCurOp->insertBefore(curOp);
    // newCurOp->setOperand(0, lateParent);
    // newCurOp->setOperand(1, newEarly);

    // newCurOp->setName(curOp->getName() + ".newCurOp");
    // newCurOp->setDebugLoc(curOp->getDebugLoc());
    Instruction *left = lateParent;
    Instruction *right = newEarly;
    if (lateParent == late->getOperand(1)) {
        left = newEarly;
        right = lateParent;
    }

    Instruction *newCurOp =
        BinaryOperator::Create(newCurOpType, left, right,
                               "newCurOp." + getLabelStripped(curOp), curOp);

    // simpleDCE() will later handle the case where lateParent no longer has
    // fanout
    curOp->replaceAllUsesWith(newCurOp);

    File() << "Associative Restructuring:\n"
           << "curOp: " << *curOp << "\n"
           << "curOp = (lateParent + earlyParent) + early\n"
           << "      = lateParent + (earlyParent + early)\n"
           << "lateParent: " << *lateParent << "\n"
           << "earlyParent: " << *earlyParent << "\n"
           << "early: " << *early << "\n"
           << "late: " << *late << "\n"
           << "newEarly: " << *newEarly << "\n"
           << "newCurOp: " << *newCurOp << "\n\n";

    return true;
}

// simple DCE on restructured graph
void ModuloScheduler::simpleDCE() {
    bool deletedInst;
    do {
        deletedInst = false;
        for (BasicBlock::iterator BBI = BB->begin(), E = BB->end(); BBI != E;) {
            Instruction *Inst = BBI++;

            if (!isInstructionTriviallyDead(Inst))
                continue;
            File() << "DCE removing: " << *Inst << "\n";
            Inst->eraseFromParent();
            deletedInst = true;
        }
    } while (deletedInst);
}

// restructure the graph to minimize recurrences
// this can be slow because after every restructure we need to calculate
// all the loop recurrences again by calling findLoopRecurrences()
void ModuloScheduler::restructureDFG() {
    File() << "Restructuring expression tree to minimize recurrences\n";

    for (BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i) {
        BinaryOperator *curOp = dyn_cast<BinaryOperator>(i);

        if (!curOp)
            continue;

        // just look at adds for now
        if (!isAdd(curOp) && !isSub(curOp) && !isMul(curOp))
            continue;

        File() << "Checking: " << *curOp << "\n";

        if (!onCriticalPath(curOp)) {
            File() << "Skipping. Not critical\n";
            continue;
        }

        Instruction *early = dyn_cast<Instruction>(getEarlyDefiner(curOp));
        Instruction *late = getLateDefiner(curOp);
        if (!early || !late) {
            File() << "Skipping. Can't find early/late definer\n";
            continue;
        }

        // just deal with all addition for now
        if (!isAdd(late) && !isSub(late) && !isMul(late)) {
            File() << "Skipping. Late definer isn't an add/sub\n";
            continue;
        }

        if (restructureBinaryOperation(curOp, early, late)) {

            // TODO: this is quite inefficient to recalculate the elementary
            // cycles and DAG every time we restructure
            dag->runOnFunction(*F, alloc);
            findLoopRecurrences();
        }
    }

    simpleDCE();
}
