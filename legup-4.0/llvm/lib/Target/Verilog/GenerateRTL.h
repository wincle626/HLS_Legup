//===-- GenerateRTL.h -------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// GenerateRTL uses the Scheduling and Binding to construct a circuit
// out of the RTL data structure.
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_GENERATERTL_H
#define LEGUP_GENERATERTL_H

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "Graph.h"
#include "RTL.h"
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <list>
#include <algorithm>

//NC changes...
#include "StateStoreInfo.h"
#include <vector>

using namespace llvm;

namespace legup {

class Binding;
class Scheduler;
class SchedulerDAG;
class FiniteStateMachine;
class RTLModule;
class RTLSignal;
class RTLConst;
class RTLOp;
class State;
class LegupPass;
class Allocation;
class RAM;
class MinimizeBitwidth;
class PropagatingSignal;
class DebugVariableLocal;
class RTLDebugPort;
class RTLModuleInstance;

// NC changes...
class StateStoreInfo;
class DebugType;



/// GenerateRTL generates an RTLModule for a Verilog module, which has a
/// one-to-one mapping with an LLVM function
/// @brief Legup Hardware Module Representation
class GenerateRTL : public InstVisitor<GenerateRTL> {
public:
    GenerateRTL(Allocation *alloc, Function* F) :
        sched(0), binding(0), fsm(0), bindingFSM(0),  alloc(alloc), Fp(F),
        rtl(0),  mc_force_wire_operand(false) {
    }

    ~GenerateRTL();

    /// generateRTL - generate the RTLModule required to implement this
    /// hardware module
    RTLModule* generateRTL(MinimizeBitwidth *MBW);

    Function *getFunction() { return Fp; }
    
    //NC changes
    RTLModule* getRTL() {return rtl;}

    // Instruction visitation functions
    // These are called by visit() in generateDatapath()
    // Make sure to set 'state' variable before calling visit()
    void visitReturnInst(ReturnInst &I);
    void visitBranchInst(BranchInst &I);
    void visitSwitchInst(SwitchInst &I);
    void visitInvokeInst(InvokeInst &I) {
      llvm_unreachable("Lowerinvoke pass didn't work!");
    }
    
    // TODO: had to change this for LLVM 3.4 update. TerminatorInst??
    //void visitUnwindInst(UnwindInst &I) {
    void visitUnwindInst(TerminatorInst &I) {
      llvm_unreachable("Lowerinvoke pass didn't work!");
    }
    void visitUnreachableInst(UnreachableInst &I);

    void visitPHINode(PHINode &I);

    RTLSignal *getLeftHandSide(Instruction *instr);

    void visitBinaryOperator(Instruction &I);
    void visitUnaryOperator(CastInst &I);
    void visitICmpInst(ICmpInst &I) {
        visitBinaryOperator(I);
    }
    void visitFCmpInst(FCmpInst &I) {
        visitBinaryOperator(I);
    }
    void visitFCastInst (CastInst &I);
    void visitFPTruncInst (FPTruncInst &I){
        visitUnaryOperator(I);
        //visitFCastInst(I);
    }
    void visitFPExtInst (FPExtInst &I){
        visitUnaryOperator(I);
        //visitFCastInst(I);
    }
    void visitFPToSIInst (FPToSIInst &I){
        visitUnaryOperator(I);
        //visitFPCastInst(I);
    }
    void visitSIToFPInst (SIToFPInst &I){
        visitUnaryOperator(I);
        //visitFPCastInst(I);
    }
    void visitCastInst (CastInst &I);
    void visitFPCastInst (CastInst &I);
    void visitSelectInst(SelectInst &I);

    void visitCallInst (CallInst &I);

    void visitAllocaInst(AllocaInst &I) {}
    int connectedToPortB(Instruction *instr);
    void visitLoadInst  (LoadInst   &I);
    void visitStoreInst (StoreInst  &I);
    void loadStoreCommon (Instruction *instr, Value *addr);

    void visitGetElementPtrInst(GetElementPtrInst &I);

    // TODO LLVM 3.4 update: these need to be implemented
    //void visitInsertElementInst(InsertElementInst &I);
    //void visitExtractElementInst(ExtractElementInst &I);

    void visitInstruction(Instruction &I) {
      errs() << "Verilog Writer does not know about " << I;
      llvm_unreachable(0);
    }

    void scheduleOperations();

    /// updateOperationUsage - updates the global OperationUsage map
    void updateOperationUsage (std::map <std::string, int> &_OperationUsage);

    FiniteStateMachine *getBindingFSM() { return bindingFSM; }
    FiniteStateMachine *getFSM() { return fsm; }
    
    //NC changes...
    std::string getVerilogName(const Value *val){return verilogName(*val);}   
    std::vector<StateStoreInfo*> statesStoreMapping;    

// Combinational path of instructions between two registers
    typedef struct path {
        //cumulative delay of all instructions in the path
        float pathDelay;
        State *state;
        //all instructions in the path in reversed order
        std::vector<Instruction *> instructions;
        //delay of each instruction
        std::vector<float> instrDelay;
        Function *func;        
    } PATH;

    struct comp{
        bool operator() (const PATH* a, const PATH* b) const {
        return (a->pathDelay > b->pathDelay);
        }
    } pathComp;

    static void printPath(raw_ostream &report, list<PATH *> *path);

    /// verilogName - return the verilog variable name of a LLVM value
    std::string verilogName(const Value *val);
    std::string verilogName(const Value &val);

    Allocation *getAllocation() { return alloc; }

    /// ------------------------------------------------------------------------
    // Debugger
    /// ------------------------------------------------------------------------
    std::vector<DebugVariableLocal *> *getDbgVars() { return &dbgVars; }
    std::vector<RTLDebugPort *> *getDbgTracePorts() { return &dbgTracePorts; }
    std::vector<RTLModuleInstance *> *getInstances() { return &instances; }
    std::vector<RTLDebugPort *> *getDbgStatePorts() { return &dbgStatePorts; }

    void addDebugTraceOutputs();
    DebugVariableLocal *dbgGetVariable(MDNode *metaNode);
    void generateInstances(int parentInst);
    void addDebugRtl();

  private:
    // get the RTLSignal that holds the expected value of cur_state
    // for a particular state. Usually this is a Verilog parameter:
    //      parameter [2:0] LEGUP_F_main_BB_0_1 = 3'd1;
    RTLSignal *getStateSignal(State *state) {
        assert(stateSignals.find(state) != stateSignals.end());
        return stateSignals[state];
    }

    /// generatePHICopiesForSuccessor - output any assignments necessary to
    /// execute in CurBlock state due to a phi instruction in Successor state
    void generatePHICopiesForSuccessor (RTLSignal* condition, State *CurBlock,
            State *Successor);

    /// allocateRAM - create a RAM object for value I
    RAM* allocateRAM(const Value *I);

    /// setOperationUsageFunction - updates the local OperationUsage map
    void setOperationUsageFunction(Instruction *instr);

    RTLSignal *getConstantSignal(const ConstantInt *c);
    RTLSignal *getFloatingPointConstantSignal(const ConstantFP *c);
    void generateAllCallInsts();
    void updateStatesAfterCallInsts();
    RTLSignal* getOpConstant(State *state, Constant *c);
    RTLSignal *getOpNonConstant(State *state, Value *op0);
    // NC changes...
    RTLSignal *getOpConstant(State *state, Constant *op, int &value);

    // connect 'signal' to 'driver' during state 'state'.
    // The 'instr' argument is optional and gives an LLVM instruction to
    // display in the RTL comments
    // There are two cases:
    // 1) Default: A connection in the standard datapath
    //      For example:
    //           connectSignalToDriverInState(memory_enable, ONE, state)
    //      In verilog this might look like:
    //           if ((cur_state == LEGUP_F_legup_lock_BB_1_1)) begin
    //               memory_controller_enable_a = 1'd1;
    // 2) A connection inside the loop pipeline datapath:
    //      This happens only if the 'state' is a pipeline wait state (i.e.
    //      'loop_pipeline_wait_loop')
    //      In Verilog this might look like:
    //           if (loop_1_ii_state == 1'd0 & loop_1_valid_bit_1) begin
    //               main_legup_memset_4 <= main_legup_memset_5;
    //      For multi-cycle operations in a loop pipeline, we may need to
    //      specify the optional 'pipelineState' argument. pipelineState
    //      disambiguates _when_ during the pipeline we want to drive the
    //      signal because the 'state' from the central FSM doesn't change
    //      during the entire loop pipeline execution. If we schedule a
    //      multicycle operation, for instance a load, we want to know whether
    //      we want to connect 'signal' when the load is scheduled to start
    //      (INPUT_READY) or when the load is scheduled to finish
    //      (OUTPUT_READY):
    enum PIPELINE_STATE { INPUT_READY, OUTPUT_READY };
    void
    connectSignalToDriverInState(RTLSignal *signal, RTLSignal *driver,
                                 State *state, Instruction *instr = 0,
                                 PIPELINE_STATE pipelineState = INPUT_READY,
                                 bool setToDriverBits = false);

    void connectSignalToDriverInStateWithCondition(RTLSignal *signal,
                                                   RTLSignal *driver,
                                                   State *state,
                                                   RTLOp *newCond = 0,
                                                   Instruction *instr = 0);

    // get the condition for an instruction in a loop pipeline, for instance:
    //      %12 = volatile load i32* %scevgep32, align 4, !tbaa !3,
    //  with the following properties from modulo scheduling:
    //      start_time: 1 avail_time: 2 stage: 0 II: 15 start_ii_state = 1 % 15
    //      = 1
    //      avail_ii_state = 2 % 15 = 2
    //
    //  if pipelineState = INPUT_READY (default) then the condition returned
    //  would be:
    //     ((loop_1_ii_state == 4'd1) & loop_1_valid_bit_1))
    //  if pipelineState = OUTPUT_READY then the condition returned would be:
    //     ((loop_1_ii_state == 4'd2) & loop_1_valid_bit_2))
    RTLOp *
    getPipelineStateCondition(RTLSignal *signal, Instruction *instr = 0,
                              PIPELINE_STATE pipelineState = INPUT_READY);

    RAM *getRam(Value *op);
    RAM *getLocalRam(Value *op);

    // for basic blocks that should be reading from a ram
    std::set<Function*> calledModules;
    std::map<Function*, RTLModule*> calledModulesToRtlMap;
    unsigned numStates;
    void generateModuleDeclarationSignals(State *wait, std::string postfix);
    void generateModuleDeclaration();
    void generateVariableDeclarationsSignalsMemory(
        RTLModule *t, Function *F, const std::string fctName,
        const std::string postfix, const std::string instanceName = "");
    void generatePropagatingSignalDeclarations(RTLModule *t, Function *F);
    void generateVariableDeclarations(CallInst *CI,
                                      const std::string functionType);
    void generateModuleInstantiation(Function *F, CallInst *CI,
                                     const int numThreads,
                                     const std::string fctName,
                                     const std::string functionType,
                                     int loopIndex = 0);
    void addTagOffsetParamToModule(Function *F, const int numThreads,
                                   const int instanceNum, RTLModule *t);
    void generateRoundRobinArbiterDeclaration(const std::string fctName,
                                              const int parallelInstances);
    void generatePthreadArbiter(const std::set<CallInst *> pthreadFunctions);
    void connectMemoryControllerSignalsToArbiter(
        RTLModule *t, const std::set<CallInst *> pthreadFunctions,
        const std::string parentFctName);
    void connectMemoryControllerSignalsToModuleOutputWithArbiterGrant(
        const std::set<CallInst *> pthreadFunctions,
        const std::string parentFctName, const std::string postfix);
    void generateArbiterDeclaration(const std::string fctName,
                                    const std::string postfix,
                                    const int parallelInstances);
    void generatePthreadMasterThreadLogic();
    void generateDatapath();
    void modifyFSMForAllLoopPipelines();
    void generateAllLoopPipelines();
    void generateLoopPipeline(BasicBlock *BB);
    RTLSignal* getLoopExitCond(BasicBlock *BB, RTLSignal *indvar);
    void modifyFSMForLoopPipeline (BasicBlock *BB);


    void generateTransition(RTLSignal *condition, State* s);
    RTLSignal *getTransitionOp(State *s);

    RTLSignal *getOp(State *state, Value *op);
    RTLSignal *getGEP(State *state, User *GEP);
    RTLSignal *getByteOffset(State *state, Value *Op, gep_type_iterator GTI);
    RTLSignal *getGEPOffset(State *state, User *GEP);
    void functionHandshaking(FiniteStateMachine *fsm);
    
    //NC changes...
    RTLSignal *getOp(State *state, Value *op, int &value);
    RTLSignal *getGEP(State *state, User *GEP, int &value);
    RTLSignal *getByteOffset(State *state, Value *op, gep_type_iterator GTI, int &offsetValue);
    RTLSignal *getGEPOffset(State *state, User *GEP, int &value);

    bool usedAcrossStates(Value *instr, State *state);
    bool usedSameState(Value *instr, State *state);
    bool fromOtherState(Value *v, State *state);
    bool fromSameState(Value *v, State *state);
    RTLSignal *getOpReg(Value *v, State *state);
    void operatorAssignment();
    RTLOp *createOptoCheckState(State *state);
    RTLSignal *createFU(Instruction *Instruction, RTLSignal *op0,
                        RTLSignal *op1);
    RTLSignal *createBindingFU(Instruction *instr, RTLSignal *op0,
                               RTLSignal *op1);
    void create_fu_enable_signals(Instruction *instr);
    void create_pattern_fu(std::string name1, Node *node1, Node *node2);
    void createBindingSignals();
    void createMultipumpSignals();
    bool isMultipumped(Instruction *I);
    int pairFUsForMultipumpFU(std::string FuName1, std::string
            FuName2, int multipump_fu_num, raw_ostream &out);
    void shareRegistersFromBinding();
    void updateRTLWithPatterns();
    std::vector<PropagatingSignal> addPropagatingMemorySignalsToModuleWithPostfix(RTLModule *_rtl, std::string postfix);
    std::vector<PropagatingSignal> addPropagatingMemorySignalsToFunctionWithPostfix(Function *F, std::string postfix);
    void addPropagatingMemorySignalsToFunction(Function *F);
    void addPropagatingSignalsToCustomVerilog(Function *F);
    void addPropagatingPortsToModule(RTLModule *);
    void addDefaultPortsToModule(RTLModule *);
    void addTagOffsetParameterToModule();
    void create_functional_units_for_pairs();
    void visitPrintf(CallInst *CI, Function *called);
    void
    createPthreadPollingFunction(std::vector<CallInst *> &pthreadFunctions);
    void createPthreadSignals(CallInst *CI);

    void connectStartSignal(CallInst *CI, State *endState,
                            const std::string moduleName,
                            const std::string instanceName, RTLOp *oneCond = 0,
                            RTLOp *zeroCond = 0);
    void connectPthreadPollFinishAndReturnVal(CallInst *CI,
                                              RTLSignal *pollThreadID,
                                              RTLSignal *pollFunctionID);

    int getNumThreads(const CallInst *CI);
    int getInstanceNum(const CallInst *CI, const int loopIndex);
    std::string getInstanceName(const CallInst *CI, const int loopIndex = 0);
    RTLOp *getOpToCheckThreadID(RTLSignal *threadIDSignal,
                                const int threadIDValue);
    bool storePthreadFunctions(std::vector<CallInst *> &pthreadFunctions,
                               CallInst *CI);
    void connectPthreadIDSignals(CallInst *CI, RTLSignal *pollThreadID,
                                 RTLSignal *pollFunctionID);
    RTLOp *selectTopOrBottomHalfBits(Value *V, State *state, bool top);
    void createFunctionMemorySignals(State *state1, CallInst *CI,
                                     std::string name, std::string postfix,
                                     const int parallelInstances,
                                     const std::string functionType);
    void connectFunctionMemorySignals(State *state1, CallInst *CI,
                                      std::string name, std::string postfix,
                                      const int numThreads,
                                      const std::string functionType,
                                      const int loopIndex = 0);
    void createFunctionPropagatingSignals(State *state1, CallInst *CI,
                                          std::string name,
                                          const int parallelInstances,
                                          const std::string functionType);
    void createMemoryStallLogic(std::string name, int parallelInstances,
                                const int threadID,
                                const std::string functionType);
    void createWaitrequestLogic(CallInst *CI, State *callState,
                                std::string name, const int parallelInstances,
                                const std::string functionType);
    void connectWaitrequestForParallelFunctions(CallInst *CI, State *callState,
                                                const std::string name,
                                                const std::string funcName,
                                                const std::string functionType,
                                                const int loopIndex = 0);
    void createMemoryFlags(std::string name, std::string postfix,
                           int parallelInstances, const int threadID,
                           const std::string functionType);
    void createMemoryReaddataLogicForParallelInstance(
        RTLSignal *gnt, const std::string name, const std::string postfix,
        const std::string instanceNum = "");
    void createMemoryReaddataStorageForParallelFunction();
    void
    createMemoryReaddataStorageForPort(RTLSignal *memReaddataValid,
                                       RTLSignal *secondStateAfterMemoryRead,
                                       std::string postfix);
    void createStateTransitions(CallInst *CI, State *&state1, State *&state2,
                                bool &isStateTerminating);
    void createStartSignal(CallInst *CI, State *state1,
                           const std::string moduleName, const int numThreads,
                           const std::string functionType);
    void createArgumentSignals(CallInst *CI, Function *called,
                               const std::string moduleName,
                               const int parallelInstances,
                               const std::string functionType);
    void createReturnSignal(State *state1, CallInst *CI, Function *called,
                            const std::string moduleName,
                            const int parallelInstances,
                            const std::string functionType);
    void connectReturnSignal(State *callState, CallInst *CI,
                             Function *calledFunction,
                             const std::string moduleName, const int numThreads,
                             const int loopIndex = 0);
    void createFinishSignal(CallInst *CI, State *state1, State *state2,
                            const std::string moduleName, int parallelInstances,
                            const std::string functionType);
    void connectFinishSignal(CallInst *CI, RTLSignal *finish_final,
                             const std::string moduleName,
                             const std::string functionType,
                             const int numThreads,
                             std::vector<RTLSignal *> &finishVector,
                             const int loopIndex = 0);
    void createFunction(CallInst &I);
    RTLSignal *getPthreadThreadID(CallInst *CI, const std::string moduleName);
    unsigned getOpSizeShared(Instruction *instr, RTLSignal *op = NULL,
                             unsigned opIndex = 0);
    RTLWidth getOutSizeShared(Instruction *instr);
    RTLSignal* createMulFU(Instruction *instr, RTLSignal *op0, RTLSignal *op1);
    RTLSignal* createDivFU(Instruction *instr, RTLSignal *op0, RTLSignal *op1);
    RTLSignal *createSerialDivFU(Instruction *instr, RTLSignal *op0, RTLSignal *op1);
    RTLSignal *createSerialDivLegUpFU(Instruction *instr, RTLSignal *op0, RTLSignal *op1);
    RTLSignal* createFPFU(Instruction *instr, RTLSignal *op0, RTLSignal *op1, unsigned opCode);
    RTLSignal* createFPFUUnary(Instruction *instr, RTLSignal *op0, unsigned opCode);
    RTLSignal* createFCmpFU(Instruction *instr, RTLSignal *op0, RTLSignal *op1);
    RTLSignal *createFP_FU_Helper(std::string fu_name, Instruction
            *instr, RTLSignal *op0, RTLSignal *op1, RTLModule *d);
    void createMultiPumpMultiplierFU(Instruction *AxB, Instruction *CxD);
    void createRTLSignals();
    void createRTLSignalsForInstructions();
    void createRTLSignalsForLocalRams();
    std::vector<RTLSignal> getPropagatingSignals();
    void connectRegistersToWires();
    list<GenerateRTL::PATH*> func_path;
    int getNumSuccInState (Instruction *I, FiniteStateMachine *fsm);
    bool isStartOfPath (SchedulerDAG *dag, Instruction *I, FiniteStateMachine *fsm);
    void timingAnalysis(SchedulerDAG* dag);    
    void addToOverallPathList ();
    void calculateDelay(SchedulerDAG *dag, Instruction *Curr, float partialDelay, State *startS, FiniteStateMachine *fsm, std::list<PATH*> *func_path, std::vector<Instruction *> instr, std::vector<float> instrDelay);
    void printSchedulingInfo();

    // Functions and typedefs for multi-cycle path constraints
    typedef std::pair<std::string, std::string>         src_dst_pair_t;
    typedef std::vector< Instruction* >                 path_t;
    typedef std::pair<path_t, unsigned>                 path_latency_pair_t;
    typedef std::multimap< src_dst_pair_t, 
                           path_latency_pair_t >        src_dst_pair_to_path_map_t;
    typedef std::map< src_dst_pair_t, unsigned >        src_dst_pair_to_min_slack_t;
    typedef std::set< src_dst_pair_t >                  duplicate_src_dst_pairs_t;
    typedef std::set< std::string >                     signals_to_keep_t;    
    void printSDCMultiCycleConstraints();
    std::string get_source_register_name(Instruction *source);
    std::string get_dest_register_name(Instruction *dest);
    int min_state_difference(BasicBlock *src, BasicBlock *dst);
    int max_state_difference(BasicBlock *src, BasicBlock *dst);
    void print_lpm_div_multicycle_constraints();
    bool can_path_still_be_combinational(Instruction * pred);
    void add_multicycle_constraint_for_path(std::vector<Instruction*> path, std::vector<string> & sdc_constraints,
        std::vector<string> & qsf_constraints, ::set< std::pair<Instruction*,Instruction*> > keep_reg_pairs,
        std::multimap< std::vector <Instruction*>, Value* > & paths_with_arg_srcs);
    std::string make_multicycle_constraint(std::string src, std::string dst, int multicycle_multiplier,
        std::string type, std::vector<Instruction*> intermediate_path);
    void adjust_multicycle_multiplier_for_specific_signals(std::string dst_name, 
        int & multicycle_multiplier, std::vector<Instruction*> path);
    void add_multicycle_constraint_for_path_with_custom_root(std::vector<Instruction*> path,
        std::vector<string> & sdc_constraints, std::vector<string> & qsf_constraints, 
        std::set< std::pair<Instruction*,Instruction*> > keep_reg_pairs, 
        std::set<Instruction*> drivers_with_reg_to_remove, 
        std::multimap< std::vector <Instruction*>, Value* > & paths_with_arg_srcs, RTLSignal *root_signal);
    void write_multicycle_constraint_to_vectors(std::string src_name, std::string dst_name, unsigned multicycle_multiplier, 
        std::vector<std::string> & sdc_constraints, std::vector<std::string> & qsf_constraints, std::vector<Instruction*> intermediate_path);
    void ipath_to_bbpath(std::vector<Instruction*> & path, std::vector<BasicBlock*> & bb_path);
    int get_dest_state(Instruction *dest);
    int get_source_state(Instruction *source);
    int num_cycles_in_path(std::vector<Instruction*> path);
    int num_cycles_in_path_with_src_arg(std::vector<Instruction*> path);
    std::vector<Value*> get_all_operands(Instruction *current);
    std::string get_store_port_name(Instruction *store, Instruction *input);
    std::string get_load_port_name(Instruction *load);
    bool remove_intermediate_register_helper(RTLOp *rtl_op_driver, RTLSignal *src_sig_w, RTLSignal *src_sig_r);
    void remove_intermediate_register(Instruction * pred, Instruction * current);
    void remove_intermediate_register_signal_dst(Instruction * src, RTLSignal * dst_sig_w);
    void root_dfs_visit_preds_of_current(Instruction *current, std::stack<Instruction*> & frontier,
        std::vector<Value*> & preds, std::vector< std::vector<Instruction*> > & paths,
        std::vector <Instruction*> & cur_path,
        std::multimap< std::vector <Instruction*>, Value* > & paths_with_arg_srcs,
        std::set<Instruction*> & visited,
        std::multimap<Instruction*,Instruction*> & remove_reg_pairs,
        std::set< std::pair<Instruction*,Instruction*> > & keep_reg_pairs,
    std::map<Instruction*, int> & num_unfinished_preds);
    void create_multicycle_paths_from_root(Instruction* root);
    void signal_dfs_visit_pred(Instruction * current, 
        Instruction *pred, std::vector <Instruction*> & cur_path,
        std::vector< std::vector<Instruction*> > & paths,
        bool revisiting_pred, std::stack<Instruction*> & frontier,
        std::set<Instruction*> & visited, 
        std::map<Instruction*, int> & num_unfinished_preds,
        std::set< std::pair<Instruction*,Instruction*> > & keep_reg_pairs,
        std::multimap<Instruction*,Instruction*> & remove_reg_pairs,
        std::set<Instruction*> & drivers_with_reg_to_remove);
    void signal_dfs_visit_preds_of_current(Instruction *current, std::stack<Instruction*> & frontier,
        std::vector<Value*> & preds, std::vector< std::vector<Instruction*> > & paths,
        std::vector <Instruction*> & cur_path,
        std::multimap< std::vector <Instruction*>, Value* > & paths_with_arg_srcs,
        std::set<Instruction*> & visited, std::multimap<Instruction*,Instruction*> & remove_reg_pairs,
        std::set< std::pair<Instruction*,Instruction*> > & keep_reg_pairs,
        std::set<Instruction*> & drivers_with_reg_to_remove,
        std::map<Instruction*, int> & num_unfinished_preds);
    void create_multicycle_paths_to_signal(RTLSignal *root);
    void remove_intermediate_registers(const std::multimap<Instruction*,Instruction*> & remove_reg_pairs);
    void remove_driver_registers_for_root_signal(RTLSignal *root, std::set<Instruction *> srcs);
    void find_signals_to_synth_keep();
    Instruction* find_instruction_unique_to_path(unsigned i, const std::vector< path_t > & paths);
    void prune_duplicate_src_dst_pairs_based_on_path_latency();
    void abort_if_poor_multicycling();
    void multi_cycle_set_force_wire_operand(bool f) { mc_force_wire_operand = f; }
    bool multi_cycle_force_wire_operand(Instruction *I);
    // End of functions for multi-cycle path constraints

    void printSchedulingDFGDot(SchedulerDAG &dag);
    void printScheduleGanttChart();
    typedef struct _GanttBar {
        Instruction *inst;
        int x, y;
        int duration;
    } GanttBar;
    void printGantt(raw_fd_ostream &file, std::vector<GanttBar> &gantt,
            std::map<Instruction *, GanttBar> &instructions, int stateNum);
    void shareRegistersForFU(
        std::set<Instruction *> &Instructions,
        std::map<Instruction*, std::set<Instruction*> >
        &IndependentInstructions);
    void setup_multipumping();
    bool isaPthreadCallWrapperWithReturnVal(Function *F);
    bool isaPthreadCallWrapper(Function *F, Function **pthreadF);
    bool hasReturnVal(Function *F);
    RTLSignal* getWaitRequest(Function *F);

    raw_fd_ostream &pipeRTLFile();
    raw_fd_ostream &File();

    Scheduler *sched;
    Binding *binding;
    FiniteStateMachine *fsm, *bindingFSM;
    Allocation *alloc;
    Function *Fp;
    RTLModule *rtl;
    typedef std::map<const Value*, RAM*>::iterator ram_iterator;
    std::map <std::string, int> OperationUsageFunction;
    State *state;
    int stage;
    int time;
    // don't use stateSignals directly, use getStateSignal()
    std::map <State*, RTLSignal*> stateSignals;
    bool found;
    void connectPatternFU(Graph::GraphNodes_iterator &GNi, int PairNumber);
    std::string getPatternFUName(Graph::GraphNodes_iterator &GNi, int PairNumber);
    unsigned getInstrMemSize (Instruction *instr);
    State *getFirstState(BasicBlock *BB);
    void printFSMDot();

    // this table gives the RTL signal holding the given LLVM value on each time
    // step of the pipeline. NULL if the value isn't available/calculated yet
    // access using:
    //      getPipelineSignal(I, timeAvail)
    //      setPipelineSignal(I, timeAvail, signal)
    std::map<const Value*, std::vector<RTLSignal*> > pipelineSignalAvailableTable;

    // this function populates pipelineSignalAvailableTable
    void findAllPipelineStageRegisters(BasicBlock *BB);

    RTLSignal *getPipelineSignal(const Value* I, int timeAvail) {
        assert(pipelineSignalAvailableTable.find(I) !=
                pipelineSignalAvailableTable.end());
        int size = pipelineSignalAvailableTable[I].size();
        assert(size);
        assert(timeAvail >= 0 && timeAvail < size);
        return pipelineSignalAvailableTable[I][timeAvail];
    }

    void setPipelineSignal(const Value* I, int timeAvail,
            RTLSignal *signal) {
        assert(pipelineSignalAvailableTable.find(I) !=
                pipelineSignalAvailableTable.end());
        int size = pipelineSignalAvailableTable[I].size();
        assert(size);
        assert(timeAvail >= 0 && timeAvail < size);
        pipelineSignalAvailableTable[I][timeAvail] = signal;
    }

    void initializePipelineSignal(const Value* I, int totalTime) {
        assert (pipelineSignalAvailableTable[I].empty());
        for (int i = 0; i < totalTime; i++) {
            pipelineSignalAvailableTable[I].push_back(NULL);
        }
    }

    void createBindingFSM();
    void modifyPipelineState(State *state);
    std::string getEnableName(Instruction *instr);

    // all pairs of Graph objects to share
    std::map<Graph*, Graph*> GraphPairs;

    // a set of all instructions in Graphs, used in visitInstruction 
    std::set<Instruction*> InstructionsInGraphs;

    std::map<std::string, std::set<Instruction *> > instructionsAssignedToFU;
    // From Binding, all pairs of instructions which are shared
    std::map<Value*, Value*> AllBindingPairs;

    RTLConst *ZERO, *ONE;
    MinimizeBitwidth *MBW;
    bool EXPLICIT_LPM_MULTS;
    bool MULTIPUMPING;
    bool USE_MB;

    // true if this function forks pthreads    
    bool usesPthreads;
    std::vector<Function*> parallelFunctions;

    struct MultipumpOperation {
        // multipump fu name i.e. "multipump_0"
        std::string name;
        // multipump fu output i.e. "multipump_0_outAxB" or "multipump_0_outCxD"
        std::string out;
        // multipump fu input for operand 0 (A or C)
        // i.e. "multipump_0_inA" or "multipump_0_inC"
        std::string op0;
        // multipump fu input for operand 1 (B or D)
        //    i.e. "multipump_0_inB" or "multipump_0_inD"
        std::string op1;
    };

    // A map from an instruction to multipump FU
    std::map<Instruction*, MultipumpOperation> multipumpOperations;

    std::set<BasicBlock *> pipelinedBBs;
    SchedulerDAG *dag;
    
    // Some multi-cycle path data structures
    bool mc_force_wire_operand;
    // Keep a set of the src,dst pairs which we actually do want to synthesis keep
    duplicate_src_dst_pairs_t duplicate_src_dst_pairs;    
    // Keep a map of every (src,dst) string to its complete path, and the path latency
    src_dst_pair_to_path_map_t src_dst_pair_to_path_map;
    // Keep a map of every (src,dst) string to its minimum latency across all paths
    src_dst_pair_to_min_slack_t src_dst_pair_to_min_slack;
    src_dst_pair_to_min_slack_t src_dst_pair_to_max_slack; // Store the max too
    // The final result is a set of signals / instructions to not synth away
    signals_to_keep_t signals_to_keep;
    // Map .sdc file register name to .qsf file register name
    std::map<std::string, std::string> sdc_name_to_qsf_name;

    /// ------------------------------------------------------------------------
    // Debugger
    /// ------------------------------------------------------------------------
    std::vector<DebugVariableLocal *> dbgVars;
    std::vector<RTLDebugPort *> dbgTracePorts;
    std::vector<RTLDebugPort *> dbgStatePorts;

    // This GenerateRTL module may have multiple instances.
    // For example, if the associated function is called from multiple
    // different functions.  This is only relevant for debugging
    // since we need to track which instance we are debugging.
    std::vector<RTLModuleInstance *> instances;
};

} // End legup namespace

#endif
